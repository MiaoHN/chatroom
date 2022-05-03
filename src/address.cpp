#include "address.h"

Address::Address(const std::string& ip, int port) {
  _addr.sin_family = AF_INET;
  _addr.sin_port = htons(port);
  _addr.sin_addr.s_addr = inet_addr(ip.c_str());
}

const sockaddr* Address::GetAddress() { return (sockaddr*)&_addr; }

socklen_t Address::GetLength() { return sizeof(_addr); }