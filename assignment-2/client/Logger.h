enum LOG_LEVEL {
    CRITICAL = -2,
    ERROR = -1,
    WARNING = 0,
    DEBUG = 1
};

void Log(LOG_LEVEL level, char * filename, const char * funcname, int linenumber, string message);

int InitializeLog();

void SetLogLevel(LOG_LEVEL level);

void ExitLog();