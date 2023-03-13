#include <csignal>
#include <fstream>
#include <thread>

#include "include/config.h"
#include "include/webserver.h"

void ws_terminate(int sig) {
  auto* _logger = &Logger::getInstance();
  _logger->log("\nWebServer is closed with signal: ", sig);
  exit(0);
}

int main() {
  Logger* logger = &Logger::getInstance();
  signal(SIGINT, ws_terminate);

  /*
   * You could set output stream to log file
   * Debug mode to log requests and specific info, enabled by default
   * */
  logger->setOutputStream(std::cout);

  logger->log("Starting web server...");
  logger->log("Reading config file...");

  Config config("/etc/httpd.conf");

  logger->setDebugMode(config.get_debug_mode());

  if (config.get_cpu_limit() > std::thread::hardware_concurrency()) {
    logger->log("CPU limit is greater than number of available cores: ",
                "setting CPU limit to ",
                std::to_string(std::thread::hardware_concurrency()), "\n");
    config.set_cpu_limit(std::thread::hardware_concurrency());
  }

  logger->log("Current webserver state",
              "\n\tCPU limit:" + std::to_string(config.get_cpu_limit()),
              "\n\tDocument root:" + config.get_document_root().string(),
              "\n\tPort:" + std::to_string(config.get_port()),
              "\n\tDebug mode:" +
                  std::string(config.get_debug_mode() ? "true" : "false"));

  WebServer ws(config.get_port(), config.get_cpu_limit(),
               config.get_document_root());

  logger->log("Ready to accept connections");

  ws.run();

  return 0;
}