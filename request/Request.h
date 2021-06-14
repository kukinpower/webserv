#pragma once
#include "HttpMethod.h"
#include "RequestStatus.h"

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
  HttpMethod method;
  std::string path;
  Headers headers;
  std::string body;
  RequestStatus status;
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
  Request(HttpMethod method,
		  const std::string &path,
		  const std::map<std::string, std::string> &headers,
		  const std::string &body = "",
		  RequestStatus status = READY)
      : method(method), path(path), headers(headers), body(body), status(status) {}

  virtual ~Request() {}
  Request(const Request &request) {
    operator=(request);
  }

  Request &operator=(const Request &request) {
    this->method = request.method;
    this->path = request.path;
    this->headers = request.headers;
    this->body = request.body;
    this->status = request.status;
    return *this;
  }

 public:
  static size_t getHeadersStartPosition(const std::string &headerFull) {
	return headerFull.find("\r\n") + 2;
  }
  
  static Headers extractHttpHeaders(const std::string &headersToken, size_t inputPos) {
	size_t headersStart = getHeadersStartPosition(headersToken);
    std::stringstream ss(headersToken.substr(headersStart, inputPos - headersStart));
    Headers requestHeaders;
    std::string line;

    while (std::getline(ss, line)) {
      size_t pos;
      if ((pos = line.find(HEADER_DELIMETER, 0)) == std::string::npos) {
        break;
      }
      std::string key = line.substr(0, pos);
      line.erase(0, pos + HEADER_DELIMETER_LENGTH);
      requestHeaders.insert(std::make_pair(key, line.substr(0, line.find('\r'))));
    }
    return requestHeaders;
  }

  static size_t getLengthFromHeaders(const Headers &requestHeaders) {
    return std::atoi(requestHeaders.find("Content-Length")->second.c_str());
  }

  static bool hasBodyLength(const Headers &requestHeaders) {
    return requestHeaders.count("Content-Length") != 0;
  }

  static bool isConnectionClose(const Headers &requestHeaders) {
    return (requestHeaders.count("Connection") != 0 && requestHeaders.find("Connection")->second == "close");
  }

  static HttpMethod extractMethod(const std::string &line) {
    if (line.find("POST") != std::string::npos) {
      return POST;
    }
    if (line.find("DELETE") != std::string::npos) {
      return DELETE;
    }
    return GET;
  }

  static std::string extractPath(const std::string &line) {
    size_t start = line.find(' ') + 1;
    size_t end = line.rfind(' ');
    return line.substr(start, end - start);
  }

  HttpMethod getMethod() const {
    return method;
  }
  const std::string &getPath() const {
    return path;
  }
  const Headers &getHeaders() const {
    return headers;
  }
  const std::string &getBody() const {
    return body;
  }
  void setBody(const std::string &bodyToSet) {
    this->body = bodyToSet;
  }

  size_t getLength() {
	return getLengthFromHeaders(headers);
  }

  RequestStatus getStatus() const {
	return status;
  }

  void setStatus(RequestStatus requestStatus) {
	this->status = requestStatus;
  }
};

const char *Request::HEADER_DELIMETER = ": ";
const Request::MimeTypes Request::MIME = initMimeTypes();
