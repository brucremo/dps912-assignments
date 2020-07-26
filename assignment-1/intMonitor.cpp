// intMonitor.cpp
// By: Havana Kaba & Bruno Alexander
// For Miguel Watler
// Class: DPS912
// Assignment #1
// July 31, 2020

#include <fcntl.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

char pathname[13] = "/tmp/assign1";

void error(const char *message);

int main(int argc, char *argv[]) {
    int sock, slen, command;
    struct sockaddr_un s_addr;

    // Setup the Socket //
    bzero((char *)&s_addr, sizeof(s_addr));
    s_addr.sun_family = AF_UNIX;
    strcpy(s_addr.sun_path, pathname);
    slen = strlen(s_addr.sun_path) + sizeof(s_addr.sun_family);
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        error("Could not create socket");
    }

    // Connect to the Network Monitor //
    if (connect(sock, (struct sockaddr *) &s_addr, slen) < 0) {
        error("Could not connect to server");
    }

    // Ready to Start //
    


    return 0;
}

void error(const char *message) {
    perror(message);
    exit(0);
}