#pragma once
#include "RuntimeWebServException.h"

class PollException : public RuntimeWebServException {

 public:
  PollException() : RuntimeWebServException(POLL_ERROR) {}

  PollException(const std::string &msg) : RuntimeWebServException(msg) {}
};
