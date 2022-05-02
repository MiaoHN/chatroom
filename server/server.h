#ifndef __SERVER_H__
#define __SERVER_H__

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <map>
#include <memory>

#include "chatroom.h"

class Server {
 public:
  Server(const std::string& address, int port);
  void Start() {
    sockaddr_in cladr;
    socklen_t len = sizeof(cladr);
    while (true) {
      int nfds = epoll_wait(epollfd_, events, 1000, -1);
      if (nfds == -1) {
        LOG_ERROR("epoll_wait %s", strerror(errno));
        continue;
      }
      for (int n = 0; n < nfds; ++n) {
        if (events[n].data.fd == sock_) {
          int clientfd = accept(sock_, (struct sockaddr*)&cladr, &len);
          if (clientfd == -1) {
            LOG_ERROR("accept: %s", strerror(errno));
            continue;
          }
          setnonblocking(clientfd);
          struct epoll_event ev;
          ev.events = EPOLLIN | EPOLLET;
          ev.data.fd = clientfd;
          epoll_ctl(epollfd_, EPOLL_CTL_ADD, clientfd, &ev);
        } else {
          handle_client(events[n].data.fd);
        }
      }
    }
  }

 private:
  void handle_client(int sock) {
    char buf[512];
    int r = recv(sock, buf, 512, 0);
    if (!strcmp(buf, "exit")) {
      close(sock_);
      LOG_INFO("exit");
      return;
    }
    LOG_INFO("receive: %s", buf);
  }

  void setnonblocking(int sock) {
    int opts;
    opts = fcntl(sock, F_GETFL);
    opts = opts | O_NONBLOCK;
    fcntl(sock, F_SETFL, opts);
  }

 private:
  int sock_;
  int epollfd_;
  sockaddr_in addr_;
  std::unique_ptr<Database> db_;
  struct epoll_event events[1000];
};

#endif  // __SERVER_H__