#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <bitset>
#include <vector>

class Server
{
    int PORT = 3000;
    int server_fd;
    int opt = 1;
    struct sockaddr_in server_addr, client_addr;
    char buffer[1024] = {0};
    int addrlen = sizeof(server_addr);

    void init_server()
    {
        if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0)
        {
            std::cout << "Couldn't create socket 1" << std::endl;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        // Converts from host byte order to network byte order
        server_addr.sin_port = htons(PORT);

        if (bind(server_fd, (const struct sockaddr *)&server_addr,
                 sizeof(server_addr)) < 0)
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        
        thread_task();
        thread_task();
    }

    void thread_task()
    {
        std::string hello = "Hello from server";
        int len, n = 0;
        len = sizeof(client_addr);
        n = recvfrom(server_fd, (char *)buffer, 1024,
                        MSG_WAITALL, (struct sockaddr *)&client_addr,
                        (socklen_t *)&len);
        std::cout << n << std::endl;
        parse_message(buffer,n, client_addr);
        // sendto(server_fd, hello, strlen(hello), MSG_CONFIRM, (struct sockaddr *)&client_addr, len);
        // printf("Hello message sent.\n");
    }

    void parse_message(char *buffer, long n, sockaddr_in client_addr){
        // Checks if message is a request
        if((buffer[0] >> 6) == 0b00){
            // 6 least significant bits of first byte:
            short message_type1 = (buffer[0] & 0b111111);
            short message_type = (message_type1 << 8) | buffer[1];
            std::cout << message_type << std::endl;
            short message_length = (buffer[2] << 8) | buffer[3];
            std::bitset<16> ml(message_length);
            std::bitset<16> mt(message_type);
            std::cout << ml << std::endl;
            std::cout << mt << std::endl;
            std::cout << message_length << std::endl;

            int magic_cookie = buffer[4]; 
            for(int i = 5; i<8; i++){
                magic_cookie = (magic_cookie << 8) | (unsigned char)buffer[i];
            }
            if(magic_cookie == 0x2112A442){
                std::cout << "Magic Cookie found" << std::endl;
                
            }
            std::string trans_id = "";
            for(int i = 8; i<20; i++){
                std::bitset<8> bits(buffer[i]);
                
                trans_id += bits.to_string();
            }

            std::cout << trans_id << std::endl;

            std::string inet_addr = inet_ntoa(client_addr.sin_addr);
            std::cout << "Addr: " << inet_addr << " Port: " << client_addr.sin_port << std::endl;

            std::vector<char> message;

            
            char first = (0b10 << 6) | (0x0101 >> 8);
            std::bitset<8> fir(first);
            char second = (0x0101 & 0b11111111);

            message.push_back(first);
            message.push_back(second);
            std::vector<char> payload;
            payload.push_back((0x0001 >> 8));
            payload.push_back((0x0001 & 0b11111111));
            payload.push_back(0);
            payload.push_back(8);
            char ma_attr[8];
            ma_attr[0] = 0;
            ma_attr[1] = 0x01;
            ma_attr[2] = (client_addr.sin_port >> 8);
            ma_attr[3] = (client_addr.sin_port & 0b11111111);
            for(int i = 0; i<4; i++){
                ma_attr[4+i] = ((client_addr.sin_addr.s_addr >> (8*(3-i))) & 0b11111111);
            }
            payload.insert(payload.end(),ma_attr,ma_attr + 8);
            message.push_back(0);
            message.push_back(payload.size());
            for(int i = 4; i<20; i++){
                message.push_back(buffer[i]);
            }
            message.insert(message.end(),payload.begin(), payload.end());
            std::cout << "Size: " << message.size() << std::endl;

            for(int i = 0; i<message.size(); i++){
                fir = message[i];
                std::cout << "Byte: " << i << ": " << fir << std::endl;
            }
            int len = sizeof(client_addr);
            sendto(server_fd, message.data(), message.size(), MSG_CONFIRM, (struct sockaddr *)&client_addr, len);


            


            // If requet container unknown attributes, error code 420.

        }

        
    }

public:
    Server()
    {
        init_server();
    }
    ~Server()
    {
        std::cout << "Closing server" << std::endl;
    }
};