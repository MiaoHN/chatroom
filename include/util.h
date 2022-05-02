#ifndef __UTIL_H__
#define __UTIL_H__

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

std::string getCurrentTime() {
  auto t =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::stringstream ss;
  ss << std::put_time(std::localtime(&t), "%F %T");
  return ss.str();
}

#endif  // __UTIL_H__