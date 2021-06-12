#pragma once
#include "Location.h"

Location::Location(void) {};

Location::Location(std::string url, std::string root, std::vector<std::string> allowMethod, 
  bool autoindex, std::vector<std::string> index, std::string uploadPath, std::vector<std::string> cgiExt,
  std::string cgiPath, std::string errorPage) : url(url), root(root), allowMethod(allowMethod),
  autoindex(autoindex), index(index), uploadPath(uploadPath), cgiExt(cgiExt),
  cgiPath(cgiPath), errorPage(errorPage) {};

Location::Location(int def) {
  this->url = "/";
  this->allowMethod.push_back("GET");
  this->allowMethod.push_back("POST");
  this->allowMethod.push_back("DELETE");
  this->root = "/var/www";
  this->autoindex = true;
  this->index.push_back("index.html");
}

Location::~Location() {
  this->allowMethod.clear();
  this->index.clear();
  this->cgiExt.clear();
}

//Location(Location const &other){};
//Location &operator=(Location const &other){};

std::string Location::getUrl() const {
  return this->url;
}

std::string Location::getRoot() const {
  return this->root;
}

std::vector<std::string> Location::getMethods() const {
  return this->allowMethod;
}

bool Location::getAutoindex() const {
  return this->autoindex;
}

std::vector<std::string> Location::getIndex() const {
  return this->index;
}

std::string Location::getUploadPath() const {
  return this->uploadPath;
}


std::vector<std::string> Location::getCgiExt() const {
 return this->cgiExt;
}

std::string Location::getCgiPath() const {
  return this->cgiPath;
}

std::string Location::getErrorPage() const {
  return this->errorPage;
}