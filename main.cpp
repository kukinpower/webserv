#include "WebServer.h"
#include "Logger.h"

#include <iostream>

int main(int ac, char *av[]) {
  try {
    WebServer server;
    server.parseConfig(ac, av);
    server.run();
  } catch (std::exception &e) {
    Logger LOGGER;
    LOGGER.error(e.what());
    return 1;
  }

  return 0;
}
