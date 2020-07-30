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
#include "libraries.h"

using namespace std;

char pathname[13] = "/tmp/assign1";
bool running = true;

void error(const char *message);
static void sigHandler(int sig);
void sendMsg(int sockfd, string msg);
string recvMsg(int sockfd, char buffer[256]);
void cleanup(int sockfd);

int main(int argc, char *argv[]) {

    int sock, slen;
    struct sockaddr_un s_addr;
    struct sigaction sigAct;
    string command;
    char buffer[256];

    // Setup Signal Handlers //    
    sigAct.sa_handler = sigHandler;
    sigemptyset(&sigAct.sa_mask);
    sigAct.sa_flags = 0;
    int err1 = sigaction(SIGINT, &sigAct, NULL);
    int err2 = sigaction(SIGTSTP, &sigAct, NULL);
    if (err1 < 0 || err2 < 0) {
       cout << "Cannot create the signal handler" << endl;
       return -1;
    }

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
    }else{
        cout << "Connected to Server! " << endl;
    }

    // Ready to Start //
    sendMsg(sock, (string)"Ready");

    do {
        command = recvMsg(sock, buffer);

        if (command.compare("Monitor") == 0) {
            string iface = argv[1];
            sendMsg(sock, "Monitoring");
            
            IMonitor * monitor = new IMonitor(iface);
            int ifStatus = monitor->runMonitor();
            
            if(ifStatus == -1){
                sendMsg(sock, "Link Down");
            }
        } 
        else if (command.compare("Set Link Up") == 0) {

        }
        else if (command.compare("Shut Down") == 0) {

        }
        else {
            cout << "Unrecognized Command" << endl;
        }
    } while (running);

    return 0;
}
void error(const char *message) {
    perror(message);
    exit(0);
}

void sendMsg(int sockfd, string msg) {
    cout << getpid() << " - Sending: " << msg << endl;
    write(sockfd, msg.c_str(), msg.length() + 1);
}

string recvMsg(int sockfd, char buffer[256]) {
    bzero(buffer, 255);
    read(sockfd, buffer, 255);
    return (string)buffer;
}

void cleanup(int sockfd) {
    sendMsg(sockfd, (string)"Done");
    close(sockfd);
}

static void sigHandler(int sig) {
    switch(sig) {
        case SIGINT:
            cout << "CTRL-C Interruption... Shutting Down..." << endl;
            running = false;
            break;
        case SIGTSTP:
            cout << "CTRL-Z Interruption..." << endl;
            // Discard //
            break;
	default:
	    cout << "Undefined signal..." << endl;
    }
}