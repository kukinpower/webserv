#include "Server.hpp"
#include <iostream>

Server::Server(void) {
  this->bodySize = 10000000;
}

Server::Server(int def) {
  (void)def;
  this->port = 8080;
  this->hostName = "localhost";
  this->serverName = "champions_server";
  this->errorPage = "error.html";
  this->bodySize = 10000000;
  Location loc = Location(1);
  this->locations.push_back(loc);
}

Server::Server(int port, std::string hostName, std::string serverName, std::string errorPage,
    int maxBodySize, std::vector<Location> locations) : port(port), hostName(hostName), 
    serverName(serverName), errorPage(errorPage), maxBodySize(maxBodySize), locations(locations) {};

Server::~Server(){
  this->locations.clear();
}

//Server(Server const &other){};
//Server &operator=(Server const &other){};

int Server::getPort() const{
  return this->port;
}

std::string Server::getHostName() const{
  return this->hostName;
}

std::string Server::getServerName() const{
  return this->serverName;
}

std::string Server::getErrorPage() const{
  return this->errorPage;
}

long int Server::getBodySize() const{
  return this->bodySize;
}

std::vector<Location> Server::getLocations() const {
  return this->locations;
}