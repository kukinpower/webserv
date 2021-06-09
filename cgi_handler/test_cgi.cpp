#include "CgiHandler.h"

int main(int argc, char **argv, char **envp){
  CgiHandler cgi('t');
  std::string dp = cgi.runScript("/bin/pwd");
  std::cout << dp << std::endl;
  read(0,0,1);
  return 0;
}

//		  if (Location.getCgiPath != "" && Location.getCgiExt.size()) !
//          CgiHandler cgi(client->getRequest());
//          try {
//            cgi.runScript(Location.getCgiPath);
//          }
//          catch (const FatalWebServException &e){
//            std::cerr << e.what() << std::endl;
//          }