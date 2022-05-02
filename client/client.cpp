#include "client.h"

Client::Client() { sock_ = socket(AF_INET, SOCK_STREAM, 0); }