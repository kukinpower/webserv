#include "ConfigReader.h"
#include "WebServer.h"
#include "Logger.h"

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>

int main(int ac, char *av[]) {
  // todo config with args
  (void)ac;
  (void)av;

  // 1. Here we just create some Application class and run it
  try {
    WebServer server;
  // todo pass arguments to it's run() method
    server.run();
  } catch (std::exception &e) {
    // todo error management
    Logger LOGGER;
    LOGGER.error(e.what());
    return 1;
  }

  return 0;
}
