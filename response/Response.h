#pragma once
#include "Status.h"
#include "CgiHandler.h"
#include "Location.hpp"

#include <map>

class Response {
 private:
  long length; //
  std::string body; //
  int fd;
  Request::Headers headers;
  Request request;
  Location location;

  typedef std::map<std::string, std::string>::iterator iterator;

 public:
  std::pair<iterator, bool> putHeader(const std::string &key, const std::string &value) {
    return headers.insert(std::make_pair(key, value));
  }

  Response(const Request &request, const Location &location) : request(request), location(location) {

  }

  std::string generateResponse() {
    //todo generate with headers, not hardcode
    if (location.getCgiPath() != "" && location.getCgiExt().size()) {
      CgiHandler cgi(request, location);
      try {
        body = cgi.runScript(location.getCgiPath());
      }
      catch (const FatalWebServException &e) {
        std::cerr << e.what() << std::endl;
      }
    } else {
      //depending on method, get file, post file, delete file
    }
    std::stringstream ss;
    ss << "HTTP/1.1 200 OK\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Champions\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nContent-Length: ";
    ss << request.getBody().length() << "\r\n";
    ss << "Content-Type: text/html\r\n\r\n" << body;
    return ss.str();
  }
};
