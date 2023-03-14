#ifndef STATIC_WEB_SERVER_LOGGER_H
#define STATIC_WEB_SERVER_LOGGER_H

#include <time.h>

#include <chrono>
#include <iostream>
#include <mutex>
#include <sstream>

#include "http_response.h"

/* Singleton logger */
class Logger {
 public:
  static Logger& getInstance() {
    static Logger instance;
    return instance;
  }

  template <typename... Args>
  void log(Args&&... args) {
    std::string msg = constructMessage(std::forward<Args>(args)...);
//    msg = "\033[31m[" + get_current_time() + "]\033[0m " + msg;
    m_outputStream << msg;
  }

  void setDebugMode(bool debug_mode) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_debug_mode = debug_mode;
  }

  bool getDebugMode() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_debug_mode;
  }

  void setOutputStream(std::ostream& outputStream) {
    m_outputStream.rdbuf(outputStream.rdbuf());
  }

  static std::string get_current_time() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    std::tm utc_tm{};
    gmtime_r(&t, &utc_tm);

    std::ostringstream oss;
    oss << std::put_time(&utc_tm, "%a, %d %b %Y %T GMT");

    return oss.str();
  }

  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

 private:
  Logger() = default;
  ~Logger() = default;

  template <typename T>
  void print(std::stringstream& ss, T&& arg) {
    ss << std::forward<T>(arg);
  }

  template <typename T, typename... Args>
  void print(std::stringstream& ss, T&& arg, Args&&... args) {
    ss << std::forward<T>(arg);
    print(ss, std::forward<Args>(args)...);
  }

  /*
   * This function is required to compose the output string. Because it is
   * asynchronous with the stream, there are no tools for atomic interaction
   * between events. Therefore, logging must be done as a single output of all
   * data.
   * */
  template <typename... Args>
  std::string constructMessage(Args&&... args) {
    std::stringstream ss;
    print(ss, std::forward<Args>(args)...);
    ss << std::endl;
    return ss.str();
  }

  std::mutex m_mutex;
  std::ostream& m_outputStream{std::cout};
  bool m_debug_mode{true};
};

#endif  // STATIC_WEB_SERVER_LOGGER_H
