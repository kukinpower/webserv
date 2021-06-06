#include "Location.hpp"
#include <iostream>

Location::Location(void) {};
Location::~Location() {};

//Location(Location const &other){};
//Location &operator=(Location const &other){};

void Location::setUrl(const std::string &url) {
  this->url = url;
}

void Location::setRoot(const std::string &root) {
  this->root = root;
}

void Location::addMethod(std::vector<std::string> &spl) {
  while (spl.size() != 1) {
    this->allowMethod.push_back(spl.back()); // add check for allowed 
    spl.pop_back();
  }  
}

void Location::setAutoindex(const std::string &autoindex){
  if (autoindex == "on")
    this->autoindex = true;
  else
    this->autoindex = false;
}

void Location::addIndex(std::vector<std::string> &spl){
  while (spl.size() != 1) {
    this->index.push_back(spl.back());
    spl.pop_back();
  }
}

void Location::setUploadPath(const std::string &path) {
  
}

void Location::addCgiExt(std::vector<std::string> &spl){
  while (spl.size() != 1) {
    this->cgiExt.push_back(spl.back());
    spl.pop_back();
  }
}

void Location::addCgiPath(const std::string &path){
  this->cgiPath = path;
}

void Location::setErrorPage(const std::string &page) {
  this->errorPage = page;
}

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