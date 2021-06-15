#pragma once
#include <sstream>
#include <string>

class StringBuilder {
 private:
  std::stringstream ss;

 public:
  StringBuilder(const std::string &str) : ss(str) {}
  StringBuilder() {}
 private:
  StringBuilder(const StringBuilder &stringBuilder) {
    operator=(stringBuilder);
  }
  StringBuilder &operator=(const StringBuilder &stringBuilder) {
    return *this;
  }

 public:
  template<class T>
  StringBuilder &append(const T &value) {
    ss << value;
    return *this;
  }

  std::string toString() const {
    return ss.str();
  }
};
