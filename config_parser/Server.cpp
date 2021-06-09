#include "Server.hpp"

Server::Server(void) {
 /*
  this->port = 8080;
  this->hostName = "localhost";
  this->serverName = "champions_server";
  this->errorPage = "error.html";
  */
  this->bodySize = 10000000;
}
Server::~Server(){
  this->locations.clear();
}

//Server(Server const &other){};
//Server &operator=(Server const &other){};
  
void Server::setPort(std::string &port){
  int n = port.length();
  char ch[n + 1];
  strcpy(ch, port.c_str());
  this->port = atoi(ch);
}

void Server::setHostname(const std::string &name){
  this->hostName = name;
}

void Server::setServerName(const std::string &name){
  this->serverName = name;
}

void Server::setErrorPage(const std::string &page){
  this->errorPage = page;
}

void Server::setBodySize(long int size){
  this->bodySize = size;
}

void Server::addLocation(Location &loc) {
  this->locations.push_back(loc);
}


size_t Server::getPort() const{
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