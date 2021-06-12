#include "ConfigReader.h"
#include "WebServer.h"
#include "Logger.h"

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>

int main(int ac, char *av[]) {
  try {
    WebServer server;
    server.parseConfig(ac, av);
    server.run();
  } catch (std::exception &e) {
    // todo error management
    Logger LOGGER;
    LOGGER.error(e.what());
    return 1;
  }

  return 0;
}
