// intMonitor.cpp
// By: Havana Kaba & Bruno Alexander
// For Miguel Watler
// Class: DPS912
// Assignment #1
// July 31, 2020

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using namespace std;

char pathname[13] = "/tmp/assign1";
bool keepRunning = true;

static void sigHandler(int sig);
void error(const char *message);
void sendMsg(int sockfd, string msg);
string recvMsg(int sockfd, char * buffer);
void cleanup();

int main(int argc, char *argv[]) {
    int masterfd, recvfd, slen, numInterface, ret;
    struct sockaddr_un s_addr, c_addr;
    vector<string> intNames;
    socklen_t clen;
    fd_set fdset;
    bool isParent = true;
    char buffer[256];

    FD_ZERO(&fdset);

    // Request User Input //
    while (true) {
        cout << "How Many Interfaces: ";
        cin >> numInterface;
        for (int i = 0; i < numInterface; i++) {
            string name;
            cout << "Name Interface #" << i + 1 << ": ";
            cin >> name;
            intNames.push_back(name);
        }
        break;
    }

    pid_t intPids[numInterface];

    for (int i = 0; i < numInterface; i++) {
        
        // Fork and Exec all Interface Monitors //
        intPids[i] = fork();
        if (intPids[i] == 0) {
            isParent = false;
            execlp("./intMonitor", "./intMonitor", intNames[i].c_str(), NULL);
            cout << "Error executing intMonitor with pid: " << getpid() << endl;
	        cout << strerror(errno) << endl;
        }
    }

    // Set Socket File Descriptor //
    if ((masterfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        error("Could not create socket");
    }

    // Setup the Socket //
    bzero((char *) &s_addr, sizeof(s_addr));
    s_addr.sun_family = AF_UNIX;
    strcpy(s_addr.sun_path, pathname);
    slen = strlen(s_addr.sun_path) + sizeof(s_addr.sun_family);

    // Bind and Listen on Socket //
    if ((bind(masterfd, (struct sockaddr *)&s_addr, slen)) < 0) { 
        error("Error binding the socket");
    }

    for (int i = 0; i < numInterface; i++) {
        cout << "Listening on socket..." << endl;
        listen(masterfd, 5);

        // Accept the Connection //
        clen = sizeof(c_addr);
        recvfd = accept(masterfd, (struct sockaddr *)&c_addr, &clen);
        if (recvfd < 0) { error("Could not accept connection."); }
        FD_SET(recvfd, &fdset);

        // Receive "Ready" from Interface //
        string message = recvMsg(recvfd, buffer);
        if(message != "Ready"){
            cout << "Error: Not ready" << endl;
        }

        // Write "Monitor" to Interface //
        sendMsg(recvfd, "Monitor");

        // Receive "Monitoring Confirmation" // 
        message = recvMsg(recvfd, buffer);

        if(message != "Monitoring"){
            cout << "Error: Monitoring failed" << endl;
            sendMsg(recvfd, "Shut Down");
        }
    }
    
    return 0;
}

void cleanup() {
    remove(pathname);
}

static void sigHandler(int sig) {
    switch(sig) {
        case SIGINT:
            cout << "CTRL-C Interruption... Shutting Down..." << endl;
            keepRunning = false;
            break;
        case SIGTSTP:
            cout << "CTRL-Z Interruption..." << endl;
            break;
	default:
	    cout << "Undefined signal..." << endl;
    }
}

void error(const char *message) {
    perror(message);
    exit(0);
}

void sendMsg(int sockfd, string msg) {
    cout << getpid() << " - Sending: " << msg << endl;
    write(sockfd, msg.c_str(), msg.length() + 1);
}

string recvMsg(int sockfd, char * buffer) {
    bzero(buffer, 255);
    read(sockfd, buffer, 255);
    return (string)buffer;
}