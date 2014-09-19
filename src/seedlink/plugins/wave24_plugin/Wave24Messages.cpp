#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>

#define _WAVE24MESSAGES_GLOBALS

#include "Wave24Messages.h"
#include "Wave24Logger.h"
#include "Wave24Exception.h"

static const unsigned char STX=0x02;
static const unsigned char ETX=0x03;
static const unsigned char DLE=0x10;
static const unsigned char CR=0x0D;
static const unsigned char LF=0x0A;

static char logmessage[1024];

TxMessage::TxMessage(int fd,Wave24Logger *logger) {
  if (fd<0 || !logger) {
    throw new Wave24Exception("Can not initialize TxMessage buffer",false);
  } else {
    this->fd=fd;
    this->message=new unsigned char[Wave24Messages::MAX_COMMAND_LEN];
    memset(this->message,0,Wave24Messages::MAX_COMMAND_LEN);
    this->logger=logger;
  }
}

TxMessage::~TxMessage() {
  delete[] this->message; //
}

//prepares message to the converter from the message in the form "type,num,message"
int TxMessage::makeMessage(unsigned char *message,int len,int trim) {
  int firstNonWhite=0;
  int lastNonWhite=0;
  unsigned short datalen=0;
  int i=0;
  int j=0;
  char waveNumString[3];

  logger->log('m',"Entering makeMessage\n",Wave24Logger::VERBOSE_DEBUG);

  memset(waveNumString,0,3);

  //odstranim pociatocne a koncove medzery
  if (trim) {
    for (i=0;i<len;i++) {
      if (    (message[i]>=0x41 && message[i]<=0x5A) ||
              (message[i]>=0x61 && message[i]<=0x7A) ) {
        firstNonWhite= i;
        break;
      }
    }
   
    for (i=len;i>=firstNonWhite;i--) {
      if ((message[i]>=0x30 && message[i]<=0x39) ||
         (message[i]>=0x41 && message[i]<=0x5A) ||
         (message[i]>=0x61 && message[i]<=0x7A) ) {
        lastNonWhite= i;
        break;
      }
    }
  
  } else {
    firstNonWhite=0;
    lastNonWhite=len;
  }
  
  i=0;

  //priradim poradove cislo spravy
  counter=(counter+1)%255;
  this->message[i++]=counter;
  if (logger->wouldLog(Wave24Logger::VERBOSE_VERYDEBUG)) {
    sprintf(logmessage,"%d:%c\n",i-1,this->message[i-1]);
    logger->log('m',logmessage,Wave24Logger::VERBOSE_VERYDEBUG);
  }
        
  //type of the message
  this->message[i++]=message[firstNonWhite];
  if (logger->wouldLog(Wave24Logger::VERBOSE_VERYDEBUG)) {
    sprintf(logmessage,"%d:%c\n",i-1,this->message[i-1]);
    logger->log('m',logmessage,Wave24Logger::VERBOSE_VERYDEBUG);
  }
  this->type=message[firstNonWhite];
        
  //number of Wave24
  switch(this->type) {
    case 'q':
    case 'c':
    case 'a':
    case 'n':
    case 'f':
    case 'p':
      if (message[firstNonWhite+3]==',') {
        waveNumString[0]=message[firstNonWhite+2];
        firstNonWhite+=4;  //T,n,...
      } else {
        waveNumString[0]=message[firstNonWhite+2];
        waveNumString[1]=message[firstNonWhite+3];
        firstNonWhite+=5; //T,nn,...
      }
      sscanf(waveNumString,"%d",&(this->wavenum));
      this->message[i++]=this->wavenum;
      if (logger->wouldLog(Wave24Logger::VERBOSE_VERYDEBUG)) {
        sprintf(logmessage,"%d:%c\n",i-1,this->message[i-1]);
        logger->log('m',logmessage,Wave24Logger::VERBOSE_VERYDEBUG);
      }
      break;
    default:
      this->wavenum=0;
      firstNonWhite+=2;   //T,...
  }
  
  //length of message
  int len1=i++;
  int len2=i++;

  //data
  for (int j=firstNonWhite;j<=lastNonWhite;i++,j++) {
    datalen++;
    this->message[i]=message[j];
    if (logger->wouldLog(Wave24Logger::VERBOSE_VERYDEBUG)) {
      sprintf(logmessage,"%d:%c\n",i,this->message[i]);
      logger->log('m',logmessage,Wave24Logger::VERBOSE_VERYDEBUG);
    }
  }
  if (trim) {
    datalen++;
    this->message[i++]=0;
  }

  memcpy(&(this->message[len1]),&datalen,sizeof(unsigned short));
  if (logger->wouldLog(Wave24Logger::VERBOSE_DEBUG)){
    sprintf(logmessage,"Datalen %d\n",datalen);
    logger->log('m',logmessage,Wave24Logger::VERBOSE_DEBUG);
  }

  //checksum
  int chksum = CheckSum::calculateCheckSum(this->message,i);
  memcpy(&(this->message[i]),&chksum,sizeof(unsigned int));
  
  this->len=len=(unsigned short) (i+ 4);
  
  switch(this->type) {
    case 'a':
    case 'A':
    case 'n':
    case 'N':         //na ACK a NACK uz necakam odpoved
      this->response=0;
      this->timeout=0;
      this->attempts=0;
      break;
    case 'q':
    case 'Q':
    case 'p':
    case 'P':         //requestovacie spravy
      this->response=true;
      this->timeout=Wave24Messages::DFLT_TIMEOUT;
      this->attempts=Wave24Messages::MAX_ATTEMPTS;
      break;
    case 'c':
    case 'C':         //konfiguracne spravy
      this->response=true;
      this->timeout=Wave24Messages::DFLT_TIMEOUT;
      this->attempts=Wave24Messages::MAX_ATTEMPTS;
      break;
    case 'f':
    case 'F':         //datove spravy - tieto vysielat asi nebudem
      this->response=true;
      this->timeout=Wave24Messages::DFLT_TIMEOUT;
      this->attempts=Wave24Messages::MAX_ATTEMPTS;
      break;
  }
  
  logger->log('m',"Whole message:",Wave24Logger::VERBOSE_VERYDEBUG);
  logger->log('m',(char *) this->message,Wave24Logger::VERBOSE_VERYDEBUG);
  logger->log('m',"Leaving makeMessage",Wave24Logger::VERBOSE_DEBUG);
  return 1;
}

int TxMessage::sendMessage() {
  int i;
  fd_set mySet;
  struct itimerval value;
 
  logger->log('m',"Entering sendMessage",Wave24Logger::VERBOSE_DEBUG);
  if (logger->wouldLog(Wave24Logger::VERBOSE_DEBUG) ) {
    sprintf(logmessage,"Message len: %d",this->len);
    logger->log('m',logmessage,Wave24Logger::VERBOSE_DEBUG);
  }

  value.it_value.tv_sec=timeout;
  value.it_value.tv_usec=0;
  value.it_interval.tv_sec=0;
  value.it_interval.tv_usec=0;
  setitimer(ITIMER_REAL,&value,NULL);

  write(fd,(void *) &STX,1);
  if (logger->wouldLog(Wave24Logger::VERBOSE_VERYDEBUG) ) {
    sprintf(logmessage,"<%d>",STX);
    logger->log('m',logmessage,Wave24Logger::VERBOSE_VERYDEBUG);
  }

  write(fd,(void *) &STX,1);
  if (logger->wouldLog(Wave24Logger::VERBOSE_VERYDEBUG) ) {
    sprintf(logmessage,"<%d>",STX);
    logger->log('m',logmessage,Wave24Logger::VERBOSE_VERYDEBUG);
  }
  
  for (i=0;i<this->len;i++) {
                //prefixovanie
    switch (message[i]) {
      case ETX:
      case STX:
      case DLE:
        write(fd,(void *) &DLE,1);
        if (logger->wouldLog(Wave24Logger::VERBOSE_VERYDEBUG) ) {
          sprintf(logmessage,"<%d>",DLE);
          logger->log('m',logmessage,Wave24Logger::VERBOSE_VERYDEBUG);
        }
        break;
    }
    write(fd,(void *) &message[i],1);
    if (logger->wouldLog(Wave24Logger::VERBOSE_VERYDEBUG) ) {
      sprintf(logmessage,"<%d>",message[i]);
      logger->log('m',logmessage,Wave24Logger::VERBOSE_VERYDEBUG);
    }
  }
        
  write(fd,(void *) &ETX,1);
  if (logger->wouldLog(Wave24Logger::VERBOSE_VERYDEBUG) ) {
    sprintf(logmessage,"<%d>",ETX);
    logger->log('m',logmessage,Wave24Logger::VERBOSE_VERYDEBUG);
  }
        
  timestamp=(long) time(NULL);

  value.it_value.tv_sec=0;
  value.it_value.tv_usec=0;
  value.it_interval.tv_sec=0;
  value.it_interval.tv_usec=0;
  setitimer(ITIMER_REAL,&value,NULL);

  logger->log('m',"Leaving sendMessage",Wave24Logger::VERBOSE_DEBUG);
  return 1;
}

RxMessage::RxMessage(int fd,Wave24Logger *logger) {
  if (fd<0 || !logger) {
    throw new Wave24Exception("Can not initialize RxMessage buffer",false);
  } else {
    this->fd=fd;
    this->message=new unsigned char[Wave24Messages::MAX_ANSWER_LEN];
    this->timeout=Wave24Messages::DFLT_TIMEOUT;
    this->logger=logger;
  }
}

RxMessage::~RxMessage() {
  delete[] this->message;
}

int RxMessage::recvMessage() {

  int readBytes;  //celkovy pocet precitanych bytov
  int nowBytes;   //pocet bytov precitanych jednym citanim
  int readBytesOld; //pocet bytev po predhcdzajucich citaniach
  int i,shift;
  unsigned char wasDLE=0;
  unsigned char wasSTX=0;
  unsigned char wasETX=0;
  unsigned char singleByte;

  struct itimerval value;

  logger->log('m',"Entering recvMessage",Wave24Logger::VERBOSE_DEBUG);

  value.it_value.tv_sec=timeout;
  value.it_value.tv_usec=0;
  value.it_interval.tv_sec=0;
  value.it_interval.tv_usec=0;
  setitimer(ITIMER_REAL,&value,NULL);
  
  readBytes=0;
  while(1) {
    if (wasETX) {
      break;
    }

    if ((nowBytes=read(fd,&singleByte,1))<=0) {
      int myError=errno;
      sprintf(logmessage,"Communication error %s",strerror(myError));
      throw new Wave24Exception(logmessage,true);
    }

    if (logger->wouldLog(Wave24Logger::VERBOSE_VERYDEBUG) ) {
      sprintf(logmessage,"<%d>",singleByte);
      logger->log('m',logmessage,Wave24Logger::VERBOSE_VERYDEBUG);
    }

    if (wasETX) { //toto je korektny koniec, nastavim spravu a exitnem
       break;
    }
      
    if (wasSTX<2) { //kym nemam dva STX nepreklapam
      if (singleByte==STX) wasSTX++;
      continue;
    }
                      
    switch (singleByte) {
      case DLE:       //prefixy
        if (!wasDLE) {
          wasDLE=1;
        } else {
          wasDLE=0;
          this->message[readBytes++]=singleByte;
        }
        break;
      case STX:
        if (wasDLE) {
          this->message[readBytes++]=singleByte;
          wasDLE=0;
        } else {
          //mimo zaciatku nemoze byt neprefixovane STX - exception
          ;
        }
        break;
      case ETX:
        if (wasDLE) {
          this->message[readBytes++]=singleByte;
          wasDLE=0;
        } else {
          //neprefixovane ETX znamena koniec spravy
          wasETX=1;
        }
        break;
      default:
        this->message[readBytes++]=singleByte;
        break;
    }
  }
  this->len=readBytes;

  value.it_value.tv_sec=0;
  value.it_value.tv_usec=0;
  value.it_interval.tv_sec=0;
  value.it_interval.tv_usec=0;
  setitimer(ITIMER_REAL,&value,NULL);

  if (logger->wouldLog(Wave24Logger::VERBOSE_DEBUG) ) {
    sprintf(logmessage,"Leaving recvMessage, read %d bytes\n",readBytes);
    logger->log('m',logmessage,Wave24Logger::VERBOSE_DEBUG);
  }
  return readBytes;
}       

//funkcia ziska telo spravy, typ, counter atd. Vrati dlzku tela alebo -1 ak chyba
int RxMessage::parseMessage(unsigned char *result) {
  int i,j,jMax;
  short bodyLen;
  unsigned char tmp;

  logger->log('m',"Entering parseMessage\n",Wave24Logger::VERBOSE_DEBUG);  
  //controla checksumy
  unsigned int checksumCalculated=CheckSum::calculateCheckSum(message,len-4);
  unsigned int *checksumRead=(unsigned int *) &message[len-4];
    
  if (*checksumRead!=checksumCalculated) {
    sprintf(logmessage,"Checksum error. Calculated chksum=%d, read chksum=%d",checksumCalculated,checksumRead);
    logger->log('w',logmessage,Wave24Logger::VERBOSE_VERBOSE);
    return -1;
  }
        
  //kontrola typu
  i=0;
  counter=message[i++];
  type=message[i++];
        
  switch(type) {
    case 'A':
      response=-1;
      break;
    case 'a':
      wavenum=message[i++];
      response=-1;
      break;
    case 'N':
      response=-1;
      break;
    case 'n':
      wavenum=message[i++];
      response=-1;
      break;
    case 'B':
      response=-1;
      break;
    case 'b':
      wavenum=message[i++];
      response=-1;
      break;
    case 'Q':
      break;
    case 'q':
      wavenum=message[i++];
      break;
    case 'C':
      break;
    case 'c':
      wavenum=message[i++];
    case 'F':
      response=1;      
      break;
    case 'f':     //ak pridu data miesto ACK na messagu, mozem ju vyhlasit za uspesnu a zabudnut
      response=1;
      wavenum=message[i++];
      break;
  }
        
  memcpy(&bodyLen,&message[i++],sizeof(short));
  i++;    //este jedna inkrementacia, pouzivam 2 byty

  if (bodyLen>Wave24Messages::MAX_ANSWER_LEN) {
    sprintf(logmessage,"Message body too long - %d bytes",bodyLen);
    logger->log('w',logmessage,Wave24Logger::VERBOSE_VERBOSE);
    return -1;
  }

  for(j=0;j<bodyLen;j++,i++) {
    *(result+j)=message[i];
  }

  logger->log('m',"Leaving parseMessage",Wave24Logger::VERBOSE_DEBUG);
  return bodyLen;
}

unsigned int CheckSum::calculateCheckSum(unsigned char *message,int len) {
  int i;
  unsigned int chksum=0;
  
  for (i=0;i<len;i++) chksum+=(unsigned int) message[i];
  return chksum;
}
