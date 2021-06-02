#pragma once
#include "WebServException.h"

class RuntimeWebServException : public WebServException {
 public:
  RuntimeWebServException() : WebServException(RUNTIME_ERROR) {}

  RuntimeWebServException(const char *msg) : WebServException(msg) {}
};
