#pragma once
#include <string>

class ConfigReader {
 public:
  ConfigReader() {}
  ConfigReader(std::string const &path) : path(path) {
  }
  ~ConfigReader() {}

 private:
  int fd;
  std::string path;

};
