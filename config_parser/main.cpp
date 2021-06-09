// your program should have a config file in argument or use a default path
// camelCase for methods
// PascalCase for classes
// no _underscore
// iostream string vector list queue stack map alghoritm exception


#include "ConfigReader.hpp"

int main(int ac, char**av)
{

  std::vector<std::string> data;

  if (ac == 1)// fill config with default settings
    {ConfigReader conf; conf.printData();}
  else {
    ConfigReader conf(av[1]);
    try {
      conf.readConfig();
      conf.printData();
    }
    catch(const std::exception& e) {
      std::cerr << e.what() << std::endl;
      return 0;
    }
  }
  return 0;
}