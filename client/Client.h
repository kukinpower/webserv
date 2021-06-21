#pragma once
#include "Logger.h"
#include "ClientStatus.h"
#include "HttpMethod.h"

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
#include <cstring>

#include <vector>
#include <iostream>

class Client {
 public:
  int fd;
  std::string fullRequestBody;
  static Logger LOGGER;
  int length; // 0 | >0
  HttpMethod method;
  std::string path;
  std::string body;
  ClientStatus clientStatus;
  bool containsRequestEnd;

  // split headers and body -----------------------------------
  std::size_t REQUEST_END_LENGTH;
  std::string REQUEST_END;
  const char *REQUEST_END_CONST_CHAR;

  // split request --------------------------------------------
  std::string HEADER_DELIMETER;
  std::size_t HEADER_DELIMETER_LENGTH;

  // split headers --------------------------------------------
  std::string HEADER_PAIR_DELIMETER;
  std::size_t HEADER_PAIR_DELIMETER_LENGTH;

 public:
  void clearInfo() {
    length = 0;
    method = UNKNOWN_METHOD;
    path.clear();
    body.clear();
    clientStatus = READ;
    containsRequestEnd = false;
  }

 public:
  Client(int fd) : fd(fd), length(0), method(UNKNOWN_METHOD), clientStatus(READ), containsRequestEnd(false),
                   REQUEST_END_LENGTH(4), REQUEST_END("\r\n\r\n"), REQUEST_END_CONST_CHAR("\r\n\r\n"),
                   HEADER_DELIMETER("\r\n"), HEADER_DELIMETER_LENGTH(2),
                   HEADER_PAIR_DELIMETER(": "), HEADER_PAIR_DELIMETER_LENGTH(2) {
  }

  virtual ~Client() {
  }

 public:
  int getFd() const {
    return fd;
  }

  bool isContainsRequestEnd() {
    return containsRequestEnd;
  }

 public:
  void appendToRequestBody(char *buf) {
    fullRequestBody.append(buf);

    char *requestEndFound = NULL;
    requestEndFound = strstr(buf, REQUEST_END_CONST_CHAR);
    if (requestEndFound) {
      containsRequestEnd = true;
    }
  }

  void appendToBody(char *buf) {
    body.append(buf);
    if (length < body.length()) {
      return closeClient();
    }
    if (length == body.length()) {
      clientStatus = WRITE;
    }
  }

  HttpMethod extractMethod(const std::string &line) {
    if ("GET" == line) {
      return GET;
    }
    if ("POST" == line) {
      return POST;
    }
    if ("DELETE" == line) {
      return DELETE;
    }
    return UNKNOWN_METHOD;
  }

  void closeClient() {
    close(fd);
    clientStatus = CLOSED;
  }

  void parseRequest() {
    // split headers and body ------------------------------------------------------------------------------------
    std::size_t start = 0U;
    std::size_t end = fullRequestBody.find(REQUEST_END);

    std::string head = fullRequestBody.substr(start, end);
    body = fullRequestBody.substr(end + REQUEST_END_LENGTH);

    // split request ------------------------------------------------------------------------------------
    std::vector<std::string> lines;

    start = 0U;
    end = head.find(HEADER_DELIMETER);
    while (end != std::string::npos) {
      lines.push_back(head.substr(start, end - start));
      start = end + HEADER_DELIMETER_LENGTH;
      end = head.find(HEADER_DELIMETER, start);
    }

    lines.push_back(head.substr(start, end));

    // split main line ------------------------------------------------------------------------------------
    start = 0U;
    end = lines.front().find(' ');
    if (end == std::string::npos) {
      return closeClient();
    }
    // HTTP METHOD
    method = extractMethod(lines.front().substr(start, end - start));
    start = end + 1;
    end = fullRequestBody.find(' ', start);
    // PATH
    path = lines.front().substr(start, end - start);

    if (method == UNKNOWN_METHOD) {
      return closeClient();
    }

    // split headers --------------------------------------------
    size_t pos;
    for (int i = 0; i < lines.size(); ++i) {
      if ((pos = lines[i].find("Content-Length", 0)) != std::string::npos) {
        length = std::atoi(lines[i].substr(pos + 16, lines[i].length()).c_str());
      }
    }

    if (length == 0 || length == body.length()) {
      clientStatus = WRITE;
      return;
    }

    if (length > body.length()) {
      clientStatus = WAITING_BODY;
      return;
    }

    if (length < body.length()) {
      return closeClient();
    }
  }

  ClientStatus getClientStatus() const {
    return clientStatus;
  }
};

Logger Client::LOGGER(Logger::DEBUG);
