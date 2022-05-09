#include "server.h"

#include <fstream>

int ServerHandler::GetLength(Event& event) {
  PacketHeader header;
  memcpy(&header, event.buf, sizeof(header));
  if (header.magic != MAGIC) return -1;
  return header.length;
}

void ServerHandler::handle_read(Event& event) {
  PacketHeader header;
  memcpy(&header, event.buf, sizeof(header));
  switch (header.cmd) {
#define XX(name, func) \
  case CMD::name:      \
    return func(event)

    XX(PLOGIN, handle_login);
    XX(PLOGOUT, handle_logout);
    XX(PREGISTER, handle_register);
    XX(PMSG, handle_message);
    XX(PQUERY, handle_query);
    XX(SENDFILE, handle_sendfile);
    XX(ACCEPTFILE, handle_recvfile);
#undef XX
    default:
      return;
  }
}

void ServerHandler::init_database() {
  std::ifstream fs;
  std::stringstream ss;
  fs.open(SQL_PATH);
  char ch;
  while (ss && fs.get(ch)) ss.put(ch);
  _db->Exec(ss.str());
}

void ServerHandler::handle_disconnect(Event& event) {
  LOG_DEBUG("disconnect");
  // 关闭连接事件，将该socket对应用户状态置为0
  for (auto& item : _users) {
    if (item.second->GetSocket()->GetFd() == event.sock->GetFd()) {
      // 找到该用户
      std::stringstream ss;
      ss << "UPDATE muser SET status = 0, time = CURRENT_TIMESTAMP WHERE uid = "
         << item.first << ";";
      _db->Exec(ss.str());
      ss.str("");
      LOG_INFO("uid=%d disconnected", item.first);
      _users.erase(item.first);
      event.sock->Close();
      return;
    }
  }
  // 直接退出
  event.sock->Close();
  return;
}

void ServerHandler::handle_register(Event& event) {
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
    event.sock->Send(&pkt, pkt.length);
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
    event.sock->Send(&pkt, pkt.length);
    LOG_INFO("REGISTER success, id=%d, username=%s", pkt.id, pkt.name);
  } else {
    LOG_ERROR("REGISTER error");
  }
}

void ServerHandler::handle_login(Event& event) {
  LOG_DEBUG("HANDLE_LOGIN");
  LoginPacket pkt;
  memcpy(&pkt, event.buf, event.size);

  std::stringstream ss;
  ss << "SELECT uname, passwd, status FROM muser WHERE uid = " << pkt.id << ";";
  _db->Query(ss.str());
  ss.str("");
  if (_db->Readable()) {
    // 存在该用户，检查passwd
    if (strcmp(_db->index("passwd").c_str(), pkt.passwd)) {
      // 密码错误
      strcpy(pkt.name, "");
      event.sock->Send(&pkt, pkt.length);
      LOG_ERROR("LOGIN passwd wrong! id=%d", pkt.id);
      return;
    } else {
      // 密码正确
      // 如果已经登陆则报错
      if (std::stoi(_db->index("status")) == 1) {
        strcpy(pkt.name, "");
        event.sock->Send(&pkt, pkt.length);
        LOG_ERROR("LOGIN id=%d have login!", pkt.id);
        return;
      }
      // 返回 username
      strcpy(pkt.name, _db->index("uname").c_str());
      event.sock->Send(&pkt, pkt.length);
      LOG_INFO("LOGIN uid=%d login!", pkt.id);
      // 改变登陆状态
      ss << "UPDATE muser SET status = 1, time = CURRENT_TIMESTAMP WHERE uid "
            "= "
         << pkt.id << ";";
      _db->Exec(ss.str());
      ss.str("");
      _users[pkt.id] = std::make_shared<User>(pkt.id, event.sock);
      // 发送未读消息
      std::stringstream ss;
      ss << "SELECT * FROM mmessage WHERE tid = " << pkt.id << ";";
      _db->Query(ss.str());
      ss.str("");
      while (_db->Readable()) {
        int uid = std::stoi(_db->index("uid"));
        int type = std::stoi(_db->index("type"));
        int tid = std::stoi(_db->index("tid"));
        std::string msg = _db->index("msg");
        std::string mtime = _db->index("time");
        MessagePacket msgpkt(uid, type, tid, mtime, msg);
        _users[pkt.id]->GetSocket()->Send(&msgpkt, msgpkt.length, 0);
      }
      // 删除未读消息
      ss << "DELETE FROM mmessage WHERE tid = " << pkt.id << ";";
      _db->Exec(ss.str());
      ss.str("");
      return;
    }
  } else {
    // 该用户不存在
    strcpy(pkt.name, "");
    event.sock->Send(&pkt, pkt.length);
    LOG_ERROR("LOGIN no such uid=%d", pkt.id);
    return;
  }
}

void ServerHandler::handle_logout(Event& event) {
  // 更新数据库，删除_user中的值，关闭socket
  LogoutPacket pkt;
  memcpy(&pkt, event.buf, sizeof(pkt));
  if (_users.find(pkt.id) == _users.end()) {
    // 没找到
    LOG_ERROR("no such uid=%d", pkt.id);
  } else {
    _users.erase(pkt.id);
    std::stringstream ss;
    ss << "UPDATE muser SET status = 0, time = CURRENT_TIMESTAMP WHERE uid = "
       << pkt.id << ";";
    _db->Exec(ss.str());
    ss.str("");
    LOG_INFO("uid=%d logout", pkt.id);
    return;
  }
}

void ServerHandler::handle_query(Event& event) {
  LOG_DEBUG("HANDLE_QUERY");
  QueryPacket pkt;
  memcpy(&pkt, event.buf, sizeof(pkt));
  std::stringstream re;
  for (auto& item : _users) {
    int id = item.first;
    std::stringstream ss;
    ss << "SELECT uname, status FROM muser WHERE uid = " << id << ";";
    _db->Query(ss.str());
    ss.str("");
    if (_db->Readable()) {
      std::string uname = _db->index("uname");
      int status = std::stoi(_db->index("status"));
      std::string statu = status == 0 ? "offline" : "online";
      re << "uname: " << uname << "\tid: " << id << "\tstatus: " << statu
         << std::endl;
    }
  }
  strcpy(pkt.data, re.str().c_str());
  event.sock->Send(&pkt, pkt.length, 0);
  LOG_DEBUG("HANDLE_QUERY uid=%d start a query", pkt.id);
}

void ServerHandler::handle_message(Event& event) {
  MessagePacket pkt;
  memcpy(&pkt, event.buf, sizeof(pkt));
  strcpy(pkt.time, getCurrentTime().c_str());
  std::stringstream ss;
  ss << "SELECT uname FROM muser WHERE uid = " << pkt.id << ";";
  _db->Query(ss.str());
  if (_db->Readable()) {
    std::string n = _db->index("uname");
    memcpy(&pkt.uname, n.c_str(), n.size());
    pkt.uname[n.size()] = '\0';
  }
  if (pkt.type == 1) {
    // 广播
    for (auto& key : _users) {
      if (key.first == pkt.id) {
        continue;
      }
      key.second->GetSocket()->Send(&pkt, pkt.length);
    }
    LOG_DEBUG("HANDLE_MESSAGE uid=%d broadcast len=%d send to users online",
              pkt.id, strlen(pkt.msg));
    // 数据库中存 boardcast 信息
    std::stringstream ss;
    ss << "SELECT uid FROM muser WHERE status = 0;";
    _db->Query(ss.str());
    while (_db->Readable()) {
      int tid = std::stoi(_db->index("uid"));
      // 添加到 mmessage 中
      int uid = pkt.id;
      int type = pkt.type;
      std::stringstream ss;
      ss << "INSERT INTO mmessage(uid, type, tid, msg) VALUES (" << uid << ","
         << type << "," << tid << ", '" << pkt.msg << "');";
      _db->Exec(ss.str());
      ss.str("");
    }
  } else {
    // 私信
    if (_users.find(pkt.tid) == _users.end()) {
      // 在线用户中找不到该用户
      std::stringstream ss;
      ss << "SELECT * FROM muser WHERE uid = " << pkt.tid << ";";
      _db->Query(ss.str());
      ss.str("");
      if (!_db->Readable()) {
        // 用户不存在
        LOG_ERROR("HANDLE_MESSAGE no uid=%d", pkt.id);
        return;
      } else {
        // 用户不在线，将消息存到数据库
        std::stringstream ss;
        ss << "INSERT INTO mmessage(uid, type,tid, msg) VALUES (" << pkt.id
           << ", " << pkt.type << "," << pkt.tid << ",'" << pkt.msg << "');";
        _db->Exec(ss.str());
        ss.str("");
        LOG_DEBUG("HANDLE_MESSAGE uid=%d tid=%d len=%d isn't online", pkt.id,
                  pkt.tid, strlen(pkt.msg));
      }
    } else {
      // 找到在线用户直接发送
      // 用户在线，直接发送
      _users[pkt.tid]->GetSocket()->Send(&pkt, pkt.length);
      LOG_DEBUG("HANDLE_MESSAGE uid=%d tid=%d len=%d send success", pkt.id,
                pkt.tid, strlen(pkt.msg));
    }
  }
}

void ServerHandler::handle_sendfile(Event& event) {
  SendFilePacket pkg;
  memcpy(&pkg, event.buf, sizeof(pkg));
  if (pkg.datasize == 0) {
    // 文件已经全部接收完成，发送到目标客户端
    std::string filename(pkg.filename);
    LOG_DEBUG("FILERECV get all file from id = %d, filename = %s", pkg.id,
              pkg.filename);
    filename += ".server";
    // TODO 等待目标客户端上线，这里假设客户端已经做好了接收文件的准备
    auto iter = _users.find(pkg.tid);
    if (iter != _users.end()) {
      auto user = _users[pkg.tid];
      std::ifstream sf;
      sf.open(filename, std::ios::in | std::ios::binary);
      sf.seekg(0, sf.end);
      int filesize = sf.tellg();
      int curr = 0;
      char buff[1024];
      AcceptFilePacket akf;
      SendFilePacket pkt(pkg.id, pkg.tid, pkg.filename, 0, 0, nullptr);
      iter->second->GetSocket()->Send(&pkt, sizeof(pkt));
    }
  } else {
    // 接收这一部分文件
    std::string filename(pkg.filename);
    filename += ".server";
    std::ofstream f;
    f.open(filename, std::ios::app | std::ios::binary);
    f.seekp(pkg.curr, f.beg);
    f.write(pkg.data, pkg.datasize);
    f.close();
    // 接收完成返回当前进度
    AcceptFilePacket akf(pkg.id, pkg.filename, pkg.curr + pkg.datasize);
    event.sock->Send(&akf, sizeof(akf));
  }
}

void ServerHandler::handle_recvfile(Event& event) {
  char buff[1024];
  AcceptFilePacket pkg;
  memcpy(&pkg, event.buf, sizeof(pkg));
  std::string fname(pkg.filename, strlen(pkg.filename));
  std::string filename(fname);
  filename += ".server";
  std::ifstream f;
  f.open(filename, std::ios::in | std::ios::binary);
  f.seekg(0, f.end);
  long long filesize = f.tellg();
  if (filesize == -1) return;
  int curr = pkg.next_curr;
  if (curr == filesize) {
    // 最后发送大小为 0 的报文作为结尾
    SendFilePacket fina(pkg.id, 0, pkg.filename, 0, curr, buff);
    event.sock->Send(&fina, sizeof(fina));
    LOG_DEBUG("Succssfully send file to id=%d filename=%s", pkg.id,
              pkg.filename);
    remove(filename.c_str());
    return;
  }
  int size = (filesize > curr + 1024) ? 1024 : filesize - curr;
  f.seekg(curr, f.beg);
  f.read(buff, size);
  f.close();
  LOG_DEBUG("HANDLE_RECV fname=%s size=%d curr=%d", pkg.filename, size, curr);
  SendFilePacket p(pkg.id, 0, pkg.filename, size, curr, buff);
  event.sock->Send(&p, sizeof(p));
}

void ServerHandler::LinkDatabase(Database::ptr db) {
  _db = db;
  init_database();
}