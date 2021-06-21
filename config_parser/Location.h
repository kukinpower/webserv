#pragma once
#include "Logger.h"
#include "HttpStatus.h"

#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <fstream>
#include <map>

class Location {
 public:
  std::string url;
  std::string root;
  std::set<HttpMethod> allowedMethods;
  bool autoIndex;
  std::vector<std::string> index;
  std::string uploadPath;
  std::vector<std::string> cgiExt;
  std::string cgiPath;
  std::map<HttpStatus, std::string> errorPage;
  std::vector<std::pair<std::string, std::string> > redirect;

 public:
  Location(void) {
  }

  Location(int def) {
    this->url = "/";
    this->allowedMethods.insert(GET);
    this->allowedMethods.insert(POST);
    this->allowedMethods.insert(DELETE);
    this->root = "./html";
    this->autoIndex = true;
    this->index.push_back("index.html");
  }

  Location(const std::string &url,
           const std::string &root,
           const std::vector<std::string> &allowedMethodsVector,
           bool autoIndex,
           const std::vector<std::string> &index,
           const std::string &uploadPath,
           const std::vector<std::string> &cgiExt,
           const std::string &cgiPath,
           const std::map<HttpStatus, std::string> &errorPage,
           const std::vector<std::pair<std::string, std::string> > &redirect)
      : url(url), root(root), allowedMethods(vectorToSet(allowedMethodsVector)),
        autoIndex(autoIndex), index(index), uploadPath(uploadPath),
        cgiExt(cgiExt), cgiPath(cgiPath), errorPage(errorPage), redirect(redirect) {
  }

  ~Location() {
    this->allowedMethods.clear();
    this->index.clear();
    this->cgiExt.clear();
  }
  //todo coplien form
  //Location(Location const &other){};
  //Location &operator=(Location const &other){};

  std::string substitutePath(const std::string &path) const {
    std::string name = path.substr(path.find_last_of('/') + 1);
    bool isRedir = false;
    for (std::vector<std::pair<std::string, std::string> >::const_iterator it = redirect.begin();
          it != redirect.end() && !name.empty(); ++it) {
      if (name == it->first){
        name = it->second;
        isRedir = true;
        break;
      }
    }
    if (path.find(url) != std::string::npos) {
      if (!isRedir)
        return root + path.substr(url.length() - 1);
      else
        return root + path.substr(url.length() - 1).substr(0, path.find_last_of('/')) + '/' + name;
    }
    throw RuntimeWebServException("Bad path string provided: " + path + ". Not matches with url: " + url);
  }

  std::string getFullCgiPath(const std::string &path) const {
    return root + '/' + path;
  }

  bool isMethodAllowed(HttpMethod method) const {
    return allowedMethods.find(method) != allowedMethods.end();
  }

  bool matches(const std::string &path) const {
    return (path.length() == 1 && url.length() == 1 && path == url) ||
        (path.substr(1).rfind(url.substr(1), 0) == 0);
  }

  std::string getUrl() const {
    return this->url;
  }

  std::string getRoot() const {
    return this->root;
  }

  std::set<HttpMethod> getMethods() const {
    return this->allowedMethods;
  }

  static HttpMethod extractMethodFromStr(const std::string &method) {
    if (method == "GET") {
      return GET;
    } else if (method == "POST") {
      return POST;
    } else if (method == "DELETE") {
      return DELETE;
    }
    throw std::runtime_error("Config file error: wrong server allowed method: " + method);
  }

  static std::string extractStringFromMethod(HttpMethod method) {
    if (method == GET) {
      return "GET";
    } else if (method == POST) {
      return "POST";
    } else if (method == DELETE) {
      return "DELETE";
    }
    throw std::runtime_error("Config file error: wrong server allowed method: " + Logger::toString(method));
  }

  std::set<HttpMethod> vectorToSet(const std::vector<std::string> &v) const {
    std::set<HttpMethod> methodsSet;
    for (std::vector<std::string>::const_iterator it = v.begin(); it != v.end(); ++it) {
      methodsSet.insert(extractMethodFromStr(*it));
    }

    return methodsSet;
  }

  std::vector<std::string> setToVector(const std::set<HttpMethod> &methodsSet) const {
    std::vector<std::string> methodsVector;
    for (std::set<HttpMethod>::const_iterator it = methodsSet.begin(); it != methodsSet.end(); ++it) {
      methodsVector.push_back(extractStringFromMethod(*it));
    }

    return methodsVector;
  }

  std::string getFirstExistingIndex(const std::string &path) const {
    for (std::vector<std::string>::const_reverse_iterator it = index.rbegin(); it != index.rend(); ++it) {
      std::ifstream f((path + *it).c_str());
      if (!f.fail())
        return *it;
    }
    std::map<HttpStatus, std::string>::const_iterator it = errorPage.find(NOT_FOUND);
    if (it == errorPage.end())
      return "";
    return it->second; //
  }

  std::vector<std::string> getMethodsVector() const {
    return setToVector(this->allowedMethods);
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

  std::map<HttpStatus, std::string> getErrorPage() const {
    return this->errorPage;
  }

  std::map<HttpStatus, std::string> &getErrorPageByRef() {
    return this->errorPage;
  }

  std::vector<std::pair<std::string, std::string> > getRedirect() const {
    return this->redirect;
  }

  bool isAutoIndex() const {
    return autoIndex;
  }

  bool getAutoIndex() const {
    return autoIndex;
  }
};
