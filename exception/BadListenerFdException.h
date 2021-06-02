#pragma once
#include "FatalWebServException.h"

class BadListenerFdException : public FatalWebServException {

 public:
  BadListenerFdException() : FatalWebServException(BAD_LISTENER_FD) {}

  BadListenerFdException(const char *msg) : FatalWebServException(msg) {}
};
