#pragma once
#include <string>
#include <vector>

class Location {

public:
  
  Location(void);
  Location(int def);
  Location(std::string url, std::string root, std::vector<std::string> allowMethod, 
          bool autoindex, std::vector<std::string> index, std::string uploadPath, 
          std::vector<std::string> cgiExt, std::string cgiPath, std::string errorPage);
  ~Location();

  //Location(Location const &other){};
  //Location &operator=(Location const &other){};
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
