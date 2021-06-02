#pragma once
#include "FatalWebServException.h"

class BindException : public FatalWebServException {

 public:
  BindException() : FatalWebServException(BIND_ERROR) {}

  BindException(const std::string &msg) : FatalWebServException(msg) {}
};
