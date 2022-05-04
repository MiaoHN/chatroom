#ifndef __PACLET_H__
#define __PACLET_H__

#include "config.h"
#include "log.h"

enum CMD {
  PLOGIN = 1,
  PLOGOUT = 2,
  PREGISTER = 3,
  PMSG = 4,  // 发送文件
  PFILE = 5,
  PQUERY = 6,  // 查询在线用户
};

struct PacketHeader {
  PacketHeader() {}
  PacketHeader(int _cmd, int _length) : cmd(_cmd), length(_length) {}
  unsigned int magic = MAGIC;
  int cmd;
  int length;
};

struct RegisterPacket : public PacketHeader {
  RegisterPacket() {}
  RegisterPacket(const std::string& _name, const std::string& _passwd) {
    if (_name.size() > 32 || _passwd.size() > 32) {
      LOG_FATAL("register");
    }
    cmd = PREGISTER;
    length = sizeof(RegisterPacket);
    id = 0;
    strcpy(name, _name.c_str());
    strcpy(passwd, _passwd.c_str());
  }
  int id;
  char name[32];
  char passwd[32];
};

struct LoginPacket : public PacketHeader {
  LoginPacket() {}
  LoginPacket(int _id, const std::string& _passwd) {
    cmd = PLOGIN;
    id = _id;
    memset(name, 0, sizeof(name));
    strcpy(passwd, _passwd.c_str());
    length = sizeof(LoginPacket);
  }
  int id;
  char name[32];
  char passwd[32];
};

struct LogoutPacket : public PacketHeader {
  LogoutPacket() {}
  LogoutPacket(int _id) {
    cmd = PLOGOUT;
    length = sizeof(LogoutPacket);
    id = _id;
  }
  int id;
};

struct MessagePacket : public PacketHeader {
  MessagePacket() { cmd = PMSG; }
  MessagePacket(int _id, int _type, int _tid, const std::string& _time,
                const std::string& _msg) {
    cmd = PMSG;
    id = _id;
    type = _type;
    tid = _tid;
    length = sizeof(MessagePacket);
    strcpy(time, _time.c_str());
    strcpy(msg, _msg.c_str());
  }
  int id;
  int type;
  int tid;
  char time[64];
  char uname[64];
  char msg[1024];
};

struct FilePacket : public PacketHeader {
  FilePacket() { cmd = PFILE; }
  FilePacket(int _uid, int _tid, const std::string& _filename, int _size,
             const std::string& _time, int _status, int _curr) {
    uid = _uid;
    tid = _tid;
    strcpy(filename, _filename.c_str());
    size = _size;
    length = sizeof(FilePacket);
    strcpy(time, _time.c_str());
    status = _status;
    curr = _curr;
  }
  int uid;
  int tid;
  char filename[64];
  int size;
  char time[64];
  int status;  // 0: 发送中，1: 接收中
  int curr;    // 待发送/接收位置
};

struct QueryPacket : public PacketHeader {
  QueryPacket() {}
  QueryPacket(int _id, const std::string& _data) {
    id = _id;
    cmd = PQUERY;
    length = sizeof(QueryPacket);
    strcpy(data, _data.c_str());
  }
  int id;
  char data[4096];
};

#endif  // __PACLET_H__