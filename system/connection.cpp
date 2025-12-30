#include <cstddef>
#include <cstdio>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string>
#include "connection.hpp"


Connection::Connection(int fd, EventLoop* event_loop) : fd(fd), event_loop(event_loop), conn_state(ConnState::Reading) {}

void Connection::cleanup()
{
    this->conn_state = ConnState::Closed;
    this->event_loop->remove(this->fd);
}

void Connection::on_event(uint32_t events)
{
    if (events & EPOLLIN)
        this->handle_read();

    if (events & EPOLLOUT)
        this->handle_write();
}

HttpRequest* Connection::parse_http_request()
{
    HttpRequest* req = new HttpRequest();

    size_t header_end = this->read_buffer.find("\r\n\r\n");
    // the line -> GET / HTTP/1.1
    std::string req_line = this->read_buffer.substr(0, this->read_buffer.find("\r\n"));
    // Let's get the different fields
    size_t start = 0, end = this->read_buffer.find(' ');
    req->method = this->read_buffer.substr(start, start + end);
    
    start = end + 1;
    end = this->read_buffer.find(' ', start);
    req->path = this->read_buffer.substr(start, start + end);

    return req;
}

void Connection::handle_read()
{
    char buffer[4096];
    ssize_t bytes_read = recv(this->fd, buffer, sizeof(buffer), 0);

    if (bytes_read <= 0) {
        this->cleanup();
        
        return;
    }

    this->read_buffer.append(buffer, bytes_read);

    if (this->read_buffer.find("\r\n\r\n") != std::string::npos) {
        this->write_buffer =    "HTTP/1.1 200 OK\r\n"
                                "Content-Length: 5\r\n\r\nHello";
        this->conn_state = ConnState::Writing;
        this->event_loop->update(fd, EPOLLOUT); 
    }
}

void Connection::handle_write()
{
    ssize_t bytes_written = send(this->fd, this->write_buffer.data(), sizeof(this->write_buffer), 0);

    if (bytes_written < 0) {
        this->cleanup();

        return;
    }

    this->write_buffer.erase(0, bytes_written);

    if (this->write_buffer.empty())
        this->cleanup();
}
