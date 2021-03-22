#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <bitset>
#include <vector>
#include "attributes_response.hpp"

#define MAGIC_COOKIE 0x2112A442
#define BINDING_REQUEST 0x0001
#define BINDING_RESPONSE 0X0101
#define BINDING_ERROR_RESPONSE 0x0111
#define MAPPED_ADDRESS_TYPE 0x0001
#define XOR_MAPPED_ADDRESS_TYPE 0X0020
#define ERROR_CODE 0x0009
#define UNKNOWN_ATTRIBUTES_CODE 0x000A
#define SOFTWARE 0x8022
#define ALTERNATE_SERVER 0x8023
#define IPv4 0x01

char *xor_mapped_address(sockaddr_in client_addr, char *buffer);
char *mapped_address(sockaddr_in client_addr);
int get_magic_cookie(char *buffer);
char *get_transaction_id(char *buffer);
void add_attribute(char *attribute, short length, short type, std::vector<char> &attributes);
char *unknown_attr(char *buffer);
std::vector<char> error_code_attr(short code);

/*
    Parses and verifes Binding Request from a client
    Checks for necessary headerfields
*/
int parse_verify_request(char *buffer, sockaddr_in client_addr, int n)
{
    // Bad Request, header must be 20 bytes
    if (n < 20 || n == 21)
    {
        return 400;
    }
    // Server won't understand any attributes.
    // If it contains 22 bytes, it must contain a attribute type
    if (n > 21)
    {
        return 420;
    }
    // Checks if first two bits are 00
    if ((buffer[0] >> 6) != 0b00)
    {
        return 400;
    }
    // Parses the messagetype
    short message_type = (buffer[0] << 8) | (unsigned char)buffer[1];
    // Checks if the messagetype is a BINDING REQUEST
    if (message_type != BINDING_REQUEST)
    {
        return 400;
    }
    //Parse message length
    short message_length = (buffer[2] << 8) | buffer[3];

    //Parses request's Magic Cookie to an int, and validates it
    int magic_cookie = get_magic_cookie(buffer);
    // Validation
    if (magic_cookie != MAGIC_COOKIE)
    {
        return 400;
    }
    // Should be removed
    // std::string inet_addr = inet_ntoa(client_addr.sin_addr);
    // std::cout << "Addr: " << inet_addr << " Port: " << ntohs(client_addr.sin_port) << std::endl;
    // std::cout << "Addr: " << client_addr.sin_addr.s_addr << " Port: " << client_addr.sin_port << std::endl;

    return 200;
}

std::vector<char> binding_response(char *buffer, sockaddr_in client_addr, bool is_xor, int code)
{
    // Container for entire response
    std::vector<char> response;

    // Message type
    short message_type = (code == 200) ? BINDING_RESPONSE : BINDING_ERROR_RESPONSE;

    // Writes message type to buffer
    response.push_back(message_type >> 8);
    response.push_back(message_type & 0b11111111);

    // Attributes
    std::vector<char> attributes;
    // MAPPED ADDRESS either xored or not or error
    char *ma_attr;
    if (code != 200)
    {
        // Get error response with error reason phrase
        std::vector<char> error_attr = error_code_attr(code);
        add_attribute(error_attr.data(), error_attr.size(), ERROR_CODE, attributes);
    }
    else if (is_xor)
    {
        // XOR MAPPED ADDRESS IPv4
        ma_attr = xor_mapped_address(client_addr, buffer);
        add_attribute(ma_attr, 8, XOR_MAPPED_ADDRESS_TYPE, attributes);
        // std::vector<char> software_attr = software();
        // add_attribute(software_attr.data(),software_attr.size(), SOFTWARE, attributes);
        // add_attribute(alternate_server(),8,ALTERNATE_SERVER, attributes);
        
    }
    else
    {
        //MAPPED ADDRESS IPv4
        ma_attr = mapped_address(client_addr);
        add_attribute(ma_attr, 8, MAPPED_ADDRESS_TYPE, attributes);
        // std::vector<char> software_attr = software();
        // add_attribute(software_attr.data(),software_attr.size(), SOFTWARE, attributes);
        // add_attribute(alternate_server(),8,ALTERNATE_SERVER, attributes);
    }

    // Adds total attributes length to response packet
    response.push_back((attributes.size() >> 8));
    response.push_back((attributes.size() & 0b11111111));

    // Adds magic cookie and transaction ID
    for (int i = 4; i < 20; i++)
    {
        response.push_back(buffer[i]);
    }
    // Adds attributes
    response.insert(response.end(), attributes.begin(), attributes.end());

    std::cout << attributes.size() << std::endl;
    std::cout << "Size: " << response.size() << std::endl;

    // Writes all bytes to console
    // std::bitset<8> fir(buffer[0]);
    // for (int i = 0; i < response.size(); i++)
    // {
    //     fir = response[i];
    //     std::cout << "Byte: " << i << ": " << fir << std::endl;
    // }

    return response;
}