#ifndef _WAVE24LOGGER
#define _WAVE24LOGGER

class Wave24Logger {
  public:
    Wave24Logger(char *logpath);
    Wave24Logger();
    void log(char type,char *message);
    void log(char type,char *message,int loglevel);
    int setLevel(int newLevel);
    int getLevel();
    bool wouldLog(int loglevel);
    static const int VERBOSE_NONE=0;
    static const int VERBOSE_VERBOSE=1;
    static const int VERBOSE_DEBUG=2;
    static const int VERBOSE_VERYDEBUG=3;
    static const int VERBOSE_LEVEL_DFLT=0;
    static void stdlog(char type,char *message);
    static void stdlog(char type,char *message,int loglevel);
  private:
    FILE *logfile;
    int level;
};

#endif


