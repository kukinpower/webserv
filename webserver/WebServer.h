#pragma once
#include "Logger.h"
#include "Server.h"
#include "ConfigReader.h"

#include "PollException.h"
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
 public:
  // constants to use
  static const int BUF_SIZE = 256;
  static const int PORT_DEFAULT = 8080;
  static const int SERVER_TIMEOUT = 22000;
  static const int SEND_CHUNK_SIZE = 256;

 private:
  static Logger LOGGER;
  std::vector<Server> servers;

 public:
  WebServer() {}
  virtual ~WebServer() {}

  WebServer(const WebServer &server) {
    operator=(server);
  }

  WebServer &operator=(const WebServer &server) {
    this->servers = server.servers;
    return *this;
  }

 private:

  void eraseServer(Server &server) {
    for (std::vector<Server>::iterator serverIt = servers.begin(); serverIt != servers.end(); ++serverIt) {
      if (serverIt->getListenerFd() == server.getListenerFd()) {
        servers.erase(serverIt);
        break;
      }
    }
  }

  void fillPollFds(struct pollfd *fds,
                   const std::map<int, Server *> &listenerFdsMap,
                   const std::map<int, Client *> &clientFdsMap) {
    int j = 0;
    for (std::map<int, Server *>::const_iterator fd = listenerFdsMap.begin(); fd != listenerFdsMap.end(); ++fd, ++j) {
      fds[j].fd = fd->first;
      fds[j].events |= POLLIN;
    }

    for (std::map<int, Client *>::const_iterator fd = clientFdsMap.begin(); fd != clientFdsMap.end(); ++fd, ++j) {
      fds[j].fd = fd->first;
      fds[j].events |= POLLIN;
      if (fd->second->isReadyToWrite()) {
        fds[j].fd = fd->first;
        fds[j].events |= POLLOUT;
      }
    }
  }

  void routine() {
    while (true) {
      try {
        std::map<Client *, Server *> clientsToServersMap;
        std::map<int, Client *> clientFdsMap;
        std::map<int, Server *> listenerFdsMap;
        for (std::vector<Server>::iterator server = servers.begin(); server != servers.end(); ++server) {
          std::vector<Client> &clients = server->getAllClients();
          for (std::vector<Client>::iterator client = clients.begin(); client != clients.end(); ++client) {
            clientsToServersMap.insert(std::make_pair(&(*client), &(*server)));
            clientFdsMap.insert(std::make_pair(client->getFd(), &(*client)));
          }
          listenerFdsMap.insert(std::make_pair(server->getListenerFd(), &(*server)));
        }

        size_t currentFdsCount = clientFdsMap.size() + listenerFdsMap.size();
        struct pollfd fds[currentFdsCount];
        memset(&fds, 0, sizeof(fds));

        fillPollFds(fds, listenerFdsMap, clientFdsMap);

        int ret = poll(fds, currentFdsCount, SERVER_TIMEOUT);
        if (ret == -1) {
          LOGGER.error(WebServException::POLL_ERROR);
          int i;
          std::cin >> i;
          throw PollException();
        } else if (ret == 0) {
          for (std::vector<Server>::iterator server = servers.begin(); server != servers.end(); ++server) {
            server->clearClients();
          }
          LOGGER.info("Timeout reached. Close all connections");
        } else {
          for (int i = 0; i < currentFdsCount; ++i) {
            if (clientFdsMap.find(fds[i].fd) != clientFdsMap.end()) {
              Client *client = clientFdsMap[fds[i].fd];

              if (fds[i].revents & POLLOUT) {
                for (std::vector<Request>::iterator request = client->getRequests().begin();
                     request != client->getRequests().end();) {
                  LOGGER.info("Start sending to client: " + Logger::toString(client->getFd()));

                  Response response(*request, clientsToServersMap[client]->createServerStruct());

                  std::string responseBody = response.generateResponse();
                  if (responseBody.length() > SEND_CHUNK_SIZE) {
                    int bytesWritten = 0;
                    while (bytesWritten < responseBody.length()) {
                      size_t sendSize;
                      if (responseBody.length() - bytesWritten < SEND_CHUNK_SIZE) {
                        sendSize = responseBody.length() - bytesWritten;
                      } else {
                        sendSize = SEND_CHUNK_SIZE;
                      }
                      if (send(client->getFd(), responseBody.c_str() + bytesWritten, sendSize, 0) == -1) {
                        std::cout << errno << std::endl; //todo remove
                        LOGGER.error(StringBuilder()
                                         .append(WebServException::SEND_ERROR)
                                         .append(" fd: ")
                                         .append(client->getFd())
                                         .toString());
                        throw SendException();
                      }
                      bytesWritten += sendSize;
                    }
                  } else {
                    if (send(client->getFd(), responseBody.c_str(), responseBody.length(), 0) == -1) {
                      std::cout << errno << std::endl;
                      LOGGER.error(StringBuilder()
                                       .append(WebServException::SEND_ERROR)
                                       .append(" fd: ")
                                       .append(client->getFd())
                                       .toString());
                      throw SendException();
                    }
                  }
                  if (response.getStatus() == INTERNAL_SERVER_ERROR) {
                    client->setClientStatus(CLOSED);
                    break;
                  }
                  request = client->getRequests().erase(request);
                }
              }
              if (client->getClientStatus() == CLOSED) {
                clientsToServersMap[client]->eraseClient(*client);
              }
              if (fds[i].revents & POLLIN) {
                try {
                  client->processReading();
                } catch (const RuntimeWebServException &e) {
                  LOGGER.error(e.what());
                }
              }
            }
              // new connection
            else if (fds[i].revents & POLLIN && listenerFdsMap.find(fds[i].fd) != listenerFdsMap.end()) {
              Server *server = listenerFdsMap[fds[i].fd];
              try {
                server->acceptConnectionPoll();
                LOGGER.info("Client connected, fd: " + Logger::toString(server->getAllClients().back().getFd()));
              } catch (const RuntimeWebServException &e) {
                LOGGER.error(e.what());
              } catch (const FatalWebServException &e) {
                eraseServer(*server);
              }
            }
          }
        }
      } catch (const RuntimeWebServException &e) {
        std::cout << errno << std::endl;
        LOGGER.error(e.what());
        continue;
      }
    }
  }

 public:
  void parseConfig(int ac, char *av[]) {
    if (ac == 1) {
      ConfigReader conf;
      conf.printData();
      this->servers = conf.getServers();
    } else {
      ConfigReader conf(av[1]);
      conf.readConfig();
      conf.printData();
      this->servers = conf.getServers();
    }
  }

  void run() {
    std::vector<Server>::iterator server = servers.begin();
    while (server != servers.end()) {
      try {
        server->run();
        ++server;
      } catch (const FatalWebServException &e) {
        LOGGER.error(e.what());
        server = servers.erase(server);
      }
    }

    routine();
  }
};

Logger WebServer::LOGGER(Logger::DEBUG);
