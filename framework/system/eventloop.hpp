#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>

#define EVENT_LOOP_CREATE_ERR 1
#define CLIENT_FD_ADD_ERR 2

class EventLoop {

using Callback = std::function<void(uint32_t)>;
public:
    EventLoop();
    ~EventLoop();
    
    void add(int fd, uint32_t events, Callback cb); 
    void update(int fd, uint32_t events);
    void remove(int fd);
    void run();

private:
    int epoll_fd; // File descriptor for the epoll instance itself
    std::unordered_map<int, Callback> callbacks; // To store fd's of clients with a callback and this Callback will be same for everyone
};
