#pragma once

class Client {
 private:
  int fd;

 public:
  Client(int fd) : fd(fd) {}
  ~Client() {}
  //todo Coplien form

  int getFd() const {
	return fd;
  }




};
