#pragma once

#include "WebServException.h"
#include "FatalWebServException.h"
#include "Logger.h"

#include <unistd.h>
#include <stdio.h>

#include <map>

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
    env[PATH_INFO] = "/Users/ilya/Desktop/a.out";
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
  }

  CgiHandler(const std::string &request) { //: request.getBody() {
//    int port = Server.getPort();
//    std::stringstream ss;
//    ss << port;
//    std::string literalPort = ss.str();
//    ss.str("");
//    int bodySize = body.size();
//    ss << bodySize;
//    std::string literalBodySize = ss.str();

    env[AUTH_TYPE] = "!request.getHeaders()[Authorization]";
//    env[CONTENT_LENGTH] = literalBodySize;
    env[CONTENT_TYPE] = "!request.getHeaders()[Content-Type]";
    env[GATEWAY_INTERFACE] = "CGI/1.1";
    env[PATH_INFO] = "!request.getPath()";
    env[PATH_TRANSLATED] = "!request.getPath()";
    env[QUERY_STRING] = "!"; //QUERY_STRING
    env[REDIRECT_STATUS] = "200"; //for php-cgi
    env[REMOTEaddr] = "!literalPort";
    env[REMOTE_IDENT] = "!request.getHeaders()[Authorization]";
    env[REMOTE_USER] = "!request.getHeaders()[Authorization]";
    env[REQUEST_URI] = "!request.getPath() + ..."; //QUERY_STRING

//    Method m = request.getMethod();
//    if (m == POST)
//      env[REQUEST_METHOD] = "!POST";
//    else if (m == GET)
//      env[REQUEST_METHOD] = "!GET";
//    else
//      env[REQUEST_METHOD] = "!DELETE";

    env[SCRIPT_NAME] = "!Location.getCgiPath";
    env[SCRIPT_FILENAME] = "!Location.getCgiPath";
    env[SERVER_NAME] = "!Server.getServerName()";
    env[SERVER_PORT] = "!literalPort";
    env[SERVER_PROTOCOL] = "HTTP/1.1";
    env[SERVER_SOFTWARE] = "WebServ/42.0";

  } //todo add requestConfig and parse request
  virtual ~CgiHandler() {}

  std::string runScript(const std::string &script) {
    char **envVars;
    try {
      envVars = getEnv();
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
    lseek(fdInput, 0, SEEK_SET); //what's the point?
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
      execve(script.c_str(), const_cast<char*const*>(args), envVars);
      LOGGER.error("Could not execute script in CgiHandler");
      exit(0);
    } else {
      waitpid(-1, NULL, 0);
      delete[] args;
      lseek(fdOutput, 0, SEEK_SET); //what's the point?
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
  char **getEnv() const {
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