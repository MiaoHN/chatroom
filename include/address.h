#ifndef __ADDRESS_H__
#define __ADDRESS_H__

#include <arpa/inet.h>
#include <netinet/in.h>

#include <memory>
#include <string>

class Address {
 public:
  using ptr = std::shared_ptr<Address>;

  Address(const std::string& ip, int port);

  const sockaddr* GetAddress();

  socklen_t GetLength();

 private:
  struct sockaddr_in _addr;
};

#endif  // __ADDRESS_H__