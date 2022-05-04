#include "event_manager.h"

EventManager::EventManager(EventHandler::ptr handler, int threads)
    : _handler(handler), _thread_num(threads), _isrunning(true) {
  for (int i = 0; i < _thread_num; ++i) {
    _threads.push_back(std::thread([this]() {
      while (true) {
        std::unique_lock<std::mutex> lock(_mutex);
        _notempty.wait(lock);
        if (!_isrunning) {
          return;
        }
        auto item = _queue.Get();
        switch (item.type) {
          case READ: {
            _handler->handle_read(item);
            break;
          }
          case DISCONNECT: {
            _handler->handle_disconnect(item);
            break;
          }
        }
      }
    }));
  }
}

EventManager::~EventManager() { Stop(); }

void EventManager::Add(Event event) {
  _queue.Add(event);
  _notempty.notify_one();
}

void EventManager::Stop() {
  _notempty.notify_all();
  for (auto& thread : _threads) {
    thread.join();
  }
}