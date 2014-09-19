#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define _WAVE24LOGGER_GLOBALS

#include "Wave24Logger.h"
#include "Wave24Exception.h"

static char* DFLT_LOGFILE="/home/sysop/logs/Wave24.log";
static char logmessage[1024];

Wave24Logger::Wave24Logger(char *logpath) {
  if (!logpath) {
    logfile=fopen(DFLT_LOGFILE,"a+");
  } else {
    logfile=fopen(logpath,"a+");
  }
  if (!logfile) {
    int myError=errno;
    sprintf(logmessage,"Error during opening log :%s",strerror(myError));
    Wave24Logger::stdlog('w',logmessage);
    Wave24Logger::stdlog('m',"Messages will go to standard output");
  }
  level=VERBOSE_LEVEL_DFLT;
}

Wave24Logger::Wave24Logger() {
  logfile=fopen(DFLT_LOGFILE,"a+");
  if (!logfile) {
    int myError=errno;
    sprintf(logmessage,"Error during opening log :%s",strerror(myError));
    Wave24Logger::stdlog('w',logmessage);
    Wave24Logger::stdlog('m',"Messages will go to standard output");
  }
  level=VERBOSE_LEVEL_DFLT;
}

int Wave24Logger::setLevel(int newLevel) {
  int oldLevel=level;
  level=newLevel;
  return oldLevel;
}

int Wave24Logger::getLevel() {
  return level;
}

bool Wave24Logger::wouldLog(int loglevel) {
  if (loglevel>level) return false;
  else return true;
}

void Wave24Logger::log(char type,char *message) {
  static char messageType[10];
  time_t currTime;
  
  switch (type) {
    case 'w':
    case 'W':
      sprintf(messageType,"Warning");
      break;
    case 'e':
    case 'E':
      sprintf(messageType,"Error  ");
      break;
    case 'n':
    case 'N':
      sprintf(messageType,"Note   ");
      break;
    default:
      sprintf(messageType,"Message");
      break;
  }
  currTime=time(NULL);
  static char timeString[30];
  char *newLine;
  memset(timeString,0,30);
  strcpy(timeString,ctime(&currTime));
  newLine=strchr(timeString,'\n');
  *newLine='\0';
  
  if (!logfile) {
    printf("%s - Wave24Drv:%s: %s\n",messageType,timeString,message);
  } else {
    fprintf(logfile,"%s - Wave24Drv:%s: %s\n",messageType,timeString,message);//ctime(&currTime),message);
    fflush(logfile);
  }
}

void Wave24Logger::log(char type,char *message,int loglevel) {
  static char messageType[10];
  time_t currTime;
  char logLevelString[5]={' ',' ',' ',' ','\0'};
  
  //this message has not to be logged because of the level
  if (loglevel>level) return;

  logLevelString[loglevel]='\0';

  switch (type) {
    case 'w':
    case 'W':
      sprintf(messageType,"Warning");
      break;
    case 'e':
    case 'E':
      sprintf(messageType,"Error  ");
      break;
    case 'n':
    case 'N':
      sprintf(messageType,"Note   ");
      break;
    default:
      sprintf(messageType,"Message");
      break;
  }
  currTime=time(NULL);
  static char timeString[30];
  char *newLine;
  memset(timeString,0,30);
  strcpy(timeString,ctime(&currTime));
  newLine=strchr(timeString,'\n');
  *newLine='\0';
  
  if (!logfile) {
    printf("%s - Wave24Drv:%s: %s%s\n",messageType,timeString,logLevelString,message);
  } else {
    fprintf(logfile,"%s - Wave24Drv:%s: %s%s\n",messageType,timeString,logLevelString,message);//ctime(&currTime),message);
    fflush(logfile);
  }
}

void Wave24Logger::stdlog(char type,char *message) {
  static char messageType[10];
  time_t currTime;
  
  switch (type) {
    case 'w':
    case 'W':
      sprintf(messageType,"Warning");
      break;
    case 'e':
    case 'E':
      sprintf(messageType,"Error  ");
      break;
    case 'n':
    case 'N':
      sprintf(messageType,"Note   ");
      break;
    default:
      sprintf(messageType,"Message");
      break;
  }
  currTime=time(NULL);
  static char timeString[30];
  char *newLine;
  memset(timeString,0,30);
  strcpy(timeString,ctime(&currTime));
  newLine=strchr(timeString,'\n');
  *newLine='\0';
  
  printf("%s - Wave24Drv:%s: %s\n",messageType,timeString,message);
}

void Wave24Logger::stdlog(char type,char *message,int loglevel) {
  static char messageType[10];
  time_t currTime;

  //loglevel is not used in stdout output

  switch (type) {
    case 'w':
    case 'W':
      sprintf(messageType,"Warning");
      break;
    case 'e':
    case 'E':
      sprintf(messageType,"Error  ");
      break;
    case 'n':
    case 'N':
      sprintf(messageType,"Note   ");
      break;
    default:
      sprintf(messageType,"Message");
      break;
  }
  currTime=time(NULL);
  static char timeString[30];
  char *newLine;
  memset(timeString,0,30);
  strcpy(timeString,ctime(&currTime));
  newLine=strchr(timeString,'\n');
  *newLine='\0';
  
  printf("%s - Wave24Drv:%s: %s\n",messageType,timeString,message);
}
