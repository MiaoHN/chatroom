#include "server.h"

void ServerHandler::handle(Event& event) {
  char buf[BUFSIZ];
  int size = event.size * sizeof(char);
  memcpy(buf, event.buf, size);
  buf[size] = '\0';
  LOG_INFO("receive: %s", buf);
  if (!strcmp(buf, "exit")) {
    return;
  }
  event.sock->Send(buf, size);
}

void ServerHandler::LinkDatabase(Database::ptr db) { _db = db; }