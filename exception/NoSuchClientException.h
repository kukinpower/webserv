#pragma once
#include "RuntimeWebServException.h"

class NoSuchClientException : public RuntimeWebServException {

 public:
  NoSuchClientException() : RuntimeWebServException(CLIENT_NOT_FOUND) {}

  NoSuchClientException(const std::string &msg) : RuntimeWebServException(msg) {}
};
