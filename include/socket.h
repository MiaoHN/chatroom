#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <memory>

#include "address.h"
#include "log.h"

class Socket {
 public:
  using ptr = std::shared_ptr<Socket>;

  Socket(int sock);
  Socket();

  void Bind(Address::ptr address);
  void Listen(int num);

  void Connect(const std::string& addr, int port);
  void Connect(Address::ptr address);

  int Recv(void* buf, size_t size, int flag = 0);
  int Send(const void* buf, size_t size, int flag = 0);

  int GetSock() const;

  void SetOpt(int optname, int val);
  void SetNoBlock();

  void Close();

 private:
  void error(const std::string& msg);

 private:
  int _sock;
};

#endif  // __SOCKET_H__