#pragma once
#include "RuntimeWebServException.h"

class MaxRequestSizeExceededException : public RuntimeWebServException {

 public:
  MaxRequestSizeExceededException() : RuntimeWebServException(MAX_REQ_SIZE_EXCEEDED) {}

  MaxRequestSizeExceededException(const std::string &msg) : RuntimeWebServException(msg) {}
};
