#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <condition_variable>
#include <mutex>

#include "chatroom.h"

class Client {
 public:
  Client();

  void Connect(const std::string& addr, int port);

  void Start();

 private:
  void SendMessage();
  void Broadcast();
  void Login();
  void Logout();
  void Query();
  void Register();

 private:
  Socket::ptr _sock;
  bool _islogin = false;
  int _id;
  char buf[BUFSIZ];
  std::shared_ptr<std::thread> _getpkt;
  std::mutex _query_m;
  std::mutex _login_m;
  std::mutex _regis_m;
  std::condition_variable_any _query_v;
  std::condition_variable_any _login_v;
  std::condition_variable_any _regis_v;
};

#endif  // __CLIENT_H__