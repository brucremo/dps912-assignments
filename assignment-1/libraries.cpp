#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include "libraries.h"

using namespace std;

string ifsyspath = "/sys/class/net/";

string IMonitor::readFileContent(string fileName){

    ifstream infile;
    infile.open(fileName.c_str());
    string data;
    if(infile.is_open()) {
        infile >> data;
    }

    infile.close();
    return data;
}
  
IMonitor::IMonitor(string ifName){
    this->ifName = ifName;
    this->ifpath = ifsyspath + ifName + "/";
    this->keepRunning = true;
}

void IMonitor::printStatus(){
    this->readStatus();
    cout << "Interface: " << this->ifName << " state: " << this->ifState << " up_count: " << this->ifup_count << " down_count: " << this->ifdown_count << endl;
    cout << "rx_bytes: " << this->ifrx_bytes << " rx_dropped: " << this->ifrx_dropped << " rx_errors: " << this->ifrx_errors << " rx_packets: " << this->ifrx_packets << endl;
    cout << "tx_bytes: " << this->iftx_bytes << " tx_dropped: " << this->iftx_dropped << " tx_errors: " << this->iftx_errors << " tx_packets: " << this->iftx_packets << endl;
    cout << endl;
}

bool IMonitor::ifDown(){
    cout << readFileContent(this->ifpath + "operstate") << endl;
    return readFileContent(this->ifpath + "operstate") != "up";
}

void IMonitor::readStatus(){
    this->ifState = readFileContent(this->ifpath + "operstate");
    this->ifup_count = readFileContent(this->ifpath + "carrier_up_count");
    this->ifdown_count = readFileContent(this->ifpath + "carrier_down_count");

    this->ifrx_bytes = readFileContent(this->ifpath + "statistics/rx_bytes");
    this->ifrx_dropped = readFileContent(this->ifpath + "statistics/rx_dropped");
    this->ifrx_errors = readFileContent(this->ifpath + "statistics/rx_errors");
    this->ifrx_packets = readFileContent(this->ifpath + "statistics/rx_packets");

    this->iftx_bytes = readFileContent(this->ifpath + "statistics/tx_bytes");
    this->iftx_dropped = readFileContent(this->ifpath + "statistics/tx_dropped");
    this->iftx_errors = readFileContent(this->ifpath + "statistics/tx_errors");
    this->iftx_packets = readFileContent(this->ifpath + "statistics/tx_packets");
}

void IMonitor::stopMonitor(){
    this->keepRunning = false;
}

int IMonitor::runMonitor(){

    do{
        this->printStatus();
        sleep(1);
    } while(this->keepRunning && !this->ifDown());

    if(this->ifDown()){
        return -1;
    }else{
        return 0;
    }
}

