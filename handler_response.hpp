#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <bitset>
#include <vector>

#define MAGIC_COOKIE 0x2112A442
#define BINDING_REQUEST 0x0001
#define BINDING_RESPONSE 0X0101
#define MAPPED_ADDRESS_TYPE 0x0001
#define XOR_MAPPED_ADDRESS_TYPE 0X0020
#define IPv4 0x01


char *xor_mapped_address(sockaddr_in client_addr, char *buffer);
char *mapped_address(sockaddr_in client_addr);
int get_magic_cookie(char *buffer);
char *get_transaction_id(char *buffer);

/*
    Parses and verifes Binding Request from a client
    Checks for necessary headerfields
*/
void parse_verify_request(char *buffer, sockaddr_in client_addr)
{
    
    // Checks if first two bits are 00
    if ((buffer[0] >> 6) == 0b00)
    {
        // Parses the messagetype
        short message_type = (buffer[0] << 8) | (unsigned char)buffer[1];
        // Checks if the messagetype is a BINDING REQUEST
        if (message_type != BINDING_REQUEST)
        {
            //TODO error handling
            return;
        }
        //Parse message length
        short message_length = (buffer[2] << 8) | buffer[3];

        //Parses request's Magic Cookie to an int, and validates it
        int magic_cookie = get_magic_cookie(buffer);
        // Validation
        if (magic_cookie == MAGIC_COOKIE)
        {
            std::cout << "Magic Cookie found" << std::endl;
        }
        // Should be removed
        std::string inet_addr = inet_ntoa(client_addr.sin_addr);
        std::cout << "Addr: " << inet_addr << " Port: " << ntohs(client_addr.sin_port) << std::endl;
        std::cout << "Addr: " << client_addr.sin_addr.s_addr << " Port: " << client_addr.sin_port << std::endl;

        //TODO If requet container unknown attributes, error code 420.
    }
}

std::vector<char> binding_response(char *buffer, sockaddr_in client_addr, bool is_xor)
{
    // Container for entire response
    std::vector<char> response;

    // Message type
    response.push_back(BINDING_RESPONSE >> 8);
    response.push_back(BINDING_RESPONSE & 0b11111111);
    // Attributes
    std::vector<char> attributes;
    // MAPPED ADDRESS either xored or not 
    char *ma_attr;
    if (is_xor)
    {
        attributes.push_back(XOR_MAPPED_ADDRESS_TYPE >> 8);
        attributes.push_back(XOR_MAPPED_ADDRESS_TYPE & 0b11111111);
        // XOR MAPPED ADDRESS IPv4
        ma_attr = xor_mapped_address(client_addr, buffer);
    }
    else
    {
        attributes.push_back(MAPPED_ADDRESS_TYPE >> 8);
        attributes.push_back(MAPPED_ADDRESS_TYPE & 0b11111111);
        //MAPPED ADDRESS IPv4
        ma_attr = mapped_address(client_addr);
    }
    // Adds attribute length, 8 bytes for mapped address
    attributes.push_back(8 >> 8);
    attributes.push_back(8 & 0b11111111);

    // Inserts mapped address into attributes
    attributes.insert(attributes.end(), ma_attr, ma_attr + 8);

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
    std::bitset<8> fir(buffer[0]);
    for (int i = 0; i < response.size(); i++)
    {
        fir = response[i];
        std::cout << "Byte: " << i << ": " << fir << std::endl;
    }

    return response;
}
// Parses and validates magic cookie
int get_magic_cookie(char *buffer)
{
    int magic_cookie = buffer[4];
    for (int i = 5; i < 8; i++)
    {
        magic_cookie = (magic_cookie << 8) | (unsigned char)buffer[i];
    }
    if (magic_cookie == MAGIC_COOKIE)
    {
        std::cout << "Magic Cookie found" << std::endl;
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

// Returns the address in MAPPED ADDRESS ATTRIBUTE form
char *mapped_address(sockaddr_in client_addr)
{
    char *ma_attr = new char[8];
    // First byte is 0
    ma_attr[0] = 0;
    // Familiy value
    ma_attr[1] = IPv4;
    // Add port
    ma_attr[2] = (client_addr.sin_port >> 8);
    ma_attr[3] = (client_addr.sin_port & 0b11111111);
    // Change IP address to network byte order
    int ip_address = htonl(client_addr.sin_addr.s_addr);
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
    // Converts address to host byte length and xors it with most 16 significant bits in magic_cookie
    short x_port = ntohs(client_addr.sin_port) ^ (magic_cookie >> 16);
    // Converts address to host byte length and xors it with magic_cookie
    int x_address = (ntohl(client_addr.sin_addr.s_addr) ^ magic_cookie);
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
