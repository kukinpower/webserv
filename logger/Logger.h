#pragma once
#include <iostream>

class Logger {
 private:
  int level;
 public:
  Logger() {
//todo logger level
//#ifdef LEVEL
//    level = LEVEL;
//#else
    level = 0;
  }

  // todo add more data: time, thread
  void info(std::string const &message) {
   std::cerr << message << std::endl;
  }

  void debug(std::string const &message) {
    std::cerr << message << std::endl;
  }

  void error(std::string const &message) {
    std::cerr << message << std::endl;
  }
};