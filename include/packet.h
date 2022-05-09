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
  SENDFILE = 7,
  ACCEPTFILE = 8,
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

struct SendFilePacket : public PacketHeader {
  SendFilePacket() {}
  SendFilePacket(int _id, int _tid, const char* _filename, int _datasize,
                 int _curr, char* buff) {
    cmd = CMD::SENDFILE;
    length = sizeof(SendFilePacket);
    id = _id;
    tid = _tid;
    strcpy(filename, _filename);
    datasize = _datasize;
    curr = _curr;
    memcpy(data, buff, _datasize);
  }
  int id;
  int tid;
  char filename[32];
  int filesize;
  int datasize;  // data size in single packet, if
  int curr;      // position in send file
  char data[1024];
};

struct AcceptFilePacket : public PacketHeader {
  AcceptFilePacket() {}
  AcceptFilePacket(int _id, const char* _filename, int _next_curr) {
    cmd = CMD::ACCEPTFILE;
    length = sizeof(AcceptFilePacket);
    id = _id;
    next_curr = _next_curr;
    strcpy(filename, _filename);
  }
  char filename[32];
  int filesize;
  int id;
  int next_curr;
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