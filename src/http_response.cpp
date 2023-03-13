#include "../include/http_response.h"

#include <ctime>
#include <iostream>
#include <sstream>
#include <unordered_map>

std::unordered_map<std::string, std::string> content_type_table{
    {".html", "text/html"},
    {".htm", "text/htm"},
    {".css", "text/css"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".png", "image/png"},
    {".swf", "application/x-shockwave-flash"},
    {".js", "application/javascript"},
    {".txt", "plain/text"}};

std::string HTTPResponse::guess_content_type(
    const std::filesystem::path& path) {
  std::string format = path.extension();
  try {
    return content_type_table.at(format);
  } catch (std::exception& ex) {
    return {};
  }
}

std::string HTTPResponse::convert_to_message(int status) {
  switch (status) {
    case 200:
      return "OK";
    case 400:
      return "Bad Request";
    case 403:
      return "Forbidden";
    case 404:
      return "Not Found";
    case 405:
      return "Method Not Allowed";
    default:
      return "Unknown Status";
  }
}

std::string HTTPResponse::get_current_time() {
  std::time_t now = std::time(nullptr);
  char time_str[50];

  std::strftime(time_str, sizeof(time_str), "%a, %d %b %Y %T GMT",
                std::gmtime(&now));

  return {time_str};
}

std::string HTTPResponse::build(uint8_t request_type) const {
  std::ostringstream oss;
  oss << "HTTP/1.1 " << get_status() << "\r\n";

  oss << get_headers();

  if (!request_type) {
    oss << get_body();
  }

  return oss.str();
}

std::string HTTPResponse::decodeURL(const std::string& encodedURL) {
  std::ostringstream decoded;

  auto it = encodedURL.begin();
  while (it != encodedURL.end()) {
    if (*it == '%') {
      char hex[3];
      hex[0] = *(++it);
      hex[1] = *(++it);
      hex[2] = '\0';
      int code = std::stoi(hex, nullptr, 16);
      decoded.put(static_cast<char>(code));
    } else if (*it == '+') {
      decoded.put(' ');
    } else {
      decoded.put(*it);
    }
    ++it;
  }

  return decoded.str();
}
