#include "server.h"

int main(int argc, char const *argv[]) {
  LOG_INIT("server.log");
  Server server("127.0.0.1", 9999);
  server.Start();
  return 0;
}
