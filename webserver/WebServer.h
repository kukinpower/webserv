#pragma once
#include "Logger.h" //todo optimize includes

#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h> //todo optimize

class WebServer {
 private:

 private:
  int fd;
  Logger LOGGER;

 public:
  WebServer() {
    fd = -1;
  }
  ~WebServer() {}

  int getSocketFd() const {
    return fd;
  }

 private:
  void createSocket() {
    fd = socket(AF_INET, SOCK_STREAM, TCP);
    if (fd == -1) {
      LOGGER.error(BAD_SOCKET_FD);
      //todo throw custom exception
    }
  }

  void bindAddress() const {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_DEFAULT);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (0 != bind(fd, (struct sockaddr *) &addr, sizeof(addr))) {
      LOGGER.error(BIND_ERROR);
      //todo throw custom exception
    }
  }

  void startListening() {
    if (-1 == listen(fd, 5)) {
      LOGGER.error(LISTEN_ERROR);
      //todo throw custom exception
    }
  }

 public:
  void run() {
    // todo add Parser and config
    // 1. parse args
    // 2. create fd
    createSocket();
    // 3. bind
    bindAddress();
    // 4. listen
    startListening();
    while (true) {

    }

  }

 public:
  // constants to use
  static const int TCP = 0;
  static const int PORT_DEFAULT;

  // messages temporarily here todo move to custom exceptions
  static const char *BAD_SOCKET_FD;
  static const char *BIND_ERROR;
  static const char *LISTEN_ERROR;
};

const int WebServer::PORT_DEFAULT = 9000;

const char *WebServer::BAD_SOCKET_FD = "Something went wrong";
const char *WebServer::BIND_ERROR = "Bind error";
const char *WebServer::LISTEN_ERROR = "Listen error";
