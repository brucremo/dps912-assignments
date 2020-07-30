#ifndef IMONITOR_H
#define IMONITOR_H

#include <string>

using namespace std;

class IMonitor {
    private:
        string ifpath;
        string ifName;
        string ifState;
        string ifup_count;
        string ifdown_count;
        string ifrx_bytes;
        string ifrx_dropped;
        string ifrx_errors; 
        string ifrx_packets;
        string iftx_bytes; 
        string iftx_dropped;
        string iftx_errors; 
        string iftx_packets;

        bool keepRunning;

        void printStatus();
        void readStatus();
        bool ifDown();
    public:
        IMonitor(string ifName);
        string readFileContent(string fileName);
        int runMonitor();
        void stopMonitor();
};
#endif