#pragma once
#include <iostream>

class Logger {
 public:
  static const int DEBUG = 1;
  static const int INFO;

 private:
  int level;

 public:
  Logger(int level = INFO) : level(level) {
  }

  // todo add more data: time, thread
  void info(std::string const &message) const {
    if (level <= 2) {
      std::cerr << message << std::endl;
    }
  }

  void debug(std::string const &message) const {
    if (level <= 1) {
      std::cerr << message << std::endl;
    }
  }

  void error(std::string const &message) const {
    std::cerr << message << std::endl;
  }
};

const int Logger::INFO = 2;
