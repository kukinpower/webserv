#pragma once
#include "RuntimeWebServException.h"

class SelectException : public RuntimeWebServException {

 public:
  SelectException() : RuntimeWebServException(SELECT_ERROR) {}

  SelectException(const char *msg) : RuntimeWebServException(msg) {}
};
