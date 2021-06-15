#pragma once
#include "RuntimeWebServException.h"

class SendException : public RuntimeWebServException {

 public:
  SendException() : RuntimeWebServException(SEND_ERROR) {}

  SendException(const std::string &msg) : RuntimeWebServException(msg) {}
};
