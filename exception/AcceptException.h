#pragma once
#include "RuntimeWebServException.h"

class AcceptException : public RuntimeWebServException {

 public:
  AcceptException() : RuntimeWebServException(ACCEPT_ERROR) {}

  AcceptException(const std::string &msg) : RuntimeWebServException(msg) {}
};
