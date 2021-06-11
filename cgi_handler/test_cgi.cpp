#include "CgiHandler.h"

int main(int argc, char **argv, char **envp){
  CgiHandler cgi('t');
  std::string dp = cgi.runScript("/Users/ilya/Desktop/cgi_tester");
  std::cout << dp;
  read(0,0,1);
  return 0;
}
