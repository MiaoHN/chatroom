#include "tcp_server.h"

TCPServer::TCPServer(const std::string& address, int port) {
  InitSocket(address, port);
}

void TCPServer::InitSocket(const std::string& address, int port) {
  _address = std::make_shared<Address>(address, port);
  _socket = std::make_shared<Socket>();
  _socket->Bind(_address);
  _socket->Listen(32);
  _socket->SetOpt(SO_REUSEADDR, 1);
}

void TCPServer::LinkHandler(EventHandler::ptr handler) {
  _handler = handler;
  InitEpoll();
}

void TCPServer::InitEpoll() {
  EventManager::ptr manager = std::make_shared<EventManager>(_handler);
  _epoller = std::make_unique<Epoller>();
  _epoller->AddManager(manager);
  _epoller->AddListener(_socket);
}