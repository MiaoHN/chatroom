#include "client.h"

Client::Client() { _sock = std::make_shared<Socket>(); }

void Client::Connect(const std::string& addr, int port) {
  _sock->Connect(addr, port);
  _getpkt = std::make_shared<std::thread>([this]() {
    while (true) {
      // 接收消息
      _sock->Recv(&this->buf, BUFSIZ, 0);
      // 解析packet类型
      PacketHeader header;
      memcpy(&header, this->buf, sizeof(header));
      switch (header.cmd) {
        case CMD::PLOGIN: {
          this->_login_v.notify_one();
          break;
        }
        case CMD::PREGISTER: {
          this->_regis_v.notify_one();
          break;
        }
        case CMD::PQUERY: {
          this->_query_v.notify_one();
          break;
        }
        case CMD::PMSG: {
          MessagePacket pkt;
          memcpy(&pkt, this->buf, sizeof(pkt));
          int sid = pkt.id;
          int type = pkt.type;
          std::string uname(pkt.uname, strlen(pkt.uname));
          std::string mtime(pkt.time, strlen(pkt.time));
          std::string msg(pkt.msg, strlen(pkt.msg));
          if (type == 1) {
            std::cout << "[GLOBAL | " << sid << " | " << uname << "] [" << mtime
                      << "]: " << msg << std::endl;
          } else {
            std::cout << "[ " << sid << " | " << uname << "] [" << mtime
                      << "]: " << msg << std::endl;
          }
          break;
        }
        case CMD::PFILE: {
          // TODO
          break;
        }
        default:
          break;
      }
    }
  });
}

void Client::Start() {
  while (true) {
    if (!_islogin) {
      std::cout << "1. login" << std::endl;
      std::cout << "2. register" << std::endl;
      std::cout << "0. exit" << std::endl;
      int num;
      std::cin >> num;
      switch (num) {
        case 0:
          exit(0);
        case 1:
          Login();
          break;
        case 2:
          Register();
          break;
        case 3:
          return;
        default:
          break;
      }
    } else {
      std::cout << "1. query" << std::endl;
      std::cout << "2. sendmsg" << std::endl;
      std::cout << "3. broadcast" << std::endl;
      std::cout << "4. logout" << std::endl;
      std::cout << "0. exit" << std::endl;
      int num;
      std::cin >> num;
      switch (num) {
        case 0:
          exit(0);
        case 1:
          Query();
          break;
        case 2:
          SendMessage();
          break;
        case 3:
          Broadcast();
          break;
        case 4:
          Logout();
          break;
        case 5:
          return;
        default:
          break;
      }
    }
  }
}

void Client::SendMessage() {
  std::cout << "uid: ";
  int tid;
  std::cin >> tid;
  std::string msg;
  std::cout << "msg: ";
  std::cin.clear();
  std::cin.ignore();
  std::getline(std::cin, msg);
  MessagePacket pkt(_id, 0, tid, "", msg);
  _sock->Send(&pkt, pkt.length, 0);
}

void Client::Broadcast() {
  std::string msg;
  std::cout << "msg: ";
  std::cin.clear();
  std::cin.ignore();
  std::getline(std::cin, msg);
  MessagePacket pkt(_id, 1, 0, "", msg);
  _sock->Send(&pkt, pkt.length, 0);
}

void Client::Login() {
  std::string passwd;
  std::cout << "id: ";
  std::cin >> _id;
  std::cout << "passwd: ";
  std::cin >> passwd;
  LoginPacket pkt(_id, passwd);
  _sock->Send(&pkt, pkt.length, 0);
  _login_v.wait(_login_m);
  memcpy(&pkt, buf, sizeof(pkt));
  if (!strcmp(pkt.name, "")) {
    LOG_ERROR("login false");
  } else {
    LOG_INFO("login successful, your username=%s", pkt.name);
    _islogin = true;
  }
}

void Client::Logout() {
  if (!_islogin) {
    LOG_ERROR("is not login");
    return;
  }
  LogoutPacket pkt(_id);
  _sock->Send(&pkt, pkt.length, 0);
  LOG_DEBUG("logout");
  _islogin = false;

  return;
}

void Client::Query() {
  if (!_islogin) {
    LOG_ERROR("is not login");
    return;
  }
  QueryPacket pkt(_id, ".");
  _sock->Send(&pkt, pkt.length, 0);
  LOG_DEBUG("QUERY send pkt");
  _query_v.wait(_query_m);
  memcpy(&pkt, buf, sizeof(pkt));
  LOG_DEBUG("QUERY recv pkt size=%d", strlen(pkt.data));
  std::cout << pkt.data;
}

void Client::Register() {
  std::string uname;
  std::string passwd;
  while (true) {
    std::cout << "username: ";
    std::cin >> uname;
    if (uname.size() > 32) {
      LOG_ERROR("Username'length should less than 32, but %d", uname.size());
      continue;
    }
    std::cout << "password: ";
    std::cin >> passwd;
    if (passwd.size() > 32) {
      LOG_ERROR("Passwd'length should less than 32, but %d", passwd.size());
      continue;
    } else {
      break;
    }
  }
  RegisterPacket pkt(uname, passwd);
  LOG_DEBUG("here");
  _sock->Send(&pkt, pkt.length, 0);
  LOG_DEBUG("HERE");
  _regis_v.wait(_regis_m);
  memcpy(&pkt, buf, sizeof(pkt));
  if (pkt.id == -1) {
    LOG_ERROR("register false");
    return;
  } else {
    LOG_INFO("successfully register! your id=%d", pkt.id);
    return;
  }
}