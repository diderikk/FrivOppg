#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <bitset>
#include <iostream>

#define MAXLINE 200

int stun_xor_addr(char *stun_server_ip, short stun_server_port, short local_port, char *return_ip_port);

int main(int argc, char *argv[])
{
        if (argc != 4)
        {
                printf("usage: %s <server_ip> <server_port> <local_port>\n\n", argv[0]);
                exit(1);
        }

        printf("Main start ... \n");

        int n = 0;
        char return_ip_port[50];
        n = stun_xor_addr(argv[1], atoi(argv[2]), atoi(argv[3]), return_ip_port);
}

int stun_xor_addr(char *stun_server_ip, short stun_server_port, short local_port, char *return_ip_port)
{
        struct sockaddr_in servaddr;
        struct sockaddr_in localaddr;
        unsigned char buf[MAXLINE];
        int sockfd, i;
        unsigned char bindingReq[20];

        int stun_method, msg_length;
        short attr_type;
        short attr_length;
        short port;
        short n;

        //# create socket
        sockfd = socket(AF_INET, SOCK_DGRAM, 0); // UDP

        // server
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        inet_pton(AF_INET, stun_server_ip, &servaddr.sin_addr);
        servaddr.sin_port = htons(stun_server_port);

        // local
        bzero(&localaddr, sizeof(localaddr));
        localaddr.sin_family = AF_INET;
        //inet_pton(AF_INET, "192.168.0.181", &localaddr.sin_addr);
        localaddr.sin_port = htons(local_port);

        n = bind(sockfd, (struct sockaddr *)&localaddr, sizeof(localaddr));

        printf("socket opened to  %s:%d  at local port %d\n", stun_server_ip, stun_server_port, local_port);

        //## first bind
        *(short *)(&bindingReq[0]) = htons(0x0001);   // stun_method
        *(short *)(&bindingReq[2]) = htons(0x0000);   // msg_length
        *(int *)(&bindingReq[4]) = htonl(0x2112A442); // magic cookie

        *(int *)(&bindingReq[8]) = htonl(0x63c7117e); // transacation ID
        *(int *)(&bindingReq[12]) = htonl(0x0714278f);
        *(int *)(&bindingReq[16]) = htonl(0x5ded3221);

        n = sendto(sockfd, bindingReq, sizeof(bindingReq), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)); // send UDP
        if (n == -1)
        {
                printf("sendto error\n");
                return -1;
        }

        // time wait
        sleep(1);

        printf("Read recv ...\n");
        n = recvfrom(sockfd, buf, MAXLINE, 0, NULL, 0); // recv UDP
        if (n == -1)
        {
                printf("recvfrom error\n");
                return -2;
        }
        //printf("Response from server:\n");
        //write(STDOUT_FILENO, buf, n);
        if (*(short *)(&buf[0]) == 0x0101)
        {
                printf("STUN binding resp: success !\n");

                // parse XOR
                n = *(short *)(&buf[2]);

                int magic_cookie = *(int *)(&buf[4]);

                short type = *(short *)(&buf[20]);
                if (type == 0x0001)
                {
                        printf("Type: %d\n", type);
                }
                short length = *(short *)(&buf[22]);
                char fam = *(char *)(&buf[25]);


                short port = *(short *)(&buf[26]);
                port = ntohs(port ^ (magic_cookie >> 16));
                std::cout << port << std::endl;

                int ip_addr = *(int *)(&buf[28]);


                ip_addr = ip_addr ^ magic_cookie;


                localaddr.sin_addr.s_addr = ip_addr;
                std::string inet_addr = inet_ntoa(localaddr.sin_addr);
                std::cout << inet_addr << std::endl;
        }

        // TODO: bind again

        close(sockfd);
        printf("socket closed !\n");

        return 0;
}