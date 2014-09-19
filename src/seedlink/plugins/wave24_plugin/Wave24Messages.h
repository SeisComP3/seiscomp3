#include "Wave24Logger.h"

#ifndef _WAVE24MESSAGES
#define _WAVE24MESSAGES

class Wave24Messages {
  public:
    static const int MAX_COMMAND_LEN=1024;
    static const int MAX_ANSWER_LEN=32678;
    static const int MAX_ATTEMPTS=3;
    static const unsigned long DFLT_TIMEOUT=60;
};

class TxMessage {
  private:
    unsigned char *message;             //message buffer
    int fd;                     //descriptor of port to send the message through
    Wave24Logger *logger;
  public:
    TxMessage(int fd,Wave24Logger *logger);
    ~TxMessage();
    char type;                        //type of the message
    unsigned char counter;
    unsigned char wavenum;           //number of converter to send the message to
    char response;                   //is response required
    int len;                          //length of the message
    int attempts;
    unsigned long timeout;
    unsigned long timestamp;
    int makeMessage(unsigned char* message,int len,int trim=0); //prepares message
    int sendMessage();            //sends message (prefixing STX,ETX,DLE characters);
};

class RxMessage {
  private:
    unsigned char *message;                   //message buffer
    int fd;                                    //descriptor of port to receive the message through
    Wave24Logger *logger;
  public:
    RxMessage(int fd,Wave24Logger *logger);
    ~RxMessage();
    char type;                                //type of the message
    unsigned char counter;
    unsigned char wavenum;                   //number of converter the message has been sent from
    char response;                           //is response required
    int len;                                  //length of the message
    int attempts;
    unsigned long timeout;
    unsigned long timestamp;
    int recvMessage();  //receives message
    int parseMessage(unsigned char *result);         //interprets the message
};

class CheckSum {
  public:
    static unsigned int calculateCheckSum(unsigned char *message,int len);
};

#endif

