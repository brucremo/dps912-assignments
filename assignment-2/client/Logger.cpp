#include "Logger.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <mutex>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <thread>
#include <pthread.h>
#include <iostream>

using namespace std;

// Thread variables
bool is_running = true;
mutex logger_mutex;

// Global variables
const int MAX_BUFFER_SIZE = 256, server_port = 4201;
struct sockaddr_in server_addr; 
int sock_fd; 
char buffer[MAX_BUFFER_SIZE]; 
LOG_LEVEL global_log_level = ERROR;
string server_ip = "127.0.0.1";
socklen_t socket_len;

// Thread functions declaration
void run_receiver(int fd);

// Interfaced functions
void Log(LOG_LEVEL level, char * filename, const char * funcname, int linenumber, char * message){

    if(level > global_log_level){
        time_t now = time(0);
        char *dt = ctime(&now);
        memset(buffer, 0, MAX_BUFFER_SIZE);
        char levelStr[][16]={"DEBUG", "WARNING", "ERROR", "CRITICAL"};
        int datalen = sprintf(buffer, "%s %s %s:%s:%d %s\n", dt, levelStr[level], filename, funcname, linenumber, message) + 1;
        buffer[datalen-1]='\0';

        sendto(sock_fd, buffer, datalen, 0, (struct sockaddr *)&server_addr, socket_len);
    }
}

void InitializeLog(){
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) { 
        perror("ERROR: Unable to create socket"); 
        exit(EXIT_FAILURE); 
    } 

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str()); 
    server_addr.sin_port = htons(server_port); 

    socket_len = sizeof(server_addr);

    thread receive_thread(run_receiver, sock_fd);
    receive_thread.detach();
}

void SetLogLevel(LOG_LEVEL level){
    global_log_level = level;
}

void ExitLog(){
    is_running = false;
    close(sock_fd);
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
        int size = recvfrom(fd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &socket_len);

        string message = buffer;

        if(size > 0){
            string log_level = message.substr((message.find("=")+1));

            if(log_level == "DEBUG"){
                global_log_level = DEBUG;
            }else if(log_level == "WARNING"){
                global_log_level = WARNING;
            }else if(log_level == "ERROR"){
                global_log_level = ERROR;
            }else if(log_level == "CRITICAL"){
                global_log_level = CRITICAL;
            }else{
                continue;
            }
        }else{
            sleep(1);
        }
        message = "";
        logger_mutex.unlock();
    }
}