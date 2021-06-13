#pragma once
#include "Logger.h"
#include "Client.h"
#include "Location.h"

#include "SelectException.h"
#include "BadListenerFdException.h"
#include "NonBlockException.h"
#include "BindException.h"
#include "ListenException.h"
#include "AcceptException.h"
#include "ReadException.h"
#include "SendException.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <vector>
#include <iostream>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/fcntl.h>

#include <vector>
#include <string>

class Server {
 private:
  // tools
  Logger LOGGER;
  // constants
  static const int TCP = 0;
  static const int BACKLOG = 5;
  static const int BUF_SIZE = 256;
  // vars
  std::vector<Client> clients;
  int port;
  std::string hostName;
  std::string serverName;
  std::string errorPage;
  int maxBodySize;
  std::vector<Location> locations;
  int listenerFd;

 public:
  // todo?
  Server(int port = 8080,
         std::string hostName = "localhost",
         std::string serverName = "champions_server",
         std::string errorPage = "html/404.html",
         int maxBodySize = 100000000,
         std::vector<Location> locations = std::vector<Location>())
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
    this->clients = server.clients;
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

  int setFdSets(fd_set *readFds, fd_set *writeFds) {
    FD_ZERO(readFds);
    FD_ZERO(writeFds);
    FD_SET(listenerFd, readFds);

    int maxFd = listenerFd;
    for (std::vector<Client>::iterator it = clients.begin();
         it != clients.end(); ++it) {
      int fd = it->getFd();
      FD_SET(fd, readFds);
      if (it->isReadyToWrite()) {
        FD_SET(fd, writeFds);
      }
      maxFd = std::max(maxFd, fd);
    }

    return maxFd;
  }

  // todo maybe fd_set writeFds & timeout
  void processSelect() {
    fd_set readFds;
    fd_set writeFds;
    int maxFd = setFdSets(&readFds, &writeFds);

    if ((select(maxFd + 1, &readFds, &writeFds, NULL, NULL)) < 1) {
      LOGGER.error(WebServException::SELECT_ERROR);
      LOGGER.error("TUT ------------------------------------------------");
      std::cin >> maxFd;
      throw SelectException();
    }

	// new connection
    if (FD_ISSET(listenerFd, &readFds)) {
      try {
        maxFd = acceptConnection(maxFd);
		std::stringstream ss;
		ss << "Client with fd " << maxFd << " connected ";
		LOGGER.info(ss.str());
      } catch (const RuntimeWebServException &e) {
        LOGGER.error(e.what());
      }
    }

    for (std::vector<Client>::iterator client = clients.begin();
         client != clients.end(); ++client) {
      if (FD_ISSET(client->getFd(), &writeFds)) {
        for (std::vector<Request>::iterator request = client->getRequests().begin();
             request != client->getRequests().end();
             ++request) {
		  LOGGER.info("We will send now");
          std::string response = Response(*request).generateResponse();
          if (send(client->getFd(), response.c_str(), response.length(), 0) == -1) {
            std::stringstream ss;
            ss << WebServException::SEND_ERROR << " fd: " << client->getFd() << ", response: " << response;
			LOGGER.error(ss.str());
			throw SendException();
          }
          client->setStatus(READ); // todo maybe creates some bugs, find out how to check if needed closing
        }
      }
      if (FD_ISSET(client->getFd(), &readFds)) {
        try {
          client->processReading();
        } catch (const RuntimeWebServException &e) {
          LOGGER.error(e.what());
        }
      }
    }
  }

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

  int getSocketFd() const {
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

  std::vector<Location> getLocations() const {
    return this->locations;
  }
};
