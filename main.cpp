#include <csignal>
#include <fstream>
#include <thread>

#include "include/config.h"
#include "include/webserver.h"

void ws_terminate(int sig) {
  auto *_logger = &Logger::getInstance();
  _logger->log("\nWebServer is closed with signal: ", sig);
  exit(0);
}

int main(int argc, char *argv[]) {
  Logger *logger = &Logger::getInstance();
  signal(SIGINT, ws_terminate);

  uint8_t cpu_count = 0;
  if (argc == 2) {
    cpu_count = std::stoi(argv[1]);
  }

  /*
   * You could set output stream to log file
   * Debug mode to log requests and specific info, enabled by default
   * */
  // std::ofstream log_file("../log.txt");
  logger->setOutputStream(std::cout);

  logger->info("Starting web server...");
  logger->info("Reading config file...");

  Config config("/etc/httpd.conf");
  if (cpu_count) {
    config.set_cpu_limit(cpu_count);
  }

  logger->setDebugMode(config.get_debug_mode());

  if (config.get_cpu_limit() > std::thread::hardware_concurrency()) {
    logger->info("CPU limit is greater than number of available cores: ",
                 "setting CPU limit to ",
                 std::to_string(std::thread::hardware_concurrency()), "\n");
    config.set_cpu_limit(std::thread::hardware_concurrency());
  }

  logger->log(
      "\033[31m", "Current webserver state", "\033[0m",
      "\n\t\033[31mCPU limit:\033[0m" + std::to_string(config.get_cpu_limit()),
      "\n\t\033[31mDocument root:\033[0m" + config.get_document_root().string(),
      "\n\t\033[31mPort:\033[0m" + std::to_string(config.get_port()),
      "\n\t\033[31mDebug mode:\033[0m" +
          std::string(config.get_debug_mode() ? "true" : "false"));

  WebServer ws(config.get_port(), config.get_cpu_limit(),
               config.get_document_root());

  logger->log("Ready to accept connections");

  ws.run();

  return 0;
}