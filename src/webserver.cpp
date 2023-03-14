#include "../include/webserver.h"

#include <fcntl.h>

#include <iterator>

#include "../include/worker.h"
#include "event2/event.h"
#include "event2/thread.h"

void WebServer::run() {
  if (evthread_use_pthreads() == -1) {
    _logger->log("[ERROR] ", "Couldn't initialize libevent threading\n");
    exit(1);
  }

  /*
   * Listener socket initialization
   * */
  auto socket = ServerSocket(_port);

  if (socket.init() == 1) {
    _logger->log("[ERROR] ",
                 "Could not initialize listener socket\n Terminating...");
    exit(1);
  }

  /*
   * Creating event base for worker
   * */
  auto listener_base = event_base_new();
  if (!listener_base) {
    _logger->log("[ERROR] ",
                 "Couldn't initialize listener event base: ", hstrerror(errno),
                 "\n");
    exit(1);
  }

  /*
   * Creating socket-listener event in worker (EV_PERSIST required)
   * */
  auto listener = event_new(listener_base, socket.getFD(), EV_READ | EV_PERSIST,
                            on_accept, (void *)this);
  if (!listener) {
    _logger->log("[ERROR] ",
                 "Couldn't initialize listener event: ", hstrerror(errno),
                 "\n");
    exit(1);
  }

  if (event_add(listener, nullptr) == -1) {
    _logger->log("[ERROR] ", "Couldn't add listener event: ", hstrerror(errno),
                 "\n");
    exit(1);
  }

  for (uint16_t i = 0; i < _num_threads; ++i) {
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
      _logger->log("[ERROR] ",
                   "Couldn't create pipe for worker: ", hstrerror(errno), "\n");
      exit(1);
    }

    auto *worker = new Worker(socket, pipe_fd[0], pipe_fd[1]);
    _workers.push_back(worker);
  }

  std::vector<std::thread> threads;
  for (uint16_t i = 0; i < _num_threads; ++i) {
    std::thread t(&Worker::run, _workers[i], _document_root);

    t.detach();
  }

  event_base_dispatch(listener_base);
  event_free(listener);
  event_base_free(listener_base);
}

void WebServer::on_accept(evutil_socket_t listener_socket, short event,
                          void *arg) {
  auto *server = (WebServer *)arg;
  Logger *_logger = &Logger::getInstance();
  sockaddr_storage client_addr{};
  socklen_t client_len = sizeof(client_addr);

  int client_fd =
      accept(listener_socket, (struct sockaddr *)&client_addr, &client_len);

  if (client_fd == -1) {
    return;
  }

  if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) {
    _logger->log("[ERROR] ", " run::on_accept: fcntl::F_SETFL failed");
    close(client_fd);
    return;
  }

  /*
   * Send client to worker (Round-robin)
   * */
  int worker_sender_fd =
      server->_workers[server->_next_worker]->get_sender_pipe();

  ssize_t n = write(worker_sender_fd, &client_fd, sizeof(client_fd));
  if (n < 0) {
    _logger->log("[ERROR] ",
                 " run::on_accept: write failed: ", hstrerror(errno));
    close(client_fd);
    return;
  }

  if (server->_next_worker == server->_workers.size() - 1) {
    server->_next_worker = 0;
  } else {
    ++server->_next_worker;
  }
}
