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
    int PORT = 8080;
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

        while (true)
        {
            thread_task();
        }
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
        parse_message(buffer, n, client_addr);
        sendResponse(buffer, n, client_addr, true);
    }

    void parse_message(char *buffer, long n, sockaddr_in client_addr)
    {
        // Checks if message is a request
        if ((buffer[0] >> 6) == 0b00)
        {
            // 6 least significant bits of first byte:
            short message_type = (buffer[0] << 8) | (unsigned char)buffer[1];
            if (message_type != 1)
            {
                return;
            }
            short message_length = (buffer[2] << 8) | buffer[3];

            //Magic Cookie
            int magic_cookie = buffer[4];
            for (int i = 5; i < 8; i++)
            {
                magic_cookie = (magic_cookie << 8) | (unsigned char)buffer[i];
            }
            if (magic_cookie == 0x2112A442)
            {
                std::cout << "Magic Cookie found" << std::endl;
            }

            // Transactional ID
            char trans_id[12];
            for (int i = 8; i < 20; i++)
            {
                trans_id[i - 8] = buffer[i];
            }

            std::string inet_addr = inet_ntoa(client_addr.sin_addr);
            std::cout << "Addr: " << inet_addr << " Port: " << ntohs(client_addr.sin_port) << std::endl;
            std::cout << "Addr: " << client_addr.sin_addr.s_addr << " Port: " << client_addr.sin_port << std::endl;

            // If requet container unknown attributes, error code 420.
        }
    }

    int get_magic_cookie(char *buffer)
    {
        int magic_cookie = buffer[4];
        for (int i = 5; i < 8; i++)
        {
            magic_cookie = (magic_cookie << 8) | (unsigned char)buffer[i];
        }
        if (magic_cookie == 0x2112A442)
        {
            std::cout << "Magic Cookie found" << std::endl;
            return magic_cookie;
        }
        return 0;
    }

    char *get_trans_id(char *buffer)
    {
        char * trans_id = new char[12];
        for (int i = 8; i < 20; i++)
        {
            trans_id[i - 8] = buffer[i];
        }
        return trans_id;
    }
    void sendResponse(char *buffer, long n, sockaddr_in client_addr, bool is_xor)
    {

        std::vector<char> message;

        message.push_back(0x0101 >> 8);
        message.push_back(0x0101 & 0b11111111);
        // Attributene
        std::vector<char> payload;
        char *ma_attr;
        // Foreløpig bare MAPPED-ADDRESS og XOR-MAPPED-ADDRESS
        if (is_xor)
        {
            payload.push_back(0x0020 >> 8);
            payload.push_back(0x0020 & 0b11111111);
            // XOR MAPPED ADDRESS IPv4
            ma_attr = xor_mapped_address(client_addr, buffer);
        }
        else
        {
            payload.push_back(0x0001 >> 8);
            payload.push_back(0x0001 & 0b11111111);
            //MAPPED ADDRESS IPv4
            ma_attr = mapped_address(client_addr);
        }
        payload.push_back(8 >> 8);
        payload.push_back(8 & 0b11111111);

        // Plasserer MAPPED-ADDRESS attributet inn i payload
        payload.insert(payload.end(), ma_attr, ma_attr + 8);

        // Plasserer inn lengden på payload
        message.push_back((payload.size() >> 8));
        message.push_back((payload.size() & 0b11111111));

        for (int i = 4; i < 20; i++)
        {
            message.push_back(buffer[i]);
        }
        // Plasserer payload inn i STUN meldingen
        message.insert(message.end(), payload.begin(), payload.end());

        std::cout << payload.size() << std::endl;
        std::cout << "Size: " << message.size() << std::endl;

        // Skriver ut alle bits
        std::bitset<8> fir(buffer[0]);
        for (int i = 0; i < message.size(); i++)
        {
            fir = message[i];
            std::cout << "Byte: " << i << ": " << fir << std::endl;
        }

        int len = sizeof(client_addr);
        sendto(server_fd, message.data(), message.size(), MSG_CONFIRM, (struct sockaddr *)&client_addr, len);
    }
    // Returnerer en peker til array adresse område
    char *mapped_address(sockaddr_in client_addr)
    {
        char *ma_attr = new char[8];
        ma_attr[0] = 0;
        ma_attr[1] = 0x01;
        ma_attr[2] = (client_addr.sin_port >> 8);
        ma_attr[3] = (client_addr.sin_port & 0b11111111);
        int ip_address = htonl(client_addr.sin_addr.s_addr);
        for (int i = 0; i < 4; i++)
        {
            ma_attr[4 + i] = (ip_address >> (8 * (3 - i))) & 0b11111111;
        }
        return ma_attr;
    }

    char *xor_mapped_address(sockaddr_in client_addr, char *buffer)
    {
        char *xma_attr = new char[8];
        xma_attr[0] = 0;
        xma_attr[1] = 0x01;
        int magic_cookie = get_magic_cookie(buffer);
        short x_port = ntohs(client_addr.sin_port) ^ (magic_cookie >> 16);

        int x_address = (client_addr.sin_addr.s_addr ^ magic_cookie);
        std::string inet_addr = inet_ntoa(client_addr.sin_addr);
        std::cout << "Addr: " << inet_addr << " Port: " << ntohs(client_addr.sin_port) << std::endl;
        std::bitset<32> adr_bit(client_addr.sin_addr.s_addr);
        std::cout << adr_bit << std::endl;
        adr_bit = magic_cookie;
        std::cout << adr_bit << std::endl;
        adr_bit = x_address;
        std::cout << adr_bit << std::endl;
        x_address = htonl(x_address);
        adr_bit = x_address;
        std::cout << adr_bit << std::endl;
        

        xma_attr[2] = (x_port >> 8);
        xma_attr[3] = (x_port & 0b11111111);
        for (int i = 0; i < 4; i++)
        {
            xma_attr[4 + i] = (x_address >> (8 * (3 - i))) & 0b11111111;
        }
        return xma_attr;
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