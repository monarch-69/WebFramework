#pragma once

#include <cstdint>
#include <unordered_map>
#include "system/eventloop.hpp"
#include "config/config.hpp"
#include "config/listener.hpp"
#include "http/httprequest.hpp"


// Errors numbers
#define SUCCESS 0
#define SOCKET_FD_FAIL_ERROR 1
#define INVALID_INTERFACE_ERROR 2
#define BIND_FAIL_ERROR 3
#define LISTEN_FAILED_ERROR 4
#define CLIENT_FD_FAILED_ERROR 5
#define SOCK_OPTS_SET_FAILED_ERROR 6
#define FCNTL_SET_FAILED_ERROR 7

#define MAX_CONN_TO_LISTEN 5

class App {

using Handler = std::function<void(const HttpRequest&)>;

public:
    App();
    ~App();
    
    void run();

private:
    std::string work_dir{}; // This is working directory and should not be confused with the parent dir. Parent dir part of working dir
    Config config; 
    TcpListener listener;
    std::unordered_map<std::string, Handler> handlers{}; // Initializing empty handler at first
};

// A routine to handle a client request and convert it to a connection of Type Connection
// The client connection<Connection> will be owned by event loop
void on_client_request(EventLoop* event_loop, int server_sockfd, uint32_t events);
