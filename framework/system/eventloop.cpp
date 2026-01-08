#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/epoll.h>

#include "eventloop.hpp"


EventLoop::EventLoop()
{
    this->epoll_fd = epoll_create1(0); // This creates the epoll instance

    if (this->epoll_fd < 0) {
        perror("Failed to create eventloop");
        exit(EVENT_LOOP_CREATE_ERR);
    }
}

EventLoop::~EventLoop()
{
    for (auto pair: this->callbacks)
        close(pair.first);

    close(this->epoll_fd);
}


void EventLoop::add(int fd, uint32_t events, Callback cb)
{
    epoll_event event{};
    event.events = events;
    event.data.fd = fd;

    if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0) {
        perror("Failed to add fd");
        return;
    }

    this->callbacks.insert({ fd, std::move(cb) });
}

void EventLoop::update(int fd, uint32_t events)
{
    epoll_event event{};
    event.events = events;
    event.data.fd = fd;

    if (epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0) // Let's not crash on update failure
        perror("Update (read -> write) failed");

}

void EventLoop::remove(int fd)
{
    this->callbacks.erase(fd);
    epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
}

void EventLoop::run()
{
    const unsigned int MAX_EVENTS = 64;
    epoll_event events[MAX_EVENTS];

    while (true) {
        int ready_fds = epoll_wait(this->epoll_fd, events, MAX_EVENTS, -1);

        for (int i{}; i < ready_fds; ++i) {
            int fd = events[i].data.fd;
            
            if (this->callbacks.find(fd) != this->callbacks.end())
                this->callbacks[fd](events[i].events);
        }
    }
}
