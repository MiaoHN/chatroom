#include <sstream>

#include "database.h"

int main(int argc, char** argv) {
  Database database;
  database.Open("test_db.sqlite");
  std::string create_table =
      "CREATE TABLE IF NOT EXISTS test(id INT NOT NULL, name VARCHAR(8) NOT "
      "NULL);";
  database.Exec(create_table);
  // std::stringstream ss;
  // for (int i = 0; i < 10; ++i) {
  //   ss << "INSERT INTO test(id, name) VALUES (" << i << ", \""
  //      << "user" << std::to_string(i) << "\");";
  //   database.Exec(ss.str());
  //   ss.str("");
  // }
  std::string query = "SELECT * FROM test;";
  database.Query(query);
  while (database.Readable()) {
    std::cout << database["id"] << " " << database["name"] << std::endl;
  }
  database.Close();

  return 0;
}