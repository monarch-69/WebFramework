#include <cstdint>
#include <memory>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "framework.hpp"
#include "system/eventloop.hpp"
#include "system/connection.hpp"

void on_client_request(EventLoop *event_loop, int server_sockfd, uint32_t events)
{
    if (!(events & EPOLLIN))
        return;

    while (true) {
        int clientfd = accept(server_sockfd, nullptr, nullptr);

        if (clientfd < 0)
            break;
        
        printf("Recieved a client request: %d\n", clientfd);

        fcntl(clientfd, F_SETFL, O_NONBLOCK);
        
        auto connection = std::make_shared<Connection>(clientfd, event_loop);

        printf("Adding the clientfd in the event_loop\n");

        event_loop->add(clientfd,
                        events,
                        [connection](uint32_t events) {
                            connection->on_event(events);
                        });
    }
}

App::App() : config(), listener(config.host, config.port, config.max_conn)
{
    printf("host: %s, port: %d, max_conn: %d\n", this->listener.host.c_str(), this->listener.port, this->listener.max_conn);

    this->listener.socketfd = socket(AF_INET, SOCK_STREAM, 0);
    printf("socketfd: %d\n", this->listener.socketfd);
    if (this->listener.socketfd < 0) {
        perror("Socket descriptor failed.");
        exit(SOCKET_FD_FAIL_ERROR);
    }
    
    int sock_options = 1;
    
    if (setsockopt(this->listener.socketfd, SOL_SOCKET, SO_REUSEADDR, &sock_options, sizeof(sock_options)) < 0) {
        perror("Error while setting options: SOL_SOCKET with SO_REUSEADDR");
        exit(SOCK_OPTS_SET_FAILED_ERROR);
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;

    /*// Interface initialization*/
    /*if (inet_pton(AF_INET, this->listener.host.c_str(), &address.sin_addr) <= 0) {*/
    /*    perror("Invalid interface.");*/
    /*    exit(INVALID_INTERFACE_ERROR);*/
    /*}*/

    // Binding to socket
    address.sin_port = htons((uint16_t)this->listener.port);

    if (bind(this->listener.socketfd, (sockaddr*)&address, sizeof(address)) < 0) {
        perror("Binding failed.");
        exit(BIND_FAIL_ERROR);
    }

    // Now adding listener
    if (listen(this->listener.socketfd, (this->listener.max_conn) ? this->listener.max_conn : MAX_CONN_TO_LISTEN) < 0) {
        perror("Listening failed.");
        exit(LISTEN_FAILED_ERROR);
    }

    if (fcntl(this->listener.socketfd, F_SETFL, O_NONBLOCK) < 0) {
        perror("Error while setting file status for listener as O_NONBLOCK");
        exit(FCNTL_SET_FAILED_ERROR);
    }

}

App::~App()
{
    close(this->listener.socketfd);
}

void App::run()
{
    EventLoop event_loop;
    int socketfd = this->listener.socketfd;
    
    // Adding the on event function for the socketfd
    // When there is / are client/s then event loop will call our 'on_client_request'
    printf("Adding the server socketfd to the eventloop\n"); 
    event_loop.add(this->listener.socketfd,
                   EPOLLIN,
                   [&event_loop, &socketfd](uint32_t events) {
                        on_client_request(&event_loop, socketfd, events);
                   });

    event_loop.run();
}
