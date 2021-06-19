#pragma once
#include "RuntimeWebServException.h"

class ExtensionNotSupported : public RuntimeWebServException {

 public:
  ExtensionNotSupported() : RuntimeWebServException(EXTENSION_NOT_SUPPORTED) {}

  ExtensionNotSupported(const std::string &msg) : RuntimeWebServException(msg) {}
};
