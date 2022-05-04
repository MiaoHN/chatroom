#ifndef __USER_H__
#define __USER_H__

#include <memory>
#include <queue>

#include "socket.h"

class User {
 public:
  using ptr = std::shared_ptr<User>;
  User() {}
  User(int id, Socket::ptr sock = nullptr) : _id(id), _sock(sock) {}

  int GetId() const { return _id; }

  void SetSocket(Socket::ptr sock) { _sock = sock; }
  Socket::ptr GetSocket() const { return _sock; }

 private:
  int _id;
  std::string _name;
  Socket::ptr _sock;
};

#endif  // __USER_H__