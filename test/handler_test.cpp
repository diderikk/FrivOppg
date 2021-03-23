#include <iostream>
#include "../stunserver/handler_response.hpp"
#include <netinet/in.h>
#include <vector>
#include <exception>
 
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


bool should_return_code_400_buffer_size_21(sockaddr_in client_addr, char *buffer){
    if(parse_verify_request(buffer,client_addr,21) == 400) return true;
    return false;
}

bool should_return_code_400_buffer_size_less_than_20(sockaddr_in client_addr, char *buffer){
    if(parse_verify_request(buffer,client_addr,19) == 400) return true;
    return false;
}

bool should_return_code_420_buffer_size_greater_than_21(sockaddr_in client_addr, char *buffer){
    if(parse_verify_request(buffer,client_addr,22) == 420) return true;
    return false;
}

bool should_return_code_400_first_two_bits_not_zero(sockaddr_in client_addr, char *buffer){
    buffer[0] = 0b11000001;
    if(parse_verify_request(buffer,client_addr,20) == 400) return true;
    return false;
}

bool should_return_code_400_not_binding_request(sockaddr_in client_addr, char *buffer){
    *(short *)(&buffer[0]) = 0x0001;
    if(parse_verify_request(buffer,client_addr,20) == 400) return true;
    return false;
}

bool should_create_binding_response(char *buffer, sockaddr_in client_addr){
    std::vector<char> response = binding_response(buffer,client_addr,true,200);
    short message_type = (response[0] << 8) | (unsigned char)response[1];
    if(message_type == 0x0101) return true;
    return false;
}

bool should_create_binding_error_response(sockaddr_in client_addr, char *buffer){
    std::vector<char> response = binding_response(buffer,client_addr,true,400);
    short message_type = (response[0] << 8) | (unsigned char)response[1];
    if(message_type == 0x0111) return true;
    return false;
}

int main(){
    char *mock_buffer_var = mock_buffer();
    bool test1 = should_return_code_400_buffer_size_21(mock_client_address(),mock_buffer_var);
    bool test2 = should_return_code_400_buffer_size_less_than_20(mock_client_address(),mock_buffer_var);
    bool test3 = should_return_code_420_buffer_size_greater_than_21(mock_client_address(),mock_buffer_var);
    bool test4 = should_return_code_400_first_two_bits_not_zero(mock_client_address(),mock_buffer_var);
    bool test5 = should_return_code_400_not_binding_request(mock_client_address(),mock_buffer_var);
    bool test6 = should_create_binding_response(mock_buffer_var,mock_client_address());
    bool test7 = should_create_binding_error_response(mock_client_address(),mock_buffer_var);

    std::cout << "Test 1: " << test1 << std::endl;
    std::cout << "Test 2: " << test2 << std::endl;
    std::cout << "Test 3: " << test3 << std::endl;
    std::cout << "Test 4: " << test4 << std::endl;
    std::cout << "Test 5: " << test5 << std::endl;
    std::cout << "Test 6: " << test6 << std::endl;
    std::cout << "Test 7: " << test7 << std::endl;


    try{
        if(!test1) throw "should_return_code_400_buffer_size_21 Failed";
        if(!test2) throw "should_return_code_400_buffer_size_less_than_20 Failed";
        if(!test3) throw "should_return_code_420_buffer_size_greater_than_21 Failed";
        if(!test4) throw "should_return_code_400_first_two_bits_not_zero Failed";
        if(!test5) throw "should_return_code_400_not_binding_request Failed";
        if(!test6) throw "should_create_binding_response Failed";
        if(!test7) throw "should_create_binding_error_response(mock_client_address Failed";
    }catch(const char* msg){
        std::cerr << msg << std::endl;
        throw 20;
    }

}