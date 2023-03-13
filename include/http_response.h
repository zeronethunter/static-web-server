#ifndef STATIC_WEB_SERVER_HTTP_RESPONSE_H
#define STATIC_WEB_SERVER_HTTP_RESPONSE_H

#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

class HTTPResponse {
 public:
  HTTPResponse() = default;

  std::string get_status() const { return _status; }

  void set_status(uint16_t status_code) {
    _status_code = status_code;
    _status =
        std::to_string(_status_code) + " " + convert_to_message(_status_code);
  }

  std::string get_body() const { return _body; }

  void set_body(const std::string& body) {
    if (_status_code != 200) {
      _body = convert_to_message(_status_code);
    } else {
      _body = body;
    }

    _headers["Content-Length"] = std::to_string(body.size());
    _headers["Content-Type"] = guess_content_type(body);
  }

  std::string get_headers() const {
    std::ostringstream oss;

    for (const auto& header : _headers) {
      oss << header.first << ": " << header.second << "\r\n";
    }

    oss << "\r\n";

    return oss.str();
  }

  void add_header(const std::string& key, const std::string& value) {
    _headers.insert({key, value});
  }

  void set_headers(
      const std::unordered_map<std::string, std::string>& headers) {
    _headers = headers;
  }

  std::string build(uint8_t request_type) const;

  enum Status {
    OK = 200,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
  };

  static std::string get_current_time();
  static std::string guess_content_type(const std::filesystem::path& filename);
  static std::string decodeURL(const std::string& encodedURL);

 private:
  uint16_t _status_code{};
  std::string _status;
  std::string _body;
  std::unordered_map<std::string, std::string> _headers;

  static std::string convert_to_message(int status);
};

#endif  // STATIC_WEB_SERVER_HTTP_RESPONSE_H
