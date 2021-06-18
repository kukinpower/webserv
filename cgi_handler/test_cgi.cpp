#include <unistd.h>
#include <stdio.h>
#include <iostream>

int main(int argc, char **argv, char **envp){
//  CgiHandler cgi('t');
//  std::string dp = cgi.runScript("/Users/ilya/Desktop/cgi_tester");
//  std::cout << dp;
  const char **args = new const char*[3];
  args[0] = "html/python-cgi/venv/bin/python3.9";
  args[1] = "html/cgi/pycgi.py";
  args[2] = NULL;
  pid_t pid = fork();
  if (pid == -1)
    std::cerr << "ERROR" << std::endl;
  else if (pid == 0) {
    execve("html/python-cgi/venv/bin/python3.9", const_cast<char*const*>(args), NULL);
    std::cerr << "Could not execute script" << std::endl;
    exit(0);
  } else {
    waitpid(-1, NULL, 0);
    delete[] args;
    std::cerr << "script was executed" << std::endl;
  }
  read(0,0,1);
  return 0;
}
