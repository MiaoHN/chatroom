#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <sqlite3.h>

#include <cstring>
#include <memory>
#include <mutex>

#include "log.h"

class Database {
 public:
  using ptr = std::unique_ptr<Database>;
  Database() { isopen_ = false; }
  Database(const std::string& name) { Open(name); }

  ~Database() {
    if (isopen_) {
      Close();
    }
  }

  void Open(const std::string& name) {
    sqlite3_open(name.c_str(), &db_);
    isopen_ = true;
  }

  void Close() {
    if (isopen_) {
      sqlite3_close(db_);
    }
  }

  void Exec(const std::string& query) {
    if (!isopen_) {
      LOG_ERROR("database is not open");
      return;
    }
    int rc = sqlite3_exec(db_, query.c_str(), nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
      error();
    }
  }

  void Query(const std::string& query) {
    // if (result != nullptr) {
    //   sqlite3_free_table(result);
    // }
    mutex_.lock();
    int rc = sqlite3_get_table(db_, query.c_str(), &result, &nrow, &ncolumn,
                               &errmsg);
    mutex_.unlock();
    idx_ = 0;
    if (rc != SQLITE_OK) {
      error();
      return;
    }
  }

  void error() {
    LOG_ERROR("database: %s", errmsg);
    sqlite3_free(errmsg);
  }

  const std::string operator[](int index) {
    return std::string(result[idx_ * ncolumn + index]);
  }

  bool Readable() {
    if (idx_ == nrow) {
      return false;
    }
    ++idx_;
    return true;
  }

  const std::string operator[](const std::string& col) {
    int i;
    for (i = 0; i < ncolumn; ++i) {
      if (!strcmp(col.c_str(), result[i])) {
        return std::string(result[idx_ * ncolumn + i]);
      }
    }
    LOG_ERROR("database::operator[] no such column named '%s'", col.c_str());
    return "";
  }

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