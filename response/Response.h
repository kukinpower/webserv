#pragma once
#include "HttpStatus.h"
#include "Location.h"
#include "Logger.h"
#include "StringBuilder.h"
#include "HttpStatusWrapper.h"
#include "Request.h"
#include "ServerStruct.h"

#include "FatalWebServException.h"
#include "FileNotFoundException.h"
#include "MethodNotAllowed.h"
#include "ExtensionNotSupported.h"
#include "CgiParamsNotSpecified.h"
#include "CgiHandler.h"

#include <map>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

class Server;

class Response {
 public:
  typedef std::map<HttpStatus, HttpStatusWrapper> HttpStatuses;
  typedef std::map<std::string, std::string> MimeTypes;
  typedef std::map<std::string, std::string> Headers;

 private:
  static const HttpStatuses STATUSES;
  static const MimeTypes MIME;

  static Logger LOGGER;

  long length;
  int fd;
  Headers headers;
  Request request;
  const ServerStruct &serverStruct;

  std::string responseBody;
  HttpStatus responseStatus;
  std::vector<Location>::const_iterator requestLocation;

  typedef std::map<std::string, std::string>::iterator iterator;

 public:
  std::pair<iterator, bool> putHeader(const std::string &key, const std::string &value) {
    return headers.insert(std::make_pair(key, value));
  }

  Response(const Request &request, const ServerStruct &server_struct)
      : request(request), serverStruct(server_struct) {}

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

  static bool isDirectory(const char *path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
  }

  static bool isErrorStatus(HttpStatus status) {
    return status == BAD_REQUEST || status == NOT_FOUND || status == INTERNAL_SERVER_ERROR;
  }

  static std::string getContentType(const std::string &path) {
    unsigned long pos;
    if ((pos = path.rfind(".") != std::string::npos)) {
      std::string mimeType = path.substr(pos);
      MimeTypes::const_iterator it;
      if ((it = MIME.find(mimeType)) != MIME.end()) {
        return it->second;
      }
    }
    return MIME.find(".html")->second;
  }

  std::string generateHeaders() {
    std::vector<std::string> responseHeaders;

    if (!isErrorStatus(responseStatus)) {
      headers.insert(std::make_pair("Content-Length", Logger::toString(responseBody.length())));
    }

    //todo server name
    headers.insert(std::make_pair("Server", serverStruct.getServerName()));
    headers.insert(std::make_pair("Content-Type", getContentType(request.getPath())));

    responseHeaders = convertHeadersToStringVector(headers);

    // insert front main line: HTTP/1.1 code NAME
    responseHeaders.insert(responseHeaders.begin(), STATUSES.find(responseStatus)->second.createMainLine());

    return joinStrings(responseHeaders, "\r\n");
  }

  std::string generateAutoIndex(const std::string &path) {
    DIR *dir = opendir(path.c_str());
    if (dir != NULL) {
      std::stringstream ss;

      ss << "<!doctype html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>" << path << "</title></head><body>";

      struct dirent *ent;
      while ((ent = readdir(dir)) != NULL) {
        std::string fileName = (ent->d_name);
        if (!(fileName == "." || fileName == "..")) {
          ss << "<li><a href=\"" << ServerStruct::getServerAddress(serverStruct) << request.getPath();
          if (request.getPath()[request.getPath().length() - 1] != '/') {
            ss << "/";
          }
          ss << ent->d_name << "\">" << ent->d_name << "</a></li>" << std::endl;
        }
      }
      closedir(dir);

      ss << "</body></html>";

      return ss.str();
    }

    throw RuntimeWebServException("Couldn't open directory by path: " + path);
  }

  void doGet() {
    const Headers &requestHeaders = request.getHeaders();

    std::string path = requestLocation->substitutePath(request.getPath());
    std::ifstream fileStream(path);
    if (fileStream.fail()) {
      throw FileNotFoundException(Logger::toString(WebServException::FILE_NOT_FOUND) + " '" + path + "'");
    }

    if (!isDirectory(path.c_str())) {
      responseBody = getDocumentContent(fileStream);
    } else {
      if (requestLocation->isAutoIndex()) {
        responseBody = generateAutoIndex(path);
      } else {
        std::ifstream indexFileStream(path + "index.html"); //нужно разные страницы загружать (см. в конфиге index)
        if (!indexFileStream.fail()) {
          responseBody = getDocumentContent(indexFileStream);
        } else {
          responseBody = "";
        }
      }
    }
    responseStatus = OK;
  }

  static std::string getDocumentContent(std::ifstream &fileStream) {
    std::istreambuf_iterator<char> eos;
    return std::string(std::istreambuf_iterator<char>(fileStream), eos);
  }

  void doPost() {
    const Headers &requestHeaders = request.getHeaders();

    if (!Request::isConnectionClose(requestHeaders)) {
      std::string path = requestLocation->substitutePath(request.getPath());
      std::string interpreter = requestLocation->getFullCgiPath(requestLocation->getCgiPath());
      std::ifstream fileStream(path);
      if (fileStream.fail()) {
        throw FileNotFoundException(Logger::toString(WebServException::FILE_NOT_FOUND) + " '" + path + "'"); //404
      }
      if (!isDirectory(path.c_str())) {
        if (!requestLocation->isMethodAllowed(POST))
          throw MethodNotAllowed(Logger::toString(WebServException::METHOD_NOT_ALLOWED) + " '" + path + "'"); //405
        if (requestLocation->getCgiPath().empty() || requestLocation->getCgiExt().empty())  //validate path?
          throw CgiParamsNotSpecified(Logger::toString(WebServException::CGI_PARAMS_NOT_SPECIFIED) + " '" + path + "'");
        std::string queryString = extractQueryString(path);
        std::string extension = findExtension(path);
        if (find(requestLocation->getCgiExt().begin(), requestLocation->getCgiExt().end(), extension) == requestLocation->getCgiExt().end())
          throw ExtensionNotSupported(Logger::toString(WebServException::EXTENSION_NOT_SUPPORTED) + " '" + path + "'");
        CgiHandler cgi(request, serverStruct, queryString, path, interpreter);
        responseBody = cgi.runScript(path, interpreter, responseStatus);
      } else {
        if (requestLocation->isAutoIndex()) {
          responseBody = generateAutoIndex(path);
        } else {
          std::ifstream indexFileStream(path + "index.html"); //нужно разные страницы загружать (см. в конфиге index)
          if (!indexFileStream.fail()) {
            responseBody = getDocumentContent(indexFileStream);
          } else {
            responseBody = "";
          }
        }
      }
    }
  }

  void doDelete() {

  }

 public:
  std::string generateResponse() {
    try {
      for (std::vector<Location>::const_iterator location = serverStruct.locations.begin();
           location != serverStruct.locations.end();
           ++location) {

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
          break;
        }
      }
    } catch (const FileNotFoundException &e) {
      LOGGER.debug(e.what());
      responseStatus = NOT_FOUND;
    } catch (const RuntimeWebServException &e) {
      responseStatus = INTERNAL_SERVER_ERROR;
    }

    return StringBuilder().append(generateHeaders()).append("\r\n\r\n").append(responseBody).toString();
  }

  HttpStatus getStatus() const {
    return responseStatus;
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
    statuses.insert(std::make_pair(INTERNAL_SERVER_ERROR,
                                   HttpStatusWrapper(INTERNAL_SERVER_ERROR, "Internal Server Error")));

    return statuses;
  }

  static Headers initMimeTypes() {
    Headers mime;
    mime.insert(std::make_pair(".htm", "text/html"));
    mime.insert(std::make_pair(".html", "text/html"));
    mime.insert(std::make_pair(".jpg", "image/jpeg"));
    mime.insert(std::make_pair(".jpeg", "image/jpeg"));
    mime.insert(std::make_pair(".js", "text/javascript"));
    mime.insert(std::make_pair(".txt", "text/plain"));
    mime.insert(std::make_pair(".sh", "application/x-sh"));
    return mime;
  }

  static std::vector<std::string> convertHeadersToStringVector(const Headers &headersToConvert) {
    std::vector<std::string> vector;
    for (Headers::const_iterator it = headersToConvert.begin(); it != headersToConvert.end(); ++it) {
      vector.push_back(it->first + ": " + it->second);
    }
    return vector;
  }
};

const Response::HttpStatuses Response::STATUSES = initHttpStatuses();
const Response::MimeTypes Response::MIME = initMimeTypes();
Logger Response::LOGGER(Logger::DEBUG);
