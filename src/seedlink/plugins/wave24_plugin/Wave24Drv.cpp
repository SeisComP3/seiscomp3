// #include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <getopt.h>
#include <termios.h>
#include <math.h>

#include "plugin.h"

#include "Wave24Messages.h"
#include "Wave24Exception.h"
#include "Wave24Logger.h"
#include "Wave24Seed.h"

static unsigned char result[Wave24Messages::MAX_ANSWER_LEN];
static char command[Wave24Messages::MAX_COMMAND_LEN];

static const unsigned char CR=0x0D;
static const unsigned char LF=0x0A;

static char* DFLT_CFGPATH="/home/sysop/config/Wave24.ini";
static char* DFLT_STATFILE="/home/sysop/status/Wave24.sqn";

static char logmessage[1024];

class Wave24Drv {
  public:
    Wave24Drv(char *cfgfile,char *logfile);
    Wave24Drv(char *cfgfile);
    Wave24Drv();
    int startDataAcquisition();
    int newData();
    int shareData(int len);
    int initADC();
    bool checkADC();
    int connectADC();
    int disconnectADC();
    int sendCommandWithAck(char *cmd);
    int sendCommand(char *cmd);
    int sendAscii(char *cmd);
    int sendAscii(char *cmd,char *expectResult,long timeout);
    int recvAscii(long timeout);
    int recvAscii(char *expectedResult,long timeout);
    void proto(char type,char *message,int verbosity);
    void proto(char type,char *message);
    void setStation(char *name);
    void setNetwork(char *name);
    void setChannel(int number,char *name);
    void setCommPort(char *path);
    int getCommPortFd(){return commPortFd;}
    void setCommPortSpeed(int speed);
    void setSampleRate(int rate);
    void setDecimRate(int rate);
    int addFdToSet(int fd);
    bool isFdInSet(int fd);
    void clearSet();
    void waitForSet(long microsecs);
    int setLogLevel(int newlevel) {if (myLog) return myLog->setLevel(newlevel);}
    void setUseSqn(long sqnDiff) {if (sqnDiff<0) useSqn=false;else {useSqn=true;allowedSqnDiff=sqnDiff;}}
    static void printHelp();
     
  private:
    int configure();
    int configure(char *cfgpath);
    int openCommPort();
    int closeCommPort();
    int commPortFd;
    int commPortSpeed;
    char commPort[100];
    bool useSqn;
    long allowedSqnDiff;
    char network[3];
    char stat[6];
    char c1[4];
    char c2[4];
    char c3[4];
    char c4[4];
    char c5[4];
    char c6[4];
    char c7[4];
    char c8[4];
    int sampleRate;
    int samplePeriod;
    int decimRate;
    unsigned char lastPacketSqn[7];
    long lastGpsOk;
    long gpsCheckPeriod;
    long sampleErrorPeriod;
    fd_set readFds;
    int maxFd;
    Wave24Logger *myLog;
    TxMessage *txMessage;
    RxMessage *rxMessage;
};

static Wave24Drv *driver;

Wave24Drv::Wave24Drv() {
  try {
    myLog=new Wave24Logger();
  } catch (Wave24Exception *e) {
    if (!e->canContinue()) {
      Wave24Logger::stdlog('e',e->describe());
      throw new Wave24Exception("Can not create logger",e->canContinue());
    } else {
      Wave24Logger::stdlog('w',e->describe());
    }
  }  

  try {
    configure();
  } catch (Wave24Exception *e) {
    if (!e->canContinue()) {
      proto('e',e->describe());
      throw new Wave24Exception("Can not configure driver",e->canContinue());
    } else {
      proto('w',e->describe(),Wave24Logger::VERBOSE_NONE);
    }
  }

  proto('n',"Driver initialized",Wave24Logger::VERBOSE_NONE);
}
  
Wave24Drv::Wave24Drv(char *cfgfile) {
  try {
    myLog=new Wave24Logger();
  } catch (Wave24Exception *e) {
    if (!e->canContinue()) {
      Wave24Logger::stdlog('e',e->describe());
      throw new Wave24Exception("Can not create logger",e->canContinue());
    } else {
      Wave24Logger::stdlog('w',e->describe());
    }
  }  

  try {
    if (!cfgfile) configure();
    else configure(cfgfile);
  } catch (Wave24Exception *e) {
    if (!e->canContinue()) {
      proto('e',e->describe());
      throw new Wave24Exception("Can not configure driver",e->canContinue());
    } else {
      proto('w',e->describe(),Wave24Logger::VERBOSE_NONE);
    }
  }

  proto('n',"Driver initialized",Wave24Logger::VERBOSE_NONE);
}

Wave24Drv::Wave24Drv(char *cfgfile,char *logfile) {
  try {
    if (!logfile) myLog=new Wave24Logger();
    else myLog=new Wave24Logger(logfile);
  } catch (Wave24Exception *e) {
    if (!e->canContinue()) {
      Wave24Logger::stdlog('e',e->describe());
      throw new Wave24Exception("Can not create logger",e->canContinue());
    } else {
      Wave24Logger::stdlog('w',e->describe());
    }
  }  

  proto('m',"Log opened");

  try {
    if (!cfgfile) configure();
    else configure(cfgfile);
  } catch (Wave24Exception *e) {
    if (!e->canContinue()) {
      proto('e',e->describe());
      throw new Wave24Exception("Can not configure driver",e->canContinue());
    } else {
      proto('w',e->describe(),Wave24Logger::VERBOSE_NONE);
    }
  }

  proto('n',"Driver initialized",Wave24Logger::VERBOSE_NONE);
}

void Wave24Drv::proto(char type,char *message,int verbosity) {
  myLog->log(type,message,verbosity);
}

void Wave24Drv::proto(char type,char *message) {
  myLog->log(type,message);
}

int Wave24Drv::closeCommPort() {
  if (commPortFd>=0) close(commPortFd);
  return 1;
}

int Wave24Drv::openCommPort() {
  int myError,res;
  struct termios props;

  commPortFd=open(this->commPort,O_RDWR);
  if (commPortFd<0) {
    myError=errno;
    proto('e',strerror(myError),Wave24Logger::VERBOSE_NONE);
    throw new Wave24Exception("Can not open port",false);
  }

  try {
    txMessage=new TxMessage(commPortFd,myLog);
    rxMessage=new RxMessage(commPortFd,myLog);
  } catch (Wave24Exception *e) {
    proto('e',e->describe());
    close(commPortFd);
    commPortFd=-1;
    throw new Wave24Exception("Can not initialize message buffers",false);
  }

  //nastavenie rychlosti atd.
  if ( tcgetattr(commPortFd,&props) < 0) {
    myError=errno;
    sprintf(logmessage,"Error when getting parameters of serial port :%s",strerror(myError));
    proto('w',logmessage,Wave24Logger::VERBOSE_VERBOSE);
  }
  
  res=0;
  switch(commPortSpeed) {
    case(4800):
      res = cfsetispeed(&props,B4800) + cfsetospeed(&props,B4800);
      break;    
    case(9600):
      res = cfsetispeed(&props,B9600) +  cfsetospeed(&props,B9600);
      break; 
    case(19200):
      res = cfsetispeed(&props,B19200) + cfsetospeed(&props,B19200);
      break;
    case(38400):
      res = cfsetispeed(&props,B38400) + cfsetospeed(&props,B38400);
      break;
    case(57600):
      res = cfsetispeed(&props,B57600) + cfsetospeed(&props,B57600);
      break;
    case(115200):
      res = cfsetispeed(&props,B115200) + cfsetospeed(&props,B115200);
      break;
    case(230400):
      res = cfsetispeed(&props,B230400) + cfsetospeed(&props,B230400);
      break;
    default:
      sprintf(logmessage,"Unsupported baudrate %d specified",commPortSpeed);
      close(commPortFd);
      commPortFd=-1;
      throw new Wave24Exception(logmessage,false);
  }
  if (res<0) {
    myError=errno;
    sprintf(logmessage,"Error when setting baudrate: %s",strerror(myError));
    close(commPortFd);
    commPortFd=-1;
    throw new Wave24Exception(logmessage,false);
  }
  
  props.c_iflag &= (IGNBRK);
  props.c_oflag &= ~(OPOST|ONLCR);
  props.c_cflag &= ~HUPCL;
  props.c_lflag &= 0;

  if (tcsetattr(commPortFd,TCSAFLUSH,&props) <0) {
    myError=errno;
    sprintf(logmessage,"Error when setting port parameters: %s",strerror(myError));
    close(commPortFd);
    commPortFd=-1;
    throw new Wave24Exception(logmessage,false);
  }

  proto('n',"Port opened",Wave24Logger::VERBOSE_NONE);
  return(commPortFd); 
}

void Wave24Drv::setNetwork(char *network) {
  memset(this->network,0,3);
  strncpy(this->network,network,2);
}

void Wave24Drv::setStation(char *name) {
  memset(this->stat,0,6);
  strncpy(this->stat,name,5);
}

void Wave24Drv::setChannel(int number,char *name) {
  switch(number) {
    case(1):
      memset(this->c1,0,4);
      strncpy(this->c1,name,3);
      break;
    case(2):
      memset(this->c2,0,4);
      strncpy(this->c2,name,3);
      break;
    case(3):
      memset(this->c3,0,4);
      strncpy(this->c3,name,3);
      break;
    case(4):
      memset(this->c4,0,4);
      strncpy(this->c4,name,3);
      break;
    case(5):
      memset(this->c5,0,4);
      strncpy(this->c5,name,3);
      break;
    case(6):
      memset(this->c6,0,4);
      strncpy(this->c6,name,3);
      break;
    case(7):
      memset(this->c7,0,4);
      strncpy(this->c7,name,3);
      break;
    case(8):
      memset(this->c8,0,4);
      strncpy(this->c8,name,3);
      break;
  }
}

void Wave24Drv::setCommPort(char *path) {
  strcpy(this->commPort,path);
}

void Wave24Drv::setCommPortSpeed(int speed) {
  this->commPortSpeed=speed;
}

void Wave24Drv::setSampleRate(int rate) {
  this->sampleRate=rate;
  this->samplePeriod=10000/rate;
}

void Wave24Drv::setDecimRate(int rate) {
  this->decimRate=rate;
}

int Wave24Drv::sendCommand(char *cmd) {
  txMessage->makeMessage((unsigned char *) cmd,(int) strlen((char *) cmd));
  txMessage->sendMessage();
  return(1);
}

int Wave24Drv::sendCommandWithAck(char *cmd) {
  int reslen,recvbytes;
  time_t currTime,startTime;
  struct timespec req_delay,rem_delay;

  req_delay.tv_sec =(time_t) 1;
  req_delay.tv_nsec=(long) 0;
  
  txMessage->makeMessage((unsigned char *) cmd,(int) strlen((char *) cmd));
  for (int i=0;i<txMessage->attempts;i++) {
    if (myLog->wouldLog(Wave24Logger::VERBOSE_VERYDEBUG)) {
      sprintf(logmessage,"Attempt no.:%d",i+1);
      proto('m',logmessage,Wave24Logger::VERBOSE_VERYDEBUG);
    }
    txMessage->sendMessage();

    //waiting for ACK
    nanosleep(&req_delay,&rem_delay);
      
    try {
      recvbytes=rxMessage->recvMessage();
    } catch (Wave24Exception *e) {
      proto('w',e->describe(),Wave24Logger::VERBOSE_VERBOSE);
      throw new Wave24Exception("Can not receive data",e->canContinue());
    }

    if (recvbytes>0) {
     rxMessage->parseMessage(result);
     if (rxMessage->type=='a' || rxMessage->type=='A') return 1;
     if (rxMessage->type=='b' || rxMessage->type=='B') return 1;
    }
  }
  sprintf(logmessage,"Cannot send command %s",cmd);
  throw new Wave24Exception(logmessage,true);

  return(0);
}

int Wave24Drv::recvAscii(long timeout) {
  int readBytes;                              //total number of read bytes
  int newBytes;
  int res;
  unsigned long startTime;
  unsigned long currTime;
  static unsigned char recArray[100];
  bool wasLineEnd=false;
  
  proto('m',"Entering recvAscii\n",Wave24Logger::VERBOSE_DEBUG);
  startTime=(unsigned long) time(NULL);
 
  readBytes=0;
  memset(recArray,0,100);

  while (1) {

    currTime=(unsigned long) time(NULL);
    if (currTime>(startTime+timeout)) {   //spravu zabudnem
      if (!readBytes) {
        proto('w',"Receive timeout",Wave24Logger::VERBOSE_VERBOSE);
        result[readBytes]='\0';
        return 0;
      } else {
        return readBytes;
      }
    }

    clearSet();
    if ((res=addFdToSet(commPortFd))<0) {
      throw new Wave24Exception("Can not listen on serial port",true);
    } else {
      sprintf(logmessage,"Fd %d",res);
      proto('m',logmessage,Wave24Logger::VERBOSE_VERYDEBUG);
    }

    waitForSet(200000);

    if (!isFdInSet(commPortFd) ) {
      if (wasLineEnd) break;
      proto('m',"Data are not available yet",Wave24Logger::VERBOSE_VERYDEBUG);
      continue;
    }
    proto('m',"Data are available",Wave24Logger::VERBOSE_VERYDEBUG);

    newBytes=read(commPortFd,recArray,100);
    if (newBytes<=0) {
      int myError=errno;
      sprintf(logmessage,"Read error %s",strerror(myError));
      throw new Wave24Exception(logmessage,true);
    } else {
      if (myLog->wouldLog(Wave24Logger::VERBOSE_VERYDEBUG) ) {
        sprintf(logmessage,"Received bytes: %d",newBytes);
        proto('m',logmessage,Wave24Logger::VERBOSE_VERYDEBUG);
      }
      for(int i=0;i<newBytes;i++) {
        if (myLog->wouldLog(Wave24Logger::VERBOSE_VERYDEBUG) ) {
          sprintf(logmessage,"<%d>",recArray[i]);
          proto('m',logmessage,Wave24Logger::VERBOSE_VERYDEBUG);
        }
        if (readBytes==0) {
          if (recArray[i]==CR || recArray[i]==LF) continue;
          else result[readBytes++]=recArray[i];
        } else {
          if (recArray[i]==CR || recArray[i]==LF) {
            wasLineEnd=true;
          } else result[readBytes++]=recArray[i];
        }
      }
    }

  }

  if (myLog->wouldLog(Wave24Logger::VERBOSE_DEBUG) ) {
    char piece[10];
    for (int i=0;i<readBytes && i<255;i++) {
      sprintf(piece,"<%d>",result[i]);
      strcat(logmessage,piece);
    }
    proto('m',logmessage,Wave24Logger::VERBOSE_DEBUG);
  }
    
  proto('m',"Leaving recvAscii",Wave24Logger::VERBOSE_DEBUG);
  return(readBytes);
}

int Wave24Drv::recvAscii(char *expectResponse, long timeout) {
  int readBytes;

  try {
    readBytes=recvAscii(timeout);
  } catch (Wave24Exception *e) {
    proto('w',e->describe(),Wave24Logger::VERBOSE_VERBOSE);
    throw new Wave24Exception("Can not read answer from serial line",e->canContinue());
  } 

  if (!readBytes) {
    proto('w',"No read bytes to search for expected answer",Wave24Logger::VERBOSE_VERBOSE);
    return 0;
  } else {
     if (strstr((char *) result,expectResponse)) return 1;
	   proto('m',(char *) result);
	   proto('w',"Receive timeout - correct answer not received");
     return 0;
  }
  return 0;
}

int Wave24Drv::sendAscii(char *cmd) {
  if (!cmd) {
    proto('m',"No ascii command to send",Wave24Logger::VERBOSE_DEBUG);
    return 0;
  } else {
    proto('m',"Command to send:",Wave24Logger::VERBOSE_DEBUG);
    proto('m',cmd,Wave24Logger::VERBOSE_DEBUG);
  }
  if (commPortFd<0) return 0;
  
  for (int i=0;i<strlen(cmd);i++) {
    write(commPortFd,(void *) &cmd[i],1);
    usleep(10000);
  }
  write(commPortFd,(void *) &CR,1);

  proto('m',"Command sent",Wave24Logger::VERBOSE_DEBUG);

  return(1);
}

int Wave24Drv::sendAscii(char *cmd,char *expectResponse,long timeout) {
  if (!cmd) {
    proto('m',"No ascii command to send",Wave24Logger::VERBOSE_DEBUG);
    return 0;
  } else {
    proto('m',"Command to send:",Wave24Logger::VERBOSE_DEBUG);
    proto('m',cmd,Wave24Logger::VERBOSE_DEBUG);
  }
  if (commPortFd<0) return 0;
  
  for (int i=0;i<strlen(cmd);i++) {
    write(commPortFd,(void *) &cmd[i],1);
    usleep(10000);
  }
  write(commPortFd,(void *) &CR,1);
  usleep(10000);

  proto('m',"Command sent",Wave24Logger::VERBOSE_DEBUG);

  try {
    return(recvAscii(expectResponse,timeout));
  } catch (Wave24Exception *e) {
    proto('w',e->describe(),Wave24Logger::VERBOSE_VERBOSE);
    throw new Wave24Exception("Did not get correct answer from converter",e->canContinue());
  }
}

int Wave24Drv::initADC() {
  int i,j;
  unsigned char c;
  
  tcflush(commPortFd,TCIOFLUSH);
  
  //terminate binary mode (if started)
  sendCommand("P,");
    usleep(1000000);
    tcflush(commPortFd,TCIFLUSH);
 
  //starting command mode
  try {
    for (i=0;i<5;i++) {
      proto('m',"Trying to get command mode ...",Wave24Logger::VERBOSE_VERBOSE);
      if (sendAscii("#SEIZMIKA#","start",5)) {
        proto('m',"Command mode started",Wave24Logger::VERBOSE_VERBOSE);
        break;
      } else {
        if (strstr((char *) result,"known")) {
          proto('m',"Command mode already started",Wave24Logger::VERBOSE_VERBOSE);
	        break;
        }
      }
    }
    if (i>=5) {
      proto('m',"Reseting port ...",Wave24Logger::VERBOSE_VERBOSE);
      try {
        disconnectADC();
        openCommPort();
      } catch (Wave24Exception *e) {
        proto('e',"Can not reset port");
        throw e;
      }
      for (j=0;j<5;j++) {
        proto('m',"Trying to get command mode ...",Wave24Logger::VERBOSE_VERBOSE);
        if (sendAscii("#SEIZMIKA#","start",5)) {
          proto('m',"Command mode started",Wave24Logger::VERBOSE_VERBOSE);
          break;
        }
      }
      if (i>=5) {
        throw new Wave24Exception("Cannot get command mode",false);
      }
    }
  
  } catch (Wave24Exception *e) {
    proto('e',e->describe(),Wave24Logger::VERBOSE_NONE);
    throw new Wave24Exception("Can not initialize converter",false);
  }

  proto('m',"Firmware version:",Wave24Logger::VERBOSE_VERBOSE);

  sendAscii("VER");
  if (recvAscii(5)) {
    proto('m',(char *) result,Wave24Logger::VERBOSE_VERBOSE);
  }
  sendAscii("CLOSE");
  proto('m',"Command mode closed",Wave24Logger::VERBOSE_VERBOSE);

  return 1;
}

bool Wave24Drv::checkADC() {
  int i;
  char c;

  proto('m',"Communication check required",Wave24Logger::VERBOSE_VERBOSE);

  tcflush(commPortFd,TCIOFLUSH);
  
  //terminate binarny mode
  sendCommand("P,");
    usleep(1000000);
    tcflush(commPortFd,TCIFLUSH);

  //trying to get command mode
  try {
    for (i=0;i<5;i++) {
      proto('m',"Trying to get command mode ...",Wave24Logger::VERBOSE_VERBOSE);
      if (sendAscii("#SEIZMIKA#","start",5)) break;
      else {
        if (strstr((char *) result,"known")) {
          proto('m',"Command mode already started",Wave24Logger::VERBOSE_VERBOSE);
	        break;
        }
      }
    }
    if (i>=5) {
      throw new Wave24Exception("Cannot start command mode",false);
    }
  } catch (Wave24Exception *e) {
    proto('e',e->describe(),Wave24Logger::VERBOSE_NONE);
    throw new Wave24Exception("Check ADC - communication with converter may be broken",false);
  }
  
  proto('m',"Command mode started, communication is working properly",Wave24Logger::VERBOSE_NONE);
  
  sendAscii("CLOSE");
  proto('m',"Command mode closed",Wave24Logger::VERBOSE_NONE);

  return true;
}

int Wave24Drv::disconnectADC() {
  if (commPortFd<0) return 0;

  proto('m',"Disconnecting ADC ...",Wave24Logger::VERBOSE_NONE);
  sendCommand("P,");
  sendAscii("CLOSE");
  
  proto('m',"Closing port ...",Wave24Logger::VERBOSE_NONE);
  closeCommPort();  
  return 1;
}

int Wave24Drv::connectADC() {
  static char cfgString[100];
  int error=0;
  
  try {
    openCommPort();
  } catch (Wave24Exception *e) {
    proto('e',e->describe(),Wave24Logger::VERBOSE_NONE);
    throw new Wave24Exception("Serial port not opened",e->canContinue());
  }
  
  //inicialicacia prevodnika
  try {
    initADC();
  } catch (Wave24Exception *e) {
    proto('e',e->describe(),Wave24Logger::VERBOSE_NONE);
    throw new Wave24Exception("Can not initialize converter",e->canContinue());
  }
  proto('m',"Communication with ADC initialized succesfully",Wave24Logger::VERBOSE_NONE);
  
  //configuracia prevodnika
  try {
    sprintf(command,"C,MASTER=1");
    sendCommandWithAck(command);

    sprintf(command,"C,NETWORK=%s",network);
    sendCommandWithAck(command);
 
    sprintf(command,"C,STATION=%s","WAV24");
    sendCommandWithAck(command);
  
    sprintf(command,"C,C1=%s",c1);
    sendCommandWithAck(command);
  
    sprintf(command,"C,C2=%s",c2);
    sendCommandWithAck(command);
  
    sprintf(command,"C,C3=%s",c3);
    sendCommandWithAck(command);
  
    sprintf(command,"C,C4=%s",c4);
    sendCommandWithAck(command);

    sprintf(command,"C,SRATE=%d",sampleRate);
    sendCommandWithAck(command);

    if (decimRate>0) {
      proto('m',"Decimation rate enabled",Wave24Logger::VERBOSE_VERBOSE);
      sprintf(command,"C,DRATE=%d",decimRate);
      sendCommandWithAck(command);

      sprintf(command,"C,C5=%s",c5);
      sendCommandWithAck(command);
  
      sprintf(command,"C,C6=%s",c6);
      sendCommandWithAck(command);
  
      sprintf(command,"C,C7=%s",c7);
      sendCommandWithAck(command);
  
      sprintf(command,"C,C8=%s",c8);
      sendCommandWithAck(command);
    }

  } catch (Wave24Exception *e) {
    proto('w',e->describe(),Wave24Logger::VERBOSE_NONE);
    throw new Wave24Exception("Cannot send configuration to AD converter",false);
  }

  sprintf(cfgString,"Configuration sent: %s,%s,%s,%s,%s,%s,%d",network,stat,c1,c2,c3,c4,sampleRate);
  proto('n',cfgString,Wave24Logger::VERBOSE_VERBOSE);
  proto('n',"ADC connected and configured",Wave24Logger::VERBOSE_NONE);
  
  return 1;
}
 
//odstartuje zber dat z prevodnika
int Wave24Drv::startDataAcquisition() {

  if (useSqn) {
    if (lastPacketSqn[0]!='\0') {
      sprintf(command,"Q,DATA %s FFFFFFFF 0 1 2 3",lastPacketSqn);
    } else {
      sprintf(command,"Q,DATA 7FFFFFFF FFFFFFFF 0 1 2 3");
    }
  } else {
    sprintf(command,"Q,DATA 7FFFFFFF FFFFFFFF 0 1 2 3");
  }

  if (!sendCommandWithAck(command) ) {
    throw new Wave24Exception("Cannot start data acquisition",false);
  } else {
    proto('n',"Data acquisition started",Wave24Logger::VERBOSE_NONE);
  }
  return 1;
}

int Wave24Drv::addFdToSet(int fd) {
  if (fd<0) return -1;
  FD_SET(fd,&readFds);
  if (fd>maxFd) maxFd=fd;
  return maxFd;
}

bool Wave24Drv::isFdInSet(int fd) {
  if (FD_ISSET(fd,&readFds)) return true;
  else return false;
}

void Wave24Drv::clearSet() {
  FD_ZERO(&readFds);
}

void Wave24Drv::waitForSet(long microsec) {
  int res;
  struct timeval selectTimeout;

  //timeout for events on file descriptors
  selectTimeout.tv_sec=microsec/1000000;
  selectTimeout.tv_usec=microsec%1000000;

  res = select(maxFd+1,&readFds,NULL,NULL,&selectTimeout);
  if (res<0) {        //toto by mala byt vynimka
    int myError=errno;
    proto('m',strerror(myError),Wave24Logger::VERBOSE_DEBUG);
  }
}

//gets new data
int Wave24Drv::newData() {
  int len;
  int reslen,recvbytes;

  proto('m',"Entering newData()",Wave24Logger::VERBOSE_DEBUG);

  try {
    recvbytes=rxMessage->recvMessage();
    if (recvbytes>0) {
      reslen=rxMessage->parseMessage(result);
      if (myLog->wouldLog(Wave24Logger::VERBOSE_DEBUG)) {
        sprintf(logmessage,"Recieved %d bytes",recvbytes);
        proto('m',logmessage,Wave24Logger::VERBOSE_DEBUG);
        sprintf(logmessage,"Parsed %d bytes, response status %d\n",reslen,rxMessage->response);
        proto('m',logmessage,Wave24Logger::VERBOSE_DEBUG);
      }

      if (rxMessage->response==0) {	//NACK
        sprintf(command,"N,%c",rxMessage->counter);
        sendCommand(command);
      } else if (rxMessage->response==1) {    //ACK
        sprintf(command,"A,%c",rxMessage->counter);
        sendCommand(command);
      }
    } else {
      return 0;
    }

  } catch (Wave24Exception *e) {
    proto('w',e->describe(),Wave24Logger::VERBOSE_VERBOSE);
    throw new Wave24Exception("Can not read new data",e->canContinue());
  }

  return reslen;
}

//poskytne data seedlinku
int Wave24Drv::shareData(int len) {
  int i,k;
  int nsamples;
  static int32_t samples[1000];
  struct ptime firstTime;
  char station[10];
  char channel[4];
  char tmpstring[40];
  int clkStatus;
  static int timingQuality=0;
  long currTime;
  static long gpsReportTime=0;
  FILE *sqnfp;
  long oldSqn,newSqn,diffSqn;
  
  proto('m',"Entering share data",Wave24Logger::VERBOSE_DEBUG);

  if (useSqn) {
    if (lastPacketSqn[0]!='\0') sscanf((char *) lastPacketSqn,"%06lX",&oldSqn);
    else oldSqn=-1;

    for (i=0;i<6;i++) {
      lastPacketSqn[i]=result[i];
    }

    if (oldSqn>=0) {
      sscanf((char *) lastPacketSqn,"%06lX",&newSqn);
      if (oldSqn>newSqn) {
        diffSqn=0xFFFFFF-newSqn+oldSqn;
      } else {
        diffSqn=newSqn-oldSqn;
      }
      if (diffSqn>allowedSqnDiff) {
        lastPacketSqn[0]='\0';
        sprintf(logmessage,"Difference in sqn numbers too large. Resuming from %06lX",newSqn);
        proto('m',logmessage,Wave24Logger::VERBOSE_NONE);
        return 0;
      }
    }

    sqnfp=fopen(DFLT_STATFILE,"w");
    if (!sqnfp) {
      int myError=errno;
      sprintf(logmessage,"Sqn status file opening error - %s",strerror(myError));
      proto('m',logmessage,Wave24Logger::VERBOSE_DEBUG);
    } else {
      for (i=0;i<6;i++) {
        fprintf(sqnfp,"%c",lastPacketSqn[i]);
      }
      fclose(sqnfp);
    }
  }

  for (k=0;k<len;k+=512) {
    nsamples=0;
    nsamples=WaveSeed::decodeSteim((char *) (result+k),samples);
    
    clkStatus=WaveSeed::clockOk((char *) (result+k));
    currTime=(long) time(NULL);
    if (clkStatus) {
      if (lastGpsOk<=0) {
        timingQuality=0;
        if ((currTime-gpsReportTime)>=gpsCheckPeriod) {
          switch(clkStatus) {
	          case 1:
	            sprintf(logmessage,"Time not synchronized yet - pps pulses not available");
	            break;
	          case 2:
	            sprintf(logmessage,"Time not synchronized yet - valid messages not available");
	            break;
	          case 3:
	            sprintf(logmessage,"Time not synchronized yet - neither pps pulses nor valid messages are available");
	            break;
          }
          proto('w',logmessage,Wave24Logger::VERBOSE_NONE);
	        gpsReportTime=currTime;
	      }
      } else {
        if ((currTime-lastGpsOk)>sampleErrorPeriod) timingQuality=0;
        else {
          timingQuality=(long) floor(100.*((float) (currTime-lastGpsOk))/(float) sampleErrorPeriod);
        }
        if ((currTime-gpsReportTime)>=gpsCheckPeriod) {
	        switch(clkStatus) {
	          case 1:
	            sprintf(logmessage,"Time reference lost - pps pulses not available");
	            break;
	          case 2:
	            sprintf(logmessage,"Time reference lost - valid messages not available");
	            break;
	          case 3:
	            sprintf(logmessage,"Time reference lost - neither pps pulses nor valid messages are available");
	            break;
          }
          proto('w',logmessage,Wave24Logger::VERBOSE_NONE);
	        gpsReportTime=currTime;
	      }
      }
    } else {
      if (timingQuality<100) proto('m',"Time synchronized",Wave24Logger::VERBOSE_NONE);
      lastGpsOk=(long) time(NULL);
      timingQuality=100;
    }
    
    WaveSeed::getFirstTime((char *) (result+k),&firstTime);
    
    memset(station,0,10);
    memset(channel,0,4);
 
    //strncpy(station,(char *) (result+k+8),5);
    strncpy(channel,(char *) (result+k+15),3);

    int j;
 
    for(i=0;i<4;i++) {
      if (channel[i]!=' ') break;
    }
    j=i;
    for (;i<4;i++) {
      channel[i-j]=channel[i];
      if (channel[i-j]=='\0') break;
      if (channel[i-j]==' ') {
        channel[i-j]='\0';
	      break;
      }
    } 
    
    strcpy(station,stat);
    nsamples--; //d0 sa nerata
    for (i=1;i<nsamples;i++) {
      samples[i]=samples[i-1]+samples[i+1];
    }
    if (myLog->wouldLog(Wave24Logger::VERBOSE_VERYDEBUG)) {
      sprintf(logmessage,"Going to share %d, samples %d\n",k,nsamples);
      proto('m',logmessage,Wave24Logger::VERBOSE_VERYDEBUG);
      sprintf(logmessage,"Last data computed %d",samples[nsamples-1]);
      proto('m',logmessage,Wave24Logger::VERBOSE_VERYDEBUG);
    }
    send_raw3(station,channel,&firstTime,0,timingQuality,samples,nsamples);
    
    proto('m',"Leaving sharedData()",Wave24Logger::VERBOSE_DEBUG);
  }
  return 1;
}

int Wave24Drv::configure() {
  configure(NULL);
}

int Wave24Drv::configure(char *cfgpath) {
  FILE *cfgfile;
  int myError;
  FILE *sqnfp;
  
  memset(lastPacketSqn,0,7);
  lastGpsOk=-1;
  sampleRate=0;
  decimRate=0;
  gpsCheckPeriod=60;
  sampleErrorPeriod=(long) floor(((float) 1/(float) sampleRate)/1.e-6);
  useSqn=true;
  allowedSqnDiff=3000;

  sqnfp=fopen(DFLT_STATFILE,"r");
  if (!sqnfp) {
    sprintf(logmessage,"Sqn status file opening error - %s",strerror(myError));
    proto('m',logmessage,Wave24Logger::VERBOSE_DEBUG);
    lastPacketSqn[0]='\0';
  } else {
    for (int i=0;i<6;i++) {
      fscanf(sqnfp,"%c",&lastPacketSqn[i]);
    }
    lastPacketSqn[6]='\0';
    fclose(sqnfp);
    sprintf(logmessage,"Resuming from sqn %s",lastPacketSqn);
    proto('m',logmessage,Wave24Logger::VERBOSE_NONE);
  }

}

static void signalHandler(int signo) {
  switch(signo) {
    case SIGTERM:
      //ukonci komunikaciu a zavrie port
      driver->disconnectADC();      
      driver->proto('m',"Going down after TERM signal ...\n");
      exit(0);
    case SIGALRM:
      driver->disconnectADC();      
      driver->proto('m',"Binary data transmit timeout. Going down ...\n");
      exit(0);
 
  }
}

void Wave24Drv::printHelp() {
  printf("Wave24Drv - plugin for Wave24 converter.\n");
  printf("Wave24Drv accepts following command line arguments:\n");
  printf("-h ..... this help\n");
  printf("-v <level>.... verbose level 0 - normal use, 1 - troubleshooting, 2 - debug, 3 - very debug\n");
  printf("-p <port> .... port to be used\n");
  printf("-s <speed> ... speed of the port\n");
  printf("-S <station> .... station\n");
  printf("-N <network> .... network\n");
  printf("-1 <name> .... name for channel 1\n");
  printf("-2 <name> .... name for channel 2\n");
  printf("-3 <name> .... name for channel 3\n");
  printf("-4 <name> .... name for channel 4\n");
  //printf("-5 <name> .... name for channel 5\n");
  //printf("-6 <name> .... name for channel 6\n");
  //printf("-7 <name> .... name for channel 7\n");
  //printf("-8 <name> .... name for channel 8\n");
  printf("-f <rate> .... sampling rate\n");
  //printf("-d <ratio>.... decimation ratio\n");
  printf("-t <period usec .... period for checking new data\n");
  printf("-q <sqn differnce> ... maximal allowed difference of sqn numbers. Negative numbers will disable use of sqn numbers\n");
  //printf("-G <period sec> .... period for protocoling GPS state\n");
}

int main(int argc,char **argv) {
	int c,len;
	extern char* optarg;
	int commPortSpeed;
	int sampleRate;
	int decimRate;
  int loglevel;
  long checkPeriod,sqnDiff;
	char cfgfile[100],commPort[20];
  bool hasToStart;

  //signal hanler registration
	signal(SIGTERM,signalHandler);
  signal(SIGALRM,signalHandler);
	
  //make driver
  try {
    driver=new Wave24Drv(NULL,"/home/sysop/logs/Wave24.log");
  } catch (Wave24Exception *e) {
    if (e->canContinue()) {
      Wave24Logger::stdlog('w',e->describe(),Wave24Logger::VERBOSE_NONE);
    } else {
      Wave24Logger::stdlog('e',e->describe());
      Wave24Logger::stdlog('e',"Critical error - Wave24Drv can not continue"); 
      exit(-1);
    }
  }

  //preprocess command line arguments
	while ((c=getopt(argc,argv,"c:l:hv:p:s:S:N:1:2:3:4:5:6:7:8:f:d:t:q:"))!=-1) {
	  switch(c) {
	    case 'c':                  //konfiguracny subor
	      //strcpy(cfgfile,optarg);
	      break;
	    case 'l':
        //strcpy(logfile,optarg);
	      break;
	    case 'h':
	      Wave24Drv::printHelp();
	      exit(0);
	    case 'v':
	      sscanf(optarg,"%d",&loglevel);
        driver->setLogLevel(loglevel);
	      break;
	    case 'p':
	      driver->setCommPort(optarg);
	      break;
	    case 's':
	      sscanf(optarg,"%d",&commPortSpeed);
	      driver->setCommPortSpeed(commPortSpeed);
	      break;
	    case 'S':
	      driver->setStation(optarg);
        break;
	    case 'N':
	      driver->setNetwork(optarg);
	      break;
	    case '1':
	      driver->setChannel(1,optarg);
	      break;
	    case '2':
	      driver->setChannel(2,optarg);
	      break;
	    case '3':
	      driver->setChannel(3,optarg);
	      break;
	    case '4':
	      driver->setChannel(4,optarg);
	      break;
      case '5':
	      driver->setChannel(5,optarg);
	      break;
	    case '6':
	      driver->setChannel(6,optarg);
	      break;
	    case '7':
	      driver->setChannel(7,optarg);
	      break;
	    case '8':
	      driver->setChannel(8,optarg);
	      break;
	    case 'f':
	      sscanf(optarg,"%d",&sampleRate);
	      driver->setSampleRate(sampleRate);
	      break;
      case 'd':
        sscanf(optarg,"%d",&decimRate);
        driver->setDecimRate(decimRate);
        break;
	    case 't':
	      sscanf(optarg,"%d",&checkPeriod);
	      break;
      case 'q':
        sscanf(optarg,"%ld",&sqnDiff);
        driver->setUseSqn(sqnDiff);
        break;
      default:
        sprintf(logmessage,"Unknown argument found: %c",c);
        driver->proto('w',logmessage,Wave24Logger::VERBOSE_NONE);
        exit(-1);
	  }
	}

  //try to connect ADC	
	try {
    driver->connectADC();
  } catch (Wave24Exception *e) {
    driver->proto('e',e->describe());
    driver->proto('e',"Cannot connect AD converter");
    exit(-1);
  }

  hasToStart=true;
  
  //main loop (listening on fd's, receiving data and sharing data)
	while(1) {

    if (hasToStart) {
      //try to start data acquisition
      try {
        driver->startDataAcquisition();
      } catch (Wave24Exception *e) {
        driver->proto('e',e->describe());
        driver->proto('e',"Cannot start data acquisition");
        exit(-1);
      }
      hasToStart=false;
    }

    driver->clearSet();

    if (driver->addFdToSet(driver->getCommPortFd())<=0) {
      driver->proto('e',"Cannot listen on comm port - going down");
      exit(-1);
    }

    driver->waitForSet(checkPeriod);

    //check for new data
    if (driver->isFdInSet(driver->getCommPortFd())) {
      try {
        len=driver->newData();
      } catch (Wave24Exception *e) {
        if (!e->canContinue()) {
          driver->proto('e',e->describe());
          driver->proto('e',"Cannot get new data");
          exit(-1);
        }

        driver->proto('w',e->describe(),Wave24Logger::VERBOSE_VERBOSE);
        driver->proto('m',"Going to check AD converter",Wave24Logger::VERBOSE_VERBOSE);

        try {
          if (driver->checkADC()) {
            driver->proto('m',"AD converter seems working properly. Trying to restart data acquisition",Wave24Logger::VERBOSE_VERBOSE);
            driver->startDataAcquisition();
          } else {
            driver->proto('e',"Communication with AD converter broken, going down ...");
            exit(-1);
          } 
        } catch (Wave24Exception *e) {
          driver->disconnectADC();
          driver->proto('e',e->describe());
          driver->proto('e',"Communication with AD converter broken, going down ...");
          exit(-1);
        }

      }

		  if (len>0) {
		  	if (driver->shareData(len)==0) hasToStart=true;
		  }
    }
	}	
	
}
