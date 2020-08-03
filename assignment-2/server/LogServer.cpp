#include<iostream>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

using namespace std;

void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

const int MAX_BUFFER_SIZE = 256, server_port = 4201;

int main()
{
    int sock_fd;
    struct sockaddr_in server_addr, client_addr; 
    char buffer[256];
    socklen_t socket_len = sizeof(client_addr);

    sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int rc = recvfrom(sock_fd, buffer, sizeof(buffer), 0,(struct sockaddr *)&client_addr, &socket_len);

    cout << buffer << endl;

    return 0;
}
