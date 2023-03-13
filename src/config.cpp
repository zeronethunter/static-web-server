#include "../include/config.h"

#include <fstream>

#include "../include/logger.h"

void Config::init() {
  auto *_logger = &Logger::getInstance();

  std::ifstream config_file(_config_path);
  if (!config_file.is_open()) {
    _logger->log("Can't open config file: ", _config_path);
    return;
  }
  std::string line;
  try {
    while (std::getline(config_file, line)) {
      if (line.empty()) {
        continue;
      }
      if (line.substr(0, line.find(' ')) == "cpu_limit") {
        line.erase(0, line.find(' ') + 1);
        _cpu_limit = std::stoi(line);
      } else if (line.substr(0, line.find(' ')) == "document_root") {
        line.erase(0, line.find(' ') + 1);
        while (line.back() == '\r') {
          line.pop_back();
        }
        _document_root = std::filesystem::path(line);
      } else if (line.substr(0, line.find(' ')) == "port") {
        line.erase(0, line.find(' ') + 1);
        _port = std::stoi(line);
      } else if (line.substr(0, line.find(' ')) == "debug") {
        line.erase(0, line.find(' ') + 1);
        if (line.find("true") != std::string::npos) {
          _debug_mode = true;
        } else if (line.find("false") != std::string::npos) {
          _debug_mode = false;
        } else {
          _logger->log("[ERROR] ", "Invalid debug mode value: ", line,
                       ", using default");
        }
      }
    }
  } catch (std::exception &e) {
    _logger->log("[ERROR] ", "Error while parsing config file: ", e.what());
    return;
  }

  _logger->log("Config file successfully parsed");
}
