#ifndef __SYNC_QUEUE_H__
#define __SYNC_QUEUE_H__

#include <mutex>
#include <queue>

#include "log.h"

template <class T>
class SyncQueue {
 public:
  SyncQueue();

  void Add(T& item);

  T Get();

  bool IsEmpty();

 private:
  std::mutex _mutex;
  std::queue<T> _queue;
};

template <class T>
SyncQueue<T>::SyncQueue() {}

template <class T>
void SyncQueue<T>::Add(T& item) {
  _mutex.lock();
  _queue.push(item);
  _mutex.unlock();
}

template <class T>
T SyncQueue<T>::Get() {
  if (!IsEmpty()) {
    _mutex.lock();
    auto item = _queue.front();
    _queue.pop();
    _mutex.unlock();
    return item;
  }
  LOG_FATAL("SUNCQUEUE::GET error occur!");
  exit(-1);
}

template <class T>
bool SyncQueue<T>::IsEmpty() {
  _mutex.lock();
  bool result = _queue.empty();
  _mutex.unlock();
  return result;
}

#endif  // __SYNC_QUEUE_H__