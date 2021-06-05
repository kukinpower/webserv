#pragma once
#include "Request.h"
#include "Response.h"
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

class Client {
 private:
  int fd;
  std::string request;
  static const int BUF_SIZE = 256;
  Logger LOGGER;

 public:
  Client(int fd) : fd(fd) {}
  ~Client() {}
  //todo Coplien form

  int getFd() const {
	return fd;
  }

  void appendToRequest(char *chunk) {
    request += chunk;
  }

  std::string getRequest() const {
    return request;
  }

  void readRequestChunk() {
	long bytesRead;
	char buf[BUF_SIZE + 1];

	if ((bytesRead = recv(fd, buf, BUF_SIZE, 0)) == -1) {
	  throw ReadException();
	}
	buf[bytesRead] = 0;
	appendToRequest(buf);
	// check \r\n\r\n
	LOGGER.info(std::string(buf));
  }

  void sendResponse(const Response &response) {

  }

};
