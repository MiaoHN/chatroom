#include "server.h"

Server::Server(const std::string& address, int port) {
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(port);
  addr_.sin_addr.s_addr = inet_addr(address.c_str());
  sock_ = socket(AF_INET, SOCK_STREAM, 0);
  socklen_t len = sizeof(addr_);
  bind(sock_, (sockaddr*)&addr_, len);
  listen(sock_, 32);
  epollfd_ = epoll_create(128);
  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = sock_;
  epoll_ctl(epollfd_, EPOLL_CTL_ADD, sock_, &ev);
}