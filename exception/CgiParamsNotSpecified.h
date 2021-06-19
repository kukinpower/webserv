#pragma once
#include "RuntimeWebServException.h"

class CgiParamsNotSpecified : public RuntimeWebServException {

 public:
  CgiParamsNotSpecified() : RuntimeWebServException(METHOD_NOT_ALLOWED) {}

  CgiParamsNotSpecified(const std::string &msg) : RuntimeWebServException(msg) {}
};
