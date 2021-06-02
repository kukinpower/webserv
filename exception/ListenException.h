#pragma once
#include "FatalWebServException.h"

class ListenException : public FatalWebServException {

 public:
  ListenException() : FatalWebServException(LISTEN_ERROR) {}

  ListenException(const std::string &msg) : FatalWebServException(msg) {}
};
