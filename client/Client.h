#pragma once
#include "Request.h"
#include "Response.h"
#include "Logger.h"
#include "ClientStatus.h"

#include "PollException.h"
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
#include <cstdlib>     /* atoi */

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
  static Logger LOGGER;
  std::vector<Request> requests;
  size_t cursorLength;
  ClientStatus clientStatus;

 public:
  Client(int fd) : fd(fd), clientStatus(READ) {}
  virtual ~Client() {}

  Client(const Client &request) {
    operator=(request);
  }
  Client &operator=(const Client &request) {
    this->fd = request.fd;
    this->requestBody = request.requestBody;
    this->requests = request.requests;
    this->clientStatus = request.clientStatus;
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
    return requestBody.find(REQUEST_END) != std::string::npos;
  }

 private:

  std::string getMainLine(const std::string &headerFull) {
	return headerFull.substr(0, headerFull.find("\r\n"));
  }
  
  void appendBodyToCurrentRequest() {
	Request &request = requests.back();

	unsigned long currentRequestBodyLength = request.getBody().length();
	size_t lengthFromHeaders = request.getLength();

	request.setBody(request.getBody() + requestBody.substr(0, lengthFromHeaders - currentRequestBodyLength));
	requestBody.erase(0, lengthFromHeaders - currentRequestBodyLength);

	if (request.getBody().length() == lengthFromHeaders) {
	  request.setStatus(READY);
	}
  }

  // todo use cursorLength ?
  void parseRequests() {
    size_t pos = 0;
    std::string headersToken;
    while ((pos = requestBody.find(REQUEST_END)) != std::string::npos) {
		const std::string &headerFull = requestBody.substr(0, pos);

		const std::string &mainLine = getMainLine(headerFull);
		HttpMethod method = Request::extractMethod(mainLine);
		const std::string &path = Request::extractPath(mainLine);

		Request::Headers requestHeaders = Request::extractHttpHeaders(headerFull, pos);
		pos += REQUEST_END_LENGTH;
		std::string body = "";

		// todo what if not full body is read?
		if (Request::isConnectionClose(requestHeaders)) {
		  body = requestBody.substr(pos);
		  requests.push_back(Request(method, path, requestHeaders, body));
		  return;
		}

		requestBody = requestBody.erase(0, pos);

		RequestStatus requestStatus = READY;

		if (Request::hasBodyLength(requestHeaders)) {
		  size_t length = Request::getLengthFromHeaders(requestHeaders);
		  body = requestBody.substr(0, length);
		  requestBody = requestBody.erase(0, length);
		  if (body.length() < length) {
		  	requestStatus = WAITING_BODY;
		  }
		}
		requests.push_back(Request(method, path, requestHeaders, body, requestStatus));
    }
  }

  void readRequestChunk() {
    long bytesRead;
    char buf[BUF_SIZE + 1];

    if ((bytesRead = recv(fd, buf, BUF_SIZE, 0)) == -1) {
      throw ReadException();
    }
    if (bytesRead == 0) {
      // todo if we are here, we hadn't read anything
      //  and no responses are pending
      close(fd);
      return;
    }
    buf[bytesRead] = 0;
    appendToRequest(buf);
    LOGGER.debug(std::string(buf));
  }

  bool hasRequestWaitingBody() const {
	return !requests.empty() && requests.back().getStatus() == WAITING_BODY;
  }

 public:
  bool isReadyToWrite() const {
	return requests.size() > 1 || (requests.size() == 1 && requests.front().getStatus() == READY);
  }

  void processReading() {
    readRequestChunk();
    if (hasRequestWaitingBody()) {
      appendBodyToCurrentRequest();
    }
    if (containsRequestEnd()) {
      parseRequests();
    }
  }

  std::vector<Request> &getRequests() {
    return requests;
  }

  ClientStatus getClientStatus() const {
    return clientStatus;
  }

  void setClientStatus(ClientStatus client_status) {
    clientStatus = client_status;
  }
};

Logger Client::LOGGER(Logger::DEBUG);
const char *Client::REQUEST_END = "\r\n\r\n";
