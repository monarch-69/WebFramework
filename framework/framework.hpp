#pragma once

#include <string>
#include <cstdint>
#include "../system/eventloop.hpp"


// Errors numbers
#define SOCKET_FD_FAIL_ERROR 1
#define INVALID_INTERFACE_ERROR 2
#define BIND_FAIL_ERROR 3
#define LISTEN_FAILED_ERROR 4
#define CLIENT_FD_FAILED_ERROR 5
#define SOCK_OPTS_SET_FAILED_ERROR 6
#define FCNTL_SET_FAILED_ERROR 7

#define MAX_CONN_TO_LISTEN 5

class App {
public:
    App(const std::string& host, unsigned int port, unsigned int max_conn);
    ~App();
    
    void run();

private:
    struct TcpListener {
    public:
        TcpListener(const std::string& host, unsigned int port, unsigned int max_conn) : host(host), port(port), max_conn(max_conn) {}

        int socketfd;
        unsigned int port;
        unsigned int max_conn;
        std::string host;
    };
    
    TcpListener listener;
};

// A routine to handle a client request and convert it to a connection of Type Connection
// The client connection<Connection> will be owned by event loop
void on_client_request(EventLoop* event_loop, int server_sockfd, uint32_t events);
