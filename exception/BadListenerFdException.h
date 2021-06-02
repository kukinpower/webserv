#pragma once
#include "FatalWebServException.h"

class BadListenerFdException : public FatalWebServException {

 public:
  BadListenerFdException() : FatalWebServException(BAD_LISTENER_FD) {}

  BadListenerFdException(const std::string &msg) : FatalWebServException(msg) {}
};
