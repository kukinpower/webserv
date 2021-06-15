#pragma once
#include <iostream>
#include <sstream>
#include <string>

class Logger {
 public:
  static const int DEBUG = 1;
  static const int INFO = 2;
  static const char *SPRING_GREEN_SET;
  static const char *RESET;

 private:
  int level;

 public:
  Logger(int level = INFO) : level(level) {
  }

  // todo add more data: time, thread
  void info(std::string const &message) const {
    if (level <= 2) {
      std::cerr << SPRING_GREEN_SET << message << RESET << std::endl;
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

  template<class T>
  static std::string toString(const T &val) {
    std::stringstream ss;
    ss << val;
    return ss.str();
  }
};

const char *Logger::SPRING_GREEN_SET = "\033[38;2;0;255;127m";
const char *Logger::RESET = "\033[0m";
