#pragma once
#include <string>
#include <vector>

class Location {
 public:
  Location(void) {}
  Location(int def) {
    this->url = "/";
    this->allowMethod.push_back("GET");
    this->allowMethod.push_back("POST");
    this->allowMethod.push_back("DELETE");
    this->root = "./html";
    this->autoindex = true;
    this->index.push_back("index.html");
  }

  Location(std::string url, std::string root, std::vector<std::string> allowMethod,
           bool autoindex, std::vector<std::string> index, std::string uploadPath,
           std::vector<std::string> cgiExt, std::string cgiPath, std::string errorPage)
      : url(url), root(root), allowMethod(allowMethod),
        autoindex(autoindex), index(index), uploadPath(uploadPath),
        cgiExt(cgiExt), cgiPath(cgiPath), errorPage(errorPage) {
  }

  ~Location() {
    this->allowMethod.clear();
    this->index.clear();
    this->cgiExt.clear();
  }

  //todo coplien form
  //Location(Location const &other){};
  //Location &operator=(Location const &other){};

  std::string getUrl() const {
    return this->url;
  }

  std::string getRoot() const {
    return this->root;
  }

  std::vector<std::string> getMethods() const {
    return this->allowMethod;
  }

  bool getAutoindex() const {
    return this->autoindex;
  }

  std::vector<std::string> getIndex() const {
    return this->index;
  }

  std::string getUploadPath() const {
    return this->uploadPath;
  }

  std::vector<std::string> getCgiExt() const {
    return this->cgiExt;
  }

  std::string getCgiPath() const {
    return this->cgiPath;
  }

  std::string getErrorPage() const {
    return this->errorPage;
  }

 private:
  std::string url;
  std::string root;
  std::vector<std::string> allowMethod;
  bool autoindex;
  std::vector<std::string> index;
  std::string uploadPath;
  std::vector<std::string> cgiExt;
  std::string cgiPath;
  std::string errorPage;
};
