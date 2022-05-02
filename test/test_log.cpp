#include <thread>

#include "log.h"

int sum = 0;

void func1() {
  for (int i = 0; i < 50000; ++i) {
    LOG_INFO("==================== sum=%d", sum++);
  }
}
void func2() {
  for (int i = 0; i < 50000; ++i) {
    LOG_INFO("++++++++++++++++++++ sum=%d", sum++);
  }
}

void test_thread() {
  std::thread thread1(func1);
  std::thread thread2(func2);
  thread1.join();
  thread2.join();
}

int main(int argc, char const *argv[]) {
  LOG_INIT("test_log");
  test_thread();
  return 0;
}
