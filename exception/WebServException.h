#pragma once
#include <exception>
#include <stdarg.h>
#include <sstream>

class WebServException : public std::exception {
 public:
  WebServException(const char *msg) : exception() {
    message = msg;
  }

  virtual const char *what() const throw() {
    return message;
  }

 protected:
  const char *message;

 public:
  static const char *FATAL_ERROR;
  static const char *RUNTIME_ERROR;
  static const char *SELECT_ERROR;
  static const char *BAD_LISTENER_FD;
  static const char *BIND_ERROR;
  static const char *LISTEN_ERROR;
  static const char *ACCEPT_ERROR;
  static const char *FCNTL_ERROR;
  static const char *RECV_ERROR;
};

const char *WebServException::FATAL_ERROR = "FATAL ERROR";
const char *WebServException::RUNTIME_ERROR = "RUNTIME ERROR";
const char *WebServException::BAD_LISTENER_FD = "Bad listener fd error";
const char *WebServException::BIND_ERROR = "Bind error";
const char *WebServException::LISTEN_ERROR = "Listen error";
const char *WebServException::SELECT_ERROR = "Select error";
const char *WebServException::ACCEPT_ERROR = "Accept error";
const char *WebServException::FCNTL_ERROR = "Fcntl error";
const char *WebServException::RECV_ERROR = "Recv error";
