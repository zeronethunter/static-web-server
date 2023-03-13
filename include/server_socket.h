#ifndef STATIC_WEB_SERVER_SERVER_SOCKET_H
#define STATIC_WEB_SERVER_SERVER_SOCKET_H

#include <netinet/in.h>
#include <unistd.h>

#include "logger.h"

class ServerSocket {
 public:
  explicit ServerSocket(uint16_t port)
      : _port(port), _logger(&Logger::getInstance()){};
  ~ServerSocket() = default;
  uint8_t init();
  [[nodiscard]] int getFD() const { return _fd; }

  void close_socket() {
    if (_fd) {
      close(_fd);
      _fd = 0;
    }
  }

 private:
  int _fd{};
  sockaddr_in _addr{};
  uint16_t _port;
  Logger* _logger;
};

#endif  // STATIC_WEB_SERVER_SERVER_SOCKET_H
