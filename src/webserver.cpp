#include "../include/webserver.h"

#include "../include/worker.h"
#include "event2/event.h"

void WebServer::run() {
  /*
   * Listener socket initialization
   * */
  auto socket = ServerSocket(_port);

  if (socket.init() == 1) {
    _logger->log("[ERROR] ",
                 "Could not initialize listener socket\n Terminating...");
    exit(1);
  }

  for (uint16_t i = 0; i < _num_threads; ++i) {
    std::thread t([&socket, this]() { Worker(socket).run(_document_root); });

    t.detach();
  }

  while (true) sleep(UINT_MAX);
}
