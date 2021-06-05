#pragma once
#include "Logger.h"
#include "Server.h"

#include "SelectException.h"
#include "BadListenerFdException.h"
#include "NonBlockException.h"
#include "BindException.h"
#include "ListenException.h"
#include "AcceptException.h"
#include "ReadException.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <vector>
#include <iostream>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/fcntl.h>

#include <sstream>

class WebServer {
 public:
  // constants to use
  static const int BUF_SIZE = 256;
  static const int PORT_DEFAULT = 8080;

 private:
  Logger LOGGER;
  std::vector<Server> servers;

 public:
  WebServer() {}
  ~WebServer() {}
  // todo coplien

 private:
  void routine() {
	while (true) {
	  std::vector<Server>::iterator server = servers.begin();
	  while (server != servers.end()) {
		try {
		  server->processSelect();
		  ++server;
		} catch (const RuntimeWebServException &e) {
		  LOGGER.error(e.what());
		  ++server;
		} catch (const FatalWebServException &e) {
		  server = servers.erase(server);
		}
	  }
	}
  }

 public:
  void parseConfig() {
	// todo stuff
  }

  void run() {
	servers.push_back(Server(8080));
	std::vector<Server>::iterator server = servers.begin();
	while (server != servers.end()) {
	  try {
		server->run();
		++server;
	  } catch (const FatalWebServException &e) {
		LOGGER.error(e.what());
		server = servers.erase(server);
	  }
	}

	routine();
  }
};
