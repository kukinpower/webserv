#pragma once
#include "Logger.h" //todo optimize includes
#include "Client.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <vector>
#include <iostream>
#include <netinet/in.h> //todo optimize
#include <sys/select.h>
#include <sys/fcntl.h>

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
  void createSocket() {
	listenerFd = socket(AF_INET, SOCK_STREAM, TCP);
	if (listenerFd == -1) {
	  LOGGER.error(BAD_SOCKET_FD);
	  //todo throw custom exception
	}
  }

  void bindAddress() const {
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT_DEFAULT);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (0 != bind(listenerFd, (struct sockaddr *) &addr, sizeof(addr))) {
	  LOGGER.error(BIND_ERROR);
	  //todo throw custom exception
	}
  }

  void startListening() {
	if (-1 == listen(listenerFd, 5)) {
	  LOGGER.error(LISTEN_ERROR);
	  //todo throw custom exception
	}
  }

  int acceptConnection(int maxFd) {
	struct sockaddr addr;
	socklen_t socklen;
	int newClientFd;

	// if no connection, accept is blocking process and start waiting
	if ((newClientFd = accept(listenerFd, (struct sockaddr *) &addr, &socklen)) == -1) {
	  LOGGER.error(ACCEPT_ERROR);
	  //todo throw custom exception
	}
	// set nonblock
	if (fcntl(newClientFd, F_SETFL, O_NONBLOCK) < 0) {
	  LOGGER.error(FCNTL_ERROR);
	  //todo throw custom exception
	}
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
	  LOGGER.error(SELECT_ERROR);
	  // todo throw custom exception
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
		  LOGGER.error(RECV_ERROR);
		}
		buf[nbytes] = 0;
		it->appendToRequest(buf);
		LOGGER.info("BUF: " + std::string(buf));
	  }
	}
  }

  void routine() {
	while (true) {
	  processSelect();
	  // for now it is just running and we can see that port 9000 is listening
	  // run this in shell to check:
	  // netstat -a -n | grep LISTEN
	  // todo 6. set non blocking
	}
  }

 public:
  void run() {
	// todo add Parser and config
	// todo 1. parse args
	// 2. create listenerFd
	createSocket();
	int yes = 1;
	setsockopt(listenerFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	// 3. bind
	bindAddress();
	// 4. listen
	startListening();
	// 5. routine
	routine();
  }

 public:
  // constants to use
  static const int BUF_SIZE = 256;
  static const int TCP = 0;
  static const int PORT_DEFAULT;

  // messages temporarily here todo move to custom exceptions
  static const char *BAD_SOCKET_FD;
  static const char *BIND_ERROR;
  static const char *LISTEN_ERROR;
  static const char *SELECT_ERROR;
  static const char *ACCEPT_ERROR;
  static const char *FCNTL_ERROR;
  static const char *RECV_ERROR;
};

const int WebServer::PORT_DEFAULT = 8080;

const char *WebServer::BAD_SOCKET_FD = "Something went wrong";
const char *WebServer::BIND_ERROR = "Bind error";
const char *WebServer::LISTEN_ERROR = "Listen error";
const char *WebServer::SELECT_ERROR = "Select error";
const char *WebServer::ACCEPT_ERROR = "Accept error";
const char *WebServer::FCNTL_ERROR = "Fcntl error";
const char *WebServer::RECV_ERROR = "Recv error";
