#ifndef __SERVER_H__
#define __SERVER_H__

#include <map>

#include "chatroom.h"

const std::string sql_path =
    "/home/miaohn/codes/Chatroom.bak/sql/create_table.sql";

class ServerHandler : public EventHandler {
 public:
  using ptr = std::shared_ptr<ServerHandler>;
  ServerHandler() {}
  ~ServerHandler() {}

  void handle(Event& event) override;

  void LinkDatabase(Database::ptr db);

 private:
  void init_database();
  void handle_disconnect(Event& event);
  void handle_echo(Event& event);
  void handle_register(Event& event);
  void handle_login(Event& event);
  void handle_logout(Event& event);

 private:
  Database::ptr _db;
  std::map<int, int> _users;  // <sockfd, id>
};

#endif  // __SERVER_H__