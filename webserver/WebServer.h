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

  void acceptConnection() {
    struct sockaddr addr;
    socklen_t socklen;
    int newSocket;
    // if no connection, accept is blocking process and start waiting
    if ((newSocket = accept(fd, (struct sockaddr *) &addr, (socklen_t *) &socklen) == -1)) {
      LOGGER.error(LISTEN_ERROR);
      //todo throw custom exception
    }
  }

 public:
  void run() {
    // todo add Parser and config
    // todo 1. parse args
    // 2. create fd
    createSocket();
    // 3. bind
    bindAddress();
    // 4. listen
    startListening();
    // 5. accept
    // for now separate logic into other method, maybe change it later
    acceptConnection();
    // todo 6. set non blocking
    while (true) {
      // for now it is just running and we can see that port 9000 is listening
      // run this in shell to check:
      // netstat -a -n | grep LISTEN
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
  static const char *ACCEPT_ERROR;
};

const int WebServer::PORT_DEFAULT = 9000;

const char *WebServer::BAD_SOCKET_FD = "Something went wrong";
const char *WebServer::BIND_ERROR = "Bind error";
const char *WebServer::LISTEN_ERROR = "Listen error";
const char *WebServer::ACCEPT_ERROR = "Accept error";
