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
#include <sys/ioctl.h>
#include <net/if.h>
#include "libraries.h"

using namespace std;

char pathname[13] = "/tmp/assign1";
bool running = true;

void error(const char *message);
static void sigHandler(int sig);
void sendMsg(int sockfd, string msg);
string recvMsg(int sockfd, char buffer[256]);
void cleanup(int sockfd);
int setInterfaceUp(int sockfd, string iface);

int main(int argc, char *argv[]) {
    string iface = argv[1];
    int sock, slen;
    struct sockaddr_un s_addr;
    string command;
    char buffer[256];

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
        cout << "Received: " << command << endl << endl;

        if (command.compare("Monitor") == 0) {
            string iface = argv[1];
            sendMsg(sock, "Monitoring");
            
            IMonitor * monitor = new IMonitor(iface);
            int ifStatus = monitor->runMonitor();
            
            if(ifStatus == -1){
                sendMsg(sock, "Link Down");
                monitor->stopMonitor();
                delete monitor;
            }
        } 
        else if (command.compare("Set Link Up") == 0) {
            int n = setInterfaceUp(sock, iface);
            if(n < 0){
                cout << strerror(errno) << endl;
                return -1;
            }else{
                sendMsg(sock, "Link Up");
            }
        }
        else if (command.compare("Shut Down") == 0) {
            sendMsg(sock, "Done");
            cleanup(sock);
            running = false;
        }
        else {
            cout << "Unrecognized Command" << endl;
        }
    } while (running);

    return 0;
}

int setInterfaceUp(int sockfd, string iface){
    struct ifreq ifr;

    memset(&ifr, 0, sizeof ifr);

    strncpy(ifr.ifr_name, iface.c_str(), IFNAMSIZ);

    ifr.ifr_flags |= IFF_UP;
    return ioctl(sockfd, SIOCSIFFLAGS, &ifr);
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