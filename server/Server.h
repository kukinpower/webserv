#pragma once
#include "Logger.h"
#include "Client.h"
#include "Location.h"
#include "StringBuilder.h"

#include "PollException.h"
#include "BadListenerFdException.h"
#include "NonBlockException.h"
#include "BindException.h"
#include "ListenException.h"
#include "AcceptException.h"
#include "ReadException.h"
#include "SendException.h"
#include "NoSuchClientException.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <vector>
#include <iostream>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/fcntl.h>
#include <poll.h>

#include <vector>
#include <string>

class Server {
 public:
  // tools
  static Logger LOGGER;
  // constants
  static const int TCP = 0;
  static const int BACKLOG = 128;
  // vars
  int port;
  std::string hostName;
  std::string serverName;
  std::string errorPage;
  int maxBodySize;
  std::vector<Location> locations;
  int listenerFd;

 public:
  Server(int port = 8080,
         const std::string &hostName = "localhost",
         const std::string &serverName = "champions_server",
         const std::string &errorPage = "html/404.html",
         int maxBodySize = 100000000,
         const std::vector<Location> &locations = std::vector<Location>())
      :
      port(port),
      hostName(hostName),
      serverName(serverName),
      errorPage(errorPage),
      maxBodySize(maxBodySize),
      locations(locations),
      listenerFd(-1) {

    if (locations.empty()) {
      Location loc = Location(1);
      this->locations.push_back(loc);
    }
  }

  virtual ~Server() {
  }

  Server(const Server &server) {
    operator=(server);
  }

  Server &operator=(const Server &server) {
    this->maxBodySize = server.maxBodySize;
    this->listenerFd = server.listenerFd;
    this->port = server.port;
    this->hostName = server.hostName;
    this->serverName = server.serverName;
    this->errorPage = server.errorPage;
    this->locations = server.locations;
    return *this;
  }

 private:
  static void setNonBlock(int fd) {
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
      throw NonBlockException(StringBuilder()
                                  .append(WebServException::FCNTL_ERROR)
                                  .append(" on fd: ")
                                  .append(fd)
                                  .toString());
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
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (0 != bind(listenerFd, (struct sockaddr *) &addr, sizeof(addr))) {
      throw BindException();
    }
  }

  void startListening() {
    if (-1 == listen(listenerFd, BACKLOG)) {
      throw ListenException();
    }
  }

 public:
  void run() {
    // 1. create listenerFd
    createSocket();
    // 2. make port not busy for the next use
    int YES = 1;
    setsockopt(listenerFd, SOL_SOCKET, SO_REUSEADDR, &YES, sizeof(int));
    // 3. bind
    bindAddress();
    // 4. listen
    // check that port is listening:
    // netstat -a -n | grep LISTEN
    startListening();
  }

  int getListenerFd() const {
    return listenerFd;
  }

  int getPort() const {
    return this->port;
  }

  std::string getHostName() const {
    return this->hostName;
  }

  std::string getServerName() const {
    return this->serverName;
  }

  std::string getErrorPage() const {
    return this->errorPage;
  }

  long int getBodySize() const {
    return this->maxBodySize;
  }

  std::vector<Location> &getLocations() {
    return this->locations;
  }
};

Logger Server::LOGGER(Logger::DEBUG);
