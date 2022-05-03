#include "server.h"

int main(int argc, char const *argv[]) {
  LOG_INIT("server.log");
  TCPServer::ptr server(new TCPServer("127.0.0.1", 9999));
  Database::ptr db(new Database("server.sqlite"));
  ServerHandler::ptr handler = std::make_shared<ServerHandler>();
  handler->LinkDatabase(db);
  server->LinkHandler(handler);
  server->Start();
  return 0;
}
