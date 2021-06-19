#pragma once
#include "RuntimeWebServException.h"

class MethodNotAllowed : public RuntimeWebServException {

 public:
  MethodNotAllowed() : RuntimeWebServException(METHOD_NOT_ALLOWED) {}

  MethodNotAllowed(const std::string &msg) : RuntimeWebServException(msg) {}
};
