#include "../include/server_socket.h"

#include "event2/event.h"

/* Socket initialization
 *
 * Returns 0 if success, 1 on failure
 * */
uint8_t ServerSocket::init() {
  _fd = socket(AF_INET, SOCK_STREAM, 0);
  if (_fd == -1) {
    _logger->log("[ERROR] ", "Could not create socket: ", hstrerror(errno));
    return 1;
  }

  if (evutil_make_listen_socket_reuseable(_fd) == -1) {
    _logger->log("[ERROR] ",
                 "Could not make socket reusable: ", hstrerror(errno));
    return 1;
  }

  if (evutil_make_socket_nonblocking(_fd) == -1) {
    _logger->log("[ERROR] ",
                 "Could not make socket nonblocking: ", hstrerror(errno));
    return 1;
  }

  _addr.sin_family = AF_INET;
  _addr.sin_addr.s_addr = INADDR_ANY;
  _addr.sin_port = htons(_port);

  if (bind(_fd, reinterpret_cast<struct sockaddr *>(&_addr), sizeof(_addr)) ==
      -1) {
    _logger->log("[ERROR] ", "Could not bind to port ", _port, ": ",
                 hstrerror(errno));
    return 1;
  }

  if (listen(_fd, SOMAXCONN) == -1) {
    _logger->log("[ERROR] ", "Could not listen on port ", _port, ": ",
                 hstrerror(errno));
    return 1;
  }

  return 0;
}
