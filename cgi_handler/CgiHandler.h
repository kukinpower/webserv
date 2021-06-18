#pragma once

#include "ServerStruct.h"
#include "WebServException.h"
#include "FatalWebServException.h"
#include "Logger.h"
#include "Location.h"
#include "Request.h"

#include <unistd.h>
#include <stdio.h>

#include <map>

static std::string extractQueryString(std::string &path) {
  size_t delimiter = path.find('?');
  if (delimiter == std::string::npos)
    return "";
  std::string queryString = path.substr(delimiter + 1);
  path = path.substr(0, delimiter);
  return queryString;
}

static std::string findExtension(const std::string &path) {
  size_t delimiter = path.find_last_of(".");
  return path.substr(delimiter);
}

class CgiHandler {
 public:

  static const char *AUTH_TYPE;
  static const char *CONTENT_LENGTH;
  static const char *CONTENT_TYPE;
  static const char *GATEWAY_INTERFACE;
  static const char *PATH_INFO;
  static const char *PATH_TRANSLATED;
  static const char *QUERY_STRING;
  static const char *REDIRECT_STATUS;
  static const char *REMOTEaddr;
  static const char *REMOTE_IDENT;
  static const char *REMOTE_USER;
  static const char *REQUEST_URI;
  static const char *REQUEST_METHOD;
  static const char *SCRIPT_NAME;
  static const char *SCRIPT_FILENAME;
  static const char *SERVER_NAME;
  static const char *SERVER_PORT;
  static const char *SERVER_PROTOCOL;
  static const char *SERVER_SOFTWARE;

  static const int STDIN;
  static const int STDOUT;

  static const int BUFFER_SIZE;

  //for testing purposes
  CgiHandler(char c) {
    body = "";
    std::string literalPort = "8080";
    env[AUTH_TYPE] = "";
    env[CONTENT_LENGTH] = "0";
    env[CONTENT_TYPE] = "text/html";
    env[GATEWAY_INTERFACE] = "CGI/1.1";
    env[PATH_INFO] = "/Users/ilya/Desktop/a.out?var1=val1&var2=val2";
    env[PATH_TRANSLATED] = "/Users/ilya/Desktop/a.out";
    env[QUERY_STRING] = "";
    env[REDIRECT_STATUS] = "200";
    env[REMOTEaddr] = "8080";
    env[REMOTE_IDENT] = "";
    env[REMOTE_USER] = "";
    env[REQUEST_URI] = "/Users/ilya/Desktop/a.out";
    env[REQUEST_METHOD] = "GET";
    env[SCRIPT_NAME] = "/Users/ilya/Desktop/a.out";
    env[SCRIPT_FILENAME] = "/Users/ilya/Desktop/a.out";
    env[SERVER_NAME] = "http://localhost";
    env[SERVER_PORT] = "8080";
    env[SERVER_PROTOCOL] = "HTTP/1.1";
    env[SERVER_SOFTWARE] = "WebServ/42.0";
    std::string path = "/Users/ilya/Desktop/a.out?var1=val1&var2=val2";
    std::cerr << "pathQS: " << path << std::endl;
    std::string queryString = extractQueryString(path);
    std::cerr << "path: " << path << std::endl;
    std::cerr << "QS: " << queryString << std::endl;
    std::string extension = findExtension(path); //NEVER USED
    std::cerr << "extension: " << extension << std::endl;
  }



  CgiHandler(const Request &request, const ServerStruct &server,
             std::vector<Location>::const_iterator requestLocation,
             const std::string &queryString, const std::string &path) : body(request.getBody()) {
    env[REQUEST_URI] = path;
    std::string literalPort = _toLiteral(server.getPort());
    env[SERVER_PORT] = literalPort;
    env[REMOTEaddr] = literalPort;
    std::string literalBodySize = _toLiteral(body.size());
    env[CONTENT_LENGTH] = literalBodySize;
    if (request.getHeaders().find("Authorization") != request.getHeaders().end()) {
      std::map<std::string, std::string>::const_iterator it = request.getHeaders().find("Authorization");
      env[AUTH_TYPE] = it->second;
      env[REMOTE_IDENT] = it->second;
      env[REMOTE_USER] = it->second;
    } else {
      env[AUTH_TYPE] = "";
      env[REMOTE_IDENT] = "";
      env[REMOTE_USER] = "";
    }
    if (request.getHeaders().find("Content-Type") != request.getHeaders().end()) {
      std::map<std::string, std::string>::const_iterator it = request.getHeaders().find("Content-Type");
      env[CONTENT_TYPE] = it->second;
    } else {
      env[CONTENT_TYPE] = "";
    }
    env[GATEWAY_INTERFACE] = "CGI/1.1";
    env[PATH_INFO] = path;
    env[PATH_TRANSLATED] = path;
    env[QUERY_STRING] = queryString;
    env[REDIRECT_STATUS] = "200"; //for php-cgi


    HttpMethod m = request.getMethod();
    if (m == POST)
      env[REQUEST_METHOD] = "POST";
    else if (m == GET)
      env[REQUEST_METHOD] = "GET";
    else
      env[REQUEST_METHOD] = "DELETE";

    env[SCRIPT_NAME] = requestLocation->getCgiPath();//?
    env[SCRIPT_FILENAME] = requestLocation->getCgiPath();//?
    env[SERVER_NAME] = server.getServerName();
    env[SERVER_PROTOCOL] = "HTTP/1.1";
    env[SERVER_SOFTWARE] = "WebServ/42.0";

  }
  virtual ~CgiHandler() {}

  std::string runScript(const std::string &script, const std::string &interpreter, HttpStatus &responseStatus) {
    responseStatus = OK;

    char **envVars;
    try {
      envVars = _getEnv();
    }
    catch (std::bad_alloc &e) {
      LOGGER.error(e.what());
      throw FatalWebServException("Could not allocate memory for env vars (char**) in CgiHandler"); //?
    }
    int input = dup(STDIN);
    int output = dup(STDOUT);
    FILE *fsInput = tmpfile();
    FILE *fsOutput = tmpfile();
    if (!fsInput || !fsOutput)
      throw FatalWebServException("Could not create temporary file in CgiHandler");
    int fdInput = fileno(fsInput);
    int fdOutput = fileno(fsOutput);
    write(fdInput, body.c_str(), body.size());
    lseek(fdInput, 0, SEEK_SET);
    std::string dynamicPage;
    const char **args = new const char*[2];
    args[0] = script.c_str();
    args[1] = NULL;

    pid_t pid = fork();
    if (pid == -1)
      throw FatalWebServException("Could not create process in CgiHandler");
    else if (pid == 0) {
      dup2(fdInput, STDIN);
      dup2(fdOutput, STDOUT);
      execve(script.c_str(), const_cast<char*const*>(args), envVars); //python3 test.py
      LOGGER.error("Could not execute script in CgiHandler");
      responseStatus = BAD_REQUEST;
      exit(0);
    } else {
      waitpid(-1, NULL, 0);
      delete[] args;
      lseek(fdOutput, 0, SEEK_SET);
      char buf[BUFFER_SIZE];
      for (int hasRead = 1; hasRead > 0;) {
        bzero(buf, BUFFER_SIZE);
        hasRead = read(fdOutput, buf, BUFFER_SIZE);
        dynamicPage += buf;
      }

      dup2(STDIN, input);
      dup2(STDOUT, output);
      close(input);
      close(output);
      close(fdInput);
      close(fdOutput);
      fclose(fsInput);
      fclose(fsOutput);

      for (int i = 0; envVars[i]; ++i)
        delete[] envVars[i];
      delete[] envVars;
      return dynamicPage;
    }
  }

 private:
  CgiHandler() {}
  CgiHandler(const CgiHandler &c) {}
  CgiHandler &operator=(CgiHandler const &src) {return *this;}

  //heap allocation
  char **_getEnv() const {
    char **envVars = new char*[env.size() + 1];
    envVars[env.size()] = NULL;
    int i = 0;
    for (std::map<std::string, std::string>::const_iterator it = env.begin();
        it != env.end(); ++it, ++i) {
      std::string keyValue = it->first + '=' + it->second;
      envVars[i] = new char[keyValue.size() + 1];
      strcpy(envVars[i], keyValue.c_str());
    }
    return envVars;
  }

  std::string _toLiteral(int num) {
    std::stringstream ss;
    ss << num;
    return ss.str();
  }

  std::map<std::string, std::string> env;
  std::string body;
  Logger LOGGER;
};

const char *CgiHandler::AUTH_TYPE = "AUTH_TYPE";
const char *CgiHandler::CONTENT_LENGTH = "CONTENT_LENGTH";
const char *CgiHandler::CONTENT_TYPE = "CONTENT_TYPE";
const char *CgiHandler::GATEWAY_INTERFACE = "GATEWAY_INTERFACE";
const char *CgiHandler::PATH_INFO = "PATH_INFO";
const char *CgiHandler::PATH_TRANSLATED = "PATH_TRANSLATED";
const char *CgiHandler::QUERY_STRING = "QUERY_STRING";
const char *CgiHandler::REDIRECT_STATUS = "REDIRECT_STATUS";
const char *CgiHandler::REMOTEaddr = "REMOTEaddr";
const char *CgiHandler::REMOTE_IDENT = "REMOTE_IDENT";
const char *CgiHandler::REMOTE_USER = "REMOTE_USER";
const char *CgiHandler::REQUEST_URI = "REQUEST_URI";
const char *CgiHandler::REQUEST_METHOD = "REQUEST_METHOD";
const char *CgiHandler::SCRIPT_NAME = "SCRIPT_NAME";
const char *CgiHandler::SCRIPT_FILENAME = "SCRIPT_FILENAME";
const char *CgiHandler::SERVER_NAME = "SERVER_NAME";
const char *CgiHandler::SERVER_PORT = "SERVER_PORT";
const char *CgiHandler::SERVER_PROTOCOL = "SERVER_PROTOCOL";
const char *CgiHandler::SERVER_SOFTWARE = "SERVER_SOFTWARE";

const int CgiHandler::STDIN = 0;
const int CgiHandler::STDOUT = 1;

const int CgiHandler::BUFFER_SIZE = 1024;