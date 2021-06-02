#pragma once
#include "Logger.h"
#include "Client.h"

#include "SelectException.h"
#include "BadListenerFdException.h"
#include "NonBlockException.h"
#include "BindException.h"
#include "ListenException.h"
#include "AcceptException.h"
#include "ReadException.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <vector>
#include <iostream>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/fcntl.h>

#include <sstream>


class WebServer {
 private:
  Logger LOGGER;

  int listenerFd;
  std::vector<Client> clients;

 public:
  WebServer() {
    listenerFd = -1;
  }
  ~WebServer() {}

  int getSocketFd() const {
    return listenerFd;
  }

 private:
  static void setNonBlock(int fd) {
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
      std::stringstream ss;
      ss << WebServException::FCNTL_ERROR << " on fd: " << fd;
      throw NonBlockException(ss.str());
    }
  }

  void createSocket() {
    listenerFd = socket(AF_INET, SOCK_STREAM, TCP);
    if (listenerFd == -1) {
      throw BadListenerFdException();
    }
    try {
      setNonBlock(listenerFd);
    } catch (const NonBlockException &e) {
      throw FatalWebServException("Non block exception failure on listener fd");
    }
  }

  void bindAddress() const {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_DEFAULT);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (0 != bind(listenerFd, (struct sockaddr *) &addr, sizeof(addr))) {
      throw BindException();
    }
  }

  void startListening() {
    if (-1 == listen(listenerFd, 5)) {
      throw ListenException();
    }
  }

  int acceptConnection(int maxFd) {
    struct sockaddr addr;
    socklen_t socklen;
    int newClientFd;

    // if no connection, accept is blocking process and start waiting
    if ((newClientFd = accept(listenerFd, (struct sockaddr *) &addr, &socklen)) == -1) {
      throw AcceptException();
    }
    // set nonblock
    setNonBlock(newClientFd);

    Client newClient(newClientFd); //todo maybe on heap?
    clients.push_back(newClient);
    return std::max(maxFd, newClientFd);
  }

  int setReadFds(fd_set *readfds) {
    FD_ZERO(readfds);
    FD_SET(listenerFd, readfds);

    int maxFd = listenerFd;
    for (std::vector<Client>::iterator it = clients.begin();
         it != clients.end(); ++it) {
      int fd = it->getFd();
      FD_SET(fd, readfds);
      maxFd = std::max(maxFd, fd);
    }

    return maxFd;
  }

  void processSelect() {
    fd_set readfds;
    int maxFd = setReadFds(&readfds);

    int res;
    if ((res = select(maxFd + 1, &readfds, NULL, NULL, NULL)) < 1) {
      LOGGER.error(WebServException::SELECT_ERROR);
      throw SelectException();
    }

    if (FD_ISSET(listenerFd, &readfds)) {
      maxFd = acceptConnection(maxFd);
    }

    for (std::vector<Client>::iterator it = clients.begin();
         it != clients.end(); ++it) {
      if (FD_ISSET(it->getFd(), &readfds)) {
        int nbytes;
        char buf[BUF_SIZE + 1];
        if ((nbytes = recv(it->getFd(), buf, BUF_SIZE, 0)) == -1) {
          throw ReadException();
        }
        buf[nbytes] = 0;
        it->appendToRequest(buf);
        LOGGER.info("BUF: " + std::string(buf));
      }
    }
  }

  void routine() {
    int errorCounter = 0;
    while (true) {
      try {
        processSelect();
      } catch (const RuntimeWebServException &e) {
        ++errorCounter;
        LOGGER.error(e.what());
        if (errorCounter == 15) {
          LOGGER.error(WebServException::FATAL_ERROR);
          return;
        }
      }
    }
  }

 public:
  void run() {

    // todo add Parser and config
    // todo 1. parse args
    try {
      // 2. create listenerFd
      createSocket();
      int yes = 1;
      setsockopt(listenerFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
      // 3. bind
      bindAddress();
      // 4. listen
      // check that port is listening:
      // netstat -a -n | grep LISTEN
      startListening();
      // 5. routine
      routine();
    } catch (const FatalWebServException &e) {
      LOGGER.error(e.what());
      exit(1);
    }
  }

 public:
  // constants to use
  static const int BUF_SIZE = 256;
  static const int TCP = 0;
  static const int PORT_DEFAULT;

};

const int WebServer::PORT_DEFAULT = 8080;
