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
  Worker(ServerSocket& socket) : _socket(socket) {}

  ~Worker() {
    auto* _logger = &Logger::getInstance();
    if (_logger->getDebugMode()) {
      _logger->log(std::this_thread::get_id(), " Worker closed");
    }
    event_free(_listener);
    event_base_free(_base);
  }

  void run(std::filesystem::path document_root);

  Worker(const Worker&) = delete;
  Worker& operator=(const Worker&) = delete;

  Worker(Worker&&) = delete;
  Worker& operator=(Worker&&) = delete;

  inline static std::filesystem::path _document_root;

 private:
  static void on_connection(evutil_socket_t listener, short event, void* arg);
  static void on_request(int client_fd, short ev, void* arg);
  static void handle_request(int client_fd, event_base* base, char* request,
                             size_t length);
  static void kill_client(int client_fd, event_base* base);
  static void send_file(int client_fd, event_base* base,
                        const std::filesystem::path& file_path,
                        std::unordered_map<std::string, std::string>& headers);

  event* _listener{};
  event_base* _base{};
  ServerSocket _socket;
};

#endif  // STATIC_WEB_SERVER_WORKER_H
