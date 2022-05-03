#include "database.h"

Database::Database() { isopen_ = false; }

Database::Database(const std::string& name) { Open(name); }

Database::~Database() {
  if (isopen_) {
    Close();
  }
}

void Database::Open(const std::string& name) {
  sqlite3_open(name.c_str(), &db_);
  isopen_ = true;
}

void Database::Close() {
  if (isopen_) {
    sqlite3_close(db_);
  }
}

void Database::Exec(const std::string& query) {
  if (!isopen_) {
    LOG_ERROR("database is not open");
    return;
  }
  int rc = sqlite3_exec(db_, query.c_str(), nullptr, nullptr, &errmsg);
  if (rc != SQLITE_OK) {
    error();
  }
}

void Database::Query(const std::string& query) {
  // if (result != nullptr) {
  //   sqlite3_free_table(result);
  // }
  mutex_.lock();
  int rc =
      sqlite3_get_table(db_, query.c_str(), &result, &nrow, &ncolumn, &errmsg);
  mutex_.unlock();
  idx_ = 0;
  if (rc != SQLITE_OK) {
    error();
    return;
  }
}

void Database::error() {
  LOG_ERROR("database: %s", errmsg);
  sqlite3_free(errmsg);
}

const std::string Database::operator[](int index) {
  if (index >= ncolumn) {
    LOG_ERROR("database[] overlap");
    return "";
  }
  return std::string(result[idx_ * ncolumn + index]);
}

bool Database::Readable() {
  if (idx_ == nrow) {
    return false;
  }
  ++idx_;
  return true;
}

const std::string Database::operator[](const std::string& col) {
  int i;
  for (i = 0; i < ncolumn; ++i) {
    if (!strcmp(col.c_str(), result[i])) {
      return std::string(result[idx_ * ncolumn + i]);
    }
  }
  LOG_ERROR("database::operator[] no such column named '%s'", col.c_str());
  return "";
}