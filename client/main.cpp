#include "client.h"

int main(int argc, char const *argv[]) {
  Client client;
  client.Connect("127.0.0.1", 9999);
  client.Start();
  return 0;
}
