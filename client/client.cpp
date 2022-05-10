#include "client.h"

#include <fstream>
#include <functional>

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
        case CMD::ACCEPTFILE: {
          this->_sfile_v.notify_one();
          break;
        }
        case CMD::SENDFILE: {
          _rfile_m.lock();
          RecvFile();
          memset(this->buf, 0, sizeof(this->buf));
          _rfile_m.unlock();
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
      std::cout << "4. sendfile" << std::endl;
      std::cout << "5. logout" << std::endl;
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
          SendFile();
          break;
        case 5:
          Logout();
          break;
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

void Client::SendFile() {
  int tid;
  std::cout << "Please input target id: ";
  std::cin >> tid;
  std::cout << "Please input file name: ";
  // TODO 文件绝对路径和相对路径转化
  std::string name;
  std::cin >> name;
  std::ifstream f;
  f.open(name, std::ios::in | std::ios::binary);
  f.seekg(0, f.end);
  int filesize = f.tellg();
  int curr = 0;
  char buff[1024];
  AcceptFilePacket akf;
  while (curr < filesize) {
    int size = (filesize > curr + 1024) ? 1024 : filesize - curr;
    f.seekg(curr, f.beg);
    f.read(buff, size);
    SendFilePacket pkg(_id, tid, name.c_str(), size, curr, buff);
    _sock->Send(&pkg, sizeof(pkg));
    _sfile_v.wait(_sfile_m);
    memcpy(&akf, buf, sizeof(akf));
    curr = akf.next_curr;
    std::cout << "\r" << name << ": " << (int)(((float)curr / filesize) * 100)
              << "%" << std::endl;
  }
  // 最后发送大小为 0 的报文作为结尾
  SendFilePacket fina(_id, tid, name.c_str(), 0, curr, buff);
  _sock->Send(&fina, sizeof(fina));
}

void Client::RecvFile() {
  SendFilePacket pkg;
  memcpy(&pkg, buf, sizeof(pkg));
  if (pkg.curr == 0 && pkg.datasize == 0) {
    AcceptFilePacket akf(pkg.id, pkg.filename, 0);
    _sock->Send(&akf, sizeof(akf));
    return;
  }
  if (pkg.datasize == 0) {
    std::string filename(pkg.filename);
    LOG_DEBUG("FILERECV get all file from id = %d, filename = %s", pkg.id,
              pkg.filename);
    std::string fn(pkg.filename, strlen(pkg.filename));
    fn += ".receive";
    rename(fn.c_str(), pkg.filename);
    return;

  } else {
    // 接收这一部分文件
    std::string filename(pkg.filename);
    filename += ".receive";
    std::ofstream f;
    f.open(filename, std::ios::app | std::ios::binary);
    f.seekp(pkg.curr, f.beg);
    f.write(pkg.data, pkg.datasize);
    f.close();
    // 接收完成返回当前进度
    AcceptFilePacket akf(pkg.id, pkg.filename, pkg.curr + pkg.datasize);
    _sock->Send(&akf, sizeof(akf));
  }
}