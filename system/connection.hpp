#pragma once

#include <cstdint>
#include <string>
#include "../http/httprequest.hpp"
#include "eventloop.hpp"


enum class ConnState {
    Reading,
    Processing,
    Writing,
    Closed
};

class Connection {
public:
    explicit Connection(int fd, EventLoop* event_loop);
    void on_event(uint32_t events);
    void cleanup();
    ~Connection();
    
private:
    void handle_read();
    void handle_write();
    HttpRequest* parse_http_request();

    int fd;
    EventLoop* event_loop;
    ConnState conn_state;
    std::string read_buffer;
    std::string write_buffer;
};
