#include <string>

using namespace std;

enum LOG_LEVEL {
    DEBUG = -1,
    WARNING = 0,
    ERROR = 1,
    CRITICAL = 2
};

void Log(LOG_LEVEL level, char * filename, const char * funcname, int linenumber, char * message);

void InitializeLog();

void SetLogLevel(LOG_LEVEL level);

void ExitLog();