#ifndef __SERVER_H__
#define __SERVER_H__

#include "chatroom.h"

class ServerHandler : public EventHandler {
 public:
  using ptr = std::shared_ptr<ServerHandler>;
  ServerHandler() {}
  ~ServerHandler() {}

  void handle(Event& event) override;

  void LinkDatabase(Database::ptr db);

 private:
  Database::ptr _db;
};

#endif  // __SERVER_H__