# StunServer


## Introduction

StunServer is STUN-server implemented to return your public IPv4-address when you send a STUN-request. It currently only accepts UDP-protocol messages, but TCP may be added later. StunServer parses and validates your request and returns a adequate response. Response will either return your public IPv4-address or return a STUN error code.


- Implemented functionality
- Future work
- External dependencies
- Installation instructions
- Instructions for starting STUN-server
- API Sources

## Implemented functionality
- DEPLOYABLE
    - Pushing to Github, will start an Github action defined from stun.ymlstart running, connecting to an Virtual Machine through SSH. After connecting, it will copy over files before running the Makefile. Makefile run docker scripts for removing previous installations og running a new container that is running the StunServer.
- DEPLOYED
    - StunServer should be running on 40.85.141.137:3478. The Peer to Peer application is already using it. This is a Virtual Machine from Azure Virtual Machines. It is currently running with Ubuntu 18.04-LTS on 1 vCPU and 0.5 GiB RAM. This can scaled, by upgrading.
- MAPPED-ADDRESS
    - Returns your network address and port
- XOR-MAPPED-ADDRESS
    - Returns your network address and port xored with the Magic Cookie
- ERROR-CODE
    - Validates received request and returns adequate response
    - Only implemented error codes 400 and 420, 420 for requests with more than 21 bytes. This is because StunServer does not expect any attributes in an request. This means any STUN request with 22 bytes the server expects it to be an request with an attribute type(2 bytes).
- SOFTWARE
    - Contains information about the software being used by STUN agent. Have been implemented but commented out, as it is not necessary for either STUN agents.
- ALTERNATE-SERVER
    - Reroutes the client to another STUN server. Used nslookup to find IPv4 address to stun1.l.google.com:19306, and redirects the STUN client to it. Currently, this is also implemented, but commented out as it not necessary for communication.
- THREADING
    - Main thread receives requests and sends the handler to a event loop on a seperate thread. This way main thread will continously be waiting for request, while event loop validates request and sends responses with addresses.

## Future work
- Accept requests with the protcols TCP and TLS. This would mean to host a TCP or TLS server that receives requests on same address and port as StunServer does with UDP. TLS would require valid certification and a private key generated for both client and server for client to get their public IPv4 address.
- Implementing USERNAME and MESSAGE-INTEGRITY attributes to increase security of the response. The response would then be hashed, and the reponse would only be sent if both attributes are present. This would also imply only clients with the username and password can send request and expect a response.
- Add other attributes, REALM, NONCE and UNKNOWN-ATTRIBUTES. Most relevant would be UNKNOWN-ATTRIBUTES, as it's error code has been implemented, but not telling client what attribute is not understood by server. NONCE would require the use of USERNAME and MESSAGE-INTEGRITY.  
- IPv6. Currently, StunServer only accepts IPv4 protocol, and therefore only return a reponse with the public IPv4. 
- Unit testing. Currently, only CI job implemented is "prettier" package from Node for spell and format checking for all JavaScript code in the Peer to Peer application code. 


## External depedencies
Docker \
Nodejs

## Optional depedencies
Makefile \
C++ Compiler


## Installation instructions






[Last Run CI/CD](https://github.com/diderikk/FrivOppg/actions)