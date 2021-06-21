#pragma once
#include "RuntimeWebServException.h"

class BadRequestException : public RuntimeWebServException {

 public:
  BadRequestException() : RuntimeWebServException(BAD_REQUEST) {}

  BadRequestException(const std::string &msg) : RuntimeWebServException(msg) {}
};
