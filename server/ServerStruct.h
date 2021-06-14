#pragma once
#include "Logger.h"

#include <string>
#include <sstream>

struct ServerStruct {
  const int port;
  const std::string hostName;
  const std::string serverName;
  const int maxBodySize;
  const std::vector<Location> &locations;

  ServerStruct(const int port,
               const std::string &host_name,
               const std::string &server_name,
               const int max_body_size,
               const std::vector<Location> &locations)
      : port(port), hostName(host_name), serverName(server_name), maxBodySize(max_body_size), locations(locations) {}

  const int getPort() const {
    return port;
  }
  const std::string &getHostName() const {
    return hostName;
  }
  const std::string &getServerName() const {
    return serverName;
  }
  const int getMaxBodySize() const {
    return maxBodySize;
  }
  const std::vector<Location> &getLocations() const {
    return locations;
  }

  static std::string getServerAddress(const ServerStruct &serverStruct) {
    return "http://" + serverStruct.getHostName() + ":" + Logger::toString(serverStruct.getPort());
  }
};
