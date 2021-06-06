#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include "Server.hpp"

class ConfigReader {
 public:
  ConfigReader();
  ConfigReader(std::string const &path);
  //ConfigReader(ConfigReader const &other){};
  //ConfigReader &operator=(ConfigReader const &other){};
  ~ConfigReader();

  void readConfig();
  bool removeSymbols(std::string &str);
  void setConfig(std::vector<std::string> data);
  std::vector<std::string> strSplit(const std::string &text);
  void addServer(int count, std::vector<std::string> data);

  void printData();

 private:
  //int fd;
  std::string path;
  std::vector<Server> servers;
};
