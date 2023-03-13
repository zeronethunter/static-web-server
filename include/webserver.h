#ifndef STATIC_WEB_SERVER_WEBSERVER_H
#define STATIC_WEB_SERVER_WEBSERVER_H

#include <filesystem>
#include <iostream>
#include <thread>
#include <utility>
#include <vector>

#include "event.h"
#include "logger.h"
#include "server_socket.h"
#include "worker.h"

#define DEFAULT_PORT 80
#define DEFAULT_DOCUMENT_ROOT "/var/www/html"
#define DEFAULT_NUM_THREADS 1

class WebServer {
 public:
  WebServer(uint16_t port, uint32_t num_threads,
            std::filesystem::path document_root)
      : _port(port), _document_root(std::move(document_root)) {
    if (!num_threads) {
      _num_threads = std::thread::hardware_concurrency();
    } else {
      _num_threads = num_threads;
    }

    _logger = &Logger::getInstance();
  };

  ~WebServer() {
    // free resources
    for (auto* base : _http_event_bases) {
      event_base_free(base);
    }
  };

  void run();

 private:
  uint16_t _port{DEFAULT_PORT};
  uint16_t _num_threads{DEFAULT_NUM_THREADS};
  std::filesystem::path _document_root{DEFAULT_DOCUMENT_ROOT};
  std::vector<event_base*> _http_event_bases{};

  Logger* _logger;
};

#endif  // STATIC_WEB_SERVER_WEBSERVER_H
