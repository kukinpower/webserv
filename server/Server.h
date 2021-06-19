#pragma once
#include "Logger.h"
#include "Client.h"
#include "Location.h"
#include "StringBuilder.h"
#include "ServerStruct.h"

#include "PollException.h"
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
#include <poll.h>

#include <vector>
#include <string>

class Server {
 private:
  // tools
  static Logger LOGGER;
  // constants
  static const int TCP = 0;
  static const int BACKLOG = 5;
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
  void fillPollFds(struct pollfd *fds) {
    fds[0].fd = listenerFd;
    fds[0].events = POLLIN;

    int i = 1;
    int writeIndex = 0;
    for (std::vector<Client>::iterator it = clients.begin();
         it != clients.end(); ++it) {
      int fd = it->getFd();
      fds[i].fd = fd;
      fds[i].events |= POLLIN;
      if (it->isReadyToWrite()) {
        fds[i].fd = fd;
        fds[i].events |= POLLOUT;
      }
      ++i;
    }
  }

  void acceptConnectionPoll() {
    struct sockaddr addr;
    socklen_t socklen;
    int newClientFd;

    // if no connection, accept is blocking process and start waiting
    if ((newClientFd = accept(listenerFd, (struct sockaddr *) &addr, &socklen)) == -1) {
      throw AcceptException();
    }
    // set nonblock
    setNonBlock(newClientFd);

    Client newClient(newClientFd);
    clients.push_back(newClient);
  }

  void processPoll() {
    int currentFdsCount = clients.size() + 1;
    struct pollfd fds[currentFdsCount];
    memset(&fds, 0, sizeof(fds));

    fillPollFds(fds);

    int ret = poll(fds, currentFdsCount, 60000);
    if (ret == -1) {
      LOGGER.error(WebServException::POLL_ERROR);
      int i;
      std::cin >> i;
      throw PollException();
    } else if (ret == 0) {
      // timeout, no events
      // todo
      //  client->setClientStatus(CLOSED);
    } else {
      // new connection
      if (fds[0].revents & POLLIN) {
        try {
          acceptConnectionPoll();
          LOGGER.info("Client connected, fd: " + Logger::toString(clients.back().getFd()));
        } catch (const RuntimeWebServException &e) {
          LOGGER.error(e.what());
        }
      }
      int i = 1;
      for (std::vector<Client>::iterator client = clients.begin();
           client != clients.end() && i < currentFdsCount; ++client, ++i) {
        if (fds[i].revents & POLLOUT) {
          for (std::vector<Request>::iterator request = client->getRequests().begin();
               request != client->getRequests().end();) {
            LOGGER.info("Start sending to client: " + Logger::toString(client->getFd()));

            Response response(*request, createServerStruct());
            std::string responseBody = response.generateResponse();
            if (send(client->getFd(), responseBody.c_str(), responseBody.length(), 0) == -1) {
              LOGGER.error(StringBuilder()
                               .append(WebServException::SEND_ERROR)
                               .append(" fd: ")
                               .append(client->getFd())
                               .append(", response body: ")
                               .append(responseBody)
                               .toString());
              throw SendException();
            }
            if (response.getStatus() == INTERNAL_SERVER_ERROR) {
              client->setClientStatus(CLOSED);
              break;
            }
            request = client->getRequests().erase(request);
          }
        }

        if (client->getClientStatus() == CLOSED) {
          client = clients.erase(client);
        }

        if (fds[i].revents & POLLIN) {
          try {
            client->processReading();
          } catch (const RuntimeWebServException &e) {
            LOGGER.error(e.what());
            std::cin >> i;
          }
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

 private:
  ServerStruct createServerStruct() const {
    return ServerStruct(port, hostName, serverName, maxBodySize, locations);
  }
};

Logger Server::LOGGER(Logger::DEBUG);
