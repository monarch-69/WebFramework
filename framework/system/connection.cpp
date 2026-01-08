#include <cstddef>
#include <cstdio>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string>
#include <fstream>
#include "connection.hpp"


Connection::Connection(int fd, EventLoop* event_loop) : fd(fd), event_loop(event_loop), conn_state(ConnState::Reading) {}

void Connection::cleanup()
{
    this->conn_state = ConnState::Closed;
    this->event_loop->remove(this->fd);
}

void Connection::on_event(uint32_t events)
{
    if (events & EPOLLIN) {
        printf("In Connection::on_event: EPOLLIN\n");
        this->handle_read();
    }

    if (events & EPOLLOUT) {
        printf("In Connection::on_event: EPOLLOUT\n");
        this->handle_write();
    }
}

void Connection::parse_req_line(HttpRequest* req, std::string& req_line)
{
    printf("Parsing the req_line...\n");
    
    // Let's get the different fields
    size_t start = 0, end = req_line.find(' ');
    req->method = req_line.substr(start, end - start);
    printf("Read the method...\n");

    start = end + 1;
    end = req_line.find(' ', start);
    req->path = req_line.substr(start, end - start);
    printf("Read the request path...\n");
    
    start = end + 1;
    end = req_line.length();
    req->version = req_line.substr(start, end - start);
    printf("Read the version...\n");
    
    printf("Done parsing the req_line....\n");
}

void Connection::parse_header_and_body(HttpRequest* req, std::string& to_parse)
{
    printf("In the parse_header_and_body...\n");
    size_t start = 0, header_len = to_parse.find("\r\n\r\n");
    
    // Let's extract headers
    while (start < header_len) {
        size_t temp_end = to_parse.find("\r\n", start);
        std::string temp_str = to_parse.substr(start, temp_end - start);
        
        printf("temp_str: %s, start: %zu and temp_end: %zu\n", temp_str.c_str(), start, temp_end);
        // now seperate on ':'
        
        printf("Getting the colon_pos...\n");
        size_t colon_pos = temp_str.find(':');
        printf("Got the colon_pos...\n");
        printf("colon_pos: %zu and start: %zu\n", colon_pos, start);
        std::string heading = temp_str.substr(0, colon_pos); // this is the heading => heading: ...
        std::string content = temp_str.substr(colon_pos + 2, std::string::npos); // this is the content => heading: <content>
        printf("heading: %s and value: %s\n", heading.c_str(), content.c_str());

        req->headers.insert({ heading, content });

        start = temp_end + 2;
    }

    // Now let's extract the body
    size_t body_len = 0; 
    if ( auto iter = req->headers.find("Content-Length"); iter != req->headers.end() )
        body_len = std::stoull(iter->second);

    // now we have to read no of. 'body_len' chars from the body
    start = header_len + 4;
    req->body = to_parse.substr(start, body_len);
}

HttpRequest Connection::parse_http_request()
{
    printf("In parse_http_request...\n");
    HttpRequest req{};

    // the line -> GET / HTTP/1.1
    printf("Reading the req_line...\n");
    std::string req_line = this->read_buffer.substr(0, this->read_buffer.find("\r\n"));
    // parse the above request req line
    this->parse_req_line(&req, req_line);
    
    // now parse for headers and body
    size_t host_start = this->read_buffer.find("\r\n") + 2;
    size_t host_end = this->read_buffer.find("\r\n", host_start);
    std::string header_and_body = this->read_buffer.substr(host_end + 2, std::string::npos);

    this->parse_header_and_body(&req, header_and_body);
    
    // After the above req is populated with method, path, headers, HTTP version, body

    return req;
}

void Connection::handle_read()
{
    printf("In Connection::handle_read\n");
    char buffer[4096];
    ssize_t bytes_read = recv(this->fd, buffer, sizeof(buffer), 0);

    if (bytes_read <= 0) {
        this->cleanup();
        
        return;
    }

    printf("Reading buffer...\n");
    this->read_buffer += buffer;
    printf("Done reading buffer...\n");
    
    // Create an HttpRequest object which would contain method, path, headers, body, HTTP version
    HttpRequest req = this->parse_http_request();

    // Now here we need to check whether the request is for static assests (html, css, images, etc) or registered handler.
    // Order of procedence will be: check in static assests -> in app factory or handlers list
    

    printf("Writing response in the write_buffer...\n");

    // For the testing purpose let's open test.html file from the test directory
    std::ifstream if_stream{"./test/test.html", std::ios::in};
    
    if (!if_stream.is_open()) {
        perror("Error opening file \"test/test.html\"");
        exit(10);
    } 
    

    this->write_buffer +=   "HTTP/1.1 200 OK\r\n"
                            "Content-Length: 26\r\n\r\nHello, I think this works";
    printf("Done writing response in the write_buffer...\n");
    this->conn_state = ConnState::Writing;
    this->event_loop->update(fd, EPOLLOUT); 
}

void Connection::handle_write()
{
    printf("In Connection::handle_write\n");
    printf("Writing response to the client...\n");
    ssize_t bytes_written = send(this->fd, this->write_buffer.data(), sizeof(this->write_buffer), 0);
    
    if (bytes_written < 0) {
        this->cleanup();

        return;
    }

    this->write_buffer.erase(0, bytes_written);

    if (this->write_buffer.empty())
        this->cleanup();
}
