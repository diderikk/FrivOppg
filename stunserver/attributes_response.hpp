#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#pragma once

#define IPv4 0x01
#define MAGIC_COOKIE 0x2112A442

int get_magic_cookie(char *buffer)
{
    int magic_cookie = buffer[4];
    for (int i = 5; i < 8; i++)
    {
        magic_cookie = (magic_cookie << 8) | (unsigned char)buffer[i];
    }
    if (magic_cookie == MAGIC_COOKIE)
    {
        return magic_cookie;
    }
    return 0;
}
// Returns a array with the transaction_id
char *get_transaction_id(char *buffer)
{
    char *transaction_id = new char[12];
    for (int i = 8; i < 20; i++)
    {
        transaction_id[i - 8] = buffer[i];
    }
    return transaction_id;
}

void add_attribute(char *attribute, short length, short type, std::vector<char> &attributes)
{
    // Adds attibute type
    attributes.push_back(type >> 8);
    attributes.push_back(type & 0b11111111);
    attributes.push_back(length >> 8);
    attributes.push_back(length & 0b11111111);
    attributes.insert(attributes.end(), attribute, attribute + length);
}

// Returns the address in MAPPED ADDRESS ATTRIBUTE form
char *mapped_address(sockaddr_in client_addr)
{
    char *ma_attr = new char[8];
    // First byte is 0
    ma_attr[0] = 0;
    // Familiy value
    ma_attr[1] = IPv4;
    // Add port
    short port = htons(client_addr.sin_port);
    ma_attr[2] = (port >> 8);
    ma_attr[3] = (port & 0b11111111);
    // Change IP address to network byte order
    int ip_address = ntohl(client_addr.sin_addr.s_addr);
    // Adds IP address
    for (int i = 0; i < 4; i++)
    {
        ma_attr[4 + i] = (ip_address >> (8 * (3 - i))) & 0b11111111;
    }
    // Returns attribute
    return ma_attr;
}

// Returns the address in XOR MAPPED ADDRESS ATTRIBUTE form
char *xor_mapped_address(sockaddr_in client_addr, char *buffer)
{
    char *xma_attr = new char[8];
    // First byte is 0
    xma_attr[0] = 0;
    // Familie value
    xma_attr[1] = IPv4;
    int magic_cookie = get_magic_cookie(buffer);
    // Converts address to network byte length and xors it with most 16 significant bits in magic_cookie
    short x_port = htons(client_addr.sin_port) ^ (magic_cookie >> 16);
    // Converts address to network byte length and xors it with magic_cookie
    int x_address = (htonl(client_addr.sin_addr.s_addr) ^ magic_cookie);
    // Adds IP address
    xma_attr[2] = (x_port >> 8);
    xma_attr[3] = (x_port & 0b11111111);
    // Adds IP address
    for (int i = 0; i < 4; i++)
    {
        xma_attr[4 + i] = (x_address >> (8 * (3 - i))) & 0b11111111;
    }
    return xma_attr;
}

std::vector<char> error_code_attr(short code)
{
    std::vector<char> error_attr;
    error_attr.push_back(0);
    error_attr.push_back(0);
    // Writes error class
    error_attr.push_back(code / 100);
    // Writes error number
    error_attr.push_back(code % 100);

    std::string bad_request;

    if (code == 400)
        bad_request = "The request was malformed.  The client SHOULD NOT retry the request without modification from the previous"
                      "attempt.  The server may not be able to generate a valid MESSAGE-INTEGRITY for this error, so the client MUST NOT expect"
                      "a valid MESSAGE-INTEGRITY attribute on this response.";
    else if (code == 420)
        bad_request = "Unknown Attribute: The server received a STUN packet containing"
                      "a comprehension-required attribute that it did not understand.";

    error_attr.insert(error_attr.end(), bad_request.begin(), bad_request.end());

    return error_attr;
}

// Returns SOFTWARE attribute
std::vector<char> software()
{
    std::vector<char> software;
    // Written from VM's properties
    std::string software_string = "Virtual machine"
                                  "Computer name: stunserver1"
                                  "Operating system: Linux(ubuntu 18.04)"
                                  "vCPUs: 1"
                                  "RAM: 0.5 GiB";
    software.insert(software.end(), software_string.begin(), software_string.end());

    return software;
}

// Returns ALTERNATE SERVER attribute
char *alternate_server(){
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
	inet_pton(AF_INET, "74.125.200.127", &server_addr.sin_addr);
	server_addr.sin_port = htons(19302);
    char *mapped_address_atr = mapped_address(server_addr);
    return mapped_address_atr;
}
