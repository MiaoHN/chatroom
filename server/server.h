#ifndef __SERVER_H__
#define __SERVER_H__

#include "chatroom.h"

class Server {
 public:
  Server();

 private:
  int sock_;
};

#endif  // __SERVER_H__