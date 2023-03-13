#ifndef STATIC_WEB_SERVER_CONFIG_H
#define STATIC_WEB_SERVER_CONFIG_H

#include <filesystem>
#include <iostream>

#define DEFAULT_CONFIG_PATH "/etc/httpd.conf"

class Config {
 public:
  explicit Config(std::string config_path)
      : _config_path(std::move(config_path)) {
    init();
  }

  [[nodiscard]] uint8_t get_cpu_limit() const { return _cpu_limit; };
  void set_cpu_limit(uint8_t cpu_limit) { _cpu_limit = cpu_limit; }
  [[nodiscard]] std::filesystem::path get_document_root() const {
    return _document_root;
  }
  [[nodiscard]] uint16_t get_port() const { return _port; }
  [[nodiscard]] bool get_debug_mode() const { return _debug_mode; }

 private:
  void init();

  std::filesystem::path _config_path{DEFAULT_CONFIG_PATH};
  uint8_t _cpu_limit{};
  std::filesystem::path _document_root;
  uint16_t _port{80};
  bool _debug_mode{true};
};

#endif  // STATIC_WEB_SERVER_CONFIG_H
