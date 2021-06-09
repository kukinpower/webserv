#include "ConfigReader.hpp"

ConfigReader::ConfigReader() {
  // set default params here
}
ConfigReader::ConfigReader(std::string const &path) : path(path) {
  
}

ConfigReader::~ConfigReader() {
  this->servers.clear();
}

void ConfigReader::readConfig() {

  std::ifstream fileStream(this->path);

  if (fileStream.fail())
    throw std::runtime_error("Error reading config file. Exiting...");

  std::string str;
  std::vector<std::string> data;
  
  while (std::getline(fileStream, str)) {
    if (removeSymbols(str)) {
      data.push_back(str);
    }
  }
  fileStream.close();
  setConfig(data);
}

bool ConfigReader::removeSymbols(std::string &str) {
  
  while (str[0] == ' ' || str[0] == '\t')
    str.erase(0, 1);
  while (str[str.length() - 1] == ' ' || str[str.length() - 1] == '\t' ) // remove tabs and spaces before and after
    str.erase((str.length() - 1), 1);
  
  size_t doubleSpace = str.find('\t'); // remove tabs and double spaces inside string
  doubleSpace = str.find('\t');
  while (doubleSpace != std::string::npos) {
    str.replace(doubleSpace, 1, " ");
    doubleSpace = str.find('\t');
  }
  
  doubleSpace = str.find("  ");
  while (doubleSpace != std::string::npos) {
    str.erase(doubleSpace, 1);
    doubleSpace = str.find("  ");
  }
  
  if (str.length() == 0 || str[0] == '#' || str[0] == '\n')
    return false;
  return true;
}

std::vector<std::string> strSplit(const std::string &text) {
  std::vector<std::string> res;
  size_t start = 0;
  size_t end = 0;
  while ((end = text.find(' ', start)) != std::string::npos) {
    res.push_back(text.substr(start, end - start));
    start = end + 1;
  }
  res.push_back(text.substr(start));
  return res;
}

void addServerData(Server &srv, std::string &str) {
  std::vector<std::string> spl = strSplit(str);
  if (spl.front().compare("port") == 0) 
    srv.setPort(spl.back());
  else if (spl.front().compare("host") == 0)
    srv.setHostname(spl.back());
  else if (spl.front().compare("server_name") == 0)
    srv.setServerName(spl.back());
  else if (spl.front().compare("error_page") == 0)
   srv.setErrorPage(spl.back());
  else
    throw std::runtime_error("Config file error: wrong server option. Exiting...");
}


void addLocationData(Location &loc, std::string &str) {
  std::vector<std::string> spl = strSplit(str);
  if (spl.front().compare("location") == 0)
    loc.setUrl(spl[1]);
  else if (spl.front().compare("root") == 0) 
    loc.setRoot(spl.back());
  else if (spl.front().compare("allow_method") == 0)
    loc.addMethod(spl);
  else if (spl.front().compare("autoindex") == 0)
    loc.setAutoindex(spl.back());
  else if (spl.front().compare("index") == 0)
    loc.addIndex(spl);
  else if (spl.front().compare("cgi_ext") == 0)
    loc.addCgiExt(spl);
  else if (spl.front().compare("cgi_path") == 0)
   loc.addCgiPath(spl.back());
  else if (spl.front().compare("error_page") == 0)
    loc.setErrorPage(spl.back());
  else
    throw std::runtime_error("Config file error: wrong location option. Exiting...");
}

void ConfigReader::addServer(int count, std::vector<std::string> data) {
  
  bool loc_bracket = false;
  std::vector<std::string>::iterator it = data.begin() + 1;
  int i = 0;
  Server *srv = new Server();
  
  while (i < count - 1) {
    if (!loc_bracket && (*it).find("location") == std::string::npos && *it != "}")
      addServerData(*srv, *it);
    else if ((*it).find("location") != std::string::npos) {
      loc_bracket = true;
      Location *loc = new Location();
      addLocationData(*loc, *it);
      it++;
      
      while ((*it).find("}") == std::string::npos) {
        addLocationData(*loc, *it);
        i++; it++;
      }
      
      loc_bracket = false;
      srv->addLocation(*loc);
      i++;
    }
    if ((*it == "}" && i == count - 2) || it == data.end() - 1)
      break;
    i++; it++;
  }
  this->servers.push_back(*srv);
}


void ConfigReader::setConfig(std::vector<std::string> data) {

  while (data.size() > 0)
  {
    bool srv_bracket = false;
    bool loc_bracket = false;
    int count = 0;
    std::vector<std::string>::iterator it = data.begin();
    
    while (it != data.end())
    {
      std::string str = *it;
      
      if (str.find("server {") != std::string::npos) {
        srv_bracket = true;
        count++;
      }
      else if (str.find("location") != std::string::npos) {
        loc_bracket = true;
        count++;
      }
      else if (str[0] == '}' && loc_bracket && srv_bracket) {
        loc_bracket = false;
        count++;
      }
      else if (srv_bracket && !loc_bracket && str[0] != '}')
        count ++;
      else if (srv_bracket && loc_bracket)
        count++;         
       else if (str[0] == '}' && !loc_bracket && srv_bracket) {
        srv_bracket = false;
        count++;
      }
      
      if (!loc_bracket && !srv_bracket)
        break;
      it++;
    }

    addServer(count, data);
    data.erase(data.begin(), data.begin() + count);  
  }

  if (this->servers.size() == 0)
    throw std::runtime_error("Config file error: no server data found. Exiting...");

  // check same servernames, ports, if locations present
}

void ConfigReader::printData()
{
  std::vector<Server>::iterator it = this->servers.begin();
  int i  = 1;
  while (it != this->servers.end())
  {
    std::cout << "\033[91m";
    int k = 1;
    std::cout << "Server #" << i << " config:" << std::endl;
    Server tmp = *it;
    std::cout << "Port: " << tmp.getPort() << std::endl;
    std::cout << "Hostname: " << tmp.getHostName() << std::endl;
    std::cout << "Server Name: " << tmp.getServerName() << std::endl;
    std::cout << "Error page: " << tmp.getErrorPage() << std::endl;
    std::cout << "Size limit: " << tmp.getBodySize() << std::endl << std::endl;

    
    std::vector<Location> loc = it->getLocations();
    std::vector<Location>::iterator lit = loc.begin();
    std::vector<std::string> vec;
    std::vector<std::string>::iterator vit;
    while (lit != loc.end())
    {
      std::cout << "\033[93m";
      std::cout << "Server #" << i << " location #" << k << ":" << std::endl;
      Location ltmp = *lit;
      std::cout << "Url: " << ltmp.getUrl() << std::endl;
      std::cout << "Root: " << (ltmp.getRoot().length() > 0 ? ltmp.getRoot() : "NONE") << std::endl;
      std::cout << "Allowed methods: ";
      vec = ltmp.getMethods();
      vit = vec.begin();
      while (vit != vec.end())
        std::cout << *vit++ << " ";
      if (vec.size() == 0)
        std::cout << "NONE";
      std::cout << std::endl;
      
      std::cout << "Autoindex: " << ltmp.getAutoindex() << " (1 = on, 0 = off)" << std::endl;
      std::cout << "Index: ";
      vec = ltmp.getIndex();
      vit = vec.begin();
      while (vit != vec.end())
        std::cout << *vit++ << " ";
      if (vec.size() == 0)
        std::cout << "NONE";
      std::cout << std::endl;
      
      std::cout << "CGI ext: ";
      vec = ltmp.getCgiExt();
      vit = vec.begin();
      while (vit != vec.end())
        std::cout << *vit++ << " ";
      if (vec.size() == 0)
        std::cout << "NONE";
      std::cout << std::endl;
      std::cout << "CGI path: " << (ltmp.getCgiPath().length() > 0 ? ltmp.getCgiPath() : "NONE") << std::endl;
      std::cout << "Error page: " << (ltmp.getErrorPage().length() > 0 ? ltmp.getErrorPage() : "NONE") << std::endl << std::endl;
      k++; lit++;
    }
    std::cout << "\033[0m";
    std::cout << "===========================\n" << std::endl;
    i++; it++;

  }
}






