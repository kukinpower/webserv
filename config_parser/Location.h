#pragma once
#include <string>
#include <vector>
#include <set>

class Location {
 private:
  std::string url;
  std::string root;
  std::set<HttpMethod> allowedMethods;
  bool autoIndex;
  std::vector<std::string> index;
  std::string uploadPath;
  std::vector<std::string> cgiExt;
  std::string cgiPath;
  std::string errorPage;

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

  Location(const std::string& url, const std::string& root, std::vector<HttpMethod> allowMethod,
           bool autoIndex, const std::vector<std::string>& index, const std::string& uploadPath,
           const std::vector<std::string>& cgiExt, const std::string& cgiPath, const std::string& errorPage)
      : url(url), root(root), allowedMethods(allowMethod.begin(), allowMethod.end()),
		autoIndex(autoIndex), index(index), uploadPath(uploadPath),
		cgiExt(cgiExt), cgiPath(cgiPath), errorPage(errorPage) {
  }

  ~Location() {
    this->allowedMethods.clear();
    this->index.clear();
    this->cgiExt.clear();
  }

  //todo coplien form
  //Location(Location const &other){};
  //Location &operator=(Location const &other){};

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

  bool getAutoindex() const {
    return this->autoIndex;
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
};
