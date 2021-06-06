#pragma once
#include <string>
#include <vector>

class Location {

public:
  
  Location(void);
  ~Location();

  //Location(Location const &other){};
  //Location &operator=(Location const &other){};
  
  void setUrl(const std::string &url);
  void setRoot(const std::string &root);
  void addMethod(std::vector<std::string> &spl);
  void setAutoindex(const std::string &autoindex);
  void addIndex(std::vector<std::string> &pl);
  void setUploadPath(const std::string &path);
  void addCgiExt(std::vector<std::string> &spl);
  void addCgiPath(const std::string &path);
  void setErrorPage(const std::string &path);

  std::string getUrl() const;
  std::string getRoot() const;
  std::vector<std::string> getMethods() const;
  bool getAutoindex() const;
  std::vector<std::string> getIndex() const;
  std::string getUploadPath() const;
  std::vector<std::string> getCgiExt() const;
  std::string getCgiPath() const;
  std::string getErrorPage() const;

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
