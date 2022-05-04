#include "socket.h"

Socket::Socket(int sock) : _sock(sock) {}
Socket::Socket() {
  _sock = socket(AF_INET, SOCK_STREAM, 0);
  if (_sock == -1) {
    error("Socket");
  }
}

Socket::ptr Socket::Accept() {
  int sock = accept(_sock, nullptr, nullptr);
  return std::make_shared<Socket>(sock);
}

void Socket::Close() { close(_sock); }

void Socket::Bind(Address::ptr address) {
  int ret = bind(_sock, address->GetAddress(), address->GetLength());
  if (ret == -1) {
    error("Bind");
  }
}

int Socket::GetFd() const { return _sock; }

void Socket::Listen(int num) {
  int ret = listen(_sock, num);
  if (ret == -1) {
    error("Listen");
  }
}

void Socket::Connect(const std::string& addr, int port) {
  Address::ptr address(new Address(addr, port));
  Connect(address);
}

void Socket::Connect(Address::ptr address) {
  int ret = connect(_sock, address->GetAddress(), address->GetLength());
  if (ret == -1) {
    error("Connect");
  }
}

void Socket::SetOpt(int optname, int val) {
  int num = val;
  setsockopt(_sock, SOL_SOCKET, optname, &num, sizeof(num));
}

int Socket::Recv(void* buf, size_t size, int flag) {
  return ::recv(_sock, buf, size, flag);
}

int Socket::Send(const void* buf, size_t size, int flag) {
  return ::send(_sock, buf, size, flag);
}

void Socket::SetNoBlock() {
  int opts;
  opts = fcntl(_sock, F_GETFL);
  opts = opts | O_NONBLOCK;
  fcntl(_sock, F_SETFL, opts);
}

void Socket::error(const std::string& msg) {
  LOG_FATAL("SOCLET::%s %s", msg.c_str(), strerror(errno));
  exit(-1);
}