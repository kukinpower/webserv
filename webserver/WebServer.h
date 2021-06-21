#pragma once
#include "Logger.h"
#include "Server.h"
#include "ConfigReader.h"

#include "PollException.h"
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

#include <sstream>

#include "HttpStatus.h"
#include "Location.h"
#include "Logger.h"
#include "StringBuilder.h"
#include "HttpStatusWrapper.h"
#include "Request.h"
#include "ServerStruct.h"
#include "CgiHandler.h"

#include "FatalWebServException.h"
#include "FileNotFoundException.h"
#include "MethodNotAllowed.h"
#include "ExtensionNotSupported.h"
#include "CgiParamsNotSpecified.h"
#include "BadRequestException.h"

#include <map>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

class WebServer {
 public:
  static const int BUF_SIZE = 1024;
  static const int PORT_DEFAULT = 8080;
  static const int SERVER_TIMEOUT = 22000;
  static const int SEND_CHUNK_SIZE = 100000;
  static const int POLL_FD_ARR_SIZE = 1024;

 private:
  static Logger LOGGER;
  std::vector<Server *> servers;

 public:
  WebServer() : STATUSES(initHttpStatuses()), MIME(initMimeTypes()) {}
  virtual ~WebServer() {}

//  WebServer(const WebServer &server) {
//    operator=(server);
//  }
//
//  WebServer &operator=(const WebServer &server) {
//    this->servers = server.servers;
//    return *this;
//  }

 private:
  void setNonBlock(int fd) {
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
      throw NonBlockException(StringBuilder()
                                  .append(WebServException::FCNTL_ERROR)
                                  .append(" on fd: ")
                                  .append(fd)
                                  .toString());
    }
  }

  void eraseServer(Server &server) {
    for (std::vector<Server *>::iterator serverIt = servers.begin(); serverIt != servers.end(); ++serverIt) {
      if ((*serverIt)->getListenerFd() == server.getListenerFd()) {
        servers.erase(serverIt);
        break;
      }
    }
  }

  void fillPollFds(struct pollfd *fds,
                   const std::map<int, Server *> &listenerFdsMap,
                   const std::map<int, Client *> &clientFdsMap) {
    int j = 0;
    for (std::map<int, Client *>::const_iterator fd = clientFdsMap.begin(); fd != clientFdsMap.end(); ++fd, ++j) {
      fds[j].fd = fd->first;
      fds[j].events |= POLLIN;
      if (fd->second->getClientStatus() == WRITE) {
        fds[j].fd = fd->first;
        fds[j].events |= POLLOUT;
      }
    }
  }

  std::map<Client *, Server *> clientsToServersMap;
  std::map<int, Client *> clientFdsMap;
  std::map<int, Server *> serverFdsMap;
  struct pollfd *fds;
  int currentFd;

  void writeToClientSocket(Client &client, std::map<Client *, Server *>::iterator &clientIt) {

    generateResponse(client, *(clientIt->second));

    std::size_t bytesWritten = 0;

    // if was error status, send error response
    if (isErrorStatus()) {
      const std::string &errorResponse = requestLocation->errorPage[responseStatus];
      if ((bytesWritten = send(currentFd, errorResponse.c_str(), errorResponse.length(), 0)) == -1) {
        return;
      }
    } else {
      // generate headers
      std::stringstream ss;
      ss << STATUSES[responseStatus];

      // Content-Length
      if (!responseBody.empty()) {
        ss << "Content-Length: " << responseBody.length() << "\r\n";
      }

      // Content-Type
      unsigned long pos;
      ss << "Content-Type: ";
      if ((pos = client.path.find_last_of('.')) != std::string::npos) {
        MimeTypes::const_iterator it;
        if ((it = MIME.find(client.path.substr(pos))) != MIME.end()) {
          ss << it->second;
        } else {
          ss << MIME[".html"] << "\r\n";
        }
      } else {
        ss << MIME[".html"] << "\r\n";
      }

      // Connection
      ss << "Connection: close";

      // end of response headers
      ss << "\r\n\r\n";

      const std::string &headersString = ss.str();

      if ((bytesWritten = send(currentFd, headersString.c_str(), headersString.length(), 0)) == -1) {
        return;
      }

      // if body exists — send body
      if (!responseBody.empty()) {
        if ((bytesWritten = send(currentFd, responseBody.c_str(), responseBody.length(), 0)) == -1) {
          return;
        }
      }
    }
  }

  void readRequestChunk(Client &client) {
    long bytesRead;
    char buf[BUF_SIZE + 1];
    int fd = client.getFd();

    if ((bytesRead = recv(fd, buf, BUF_SIZE, 0)) == -1) {
      throw ReadException();
    }
    if (bytesRead == 0) {
      //todo timeout disconnect
      client.closeClient();
      return;
    }
    buf[bytesRead] = 0;
    if (client.getClientStatus() == READ) {
      client.appendToRequestBody(buf);
    } else if (client.getClientStatus() == WAITING_BODY) {
      client.appendToBody(buf);
    }

    LOGGER.debug(std::string(buf));
  }

  void processReading(Client &client) {
    readRequestChunk(client);
    if (client.getClientStatus() == READ && client.isContainsRequestEnd()) {
      client.parseRequest();
    }
  }

  void readFromClientSocket(Client &client) {
    try {
      processReading(client);
    } catch (const RuntimeWebServException &e) {
      LOGGER.error(e.what());
    }
  }

  void handleNewConnection() {
    Server *server = serverFdsMap[fds[currentFd].fd];
    try {
      struct sockaddr addr;
      socklen_t socklen;
      int newClientFd;

      // if no connection, accept is blocking process and start waiting
      if ((newClientFd = accept(server->getListenerFd(), (struct sockaddr *) &addr, &socklen)) == -1) {
        throw AcceptException();
      }
      // set nonblock
      setNonBlock(newClientFd);
      Client *newClient = new Client(newClientFd); //todo malloc free
      clientsToServersMap[newClient] = server;
      clientFdsMap[newClientFd] = newClient;

      fds[newClientFd].fd = newClientFd;
      fds[newClientFd].events |= POLLIN;

      LOGGER.info("Client connected, fd: " + std::to_string(newClientFd));
    } catch (const RuntimeWebServException &e) {
      LOGGER.error(e.what());
    } catch (const FatalWebServException &e) {
      eraseServer(*server);
    }
  }

  void clearAllClients() {

  }

  void routine() {
    while (true) {
      try {
        int ret = poll(fds, POLL_FD_ARR_SIZE, SERVER_TIMEOUT);
        if (ret == -1) {
          LOGGER.error(WebServException::POLL_ERROR);
          int i;
          throw PollException(); // todo handle this
        } else if (ret == 0) {
          clearAllClients(); // todo handle this
          LOGGER.info("Timeout reached. Close all connections");
        } else {
          bool establishedNewConnection = false;
          // new connection ------------------------------------------------------------------------------------------
          for (std::map<int, Server *>::iterator serverIt = serverFdsMap.begin();
               serverIt != serverFdsMap.end(); ++serverIt) {

            currentFd = serverIt->first;

            if (fds[currentFd].revents == 0) {
              continue;
            }

            if (fds[currentFd].revents & POLLIN) {
              LOGGER.info("New Connection: " + std::to_string(currentFd));
              handleNewConnection();
              establishedNewConnection = true;
              break;
            }
          }

          if (!establishedNewConnection) {
            std::map<Client *, Server *>::iterator clientIt = clientsToServersMap.begin();
            std::map<Client *, Server *>::iterator clientIte = clientsToServersMap.end();
            while (clientIt != clientIte) {
              currentFd = clientIt->first->getFd();

              if (fds[currentFd].revents == 0) {
                ++clientIt;
                continue;
              }

              Client &client = *(clientIt->first);

              // write ------------------------------------------------------------------------------------------------
              if (fds[currentFd].revents & POLLOUT) {
                LOGGER.info("Write to: " + std::to_string(currentFd));

                writeToClientSocket(client, clientIt);
                client.closeClient();
                requestLocation = NULL;
                // todo maybe set more
              }
                // read ------------------------------------------------------------------------------------------------
              else if (fds[currentFd].revents & POLLIN) {
                LOGGER.info("Read from: " + std::to_string(currentFd));

                readFromClientSocket(client);
                if (client.getClientStatus() == WRITE) {
                  fds[currentFd].events |= POLLOUT;
                }
              }

              if (client.getClientStatus() == CLOSED) {
//                clearOneClient(clientIt);
                fds[currentFd].fd = 0;
                fds[currentFd].events = 0;
                fds[currentFd].revents = 0;
                clientFdsMap.erase(currentFd);
                delete clientIt->first;
                clientIt = clientsToServersMap.erase(clientIt);
              } else {
                fds[currentFd].revents = 0;
                ++clientIt;
              }
            }
          }
        }
      } catch (const RuntimeWebServException &e) {
        std::cout << errno << std::endl;
        LOGGER.error(e.what());
        continue;
      }
    }
  }
 public:
  void parseConfig(int ac, char *av[]) {
    std::vector<Server> vector;
    if (ac == 1) {
      ConfigReader conf;
      conf.printData();
      vector = conf.getServers();
    } else {
      ConfigReader conf(av[1]);
      conf.readConfig();
      conf.printData();
      vector = conf.getServers();
    }
    std::vector<Server>::iterator srv = vector.begin();
    while (srv != vector.end()) {
      servers.push_back(new Server(*srv));
      ++srv;
    }
  }

  void run() {
    std::vector<Server *>::iterator server = servers.begin();

    fds = new struct pollfd[POLL_FD_ARR_SIZE]; // todo remember to clear memory
    memset(fds, 0, sizeof(struct pollfd) * POLL_FD_ARR_SIZE);

    while (server != servers.end()) {
      try {
        (*server)->run();
        serverFdsMap[(*server)->getListenerFd()] = *server;
        fds[(*server)->getListenerFd()].fd = (*server)->getListenerFd();
        fds[(*server)->getListenerFd()].events |= POLLIN;
        ++server;
      } catch (const FatalWebServException &e) {
        LOGGER.error(e.what());

        server = servers.erase(server);
      }
    }

    if (!servers.empty()) {
      routine();
    }
  }

// RESPONSE GENERATION ----------------------------------------------------------------------------------------------------

 public:
  typedef std::map<std::string, std::string> MimeTypes;
  typedef std::map<std::string, std::string> Headers;

  std::map<HttpStatus, std::string> STATUSES;
  MimeTypes MIME;
  const int MAX_FILESIZE = 10485760; // 10mb
  Headers headers;

  std::string responseBody;
  HttpStatus responseStatus;
  Location *requestLocation;

  typedef std::map<std::string, std::string>::iterator iterator;

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

  bool isDirectory(const char *path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
  }

  bool isErrorStatus() {
    return responseStatus != OK && responseStatus != CREATED && responseStatus != NO_CONTENT;
  }

  std::string getContentType(const std::string &path) {
    unsigned long pos;
    if ((pos = path.find_last_of('.')) != std::string::npos) {
      std::string mimeType = path.substr(pos);
      MimeTypes::const_iterator it;
      if ((it = MIME.find(mimeType)) != MIME.end()) {
        return it->second;
      }
    }
    return MIME.find(".html")->second;
  }

  void generateAutoIndex(Client &client, Server &server, const std::string &path) {
    DIR *dir = opendir(path.c_str());
    if (dir != NULL) {
      // generate host address
      std::stringstream serverAddr;
      serverAddr << "http://" << server.hostName + ":" << server.port;
      const std::string &serverAddress = serverAddr.str();

      std::stringstream ss;
      ss << "<!doctype html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>" << path << "</title></head><body>";

      struct dirent *ent;
      while ((ent = readdir(dir)) != NULL) {
        std::string fileName = (ent->d_name);
        if (!(fileName == "." || fileName == "..")) {
          ss << "<li><a href=\"" << serverAddress << client.path;
          if (client.path[client.path.length() - 1] != '/') {
            ss << "/";
          }
          ss << ent->d_name << "\">" << ent->d_name << "</a></li>" << std::endl;
        }
      }
      closedir(dir);

      ss << "</body></html>";

      responseBody = ss.str();
      responseStatus = OK;
    }

    responseStatus = INTERNAL_SERVER_ERROR;
  }

  bool fileSizeIsBiggerThanLimit(const std::string &path) {
    struct stat fileStat;
    stat(path.c_str(), &fileStat);
    return fileStat.st_size > MAX_FILESIZE;
  }

  std::string getDocumentContent(std::ifstream &fileStream) {
    fileStream.seekg(0, std::ios::end);
    std::streampos length = fileStream.tellg();
    fileStream.seekg(0, std::ios::beg);
    std::vector<char> buffer(length);
    fileStream.read(&buffer[0], length);
    return std::string(buffer.begin(), buffer.end());
  }

  void getDocumentContentInside(std::ifstream &fileStream) {
    fileStream.seekg(0, std::ios::end);
    std::streampos length = fileStream.tellg();
    fileStream.seekg(0, std::ios::beg);
    std::vector<char> buffer(length);
    fileStream.read(&buffer[0], length);

    responseBody = std::string(buffer.begin(), buffer.end());
  }

  std::string getDocumentContentByPath(const std::string &path) {
    std::ifstream fileStream(path);
    if (fileStream.fail()) {
      throw RuntimeWebServException();
    }
    return getDocumentContent(fileStream);
  }

  void doGet(Client &client, Server &server) {
    const std::string &path = requestLocation->substitutePath(client.path);

    std::ifstream fileStream(path);
    if (fileStream.fail()) {
      responseStatus = NOT_FOUND;
      return;
    }
    if (fileSizeIsBiggerThanLimit(path)) {
      responseStatus = BAD_REQUEST;
      return;
    }

    if (!isDirectory(path.c_str())) {
      getDocumentContentInside(fileStream);
    } else {
      if (requestLocation->isAutoIndex()) {
        generateAutoIndex(client, server, path);
      } else {
        std::ifstream indexFileStream(path + "index.html"); //нужно разные страницы загружать (см. в конфиге index)
        if (!indexFileStream.fail()) {
          getDocumentContentInside(indexFileStream);
        } else {
          responseBody.clear();
          responseStatus = INTERNAL_SERVER_ERROR;
          return;
        }
      }
    }
    responseStatus = OK;
  }

  void doPost(Client &client, Server &server) {
//    std::string path = requestLocation->substitutePath(request.getPath());
//    std::string interpreter = requestLocation->getFullCgiPath(requestLocation->getCgiPath());
//    std::string queryString = extractQueryString(path);
//    std::ifstream fileStream(path);
//    if (fileStream.fail()) {
//      // todo change all exceptions to statuses
//      throw FileNotFoundException(Logger::toString(WebServException::FILE_NOT_FOUND) + " '" + path + "'"); //404
//    }
//    if (!isDirectory(path.c_str())) {
//      if (requestLocation->getCgiPath().empty() || requestLocation->getCgiExt().empty()) {  //validate path?
//        throw CgiParamsNotSpecified(Logger::toString(WebServException::CGI_PARAMS_NOT_SPECIFIED) + " '" + path + "'");
//      }
//      std::string extension = findExtension(path);
//      if (find(requestLocation->getCgiExt().begin(), requestLocation->getCgiExt().end(), extension)
//          == requestLocation->getCgiExt().end()) {
//        throw ExtensionNotSupported(Logger::toString(WebServException::EXTENSION_NOT_SUPPORTED) + " '" + path + "'");
//      }
//      CgiHandler cgi(request, serverStruct, queryString, path, interpreter);
//      responseBody = cgi.runScript(path, interpreter, responseStatus);
//    } else {
//      // create file
//    }
  }

  void doDelete() {
//    std::string path = requestLocation->substitutePath(request.getPath());
//    std::ifstream infile(path);
//    if (infile.good() && (remove(path.c_str())) == 0) {
//      responseBody = "HTTP/1.1 200 OK\n"
//                     "<html>\n"
//                     "  <body>\n"
//                     "    <h1>File deleted.</h1>\n"
//                     "  </body>\n"
//                     "</html>";
//      infile.close();
//      responseStatus = OK; // unstoppable "Select error" here
//    } else {
//      responseStatus = NOT_FOUND; // hangs here, status showing only after process stops
//    }
  }

 public:
  void generateResponse(Client &client, Server &server) {
    std::vector<Location> &locations = server.getLocations();
    try {
      for (std::vector<Location>::iterator location = locations.begin();
           location != locations.end();
           ++location) {

        if (location->matches(client.path) && location->isMethodAllowed(client.method)) {
          requestLocation = &(*location);
          if (location->getUrl() != "/") {
            break;
          }
        }
      }
      if (requestLocation == NULL) {
        responseStatus = BAD_REQUEST;
        return;
      }

      if (client.method == GET) {
        doGet(client, server);
      } else if (client.method == POST) {
        doPost(client, server);
      } else if (client.method == DELETE) {
        doDelete();
      } else {
        responseStatus = BAD_REQUEST;
      }
    } catch (const std::exception &e) {
      LOGGER.error("Exception thrown");
      responseStatus = BAD_REQUEST;
    }
  }

  std::vector<std::string> convertHeadersToStringVector(const Headers &headersToConvert) {
    std::vector<std::string> vector;
    for (Headers::const_iterator it = headersToConvert.begin(); it != headersToConvert.end(); ++it) {
      vector.push_back(it->first + ": " + it->second);
    }
    return vector;
  }

  // INITIALIZE ------------------------------------------------------------------
 private:
  std::map<HttpStatus, std::string> initHttpStatuses() {
    std::map<HttpStatus, std::string> statuses;
    statuses.insert(std::make_pair(OK, "HTTP/1.1 200 OK\r\n"));
    statuses.insert(std::make_pair(CREATED, "HTTP/1.1 201 Created\r\n"));
    statuses.insert(std::make_pair(NO_CONTENT, "HTTP/1.1 204 No Content\r\n"));
    statuses.insert(std::make_pair(NOT_FOUND, "HTTP/1.1 404 Not Found\r\n"));
    statuses.insert(std::make_pair(BAD_REQUEST, "HTTP/1.1 400 Bad Request\r\n"));
    statuses.insert(std::make_pair(MOVED_PERMANENTLY, "HTTP/1.1 301 Moved Permanently\r\n"));
    statuses.insert(std::make_pair(INTERNAL_SERVER_ERROR, "HTTP/1.1 500 Internal Server Error\r\n"));

    return statuses;
  }

  Headers initMimeTypes() {
    Headers mime;
    mime.insert(std::make_pair(".htm", "text/html"));
    mime.insert(std::make_pair(".html", "text/html"));
    mime.insert(std::make_pair(".jpg", "image/jpeg"));
    mime.insert(std::make_pair(".jpeg", "image/jpeg"));
    mime.insert(std::make_pair(".png", "image/png"));
    mime.insert(std::make_pair(".js", "text/javascript"));
    mime.insert(std::make_pair(".txt", "text/plain"));
    mime.insert(std::make_pair(".sh", "application/x-sh"));
    return mime;
  }
};

Logger WebServer::LOGGER(Logger::DEBUG);
