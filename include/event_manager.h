#ifndef __EVENT_MANAGER_H__
#define __EVENT_MANAGER_H__

#include <condition_variable>
#include <cstdio>
#include <memory>
#include <thread>

#include "socket.h"
#include "sync_queue.h"
#include "uncopyable.h"

struct Event {
  Socket::ptr sock;
  char buf[BUFSIZ];
  int size;
};

class EventHandler {
 public:
  using ptr = std::shared_ptr<EventHandler>;
  EventHandler() {}
  virtual ~EventHandler() {}

  virtual void handle(Event& event) = 0;
};

class EventManager {
 public:
  using ptr = std::shared_ptr<EventManager>;

  EventManager(EventHandler::ptr handler, int threads = 10);

  ~EventManager();

  void Add(Event event);
  void Stop();

 private:
  EventHandler::ptr _handler;  // 事件处理句柄
  SyncQueue<Event> _queue;
  std::mutex _mutex;
  std::condition_variable _notempty;
  std::vector<std::thread> _threads;
  int _thread_num;
  bool _isrunning;
};

#endif  // __EVENT_MANAGER_H__