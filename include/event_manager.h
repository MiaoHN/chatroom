#ifndef __EVENT_MANAGER_H__
#define __EVENT_MANAGER_H__

#include <condition_variable>
#include <cstdio>
#include <memory>
#include <thread>

#include "socket.h"
#include "sync_queue.h"
#include "uncopyable.h"

enum EventType {
  READ,
  DISCONNECT,
};

struct Event {
  Socket::ptr sock;
  char buf[BUFSIZ];
  int size;
  EventType type;
};

class EventHandler {
 public:
  using ptr = std::shared_ptr<EventHandler>;
  EventHandler() {}
  virtual ~EventHandler() {}

  /**
   * @brief 获取报文长度，如果报文非法返回 -1
   *
   */
  virtual int GetLength(Event& event) = 0;

  virtual void handle_read(Event& event) = 0;
  virtual void handle_disconnect(Event& event) = 0;
};

class EventManager {
 public:
  using ptr = std::shared_ptr<EventManager>;

  EventManager(EventHandler::ptr handler, int threads = 10);

  ~EventManager();

  void Add(Event event);
  void Stop();

  int GetSize(Event& event) { return _handler->GetLength(event); }

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