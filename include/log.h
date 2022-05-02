#ifndef __LOG_H__
#define __LOG_H__

#include <cstdarg>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

#include "util.h"

#define LOG_INIT(name) Logger::GetInstance(name);
#define LOG_INFO(fmt, args...) \
  Logger::GetInstance()->log(LogLevel::INFO, fmt, ##args)
#define LOG_DEBUG(fmt, args...) \
  Logger::GetInstance()->log(LogLevel::DEBUG, fmt, ##args)
#define LOG_WARN(fmt, args...) \
  Logger::GetInstance()->log(LogLevel::WARN, fmt, ##args)
#define LOG_ERROR(fmt, args...) \
  Logger::GetInstance()->log(LogLevel::ERROR, fmt, ##args)
#define LOG_FATAL(fmt, args...) \
  Logger::GetInstance()->log(LogLevel::FATAL, fmt, ##args)

class LogLevel {
 public:
  enum Level {
    INFO = 0,
    DEBUG = 1,
    WARN = 2,
    ERROR = 3,
    FATAL = 4,
  };

  static std::string ToString(LogLevel::Level level) {
    switch (level) {
      case LogLevel::INFO:
        return "INFO";
      case LogLevel::DEBUG:
        return "DEBUG";
      case LogLevel::WARN:
        return "WARN";
      case LogLevel::ERROR:
        return "ERROR";
      case LogLevel::FATAL:
        return "FATAL";
      default:
        return "UNKNOWN";
    }
  }
};

class Logger {
 public:
  using ptr = std::shared_ptr<Logger>;
  Logger(const std::string name) : name_(name) {}

 public:
  static Logger::ptr GetInstance(const std::string& name = "log.log") {
    static Logger::ptr s_logger(new Logger(name));
    return s_logger;
  }

 public:
  void log(LogLevel::Level level, const char* fmt, ...) {
    va_list al;
    va_start(al, fmt);
    log(level, fmt, al);
    va_end(al);
  }

 private:
  void log(LogLevel::Level level, const char* fmt, va_list al) {
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if (len != -1) {
      std::stringstream ss;
      ss << "[" << LogLevel::ToString(level) << "]\t" << getCurrentTime() << " "
         << buf;
      mutex_.lock();
      std::cout << ss.str() << std::endl;
      mutex_.unlock();
      free(buf);
    }
  }

 private:
  std::string name_;  // 日志名称
  std::mutex mutex_;  // 线程锁
};

#endif  // __LOG_H__