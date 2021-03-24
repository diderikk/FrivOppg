# StunServer

Made by Diderik Kramer \
Last run  [CI/CD](https://github.com/diderikk/FrivOppg/actions)

## Introduction

StunServer is STUN-server implemented to return your public IPv4-address when you send a STUN-request. It currently only accepts UDP-protocol messages, but TCP may be added later. StunServer parses and validates your request and returns a adequate response. Response will either return your public IPv4-address or return a STUN error code.


- Implemented functionality
- Future work
- External dependencies
- Installation and run instructions
- References

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
    - Contains information about the software being used by STUN agent. This is information about a virtual machine run on Azure.
- ALTERNATE-SERVER
    - Reroutes the client to another STUN server. Used nslookup to find IPv4 address to stun1.l.google.com:19306, and redirects the STUN client to it. Currently, this is also implemented, but commented out as it not necessary for communication.
- THREADING
    - Main thread receives requests and sends the handler to a event loop on a seperate thread. This way main thread will continously be waiting for request, while event loop validates request and sends responses with addresses.

## Future work
- Accept requests with protcols TCP and TLS. This would mean to host a TCP or TLS server that receives requests on same address and port as StunServer does with UDP. TLS would require valid certification and a private key generated for both client and server for client to get their public IPv4 address. When implemented, clients can self choose what protocol to use when sending request to server.
- Implementing USERNAME and MESSAGE-INTEGRITY attributes to increase security of the response. The response would then be hashed, and the reponse would only be sent if both attributes are present. This would also imply only clients with the username and password can send request and expect a response.
- Add other attributes, REALM, NONCE and UNKNOWN-ATTRIBUTES. Most relevant would be UNKNOWN-ATTRIBUTES, as it's error code has been implemented, but the server is not telling the client what attribute is not understood by server. NONCE would require the use of USERNAME and MESSAGE-INTEGRITY.  
- IPv6. Currently, StunServer only accepts IPv4 protocol, and therefore only returns reponses with the public IPv4. For future work, set up an UDP server that accepts IPv6 packets.


## External depedencies
Docker \
Nodejs \
C++ Compiler 

## Optional depedencies
Makefile 


## Installation and run instructions
### First:

```
git clone https://github.com/diderikk/FrivOppg.git
cd FrivOppg
```

### Run StunServer:
[Make](https://www.gnu.org/software/make/): \
```make stun``` \
[Docker](https://docs.docker.com/get-docker/): \
```docker build -t stunserver ./stunserver && docker run -d -p 3478:3478/udp --rm --name stun stunserver``` \
C++ compiler: \
```g++ stunserver/server.cpp -lpthread -o server && ./server``` \
Can now be reached on port "127.0.0.1:3478" or "localhost:3478" 


### Run Peer to Peer application:
[Make](https://www.gnu.org/software/make/): \
```make p2p``` \
[Docker](https://docs.docker.com/get-docker/):
```docker build -t p2p_image ./p2p && docker run -d -p 80:3000 -p 3001:3001 --rm --name p2p p2p_image``` \
Node: \
```node p2p/P2Pserver.js``` \
If Peer to Peer was run by either Make or Docker, application can be viewed on localhost:80 or localhost. Else if node was used, application can be viewed on localhost:3000.

### Run Stun tests
Tests will be run before deploying a new version of StunServer, view.gitub/workflows/stun.yml \
[Make](https://www.gnu.org/software/make/): \
```make test``` \
C++ Compiler: \
```
g++ test/attribute_test.cpp -o test1 && ./test1 
g++ test/handler_test.cpp -o test1 && ./test1
rm test1
``` 



## References
[RFC 5389](https://tools.ietf.org/html/rfc5389#section-9) \
[UDP Server](https://www.geeksforgeeks.org/udp-server-client-implementation-c/) \
[HTTP Node Server](https://nodejs.dev/learn/the-nodejs-http-module) \
[WebRTC](https://webrtc.org/getting-started/overview) 





