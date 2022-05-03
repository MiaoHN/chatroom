#include "epoll.h"

#include "packet.h"

Epoller::Epoller(int num) {
  _epollfd = epoll_create(num);
  _magic = MAGIC;
}

void Epoller::AddManager(EventManager::ptr manager) { _manager = manager; }

void Epoller::AddListener(Socket::ptr sock) {
  _listenfd = sock->GetSock();
  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = _listenfd;
  epoll_ctl(_epollfd, EPOLL_CTL_ADD, _listenfd, &ev);
}

void Epoller::Start() {
  while (true) {
    int nfds = epoll_wait(_epollfd, events, 1000, -1);
    if (nfds == -1) {
      LOG_ERROR("EPOLLER::Start %s", strerror(errno));
      continue;
    }
    for (int n = 0; n < nfds; ++n) {
      if (events[n].data.fd == _listenfd) {
        handle_accept();
      } else {
        handle_read(events[n].data.fd);
      }
    }
  }
}

void Epoller::handle_accept() {
  sockaddr_in cladr;
  socklen_t len = sizeof(cladr);
  int clientfd = accept(_listenfd, (struct sockaddr*)&cladr, &len);
  if (clientfd == -1) {
    LOG_ERROR("accept: %s", strerror(errno));
    return;
  }
  setnonblocking(clientfd);
  struct epoll_event ev;
  ev.events = EPOLLIN | EPOLLET;
  ev.data.fd = clientfd;
  epoll_ctl(_epollfd, EPOLL_CTL_ADD, clientfd, &ev);
  LOG_DEBUG("EPOLLER::handle_accept accept a connection");
}

void Epoller::handle_read(int sock) {
  Event eve;
  Socket::ptr s(new Socket(sock));
  char buf[BUFSIZ];
  int ret = s->Recv(eve.buf, sizeof(eve.buf), 0);
  if (ret == 0) {
    eve.size = -1;
    eve.sock = s;
    _manager->Add(eve);
    return;
  } else if (ret == -1) {
    LOG_ERROR("recv error");
    return;
  }
  eve.sock = s;
  eve.size = ret;
  _manager->Add(eve);
}

void Epoller::setnonblocking(int sock) {
  int opts;
  opts = fcntl(sock, F_GETFL);
  opts = opts | O_NONBLOCK;
  fcntl(sock, F_SETFL, opts);
}