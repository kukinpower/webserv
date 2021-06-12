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

  /*void setSrvPort(std::string &port);
  void setSrvHostname(const std::string &name);
  void setServerName(const std::string &name);
  void setSrvErrorPage(const std::string &page);
  void setSrvBodySize(long int size);
  void addLocation(Location &loc);
  
  void setUrl(const std::string &url);
  void setRoot(const std::string &root);
  void addMethod(std::vector<std::string> &spl);
  void setAutoindex(const std::string &autoindex);
  void addIndex(std::vector<std::string> &pl);
  void setUploadPath(const std::string &path);
  void addCgiExt(std::vector<std::string> &spl);
  void addCgiPath(const std::string &path);
  void setErrorPage(const std::string &path);*/

  void printData();

 private:
  //int fd;
  std::string path;
  std::vector<Server> servers;
};
