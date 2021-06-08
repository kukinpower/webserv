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

#include <vector>

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
  int maxBodySize;
  // std::vector<Location> locations;
  int listenerFd;
  int port;

 public:
  Server(int port, int maxBodySize = 100000000)
  : port(port), maxBodySize(maxBodySize), listenerFd(-1) {
  }

  virtual ~Server() {}

  Server(const Server &server) {
    operator=(server);
  }
  Server &operator=(const Server &server) {
    this->clients = server.clients;
    this->maxBodySize = server.maxBodySize;
    this->listenerFd = server.listenerFd;
    this->port = server.port;
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

  int setFdSets(fd_set *readfds, fd_set *writefds) {
	FD_ZERO(readfds);
	FD_ZERO(writefds);
	FD_SET(listenerFd, readfds);

	int maxFd = listenerFd;
	for (std::vector<Client>::iterator it = clients.begin();
		 it != clients.end(); ++it) {
	  int fd = it->getFd();
	  FD_SET(fd, readfds);
	  if (it->getStatus() == WRITE) {
	    FD_SET(fd, writefds);
	  }
	  maxFd = std::max(maxFd, fd);
	}

	return maxFd;
  }

  // todo maybe fd_set writefds & timeout
  void processSelect() {
	fd_set readfds;
	fd_set writefds;
	int maxFd = setFdSets(&readfds, &writefds);

	int selectedFdsCount;
	if ((selectedFdsCount = select(maxFd + 1, &readfds, &writefds, NULL, NULL)) < 1) {
	  LOGGER.error(WebServException::SELECT_ERROR);
	  throw SelectException();
	}

	if (FD_ISSET(listenerFd, &readfds)) { // new connection
	  try {
		maxFd = acceptConnection(maxFd);
	  } catch (const RuntimeWebServException &e) {
		LOGGER.error(e.what());
	  }
	}

	for (std::vector<Client>::iterator client = clients.begin();
		 client != clients.end(); ++client) {
	  if (FD_ISSET(client->getFd(), &writefds)) {
	    for (std::vector<Request>::iterator request = client->getRequests().begin();
             request != client->getRequests().end();
             ++request) {
	      Response response(*request);
          if (send(client->getFd(), response.generateResponse().c_str(), response.generateResponse().length(), 0) == -1) {
            LOGGER.error("Bad send");
            //todo throw custom exception
          }
          client->setStatus(READ); // todo maybe creates some bugs, find out how to check if needed closing
	    }
	  }
	  if (FD_ISSET(client->getFd(), &readfds)) {
		try {
		  client->processReading();
		  // callCgi()
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
};
