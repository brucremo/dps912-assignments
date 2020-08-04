#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <mutex>
#include <thread>

using namespace std;

// Support functions declaration
static void sigHandler(int sig);
void error(char *msg);
void send_log_level(char * level);

// User input functions
void set_log_level();
void dump_log();
void shutdown();

// Global variables
const char * log_path = "/tmp/logserver_logs";
const int MAX_BUFFER_SIZE = 256, server_port = 4201;
struct sockaddr_in server_addr, client_addr; 
bool keep_running = true;
int sock_fd; 
char buffer[MAX_BUFFER_SIZE]; 
socklen_t socket_len;

// Thread variables
bool is_running = true;
mutex logger_mutex;

// Thread functions declaration
void run_receiver(int fd);

int main()
{
    socket_len = sizeof(client_addr);

    //Declare a variable of type struct sigaction
    struct sigaction sig;
    sig.sa_handler = sigHandler;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = 0;

    //Register signal handlers for ctrl-C
    int action3 = sigaction(SIGINT, &sig, NULL);

    sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket with the server address 
    if (bind(sock_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 ) 
    { 
        perror("ERROR: Cannot bind socket"); 
        exit(EXIT_FAILURE); 
    } 

    thread receive_thread(run_receiver, sock_fd);
    receive_thread.detach();

    while(keep_running){

        cout << " -------- Logging Server -------- " << endl;
        cout << " ---- Please Select an Option ---- " << endl;
        cout << "1. Set Log Level" << endl;
        cout << "2. Dump the log file here" << endl;
        cout << "0. Shutdown" << endl;
        cout << "Selection: ";

        int input;
        cin >> input;

        switch (input)
        {
        case (1):
            set_log_level();
            break;
        case(2):
            dump_log();
            break;
        case(0):
            shutdown();
            break;
        default:
            cout << "INVALID COMMAND: Please use 1, 2 or 0" << endl << endl;
        }
    }
    return 0;
}

void send_log_level(char * level){
    memset(buffer, 0, MAX_BUFFER_SIZE);
    int len = sprintf(buffer, "Set Log Level=%s", level) + 1;
    cout << "Sending: " << buffer << endl;
    sendto(sock_fd, buffer, len, 0, (struct sockaddr *)&client_addr, socket_len);
}

// User input functions
void set_log_level(){
    cout << " ---- Please Select an Option ---- " << endl;
    cout << "1. DEBUG" << endl;
    cout << "2. WARNING" << endl;
    cout << "3. ERROR" << endl;
    cout << "4. CRITICAL" << endl;
    cout << "0. Return to main menu" << endl;
    cout << "Selection: ";

    int input;
    cin >> input;

    switch (input)
    {
    case(1):
        send_log_level("DEBUG");
        break;
    case(2):
        send_log_level("WARNING");
        break;
    case(3):
        send_log_level("ERROR");
        break;
    case(4):
        send_log_level("CRITICAL");
        break;
    case(0):
        cout << "Returning to main menu..." << endl;
        break;
    default:
        cout << "INVALID COMMAND: Please use 0, 1, 2, 3 or 4" << endl << endl;
    }
}

void dump_log(){
    FILE* fp = fopen(log_path, "r");

    if (fp == NULL){
        error("Can't open file");
    }

    char* line = NULL;
    size_t len = 0;
    while ((getline(&line, &len, fp)) != -1) {
        cout << line << endl;
    }
    fclose(fp);
}

void shutdown(){
    cout << "Shutting down..." << endl;
    is_running = false;
    keep_running = false;
}

// Support functions implementation
void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

static void sigHandler(int sig) {
    switch(sig) {
        case SIGINT:
            cout << "CTRL-C Interruption..." << endl;
            is_running = false;
            keep_running = false;
            break;
        case SIGTSTP:
            cout << "CTRL-Z Interruption..." << endl;
            break;
	default:
	    cout << "Undefined signal..." << endl;
    }
}

// Thread functions implementation
void run_receiver(int fd){

    // Set 1 second timeout for the socket
    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 1;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout));

    // Run receiver logic
    while(is_running){

        memset(buffer, 0, MAX_BUFFER_SIZE);

        logger_mutex.lock();
        FILE * log_file;
        log_file = fopen (log_path, "a");

        int size = recvfrom(fd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &socket_len);

        string message = buffer;

        if(size > 0){
            fprintf(log_file, "%s", message.c_str());
        }else{
            sleep(1);
        }
        fclose(log_file);
        logger_mutex.unlock();
    }
}