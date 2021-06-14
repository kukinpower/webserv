#pragma once
#include "StringBuilder.h"

class HttpStatusWrapper {
 private:
  static const char *PROTOCOL;
  HttpStatus status;
  int code;
  std::string name;

 public:
  HttpStatusWrapper(HttpStatus status, const std::string &name) : status(status), code(status), name(name) {}

  std::string createMainLine() const {
    return StringBuilder().append(PROTOCOL).append(" ").append(code).append(" ").append(name).toString();
  }
};

const char *HttpStatusWrapper::PROTOCOL = "HTTP/1.1";
