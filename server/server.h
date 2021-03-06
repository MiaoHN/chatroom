#ifndef __SERVER_H__
#define __SERVER_H__

#include <map>

#include "chatroom.h"

class ServerHandler : public EventHandler {
 public:
  using ptr = std::shared_ptr<ServerHandler>;
  ServerHandler() {}
  ~ServerHandler() {}

  void LinkDatabase(Database::ptr db);

  int GetLength(Event& event) override;

  void handle_read(Event& event) override;
  void handle_disconnect(Event& event) override;

 private:
  /**
   * @brief 初始化数据库，如果数据库已存在则读取数据库信息
   *
   */
  void init_database();

  void handle_register(Event& event);
  void handle_login(Event& event);
  void handle_logout(Event& event);
  void handle_query(Event& event);
  void handle_message(Event& event);
  void handle_sendfile(Event& event);
  void handle_recvfile(Event& event);

 private:
  Database::ptr _db;
  std::map<int, User::ptr> _users;  // 在线的用户
};

#endif  // __SERVER_H__