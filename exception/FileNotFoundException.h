#pragma once
#include "RuntimeWebServException.h"

class FileNotFoundException : public RuntimeWebServException {

 public:
  FileNotFoundException() : RuntimeWebServException(FILE_NOT_FOUND) {}

  FileNotFoundException(const std::string &msg) : RuntimeWebServException(msg) {}
};
