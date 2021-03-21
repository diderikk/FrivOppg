#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <bitset>
#include <functional>
#include <vector>
#include "handler_response.hpp"
#include "workers.hpp"


class Server
{
    int PORT = 8080;
    int server_fd;
    int opt = 1;
    struct sockaddr_in server_addr, client_addr;
    char buffer[1024] = {0};
    int addrlen = sizeof(server_addr);
    Workers event_loop{1};

    /*
        Opens the server
    */
    void init_server()
    {
        // Creates a socker file descriptor in IPv4, and receive SOCK_DGRAM or UDP protocol
        if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0)
        {
            std::cerr << "Could NOT open socket" << std::endl;
            exit(EXIT_FAILURE);
        }
        // Sets
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        // Converts PORT from host byte order to network byte order
        server_addr.sin_port = htons(PORT);
        // Binds server to the created server address over
        // This will bind it to a given PORT 8080;
        if (bind(server_fd, (const struct sockaddr *)&server_addr,
                 sizeof(server_addr)) < 0)
        {
            std::cerr << "Could not bind the port: " << PORT << std::endl;
            exit(EXIT_FAILURE);
        }

        while (true)
        {
            int len = sizeof(client_addr);
            int n = recvfrom(server_fd, (char *)buffer, 1024,
                             MSG_WAITALL, (struct sockaddr *)&client_addr,
                             (socklen_t *)&len);
            // Convert the member function to a void(*)
            std::function<void(char *, sockaddr_in, int)> f = [this](char *buf, sockaddr_in client_addr, int n){
                this->handle_request(buf, client_addr, n);
            };
            // Posts all arguments to be used later, when task is executed
            // Can't pass "this" as i will be passed by reference
            event_loop.post(f,n, client_addr, buffer);
        }
    }

    void handle_request(char *buf, sockaddr_in client_addr, int n)
    {
        // std::cout << n << std::endl;
        int code = parse_verify_request(buf, client_addr, n);
        std::vector<char> response = binding_response(buf, client_addr, true, code);
        int len = sizeof(client_addr);
        sendto(server_fd, response.data(), response.size(), MSG_CONFIRM, (struct sockaddr *)&client_addr, len);
    }

public:
    Server()
    {
        event_loop.start();
        init_server();
    }
    ~Server()
    {
        event_loop.stop();
        std::cout << "Closing server" << std::endl;
    }
};