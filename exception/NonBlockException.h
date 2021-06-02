#pragma once
#include "RuntimeWebServException.h"

class NonBlockException : public RuntimeWebServException {

 public:
  NonBlockException() : RuntimeWebServException(NON_BLOCK_ERROR) {}

  NonBlockException(const std::string &msg) : RuntimeWebServException(msg) {}
};
