# StunServer


## Introduction

StunServer is STUN-server implemented to return your public IPv4-address when you send a STUN-request. It currently only accepts UDP-protocol messages, but TCP may be added later. StunServer parses and validates your request and returns a adequate response.


- Implemented functionality
- Future work
- External dependencies
- Installation instructions
- Instructions for starting STUN-server
- API Sources

## Implemented functionality
- MAPPED-ADDRESS
    - Returns your network address and port
- XOR-MAPPED-ADDRESS
    - Returns your network address and port xored with the Magic Cookie
- ERROR-CODE
    - Validates received request and returns adequate response
    - Only implemented error codes 400 and 420, 420 for requests with more than 21 bytes. This is because StunServer does not expect any attributes in an request. This means any STUN request with 22 bytes the server expects it to be an attribute with an attribute type.
- SOFTWARE
    - Contains information about the software being used by STUN agent. Have been implemented but commented out.
- ALTERNATE-SERVER
    - Reroutes the client to another STUN server. Used nslookup to find IPv4 address to stun1.l.google.com:19306, and redirects the STUN client to it.
- THREADING
    - Main thread receives requests and sends the handler to a event loop on a seperate thread

## Future work
- Accept requests with the protcols TCP and TLS. This would mean to host a TCP server that receives request on same address and port as StunServer does with UDP. 
- Implementing USERNAME and MESSAGE-INTEGRITY attributes to increase security of the response. The response would then be hashed, and the reponse would only be sent if both attributes are present. This would also imply only clients with the username and password can send request and expect a response.
- Add other attributes, REALM, NONCE and UNKNOWN-ATTRIBUTES. Most relevant would be UNKNOWN-ATTRIBUTES, as it's error code has been implemented, but not telling client what attribute is not understood by server. NONCE would require the use of USERNAME and MESSAGE-INTEGRITY. SOFTWARE 
- IPv6. Currently, StunServer only accepts IPv4 protocol, and therefore only returns a reponse with the public IPv4. 


## External depedencies
Docker.io

## Optional depedencies
Makefile
C++ Compiler





[Last Run CI/CD](https://github.com/diderikk/FrivOppg/actions)