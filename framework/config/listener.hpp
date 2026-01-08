#pragma once

#include <string>

struct TcpListener {
    int socketfd;
    unsigned int port;
    unsigned int max_conn;
    std::string host;

    TcpListener(const std::string& host, unsigned int port, unsigned int max_conn) : host(host), port(port), max_conn(max_conn) {}
};
