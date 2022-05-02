#include "util.h"

std::string getCurrentTime() {
  auto t =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::stringstream ss;
  ss << std::put_time(std::localtime(&t), "%F %T");
  return ss.str();
}