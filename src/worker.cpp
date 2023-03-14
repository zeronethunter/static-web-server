#include "../include/worker.h"

#include <fcntl.h>
#include <sys/stat.h>

#include <cstring>
#include <fstream>
#include <thread>
#include <utility>

#include "sys/sendfile.h"

void Worker::on_connection(evutil_socket_t fd, short event, void *arg) {
  auto *worker = (Worker *)arg;
  auto *_logger = &Logger::getInstance();

  /*
   * Read client fd to handle
   * */
  int client_fd;
  ssize_t readed = read(fd, &client_fd, sizeof(client_fd));
  if (readed == -1) {
    _logger->log("[ERROR] ", std::this_thread::get_id(),
                 " Worker::on_connection: read failed: ", hstrerror(errno));
    close(client_fd);
    return;
  } else if (readed == 0) {
    _logger->log("[DEBUG] ", std::this_thread::get_id(),
                 " Worker::on_connection: read empty: ", hstrerror(errno));
    close(client_fd);
    return;
  }

  /*
   * Creating of new client event, which will be added to event_base.
   * */
  struct event *client_event = event_new(worker->get_base(), client_fd, EV_READ,
                                         on_request, (void *)worker);

  if (!client_event) {
    _logger->log("[ERROR] ", std::this_thread::get_id(),
                 " Worker::on_connection: new_event failed");
    close(client_fd);
    return;
  }
  if (event_add(client_event, nullptr) == -1) {
    _logger->log("[ERROR] ", std::this_thread::get_id(),
                 " Worker::on_connection: event_add failed");
    event_free(client_event);
    close(client_fd);
    return;
  }
}

void Worker::on_request(evutil_socket_t fd, short ev, void *arg) {
  auto *worker = (Worker *)arg;
  char buffer[1024];
  ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
  auto *_logger = &Logger::getInstance();

  if (n == -1) {
    _logger->log("[ERROR] ", std::this_thread::get_id(),
                 " Worker::or_read: recv failed");
    return;
  } else if (n == 0) {
    /* When there is no data to read, remove the event from the
     * event_loop and close socket.
     * */
    kill_client(fd, worker->get_base());
  } else {
    handle_request(worker->get_document_root(), fd, worker->get_base(), buffer,
                   n);
  }
}

void Worker::handle_request(std::filesystem::path document_root,
                            evutil_socket_t fd, event_base *base, char *request,
                            size_t length) {
  auto *_logger = &Logger::getInstance();
  std::filesystem::path root(std::move(document_root));
  bool is_index = false;
  enum request_type { GET = 0, HEAD = 1 };

  /*
   * Logging request only if debug mode is enabled
   * */
  if (_logger->getDebugMode()) {
    _logger->log("\n\033[32m", std::this_thread::get_id(),
                 " Worker::handle_request:\n", "\033[0m",
                 std::string(request, length));
  }

  HTTPResponse response;

  std::unordered_map<std::string, std::string> headers{
      {"Connection", "close"},
      {"Date", HTTPResponse::get_current_time()},
      {"Server", "zenehu"},
  };

  /*
   * Check only GET and HEAD requests
   * */
  if (strncmp(request, "GET", 3) != 0 && strncmp(request, "HEAD", 4) != 0) {
    response.set_status(HTTPResponse::METHOD_NOT_ALLOWED);
    headers.insert({"Allow", "GET, HEAD"});
    response.set_headers(headers);
    std::string resp = response.build(GET);

    send(fd, resp.c_str(), resp.length(), 0);

    kill_client(fd, base);

    return;
  }

  request_type type;

  if (strncmp(request, "GET", 3) == 0) {
    type = GET;
  } else {
    type = HEAD;
  }

  /*
   * GET request handling
   * */

  std::string path = std::string(request);

  /*
   * Decoding and parsing of the requested path
   * */
  if (type == GET) {
    path = HTTPResponse::decodeURL(path.substr(5, path.find("HTTP") - 6));
  } else {
    path = HTTPResponse::decodeURL(path.substr(6, path.find("HTTP") - 7));
  }
  path = path.substr(0, path.find('?'));

  root = root / std::filesystem::path(path);

  if (std::filesystem::is_directory(root)) {
    root /= "index.html";
    is_index = true;
  }

  if (!std::filesystem::exists(root)) {
    if (is_index) {
      response.set_status(HTTPResponse::FORBIDDEN);
    } else {
      response.set_status(HTTPResponse::NOT_FOUND);
    }
    response.set_headers(headers);
    std::string resp = response.build(type);

    send(fd, resp.c_str(), resp.length(), 0);

    kill_client(fd, base);

    return;
  }

  std::string filetype = HTTPResponse::guess_content_type(root);

  if (filetype.empty()) {
    response.set_status(HTTPResponse::FORBIDDEN);
    response.set_headers(headers);
    std::string resp = response.build(type);

    send(fd, resp.c_str(), resp.length(), 0);

    kill_client(fd, base);

    return;
  }

  headers.insert({"Content-Type", filetype});
  if (type == HEAD) {
    headers.insert(
        {"Content-Length", std::to_string(std::filesystem::file_size(root))});
    response.set_status(HTTPResponse::OK);
    response.set_headers(headers);
    send(fd, response.build(type).c_str(), response.build(type).length(), 0);

    kill_client(fd, base);
    return;
  }
  send_file(fd, base, root, headers);
}

void Worker::run(std::filesystem::path document_root) {
  auto *_logger = &Logger::getInstance();
  _document_root = std::move(document_root);

  /*
   * Creating event base for worker
   * */
  _base = event_base_new();
  if (!_base) {
    _logger->log("[ERROR] ", std::this_thread::get_id(),
                 " Worker::run: could not initialize event_base: ",
                 hstrerror(errno), "\n");
    exit(1);
  }

  _listener = event_new(_base, _listener_pipe_fd, EV_READ | EV_PERSIST,
                        on_connection, (void *)this);

  if (!_listener) {
    _logger->log("[ERROR] ", std::this_thread::get_id(),
                 " Worker::run: new_event failed");
    close(_listener_pipe_fd);
    exit(1);
  }

  if (event_add(_listener, nullptr) == -1) {
    _logger->log("[ERROR] ", std::this_thread::get_id(),
                 " Worker::run: event_add failed");
    event_free(_listener);
    close(_listener_pipe_fd);
    exit(1);
  }

  if (_logger->getDebugMode()) {
    _logger->log("[DEBUG] ", std::this_thread::get_id(),
                 " Worker::run: started");
  }

  /*
   * Run event loop with listener and clients
   * */
  event_base_dispatch(_base);
}

void Worker::send_file(evutil_socket_t client_fd, event_base *base,
                       const std::filesystem::path &file_path,
                       std::unordered_map<std::string, std::string> &headers) {
  auto *_logger = &Logger::getInstance();
  std::ifstream file(file_path, std::ios::binary | std::ios::ate);
  if (!file) {
    _logger->log("[ERROR] ", std::this_thread::get_id(),
                 " Worker::send_file: file open failed: ", hstrerror(errno));
    kill_client(client_fd, base);
    return;
  }
  auto file_size = file.tellg();
  headers.insert({"Content-Length", std::to_string(file_size)});
  file.seekg(0);

  HTTPResponse response;
  response.set_status(HTTPResponse::OK);
  response.set_headers(headers);

  /*
   * Firstly, send the response header
   * */
  std::string resp = response.build(0);
  send(client_fd, resp.c_str(), resp.length(), MSG_NOSIGNAL);

  /*
   * Buffer of chunk size 1MB
   * */
  const long buffer_size = 1024 * 1024;
  char buffer[buffer_size];

  long total_sent = 0;
  while (total_sent < file_size) {
    long remaining = file_size - total_sent;
    long chunk_size = std::min(remaining, buffer_size);

    /*
     * Reading a chunk from file
     * */
    file.read(buffer, chunk_size);
    if (!file) {
      _logger->log("[ERROR] ", std::this_thread::get_id(),
                   " Worker::send_file: file read failed: ", hstrerror(errno));
      kill_client(client_fd, base);
      return;
    }

    /*
     * Sending a chunk to client with MSG_NOSIGNAL flag
     * */
    ssize_t sent = send(client_fd, buffer, chunk_size, MSG_NOSIGNAL);
    if (sent == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        /*
         * Socket is not ready for writing, try again later
         * */
        continue;
      }
      _logger->log("[ERROR] ", std::this_thread::get_id(),
                   " Worker::send_file: send failed: ", hstrerror(errno));
      kill_client(client_fd, base);
      return;
    }

    total_sent += sent;
  }

  /*
   * Close the socket and file
   * */
  file.close();
  kill_client(client_fd, base);
}

void Worker::kill_client(evutil_socket_t client_fd, event_base *base) {
  event_del_noblock(event_base_get_running_event(base));
  close(client_fd);
}
