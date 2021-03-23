#include <iostream>
#include "../stunserver/attributes_response.hpp"
#include <netinet/in.h>
#include <vector>
#include <exception>


// Mocks the byte buffer 123123123
char *mock_buffer()
{
    char *binding_request = new char[20];
    *(short *)(&binding_request[0]) = 0x0001;          // stun_method
    *(short *)(&binding_request[2]) = 0x0000;          // msg_length
    *(int *)(&binding_request[4]) = htonl(0x2112A442); // magic cookie

    *(int *)(&binding_request[8]) = 0x63c7117e; // transacation ID
    *(int *)(&binding_request[12]) = 0x0714278f;
    *(int *)(&binding_request[16]) = 0x5ded3221;

    return binding_request;
}

sockaddr_in mock_client_address()
{
    sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "74.125.200.127", &client_addr.sin_addr);
    client_addr.sin_port = htons(19302);
    return client_addr;
}

bool should_return_transaction_id(char *buffer)
{
    char *transaction_id = new char[12];
    *(int *)(&transaction_id[0]) = 0x63c7117e;
    *(int *)(&transaction_id[4]) = 0x0714278f;
    *(int *)(&transaction_id[8]) = 0x5ded3221;
    char *test_function = get_transaction_id(buffer);
    for (int i = 0; i < 12; i++)
    {
        if (transaction_id[i] != test_function[i])
            return false;
    }
    return true;
}

bool should_return_magic_cookie(char *buffer)
{

    if (0x2112A442 == get_magic_cookie(buffer))
        return true;
    return false;
}

bool should_create_mapped_reponse(sockaddr_in client_addr)
{
    int correct_response[8] = {0, 1, 0b01001011, 0b01100110, 0b01001010, 0b01111101, 0b11001000, 0b01111111};
    char *mapped_address_attr = mapped_address(client_addr);
    for (int i = 0; i < 8; i++)
    {
        if ((unsigned char)correct_response[i] != (unsigned char)mapped_address_attr[i])
            return false;
    }
    return true;
}

bool should_create_xor_mapped_address(sockaddr_in client_addr, char *buffer)
{
    int correct_response[8] = {0, 1, 0b01101010, 0b01110100, 0b01101011, 0b01101111, 0b01101100, 0b00111101};
    char *xor_mapped_address_attr = xor_mapped_address(client_addr, buffer);
    for (int i = 0; i < 8; i++)
    {
        if ((unsigned char)correct_response[i] != (unsigned char)xor_mapped_address_attr[i])
            return false;
    }
    return true;
}


bool should_create_400_error_code(){
    std::vector<char> attribute = error_code_attr(400);
    if(attribute[0] != 0 || attribute[1] != 0) return false;
    if(attribute[2] != 4 || attribute[3] != 0) return false;
    if(attribute.size() != 283) return false;
    return true;
}

bool should_create_420_error_code(){
    std::vector<char> attribute = error_code_attr(420);
    if(attribute[0] != 0 || attribute[1] != 0) return false;
    if(attribute[2] != 4 || attribute[3] != 20) return false;
    if(attribute.size() != 129) return false;
    return false;
}
int main()
{
    char *mock_buffer_var = mock_buffer();
    bool test1 = should_return_transaction_id(mock_buffer_var);
    bool test2 = should_return_magic_cookie(mock_buffer_var);
    bool test3 = should_create_mapped_reponse(mock_client_address());
    bool test4 = should_create_xor_mapped_address(mock_client_address(), mock_buffer_var);
    bool test5 = should_create_400_error_code();
    bool test6 = should_create_420_error_code();
    std::cout << "Test 1: " << test1 << std::endl;
    std::cout << "Test 2: " << test2 << std::endl;
    std::cout << "Test 3: " << test3 << std::endl;
    std::cout << "Test 4: " << test4 << std::endl;
    std::cout << "Test 5: " << test5 << std::endl;
    std::cout << "Test 6: " << test6 << std::endl;

    try{
        if(!test1) throw "should_return_transaction_id Failed";
        if(!test2) throw "should_return_magic_cookie Failed";
        if(!test3) throw "should_create_mapped_reponse Failed";
        if(!test4) throw "should_create_xor_mapped_address Failed";
        if(!test5) throw "should_create_400_error_code Failed";
        if(!test6) throw "should_create_420_error_code Failed";
    }catch(const char* msg){
        std::cerr << msg << std::endl;
        throw 20;
    }


    

    return 0;
}