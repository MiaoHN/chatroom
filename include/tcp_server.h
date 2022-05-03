#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include <memory>
#include <string>

#include "epoll.h"
#include "event_manager.h"

class TCPServer {
 public:
  using ptr = std::unique_ptr<TCPServer>;

  TCPServer(const std::string& address, int port);

  void LinkHandler(EventHandler::ptr handler);

  void Start() { _epoller->Start(); }

 private:
  void InitSocket(const std::string& address, int port);
  void InitEpoll();

 private:
  Socket::ptr _socket;
  Address::ptr _address;
  EventHandler::ptr _handler;
  Epoller::ptr _epoller;
};

#endif  // __TCP_SERVER_H__