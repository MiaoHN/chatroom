#include "server.h"

int main(int argc, char const *argv[]) {
  LOG_INIT("server.log");
  TCPServer::ptr server(new TCPServer(SERVER_I_IP, PORT));
  Database::ptr db(new Database("server.sqlite"));
  ServerHandler::ptr handler = std::make_shared<ServerHandler>();
  handler->LinkDatabase(db);
  server->LinkHandler(handler);
  server->Start();
  return 0;
}
