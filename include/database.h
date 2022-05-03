#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <sqlite3.h>

#include <cstring>
#include <memory>
#include <mutex>

#include "log.h"

class Database {
 public:
  using ptr = std::shared_ptr<Database>;
  Database();
  Database(const std::string& name);

  ~Database();

  void Open(const std::string& name);

  void Close();

  void Exec(const std::string& query);

  void Query(const std::string& query);

  void error();

  const std::string operator[](int index);

  bool Readable();

  const std::string index(const std::string& col);
  const std::string operator[](const std::string& col);

 private:
  sqlite3* db_;
  char** result;
  char* errmsg;
  int nrow;
  int ncolumn;
  int idx_;
  bool isopen_;
  std::mutex mutex_;
};

#endif  // __DATABASE_H__