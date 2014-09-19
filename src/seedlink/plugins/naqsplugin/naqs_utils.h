/**************************************************************************

  naqs_utils.h:  Declarations/constants for naqs_utils.c

  Based on the example code (dsClient.cpp) distrubuted by Nanometrics.
  The original copyright notice is below.  Notification was given that
  "there are no restrictions regarding the use of the dsClient.cpp snippet"
  (Neil Spriggs, Nanometrics, 7-2-2002).


  --------- Original notice ---------
  File:  dsClient.c

  Description:  Example DataStream client program to connect to NaqsServer

  Copyright 1999 Nanometrics, Inc.  All rights reserved.

  The purpose of this code is to demonstrate how to communicate with the
  NaqsServer datastream service.  It is written for Windows 95 or NT, but
  may be easily modified to run on other platforms.

  It connects to the datastream service, requests data for a single
  channel, and prints out some info about each data packet received.
  It can request and receive time-series, state-of-health or serial data.

  The requested channel name and the host name and port name for the
  datastream service are input as command-line parameters.
  By default, the program connects to port 28000 on the local machine.
  
  Note that all data received from the datastream server are in network
  byte order (most-significant byte first), EXCEPT compressed data packets.
  Compressed data packets are forwarded without modification from the 
  originating instrument; these packets are in LSB-first order.

  This source code is distributed as documentation in support of Nanometrics
  NaqsServer data streams.  As documentation, Nanometrics offers no support
  and/or warranty for this product.  In particular, Nanometrics provides no
  support and/or warranty for any software developed using part or all of
  this source code.

 ------------------------------------------------------------------------*/

/* Definitions ----------------------------------------------------------*/

#include <stdint.h>

#define INVALID_INET_ADDRESS  NO_ADDRESS

/* first 4 bytes of all messages */
#define NMX_SIGNATURE 0x7abcde0f

/* defines the message types */
#define CONNECT_MSG        100
#define CHANNEL_LIST       150
#define ERROR_MSG          190
#define TERMINATE_MSG      200
#define COMPRESSED_DATA      1
#define DECOMPRESSED_DATA    4

/* time series */
#define TIMSER_TYPE          1
#define TIMSER_ADD_REQ     120

/* state of health */
#define SOH_TYPE             2
#define SOH_ADD_REQ        121

/* transparent serial */
#define SERIAL_TYPE          6
#define SERIAL_ADD_REQ     124

/* macro to determine data type from key */
#define dataType(key) ((key >> 8) & 0xFF);

/* used for return messages */
#define SOCKET_OK          0
#define SOCKET_ERROR      -1
#define KEY_NOT_FOUND     -1
#define NAME_NOT_FOUND    NULL

/* maximum time between connection attempts (seconds) */
#define SLEEPMAX 10

/* Structures -----------------------------------------------------------*/

/* for documentation on message structures see the NaqsServer manual */

/* Header for all messages */
typedef struct MessageHeader
{
  uint32_t signature;
  uint32_t type;
  uint32_t length;
} MessageHeader;

/* Request for time series data (single channel) */
typedef struct DataAddRequest
{
  int32_t numChannels;
  int32_t channel;
  int32_t stcDelay;
  int32_t format;
  int32_t sendBuffers;
} DataAddRequest;

/* Request for soh or serial data (single channel) */
typedef struct AddRequest
{
  int32_t numChannels;
  int32_t channel;
  int32_t stcDelay;
  int32_t sendBuffers;
} AddRequest;

/* The key/name info for one channel */
typedef struct ChannelKey
{
  int32_t key;
  char name[12];
} ChannelKey;

/* A channel list structure */
typedef struct ChannelList
{
  uint32_t length;
  ChannelKey channel[500];
} ChannelList;

extern int openSocket(char *hostname, int portNum);
extern int s_send(int isock, void* data, int length);
extern int s_recv(int isock, void* data, int length);
extern int sendHeader(int isock, int type, int length);
extern int receiveHeader(int isock, MessageHeader* pmsg);
extern int sendConnectMessage(int isock);
extern int requestTypeChannel(int isock, int channel, int type);
extern int requestSerialChannel(int isock, int channel);
extern int requestSohChannel(int isock, int channel);
extern int requestDataChannel(int isock, int channel);
extern int requestDataChannels(int isock, int keyList[], int found,
			       int samprate, int shortTermComp);
extern int requestChannel(int isock, int channel);
extern int receiveChannelList(int isock, ChannelList* plist, int length);
extern int lookupChannelKey(char* name);
extern char* lookupChannelName(int key);
extern int receiveError(int isock, int length);
extern int receiveTermination(int isock, int length);
extern int flushBytes(int isock, int length);
extern void processData(char* buffer, int length);
extern int receiveData(int isock, int length);
extern int gen_log(int level, int verb, ... );
extern double myntohd(double netdbl);
