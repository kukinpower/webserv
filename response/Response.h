#pragma once
#include "HttpStatus.h"
#include "Location.h"
#include "Logger.h"
#include "StringBuilder.h"
#include "HttpStatusWrapper.h"

#include "FatalWebServException.h"

#include <map>
#include <fstream>

class Response {
 private:
  typedef std::map<HttpStatus, HttpStatusWrapper> HttpStatuses;
  static const HttpStatuses STATUSES;

  long length;
  int fd;
  Request::Headers headers;
  Request request;
  const std::vector<Location> &locations;

  std::string responseBody;
  HttpStatus status;
  std::vector<Location>::const_iterator requestLocation;

  typedef std::map<std::string, std::string>::iterator iterator;

 public:
  std::pair<iterator, bool> putHeader(const std::string &key, const std::string &value) {
	return headers.insert(std::make_pair(key, value));
  }

  Response(const Request &request, const std::vector<Location> &locations) : request(request), locations(locations) {
  }

 private:

  std::string joinStrings(const std::vector<std::string> &v, const std::string &sequence) {
    std::stringstream ss;
    for (int i = 0; i < v.size(); ++i) {
      if (i + 1 < v.size()) {
        ss << v[i] << "\r\n";
      } else {
        ss << v[i];
      }
    }
    return ss.str();
  }

  static bool isErrorStatus(HttpStatus status) {
    return status == BAD_REQUEST || status == NOT_FOUND || status == INTERNAL_SERVER_ERROR;
  }

  std::string generateHeaders() {
    std::vector<std::string> responseHeaders;
    // insert front main line: HTTP/1.1 code NAME
    responseHeaders.insert(responseHeaders.begin(), STATUSES.find(status)->second.createMainLine());


    if (!isErrorStatus(status)) {
      headers.insert(std::make_pair("Content-Length", Logger::toString(responseBody.length())));
    }

    return joinStrings(responseHeaders, "\r\n");
  }

  std::string createResponseBody() {
    return " kek ";
  }



  void doGet() {
    const Request::Headers &requestHeaders = request.getHeaders();

//    std::ifstream fileStream(request.getPath());
//    std::string path = requestLocation->substitutePath(request.getPath());


  }

  void doPost() {
    const Request::Headers &requestHeaders = request.getHeaders();

    if (!Request::isConnectionClose(requestHeaders)) {

    }
  }

  void doDelete() {

  }

  static std::vector<std::string> convertHeadersToStringVector(const Request::Headers &headersToConvert) {
    std::vector<std::string> vector;
    for (Request::Headers::const_iterator it = headersToConvert.begin(); it != headersToConvert.end(); ++it) {
      vector.push_back(it->first + ": " + it->second);
    }
    return vector;
  }

 public:
  std::string generateResponse() {
	//todo generate with headers, not hardcode
	// validation
	// status codes
	// inject server?

	for (std::vector<Location>::const_iterator location = locations.begin();
		  location != locations.end();
		  ++location) {

	  // todo
	  //  1. matches
	  //  2. method allowed?
	  //  3. default
	  if (location->matches(request.getPath()) && location->isMethodAllowed(request.getMethod())) {
        requestLocation = location;
	    if (request.getMethod() == GET) {
	      doGet();
	    } else if (request.getMethod() == POST) {
	      doPost();
	    } else if (request.getMethod() == DELETE) {
	      doDelete();
	    } else {
          throw RuntimeWebServException("Can't handle method like this: " + Logger::toString(request.getMethod()));
	    }
	  }
	}
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
	ss << "Content-Type: text/html\r\n\r\n" << request.getBody();
	return ss.str();
  }

 private:
  static HttpStatuses initHttpStatuses() {
    HttpStatuses statuses;
    statuses.insert(std::make_pair(OK, HttpStatusWrapper(OK, "OK")));
    statuses.insert(std::make_pair(CREATED, HttpStatusWrapper(CREATED, "Created")));
    statuses.insert(std::make_pair(NO_CONTENT, HttpStatusWrapper(NO_CONTENT, "No Content")));
    statuses.insert(std::make_pair(NOT_FOUND, HttpStatusWrapper(NOT_FOUND, "Not Found")));
    statuses.insert(std::make_pair(BAD_REQUEST, HttpStatusWrapper(BAD_REQUEST, "Bad Request")));
    statuses.insert(std::make_pair(MOVED_PERMANENTLY, HttpStatusWrapper(MOVED_PERMANENTLY, "Moved Permanently")));
    statuses.insert(std::make_pair(INTERNAL_SERVER_ERROR, HttpStatusWrapper(INTERNAL_SERVER_ERROR, "Internal Server Error")));

    return statuses;
  }
};

const Response::HttpStatuses Response::STATUSES = initHttpStatuses();
