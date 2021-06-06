#pragma once
#include "Status.h"

#include <map>

class Response {
 private:
  long length;
  int fd;
  std::map<std::string, std::string> headers;

  typedef std::map<std::string, std::string>::iterator iterator;
//  std::string header;

//  std::string body;

 public:
  std::pair<iterator, bool> putHeader(const std::string &key, const std::string &value) {
    return headers.insert(std::make_pair(key, value));
  }

};
