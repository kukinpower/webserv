#pragma once
#include "Method.h"

#include "MaxRequestSizeExceededException.h"

#include <map>

class Request {
 private:
  // constants
  static const int REQUEST_MAX_LENGTH_DEFAULT = 100000000; // 100_000_000
  static const int HEADER_DELIMETER_LENGTH = 2;
  static const char *HEADER_DELIMETER;
  // vars
  Method method;
  std::string path;
  std::map<std::string, std::string> headers;

 public:
  Request(Method method, const std::string &path, const std::map<std::string, std::string> &headers)
      : method(method), path(path), headers(headers) {}

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

  static std::map<std::string, std::string> extractHeaders(std::stringstream &ss) {
    std::map<std::string, std::string> requestHeaders;
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

  static Request parseRequest(const std::string &requestBody) {
    if (requestBody.length() > REQUEST_MAX_LENGTH_DEFAULT) {
      throw MaxRequestSizeExceededException();
    }
    std::stringstream ss(requestBody);
    std::map<std::string, std::string> requestHeaders = extractHeaders(ss);

    //todo------------------------------------------------------------
    for (const auto &a : requestHeaders) {
      std::cout << a.first << " " << a.second;
    }
    return Request(GET, "some path", requestHeaders);
  }
};

const char *Request::HEADER_DELIMETER = ": ";
