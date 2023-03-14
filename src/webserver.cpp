#include "../include/webserver.h"

#include "../include/worker.h"
#include "event2/event.h"
#include "event2/thread.h"

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
    auto worker = new Worker(socket);
    std::thread t([&worker, this]() { worker->run(_document_root); });
    _workers.push_back(worker);

    t.detach();
  }

  while (true) sleep(UINT_MAX);
}
