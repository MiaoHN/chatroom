#include "client.h"

int main(int argc, char const *argv[]) {
  Client client;
  client.Connect(SERVER_O_IP, PORT);
  client.Start();
  return 0;
}
