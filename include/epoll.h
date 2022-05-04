#ifndef __EPOLL_H__
#define __EPOLL_H__

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <memory>

#include "config.h"
#include "event_manager.h"
#include "log.h"
#include "socket.h"
#include "uncopyable.h"

class Epoller : public Uncopyable {
 public:
  using ptr = std::unique_ptr<Epoller>;
  Epoller(int num = 32);

  void AddManager(EventManager::ptr manager);
  void AddListener(Socket::ptr sock);

  void Start();

 private:
  void handle_accept();
  void handle_read(int sock);
  void setnonblocking(int sock);

 private:
  int _epollfd;
  Socket::ptr _listensock;
  struct epoll_event events[1000];
  EventManager::ptr _manager;
};

#endif  // __EPOLL_H__