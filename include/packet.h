#ifndef __PACLET_H__
#define __PACLET_H__

#include "config.h"
#include "log.h"

enum CMD {
  ECHO = 0,  // for debug
  LOGIN = 1,
  LOGOUT = 2,
  REGISTER = 3,
  FALSE = 4,
  OK = 5,
};

struct PacketHeader {
  PacketHeader() {}
  PacketHeader(int _cmd, int _length) : cmd(_cmd), length(_length) {}
  unsigned int magic = MAGIC;
  int cmd;
  int length;
};

struct EchoPacket : public PacketHeader {
  EchoPacket() {}
  EchoPacket(const std::string& _msg) : PacketHeader(ECHO, _msg.size()) {
    strcpy(msg, _msg.c_str());
  }

  char msg[1024];
};

struct RegisterPacket : public PacketHeader {
  RegisterPacket() {}
  RegisterPacket(const std::string& _name, const std::string& _passwd) {
    if (_name.size() > 32 || _passwd.size() > 32) {
      LOG_FATAL("register");
    }
    cmd = REGISTER;
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
    cmd = LOGIN;
    id = _id;
    memset(name, 0, sizeof(name));
    strcpy(passwd, _passwd.c_str());
  }
  int id;
  char name[32];
  char passwd[32];
};

struct LogoutPacket : public PacketHeader {
  LogoutPacket() {}
  LogoutPacket(int _id) {
    cmd = LOGOUT;
    id = _id;
  }
  int id;
};

#endif  // __PACLET_H__