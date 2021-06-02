#pragma once
#include "WebServException.h"

class FatalWebServException : public WebServException {
 public:
  FatalWebServException() : WebServException(FATAL_ERROR) {}

  FatalWebServException(const std::string &msg) : WebServException(msg) {}
};
