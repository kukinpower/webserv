#pragma once
#include "Logger.h" //todo optimize includes

#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>

class WebServer {
 private:

 private:
  int fd;
  Logger LOGGER;

 public:
  WebServer() {
    fd = socket(AF_INET, SOCK_STREAM, TCP);
    if (fd == -1) {
      LOGGER.error(BAD_SOCKET_FD);
      //todo throw custom exception
    }
  }
  ~WebServer() {}

  int getSocketFd() const {
    return fd;
  }

  void run() {

  }

 public:
  static const int TCP = 0;
  static const char *BAD_SOCKET_FD;
};

const char *WebServer::BAD_SOCKET_FD = "Something went wrong";
