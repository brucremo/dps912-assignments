#include "Logger.h"
#include <string>
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

using namespace std;

// Thread variables
bool is_running = true;
thread send_thread, receive_thread;
mutex logger_mutex;

// Global variables
const int MAX_BUFFER_SIZE = 1024, server_port = 8080;
struct sockaddr_in server_addr, client_addr; 
int sock_fd; 
char buffer[MAX_BUFFER_SIZE]; 
LOG_LEVEL global_log_level;
string server_ip = "127.0.0.1";

// Thread functions declaration


// Interfaced functions
void Log(LOG_LEVEL level, char * filename, const char * funcname, int linenumber, string message){

}

int InitializeLog(){
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) { 
        perror("ERROR: Unable to create socket"); 
        exit(EXIT_FAILURE); 
    } 

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str()); 
    server_addr.sin_port = htons(server_port); 
}

void SetLogLevel(LOG_LEVEL level){
    global_log_level = level;
}

void ExitLog(){

}

// Thread functions implementation