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

    //Declare a variable of type struct sigaction
    struct sigaction sig;
    sig.sa_handler = sigHandler;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = 0;

    //Register signal handlers for ctrl-C and ctrl-Z
    int action3 = sigaction(SIGINT, &sig, NULL);
    int action4 = sigaction(SIGTSTP, &sig, NULL);

    //Ensure there are no errors in registering the handlers
    if (action3 < 0 || action4 < 0 ) {
       cout << "Error registering handlers, exiting..." << endl;
       return -1;
    }

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
    int connectedFds [numInterface];

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
        connectedFds[i] = recvfd;

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

    do {
        // Receive Further messages// 
        string message = recvMsg(recvfd, buffer);

        if (message.compare("Link Down") == 0) {
            sendMsg(recvfd, "Set Link Up");
        } 
        else if (message.compare("Link Up") == 0) {
            sendMsg(recvfd, "Monitor");
        }else{
            continue;
        }
    } while (keepRunning);

    cout << "Shutting down..." << endl;
    
    for(int i = 0; i < numInterface; i++){
        cout << "Shutting down: " << connectedFds[i] << endl;
        // Write "Shut Down" to Interface //
        sendMsg(connectedFds[i], "Shut Down");

        // Receive "Done" // 
        string message = recvMsg(connectedFds[i], buffer);
        if (message.compare("Done") == 0) {
            close(connectedFds[i]);
        }
    }

    remove(pathname);

    return 0;
}

static void sigHandler(int sig) {
    switch(sig) {
        case SIGINT:
            cout << "CTRL-C Interruption..." << endl;
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