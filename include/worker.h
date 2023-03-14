#include "server_socket.h"

#ifndef STATIC_WEB_SERVER_WORKER_H
#define STATIC_WEB_SERVER_WORKER_H

#include <event2/event.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <thread>
#include <utility>

class Worker {
 public:
  explicit Worker(ServerSocket& socket, int listen_main_fd, int sender_pipe_fd)
      : _socket(socket) {
    _listener_pipe_fd = listen_main_fd;
    _sender_pipe_fd = sender_pipe_fd;
  }

  Worker(const Worker& worker) noexcept
      : _socket(worker._socket),
        _base(worker._base),
        _listener(worker._listener) {
    _listener_pipe_fd = worker.get_listener_pipe();
    _sender_pipe_fd = worker.get_sender_pipe();
  };

  Worker& operator=(const Worker& worker) {
    if (this == &worker) {
      return *this;
    }
    _listener_pipe_fd = worker.get_listener_pipe();
    _sender_pipe_fd = worker.get_sender_pipe();
    _socket = worker._socket;
    _base = worker._base;
    _listener = worker._listener;
    return *this;
  }

  bool operator==(const Worker& worker) const noexcept {
    return _listener_pipe_fd == worker.get_listener_pipe() &&
           _sender_pipe_fd == worker.get_sender_pipe();
  }

  ~Worker() {
    auto* _logger = &Logger::getInstance();
    if (_logger->getDebugMode()) {
      _logger->log(std::this_thread::get_id(), " Worker closed");
    }
    event_free(_listener);
    event_base_free(_base);
  }

  void run(std::filesystem::path document_root);

  [[nodiscard]] int get_listener_pipe() const { return _listener_pipe_fd; }
  [[nodiscard]] int get_sender_pipe() const { return _sender_pipe_fd; }
  [[nodiscard]] event_base* get_base() const { return _base; }
  [[nodiscard]] std::filesystem::path get_document_root() const {
    return _document_root;
  }

 private:
  static void on_connection(evutil_socket_t listener, short event, void* arg);
  static void on_request(int client_fd, short ev, void* arg);
  static void handle_request(std::filesystem::path document_root,
                             evutil_socket_t client_fd, event_base* base,
                             char* request, size_t length);
  static void kill_client(int client_fd, event_base* base);
  static void send_file(int client_fd, event_base* base,
                        const std::filesystem::path& file_path,
                        std::unordered_map<std::string, std::string>& headers);

  evutil_socket_t _listener_pipe_fd;
  evutil_socket_t _sender_pipe_fd;
  std::filesystem::path _document_root;
  event* _listener{};
  event_base* _base{};
  ServerSocket _socket;
};

#endif  // STATIC_WEB_SERVER_WORKER_H
