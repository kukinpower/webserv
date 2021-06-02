#pragma once
#include "RuntimeWebServException.h"

class ReadException : public RuntimeWebServException {

 public:
  ReadException() : RuntimeWebServException(READ_ERROR) {}

  ReadException(const std::string &msg) : RuntimeWebServException(msg) {}
};
