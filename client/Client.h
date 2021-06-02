#pragma once
#include "Request.h"
#include "Response.h"

class Client {
 private:
  int fd;
  std::string request;

 public:
  Client(int fd) : fd(fd) {}
  ~Client() {}
  //todo Coplien form

  int getFd() const {
	return fd;
  }

  void appendToRequest(char *chunk) {
    request += chunk;
  }

  std::string getRequest() const {
    return request;
  }
};
