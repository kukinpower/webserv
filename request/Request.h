#pragma once
#include "Method.h"

#include "MaxRequestSizeExceededException.h"

#include <map>

class Request {
 public:
  typedef std::map<std::string, std::string> Headers;
  typedef std::map<std::string, std::string> MimeTypes;

 private:
  // constants
  static const int REQUEST_MAX_LENGTH_DEFAULT = 100000000; // 100_000_000
  static const int HEADER_DELIMETER_LENGTH = 2;
  static const char *HEADER_DELIMETER;
  // vars
  Method method;
  std::string path;
  Headers headers;
  std::string body;
  static const MimeTypes MIME;

  static Headers initMimeTypes() {
    Headers mime;
    mime.insert(std::make_pair(".htm", "text/html"));
    mime.insert(std::make_pair(".html", "text/html"));
    mime.insert(std::make_pair(".jpg", "image/jpeg"));
    mime.insert(std::make_pair(".jpeg", "image/jpeg"));
    mime.insert(std::make_pair(".js", "text/javascript"));
    return mime;
  }

 public:
  Request(Method method, const std::string &path, const std::map<std::string, std::string> &headers, const std::string &body = "")
      : method(method), path(path), headers(headers), body(body) {}

  virtual ~Request() {}
  Request(const Request &request) {
    operator=(request);
  }

  Request &operator=(const Request &request) {
    this->method = request.method;
    this->path = request.path;
    this->headers = request.headers;
    return *this;
  }

 public:
  static Headers extractHttpHeaders(const std::string &headersToken) {
    std::stringstream ss(headersToken);
    Headers requestHeaders;
    std::string line;

    while (std::getline(ss, line)) {
      size_t pos;
      if ((pos = line.find(HEADER_DELIMETER)) != std::string::npos) {
        break;
      }
      std::string key = line.substr(0, pos);
      line.erase(0, pos + HEADER_DELIMETER_LENGTH);
      requestHeaders.insert(std::make_pair(key, line));
    }
    return requestHeaders;
  }

  static size_t getLengthFromHeaders(const Headers &requestHeaders) {
    return std::atoi(requestHeaders.find("Content-Length")->second.c_str());
  }

  static std::string extractHttpBodyByLength(const std::string &bodyToken, size_t pos, size_t length) {
    return bodyToken.substr(pos, length);
  }

  static bool hasBodyLength(const Headers &requestHeaders) {
    return requestHeaders.count("Content-Length") != 0;
  }

  static bool hasConnectionClose(const Headers &requestHeaders) {
    return (requestHeaders.count("Connection") != 0 && requestHeaders.find("Connection")->second == "Close");
  }

  static Method extractMethod(const std::string &line) {
    if (line.find("POST") != line.npos) {
      return POST;
    }
    if (line.find("DELETE") != line.npos) {
      return DELETE;
    }
    return GET;
  }

  static std::string extractPath(const std::string &line) {
    size_t start = line.find(" ") + 1;
    size_t end = line.rfind(" ");
    return line.substr(start, end - start);
  }

  Method GetMethod() const {
    return method;
  }
  const std::string &GetPath() const {
    return path;
  }
  const Headers &GetHeaders() const {
    return headers;
  }
  const std::string &GetBody() const {
    return body;
  }

};

const char *Request::HEADER_DELIMETER = ": ";
const Request::MimeTypes Request::MIME = initMimeTypes();

