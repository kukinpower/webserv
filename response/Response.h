#pragma once
#include "Status.h"
#include "Location.h"

#include <map>
#include <FatalWebServException.h>

class Response {
 private:
  long length;
  int fd;
  Request::Headers headers;
  Request request;
  const std::vector<Location> &locations;

  typedef std::map<std::string, std::string>::iterator iterator;

 public:
  std::pair<iterator, bool> putHeader(const std::string &key, const std::string &value) {
	return headers.insert(std::make_pair(key, value));
  }

  Response(const Request &request, const std::vector<Location> &locations) : request(request), locations(locations) {
  }

  std::string generateResponse() {
	//todo generate with headers, not hardcode
	// validation
	// status codes
	// inject server?

	std::vector<Location>::const_iterator location = locations.begin();
//	if (location.matches(request.getPath())) {
//
//	}


//
//	if (location.getCgiPath() != "" && location.getCgiExt().size()) {
//	  CgiHandler cgi(request, location);
//	  try {
//		body = cgi.runScript(location.getCgiPath());
//	  }
//	  catch (const FatalWebServException &e) {
//		std::cerr << e.what() << std::endl;
//	  }
//	} else {
//	  //depending on method, get file, post file, delete file
//	}
	std::stringstream ss;
	ss
		<< "HTTP/1.1 400 OK\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Champions\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nContent-Length: ";
	ss << request.getBody().length() << "\r\n";
	ss << "Content-Type: text/html\r\n\r\n" << request.getBody().c_str();
	return ss.str();
  }
};
