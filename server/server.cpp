#include "server.h"

#include <fstream>

void ServerHandler::handle(Event& event) {
  if (event.size == -1) {
    handle_disconnect(event);
    return;
  }
  PacketHeader header;
  memcpy(&header, event.buf, sizeof(header));
  if (header.magic != MAGIC) {
    LOG_ERROR("SERVERHANDLER::HANDLE error magic number: %d", header.magic);
    event.sock->Close();
    return;
  }
  switch (header.cmd) {
#define XX(name, func) \
  case CMD::name:      \
    return func(event)

    XX(ECHO, handle_echo);
    XX(LOGIN, handle_login);
    XX(LOGOUT, handle_logout);
    XX(REGISTER, handle_register);
#undef XX
    default:
      return;
  }
}

void ServerHandler::init_database() {
  std::ifstream fs;
  std::stringstream ss;
  fs.open(sql_path);
  char ch;
  while (ss && fs.get(ch)) ss.put(ch);

  _db->Exec(ss.str());
}

void ServerHandler::handle_disconnect(Event& event) {
  LOG_DEBUG("disconnect");
  // 关闭连接事件，将该socket对应用户状态置为0
  auto iter = _users.find(event.sock->GetSock());
  if (iter == _users.end()) {
    // 直接退出
    event.sock->Close();
    return;
  } else {
    std::stringstream ss;
    ss << "UPDATE muser SET status = 0, time = CURRENT_TIMESTAMP WHERE uid = "
       << iter->second << ";";
    _db->Exec(ss.str());
    ss.str("");
    LOG_INFO("uid=%d disconnected", iter->second);
    _users.erase(iter);
    return;
  }
}

void ServerHandler::handle_echo(Event& event) {
  EchoPacket pkt;
  memcpy(&pkt, event.buf, sizeof(pkt));
  LOG_INFO("receive echo packet: %s", pkt.msg);
  event.sock->Send(&pkt, sizeof(pkt), 0);
}

void ServerHandler::handle_register(Event& event) {
  LOG_DEBUG("here");
  RegisterPacket pkt;
  memcpy(&pkt, event.buf, sizeof(pkt));
  if (pkt.id != 0) {
    LOG_ERROR("id not null");
    event.sock->Close();
    return;
  }
  // 查找是否重名
  std::stringstream ss;
  ss << "SELECT uid FROM muser WHERE uname = '" << pkt.name << "';";
  _db->Query(ss.str());
  ss.str("");
  if (_db->Readable()) {
    // 有重名，返回错误
    pkt.id = -1;
    event.sock->Send(&pkt, sizeof(pkt));
    LOG_ERROR("REGISTER same username error");
    return;
  }
  ss << "INSERT INTO muser(uname, passwd) VALUES ('" << pkt.name << "', '"
     << pkt.passwd << "');";
  _db->Exec(ss.str());
  ss.str("");
  ss << "SELECT uid FROM muser WHERE uname = '" << pkt.name << "';";
  _db->Query(ss.str());
  if (_db->Readable()) {
    pkt.id = std::stoi(_db->index("uid"));
    event.sock->Send(&pkt, sizeof(pkt));
    LOG_INFO("REGISTER success, id=%d, username=%s", pkt.id, pkt.name);
  } else {
    LOG_ERROR("REGISTER error");
  }
}

void ServerHandler::handle_login(Event& event) {
  LOG_DEBUG("HANDLE_LOGIN");
  LoginPacket pkt;
  memcpy(&pkt, event.buf, sizeof(pkt));

  std::stringstream ss;
  ss << "SELECT uname, passwd, status FROM muser WHERE uid = " << pkt.id << ";";
  _db->Query(ss.str());
  ss.str("");
  if (_db->Readable()) {
    // 存在该用户，检查passwd
    if (strcmp(_db->index("passwd").c_str(), pkt.passwd)) {
      // 密码错误
      strcpy(pkt.name, "");
      event.sock->Send(&pkt, sizeof(pkt));
      LOG_ERROR("LOGIN passwd wrong! id=%d", pkt.id);
      return;
    } else {
      // 密码正确
      // 如果已经登陆则报错
      if (std::stoi(_db->index("status")) == 1) {
        strcpy(pkt.name, "");
        event.sock->Send(&pkt, sizeof(pkt));
        LOG_ERROR("LOGIN id=%d have login!", pkt.id);
        return;
      }
      // 返回 username
      strcpy(pkt.name, _db->index("uname").c_str());
      event.sock->Send(&pkt, sizeof(pkt));
      LOG_INFO("LOGIN uid=%d login!", pkt.id);
      // 改变登陆状态
      ss << "UPDATE muser SET status = 1, time = CURRENT_TIMESTAMP WHERE uid "
            "= "
         << pkt.id << ";";
      _db->Exec(ss.str());
      ss.str("");
      _users[event.sock->GetSock()] = pkt.id;
      return;
    }
  } else {
    // 该用户不存在
    strcpy(pkt.name, "");
    event.sock->Send(&pkt, sizeof(pkt));
    LOG_ERROR("LOGIN no such uid=%d", pkt.id);
    return;
  }
}

void ServerHandler::handle_logout(Event& event) {
  // TODO
}

void ServerHandler::LinkDatabase(Database::ptr db) {
  _db = db;
  init_database();
}