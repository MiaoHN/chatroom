# Chatroom

C++ 控制台聊天室

## 实现功能

- 私聊
- 广播
- 发送文件

## 协议设计

需要处理的事件有：

1. 登陆
2. 注册
3. 注销
4. 请求信息
5. 发送消息
6. 发送文件
7. 接收文件

```mermaid
sequenceDiagram
  participant Client2
  participant Client1
  Client1 ->>+ Server: login id=xxx passwd=xxx
  Server  ->>+ Database: get user's password
  Database ->>- Server: password is xxx
  Server ->>- Client1: login response
  Client1 ->>+ Server: query id=xxx
  Server  ->>+ Database: get current state for id=xxx
  Database ->>- Server: current state is xxx
  Server ->>- Client1: query response xxx
  alt Client2 online
    Client1 ->>+ Server: sendmsg id=xxx tid=xxx msg=xxx
    Server ->>- Client2: sendmsg id=xxx tid=xxx msg=xxx
  else Client2 offline
    Client1 ->>+ Server: sendmsg id=xxx tid=xxx msg=xxx
    Server ->>- Database: sendmsg=xxx id=xxx tid=xxx msg=xxx
    alt Client2 login
      Client2 ->>+ Server: login id=xxx passwd=xxx
      Server  ->>+ Database: get user's password
      Database ->>- Server: password is xxx
      Server ->>- Client2: login response
      loop have msg unread
        Server ->> Client2: msg id=xxx tid=xxx msg=xxx
      end
    end
  end
  loop
    Client1 ->>+ Server: sendfile filename=xxx id=xxx tid=xxx size=xxx curr=xxx data=xxx
    Server ->>- Client1: sendfile filename=xxx id=xxx tid=xxx size=xxx curr=xxx
  end
  loop
    Client2 ->>+ Server: getfile filename=xxx id=xxx tid=xxx size=xxx curr=xxx
    Server ->>- Client2: sendfile filename=xxx id=xxx tid=xxx size=xxx curr=xxx data=xxx
  end
  Client1 ->>+ Server: logout id=xxx
  Server ->>- Database: id=xxx logout
```

## 结构设计

### TCPServer 设计

`TCPServer` 使用 `epoll` 监听客户端请求，收到请求后将报文存储到 `SyncQueue` 并等待线程池处理请求

下面是类图：

```mermaid
classDiagram
direction LR
class TCPServer {
  +TCPServer(const std::string& address, int port)
  +LinkHandler(EventHandler::ptr handler) void
  +Start() void

  -InitSocket(const std::string& address, int port) void
  -InitEpoll() void

  -Socket::ptr _socket
  -Address::ptr _address
  -EventHandler::ptr _handler
  -Epoller::ptr _epoller
}
class Epoller {
  +Epoller(int num = 32)
  +AddManager(EventManager::ptr manager) void
  +AddListener(Socket::ptr sock) void
  +Start() void
  -handle_accept() void
  -handle_read(int sock) void
  -setnonblocking(int sock) void
  -int _epollfd
  -Socket::ptr _listensock
  -epoll_event events[1000]
  -EventManager::ptr _manager
}

class Event {
  +Socket::ptr sock
  +char buf[BUFSIZ]
  +int size
  +EventType type
}

class EventHandler {
  <<interface>>
  +EventHandler()
  +virtual ~EventHandler()

  +virtual void handle_read(Event& event)
  +virtual void handle_disconnect(Event& event)
}

class EventManager {
  +EventManager(EventHandler::ptr handler, int threads = 10)
  +~EventManager() void
  +Add(Event event) void
  +Stop() void
  -EventHandler::ptr _handler
  -SyncQueue~Event~ _queue
  -std::mutex _mutex
  -std::condition_variable _notempty
  -std::vector~std::thread~ _threads
  -int _thread_num
  -bool _isrunning
}
class SyncQueue~T~ {
  +SyncQueue()
  +Add(T& item) void
  +Get() T
  +IsEmpty() bool
  -std::mutex _mutex
  -std::queue~T~ _queue
}

class Socket {
  +Socket(int sock)
  +Socket()
  +Bind(Address::ptr address) void
  +Listen(int num) void
  +Accept() Socket::ptr
  +Connect(const std::string& addr, int port) void
  +Connect(Address::ptr address) void
  +Recv(void* buf, size_t size, int flag = 0) int
  +Send(const void* buf, size_t size, int flag = 0) int
  +GetFd() int
  +SetOpt(int optname, int val) void
  +SetNoBlock() void
  +Close() void
  -error(const std::string& msg) void
  -int _sock
}

TCPServer --> Epoller
TCPServer --> EventHandler
TCPServer --> Socket
Epoller --> EventManager
Epoller --> Socket
EventManager --> EventHandler
EventManager --> SyncQueue
EventHandler ..> Event
Event --> Socket
```

结合类图可以看出 `TCPServer` 的运作分如下几个部分：

1. `Epoller` 负责监听 `epoll` 描述符监听客户端的连接和请求操作。当 `Epoller` 接收到连接请求时，创建 `Event` 结构体并将其送至 `EventManager` 中
2. `EventManager` 一旦接收到来自 `Epoller` 的数据，便将其添加到 `SyncQueue` 这个同步队列中
3. `EventManager` 在初始化时创建了一个线程池，只要 `SyncQueue` 中有数据，就会唤醒一个线程处理从 `Epoller` 传来的数据

经过上面几个类的处理，我们将客户端请求的处理函数分离了出来，在完成我们的聊天室时只需实现 `EventHandler` 这个接口即可

### 聊天室服务端设计

```mermaid
classDiagram
class Event {
  +Socket::ptr sock
  +char buf[BUFSIZ]
  +int size
  +EventType type
}

class EventHandler {
  <<interface>>
  +EventHandler()
  +virtual ~EventHandler()

  +virtual void handle_read(Event& event)
  +virtual void handle_disconnect(Event& event)
}
class Database {
  +Database()
  +Database(const std::string& name)
  +~Database()
  +Open(const std::string& name) void
  +Close() void
  +Exec(const std::string& query) viod
  +Query(const std::string& query) void
  -error() void
  +operator[](int index) std::string
  +Readable() bool
  +index(const std::string& col) std::string
  +operator[](const std::string& col) std::String
  +Exist(const std::string& table) bool
  -sqlite3* db_
  -char** result
  -char* errmsg
  -int nrow
  -int ncolumn
  -int idx_
  -bool isopen_
  -std::mutex mutex_
}

class ServerHandler {
  +ServerHandler()
  +~ServerHandler()

  +void LinkDatabase(Database::ptr db)

  -void handle_read(Event& event) override
  -void handle_disconnect(Event& event) override

  -init_database() void

  -handle_register(Event& event) void
  -handle_login(Event& event) void
  -handle_logout(Event& event) void
  -handle_query(Event& event) void
  -handle_message(Event& event) void

  -Database::ptr _db
  -std::map~int, User::ptr~ _users
}

class User {
  +User()
  +User(int id, Socket::ptr sock)
  +GetId() int
  +SetSocket(Socket::ptr sock) void
  +GetSocket() Socket::ptr
  -int _id
  -std::string _name
  -Socket::ptr _sock
}


EventHandler --> Event
ServerHandler --> Database
ServerHandler ..|> EventHandler
ServerHandler --> User
```

### 聊天室客户端设计

## 数据库设计

### user

| uid | passwd      | uname       | status | time      |
| --- | ----------- | ----------- | ------ | --------- |
| int | varchar(32) | varchar(32) | int    | timestamp |

- 这里的 `status` 保存用户登陆状态，取值为在线（1）和下线（0）
- `time` 保存用户上次登陆时间或下线时间

### message

| uid | type | tid | msg  | time      |
| --- | ---- | --- | ---- | --------- |
| int | int  | int | text | timestamp |

该表同时保存群聊信息和私聊信息，通过 `type` 字段区分群聊和私聊：

- `type` 为 0 代表私聊
- `type` 为 1 代表群聊

### file

| uid | tid | filename | size | time      | status | curr |
| --- | --- | -------- | ---- | --------- | ------ | ---- |
| int | int | text     | int  | timestamp | int    | int  |

这里 `status` 有三种状态

- 0，客户端发送中
- 1，已发送
- 2，客户端接受中

## TODO

- 好友功能（好友添加请求 balabala）
- 创建群聊（）
