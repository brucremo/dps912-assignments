// intMonitor.cpp
// By: Havana Kaba & Bruno Alexander
// For Miguel Watler
// Class: DPS912
// Assignment #1
// July 31, 2020

#include <fcntl.h>
#include <fstream>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

char pathname[13] = "/tmp/assign1";

static void sigHandler(int sig);
void error(const char *message);

int main(int argc, char *argv[]) {
    int masterfd, recv, slen;
    socklen_t clen;
    struct sockaddr_un s_addr, c_addr;
    char buffer[128];

    // Set Socket File Descriptor //
    if ((masterfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        error("Could not create socket");
    }

    // Setup the Socket //
    bzero((char *) &s_addr, sizeof(s_addr));
    s_addr.sun_family = AF_UNIX;
    strcpy(s_addr.sun_path, pathname);
    slen = strlen(s_addr.sun_path) + sizeof(s_addr.sun_family);

    // Bind the Socket //
    if ((bind(masterfd, (struct sockaddr *)&s_addr, slen)) < 0) { 
        error("Error binding the socket");
    }

    // Listen on Socket //
    listen(masterfd, 5);
    cout << "Listening on socket..." << endl;

    // Accept the Connection //
    clen = sizeof(c_addr);
    recv = accept(masterfd, (struct sockaddr *)&c_addr, &clen);
    if (recv < 0) { error("Could not accept connection."); }
    
    return 0;
}

static void sigHandler(int sig) {
    switch(sig) {
        case SIGINT:
            cout << "CTRL-C Interrution... Shutting Down..." << endl;
            break;
        case SIGTSTP:
            cout << "intfMonitor: ctrl-Z discarded" << endl;
            break;
	default:
	    cout << "Undefined signal..." << endl;
    }
}

void error(const char *message) {
    perror(message);
    exit(0);
}