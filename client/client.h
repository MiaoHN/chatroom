#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "chatroom.h"

class Client {
 public:
  Client();

  void Connect(const std::string& addr, int port);

  void Start();

 private:
  Socket::ptr _sock;
};

#endif  // __CLIENT_H__