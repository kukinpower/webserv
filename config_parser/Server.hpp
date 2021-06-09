#pragma once
#include <string>
#include "Location.hpp"

class Server {

public:
  
  Server(void);
  ~Server();

  //Server(Server const &other){};
  //Server &operator=(Server const &other){};
  
  void setPort(std::string &port);
  void setHostname(const std::string &name);
  void setServerName(const std::string &name);
  void setErrorPage(const std::string &page);
  void setBodySize(long int size);
  void addLocation(Location &loc);

  size_t getPort() const;
  std::string getHostName() const;
  std::string getServerName() const;
  std::string getErrorPage() const;
  long int getBodySize() const;
  std::vector<Location> getLocations() const;


private:

  size_t port;
  std::string hostName;
  std::string serverName;
  std::string errorPage;
  long int bodySize;
  std::vector<Location> locations;

};
