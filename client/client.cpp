#include "client.h"

Client::Client() { _sock = std::make_shared<Socket>(); }

void Client::Connect(const std::string& addr, int port) {
  _sock->Connect(addr, port);
}

void Client::Start() {
  while (true) {
    Login();
    Logout();
  }
}

void test_epoll() {
  // while (true) {
  //   std::string str;
  //   getline(std::cin, str);
  //   EchoPacket pkt(str);
  //   _sock->Send(&pkt, sizeof(pkt), 0);
  //   if (str == "exit") {
  //     return;
  //   }
  //   char bf[BUFSIZ];
  //   int l = _sock->Recv(&pkt, sizeof(pkt), 0);
  //   if (l < 0) {
  //     LOG_FATAL("recv false");
  //     exit(-1);
  //   }
  //   LOG_INFO("echo: %s", pkt.msg);
  // }
}