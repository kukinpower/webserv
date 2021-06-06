#pragma once
#include "Request.h"
#include "Response.h"
#include "Logger.h"
#include "ClientStatus.h"

#include "SelectException.h"
#include "BadListenerFdException.h"
#include "NonBlockException.h"
#include "BindException.h"
#include "ListenException.h"
#include "AcceptException.h"
#include "ReadException.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>     /* atoi */

#include <vector>
#include <iostream>

class Client {
 private:
  static const int BUF_SIZE = 256;
  static const int REQUEST_END_LENGTH = 4;
  static const char *REQUEST_END;
 private:
  int fd;
  std::string requestBody;
  Logger LOGGER;
  std::vector<Request> requests;
  ClientStatus status;

 public:
  Client(int fd) : fd(fd), status(READ) {}
  virtual ~Client() {}

  Client(const Client &request) {
    operator=(request);
  }
  Client &operator=(const Client &request) {
    this->fd = request.fd;
    this->requestBody = request.requestBody;
    this->requests = request.requests;
    this->status = request.status;
    return *this;
  }

 public:
  int getFd() const {
    return fd;
  }

  void appendToRequest(char *chunk) {
    requestBody += chunk;
  }

  std::string getRequest() const {
    return requestBody;
  }

  bool containsRequestEnd() {
    return requestBody.find(REQUEST_END) != requestBody.npos;
  }

 private:

  void parseRequests() {
    size_t pos = 0;
    std::string headersToken;
    while ((pos = requestBody.find(REQUEST_END)) != std::string::npos) {
      const std::string &headerFull = requestBody.substr(0, pos);
      size_t headersStart = headerFull.find("\r\n") + 2;
      const std::string &mainLine = headerFull.substr(0, headerFull.find("\r\n"));

      Method method = Request::extractMethod(mainLine);
      const std::string &path = Request::extractPath(mainLine);
      Request::Headers requestHeaders = Request::extractHttpHeaders(requestBody.substr(headersStart, pos - headersStart));
      pos += REQUEST_END_LENGTH;
      std::string body = "";

      if (Request::hasConnectionClose(requestHeaders)) {
        body = requestBody.substr(pos);
        requests.push_back(Request(method, path, requestHeaders, body));
        status = CLOSED;
        return;
      }

      if (Request::hasBodyLength(requestHeaders)) {
        size_t length = Request::getLengthFromHeaders(requestHeaders);
        body = Request::extractHttpBodyByLength(requestBody, pos, length);
      } else {
        requestBody.erase(0, pos);
      }
      requests.push_back(Request(method, path, requestHeaders, body));


//      requests.push_back(Request::parseRequest(headersToken));

    }
  }

  void readRequestChunk() {
    long bytesRead;
    char buf[BUF_SIZE + 1];

    if ((bytesRead = recv(fd, buf, BUF_SIZE, 0)) == -1) {
      throw ReadException();
    }
    if (bytesRead == 0) {
      close(fd);
      status = CLOSED;
      return;
    }
    buf[bytesRead] = 0;
    appendToRequest(buf);
    // check \r\n\r\n
    LOGGER.info(std::string(buf));
  }

 public:
  bool processReading() {
    readRequestChunk();
    bool hasNewRequests = false;
    if (containsRequestEnd()) {
      hasNewRequests = true;
//      parseRequests();
    }
    return hasNewRequests;
  }



//  Request processRequest() {
//
//  }

  void sendResponse(const Response &response) {

  }

};

const char *Client::REQUEST_END = "\r\n\r\n";
