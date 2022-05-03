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

  void Login() {
    std::string passwd;
    std::cout << "id: ";
    std::cin >> _id;
    std::cout << "passwd: ";
    std::cin >> passwd;
    LoginPacket pkt(_id, passwd);
    _sock->Send(&pkt, sizeof(pkt), 0);
    _sock->Recv(&pkt, sizeof(pkt), 0);
    if (!strcmp(pkt.name, "")) {
      LOG_ERROR("login false");
    } else {
      LOG_INFO("login successful, your username=%s", pkt.name);
      _islogin = true;
    }
  }

  void Logout() {
    if (!_islogin) {
      LOG_ERROR("is not login");
      return;
    }
    LogoutPacket pkt(_id);
    _sock->Send(&pkt, sizeof(pkt), 0);
    LOG_DEBUG("logout");

    return;
  }

  void Register() {
    std::string uname;
    std::string passwd;
    while (true) {
      std::cout << "username: ";
      std::cin >> uname;
      if (uname.size() > 32) {
        LOG_ERROR("Username'length should less than 32, but %d", uname.size());
        continue;
      }
      std::cout << "password: ";
      std::cin >> passwd;
      if (passwd.size() > 32) {
        LOG_ERROR("Passwd'length should less than 32, but %d", passwd.size());
        continue;
      } else {
        break;
      }
    }
    RegisterPacket pkt(uname, passwd);
    LOG_DEBUG("here");
    _sock->Send(&pkt, sizeof(pkt), 0);
    LOG_DEBUG("HERE");
    _sock->Recv(&pkt, sizeof(pkt), 0);
    if (pkt.id == -1) {
      LOG_ERROR("register false");
      return;
    } else {
      LOG_INFO("successfully register! your id=%d", pkt.id);
      return;
    }
  }

 private:
  Socket::ptr _sock;
  bool _islogin = false;
  int _id;
};

#endif  // __CLIENT_H__