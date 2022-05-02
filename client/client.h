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

  void Connect(const std::string& addr, int port) {
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(addr.c_str());
    socklen_t len = sizeof(addr);
    connect(sock_, (sockaddr*)&address, len);
  }

  void Start() {
    while (true) {
      std::string str;
      std::cin >> str;
      Send(str.c_str(), str.size());
      if (str == "exit") {
        return;
      }
    }
  }

  void Send(const char* buf, int size, int flag = 0) {
    send(sock_, buf, size, flag);
  }

 private:
  int sock_;
};

#endif  // __CLIENT_H__