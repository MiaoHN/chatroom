#include "client.h"

Client::Client() { _sock = std::make_shared<Socket>(); }

void Client::Connect(const std::string& addr, int port) {
  _sock->Connect(addr, port);
}

void Client::Start() {
  while (true) {
    std::string str;
    getline(std::cin, str);
    char buf[BUFSIZ];
    memset(buf, 0, sizeof(buf));
    memcpy(buf, &MAGIC, sizeof(unsigned int));
    int len = str.size();
    memcpy(buf + sizeof(unsigned int), &len, sizeof(int));
    strcpy(buf + sizeof(unsigned int) + sizeof(int), str.c_str());
    _sock->Send(buf, len + sizeof(unsigned int) + sizeof(int));
    if (str == "exit") {
      return;
    }
    char bf[BUFSIZ];
    int l = _sock->Recv(bf, sizeof(bf), 0);
    if (l < 0) {
      LOG_FATAL("recv false");
      exit(-1);
    }
    char st[BUFSIZ];
    memcpy(st, bf, l * sizeof(char));
    st[l] = '\0';
    LOG_INFO("echo: %s", st);
  }
}