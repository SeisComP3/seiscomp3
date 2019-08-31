/**************************************************************************

  naqs_utils.c:  Utility functions for communicating with a NAQS.

  Based on the example code (dsClient.cpp) distrubuted by Nanometrics.
  The original copyright notice is below.  Notification was given that
  "there are no restrictions regarding the use of the dsClient.cpp snippet"
  (Neil Spriggs, Nanometrics, 7-2-2002).

  Slapped together by
  Chad Trabant ORFEUS/EC-Project MEREDIAN


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

/* Includes -------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "naqs_utils.h"
#include "plugin.h"

/* Swap 8 bytes, stolen from qlib2 */
void swab8 (double *in);

/* It's ugly, but simplifies things greatly */
extern ChannelList channelList;
extern char *network;

/**************************************************************************

  Function:  openSocket

  Purpose:   looks up target host, opens a socket and connects

 ------------------------------------------------------------------------*/
extern int
openSocket(char *hostname, int portNum)
{
  static int sleepTime = 1;
  int isock = 0;
  struct hostent *hostinfo = NULL;
  struct sockaddr_in psServAddr;
  struct in_addr hostaddr;
  
  if (!hostname)
  {
    gen_log(1,0, "Empty host name?\n");
    return -1;
  }

  if ( (hostinfo = gethostbyname(hostname)) == NULL) {
    gen_log(1,0, "Cannot lookup host: %s\n", hostname);
    return -1;
  }

  while(1)
  {
    isock = socket (AF_INET, SOCK_STREAM, 0);
    if (isock < 0)
    {
      gen_log (1,0, "Can't open stream socket\n");
      exit(1);
    }

    /* Fill in the structure "psServAddr" with the address of server
       that we want to connect with */
    memset (&psServAddr, 0, sizeof(psServAddr));
    psServAddr.sin_family = AF_INET;
    psServAddr.sin_port = htons((unsigned short) portNum);
    psServAddr.sin_addr = *(struct in_addr *)hostinfo->h_addr_list[0];

    /* Report action and resolved address */
    memcpy(&hostaddr.s_addr, *hostinfo->h_addr_list,
	   sizeof (hostaddr.s_addr));
    gen_log(0,1, "attempting to connect to %s port %d\n",
	    inet_ntoa(hostaddr), portNum);

    if (connect(isock, (struct sockaddr *)&psServAddr, sizeof(psServAddr)) >= 0)
    {
      sleepTime = 1;
      gen_log(0,0, "Connection established: socket=%i,IP=%s,port=%d\n",
	      isock, inet_ntoa(hostaddr), portNum);
      return isock;
    }
    else
    {
      gen_log(0,1, "Trying again later...Sleeping\n");
      close (isock);
      sleep (sleepTime);
      sleepTime *= 2;
      if (sleepTime > SLEEPMAX)
        sleepTime = SLEEPMAX;
    }
  }
}

/**************************************************************************

  Function:  s_send

  Purpose:  sends a message and computes rc

  Return:   rc = SOCKET_OK on success
            rc = SOCKET_ERROR on error

 ------------------------------------------------------------------------*/
extern int
s_send(int isock, void* data, int length)
{
  int sendCount = send(isock, (char*) data, length, 0);
  
  if (sendCount != length)
    return SOCKET_ERROR;

  return SOCKET_OK;
}

/**************************************************************************

  Function:  s_recv

  Purpose:  receives a message and computes rc

  Return:   rc = SOCKET_OK on success
            rc = SOCKET_ERROR on error

 ------------------------------------------------------------------------*/
extern int
s_recv(int isock, void* data, int length)
{
  int recvCount;
  recvCount= recv(isock, (char*) data, length, MSG_WAITALL);

  if (recvCount != length)
    return SOCKET_ERROR;
  
  return SOCKET_OK;
}

/**************************************************************************

  Function:  sendConnectMessage

  Purpose:  sends a Connect message to server

 ------------------------------------------------------------------------*/
extern int
sendConnectMessage(int isock)
{
  return sendHeader(isock, CONNECT_MSG, 0);
}

/**************************************************************************

  Function:  sendHeader

  Purpose:  sends a MessageHeader to the server

 ------------------------------------------------------------------------*/
extern int
sendHeader(int isock, int type, int length)
{  
  MessageHeader msg;
  msg.signature = htonl(NMX_SIGNATURE);
  msg.type      = htonl(type);
  msg.length    = htonl(length);

  return s_send(isock, &msg, sizeof(msg));
}

/**************************************************************************

  Function:  requestTypeChannel

  Purpose:  requests one channel of serial or soh data

 ------------------------------------------------------------------------*/
extern int
requestTypeChannel(int isock, int channel, int type)
{
  AddRequest request;

  int rc = sendHeader(isock, type, sizeof(request));

  if (rc == SOCKET_OK)
  {
    request.numChannels = htonl(1);
    request.channel     = htonl(channel);
    request.stcDelay    = htonl(0);
    request.sendBuffers = htonl(0);
    rc = s_send(isock, &request, sizeof(request));
  }

  return rc;
}

/**************************************************************************

  Function:  requestSerialChannel

  Purpose:  requests one channel of serial data

 ------------------------------------------------------------------------*/
extern int
requestSerialChannel(int isock, int channel)
{
  gen_log(0,1, "Requesting serial channel 0x%8.8x\n", channel);
  return requestTypeChannel(isock, channel, SERIAL_ADD_REQ);
}

/**************************************************************************

  Function:  requestSohChannel

  Purpose:  requests one channel of SOH data

 ------------------------------------------------------------------------*/
extern int
requestSohChannel(int isock, int channel)
{
  gen_log(0,1, "Requesting soh channel 0x%8.8x\n", channel);
  return requestTypeChannel(isock, channel, SOH_ADD_REQ);
}

/**************************************************************************

  Function:  requestDataChannel

  Purpose:  requests one channel of uncompressed time series data

 ------------------------------------------------------------------------*/
extern int
requestDataChannel(int isock, int channel)
{
  DataAddRequest request;
  int rc = 0;

  gen_log(0,1, "Requesting time series channel 0x%8.8x\n", channel);

  rc = sendHeader(isock, TIMSER_ADD_REQ, sizeof(request));

  if (rc == SOCKET_OK)
  {
    request.numChannels = htonl(1);       /* One Channel */
    request.channel     = htonl(channel); /* Channel key */
    request.stcDelay    = htonl(60);  /* Short-term completion time, 60 sec */
    request.format      = htonl(0);   /* Uncompressed, orig. samp. rate */
    request.sendBuffers = htonl(1);   /* Do send buffered data */
    rc = s_send(isock, &request, sizeof(request));
  }

  return rc;
}

/**************************************************************************

  Function:  requestDataChannels

  Purpose:  request multiple channels of uncompressed time series data

 ------------------------------------------------------------------------*/
extern int
requestDataChannels(int isock, int keyList[], int found,
		    int samprate, int shortTermComp)
{
  int rc = 0;
  int count;
  int contentsize;
  int bufidx = 0;
  char *buffer;
  long netlong;

  /* Calculate total size of content */
  contentsize = 16 + ( 4 * found );
  buffer = (char *) malloc ( contentsize );

  gen_log(0,1, "Requesting time series channels\n");

  rc = sendHeader(isock, TIMSER_ADD_REQ, contentsize );

  if ( rc == SOCKET_OK )
  {
    /* Build up the content part of the request */
    netlong = htonl(found);                  /* Number of channels */
    memcpy(&buffer[bufidx], &netlong, 4); bufidx += 4;
    for (count = 0; count < found; count++) {
      netlong = htonl(keyList[count]);
      memcpy(&buffer[bufidx], &netlong, 4); bufidx +=4; /* Channel */
      gen_log(0,1, "Requesting channel: %s\n",
	      lookupChannelName(keyList[count]));
    }
    netlong = htonl(shortTermComp);
    memcpy(&buffer[bufidx], &netlong, 4); bufidx += 4; /* STC delay */
    netlong = htonl(samprate);
    memcpy(&buffer[bufidx], &netlong, 4); bufidx += 4; /* Format */
    netlong = htonl(0);
    memcpy(&buffer[bufidx], &netlong, 4); bufidx += 4; /* Send buffer? */

    rc = s_send( isock, (void *) buffer, contentsize );
  }

  free( buffer );

  return rc;
}

/**************************************************************************

  Function:  requestChannel

  Purpose:  requests one channel of any type

 ------------------------------------------------------------------------*/
extern int
requestChannel(int isock, int channel)
{
  int type = dataType(channel);
  if (type == TIMSER_TYPE)
    return requestDataChannel(isock, channel);
  else if (type == SOH_TYPE)
    return requestSohChannel(isock, channel);
  else
    return requestSerialChannel(isock, channel);
}

/**************************************************************************

  Function:  receiveHeader

  Purpose:  receives a MessageHeader from the server

 ------------------------------------------------------------------------*/
extern int
receiveHeader(int isock, MessageHeader* pmsg)
{
  int rc;
  rc = s_recv(isock, pmsg, sizeof(MessageHeader));

  if (rc == SOCKET_OK)
  {
    pmsg->signature = ntohl(pmsg->signature);
    pmsg->type      = ntohl(pmsg->type);
    pmsg->length    = ntohl(pmsg->length);

    if (pmsg->signature != NMX_SIGNATURE)
      rc = SOCKET_ERROR;
  }

  return rc;
}

/**************************************************************************

  Function:  receiveChannelList

  Purpose:  receives a ChannelList from the server

 ------------------------------------------------------------------------*/
extern int
receiveChannelList(int isock, ChannelList* plist, int length)
{
  int ich = 0;
  int recvCount = recv(isock, (char*) plist, length, MSG_WAITALL);
  
  if (recvCount != length) {
      printf(" dying, recvCount: %d, length: %d\n", recvCount, length);
    return SOCKET_ERROR;
  }

  plist->length = ntohl(plist->length);
  if ((unsigned) length != 4 + plist->length * sizeof(ChannelKey))
  {
    gen_log(1,0, "wrong number of channels in Channel List\n");
    return SOCKET_ERROR;
  }

  for (ich = 0; ich < (int) plist->length; ich++)
  {
    plist->channel[ich].key = ntohl(plist->channel[ich].key);
    gen_log(0,2, "Channel %s has key 0x%8.8lx\n", plist->channel[ich].name,
	   plist->channel[ich].key);
  }

  return SOCKET_OK;
}

/**************************************************************************

  Function:  receiveData

  Purpose:  receives uncompressed data from the server

 ------------------------------------------------------------------------*/
extern int
receiveData(int isock, int length)
{
  int rc = 0;

  if (length > 0)
  {
    char* buffer = (char*) malloc(length);
    rc = s_recv(isock, buffer, length);
    if (rc == SOCKET_OK) {
      processData(buffer, length);
    }
    free(buffer);
  }

  return rc;
}

/**************************************************************************

  Function:  receiveError

  Purpose:  receives an Error message from the server

 ------------------------------------------------------------------------*/
extern int
receiveError(int isock, int length)
{
  int rc = 0;
  
  if (length > 0)
  {
    char* buffer = (char*) malloc(length);
    rc = s_recv(isock, buffer, length);
    if (rc == SOCKET_OK)
      gen_log(0,0, "%s\n", buffer);
    free(buffer);
  }
  return rc;
}

/**************************************************************************

  Function:  receiveTermination

  Purpose:  receives a Terminate message from the server

 ------------------------------------------------------------------------*/
extern int
receiveTermination(int isock, int length)
{
  int reason = 0;
  int rc = s_recv(isock, &reason, 4);
  
  if (rc == SOCKET_OK)
  {
    gen_log(0,0, "Connection closed by server, reason = %d\n", ntohl(reason));

    if (length > 4)
      rc = receiveError(isock, length - 4);
  }
  return rc;
}

/**************************************************************************

  Function:  processData

  Purpose:  processes compressed data from the server and sends them
            to the controlling SeedLink server.

 ------------------------------------------------------------------------*/
extern void
processData(char* buffer, int length )
{
  int32_t   netInt    = 0;
  int32_t   pKey      = 0;
  double    pTime     = 0.0;
  int32_t   pNSamp    = 0;
  int32_t   pSampRate = 0;
  int32_t  *pDataPtr  = 0;
  int       swap      = 0;
  int       idx;

  char *sta = 0;      /* The station code */
  char *chan = 0;     /* The channel code */

  /* copy the header contents into local fields and swap */
  memcpy(&netInt, &buffer[0], 4);
  pKey = ntohl(netInt);
  if ( pKey != netInt ) { swap = 1; }

  memcpy(&pTime, &buffer[4], 8);
  if ( swap ) { swab8(&pTime); }

  memcpy(&netInt, &buffer[12], 4);
  pNSamp = ntohl(netInt);
  memcpy(&netInt, &buffer[16], 4);
  pSampRate = ntohl(netInt);

  /* There should be (length - 20) bytes of data as 32-bit ints here */
  pDataPtr = (int32_t *) &buffer[20];

  /* Swap the data samples to host order */
  for ( idx=0; idx < pNSamp; idx++ ) {
      netInt = ntohl(pDataPtr[idx]);
      pDataPtr[idx] = netInt;
  }

  /* Lookup the station and channel code */
  sta = strdup(lookupChannelName(pKey));
  if ( (chan = strchr(sta, '.')) == NULL ) {
    gen_log(1,0, "Channel name not in STN.CHAN format: %s\n", sta);
    free(sta);
    return;
  }
  *chan++ = '\0';

  {
    char sta_id[11];
    snprintf(sta_id, 11, "%s.%s", network, sta);
    /* Send it off to the controlling SeedLink server */
    if ( send_raw_depoch(sta_id, chan, pTime, 0, -1, pDataPtr, pNSamp) < 0 ) {
      gen_log(1,0, "cannot send data to seedlink: %s", strerror(errno));
      exit(1);
    }
  }

  /* print out header and/or data for different packet types */
  gen_log(0,2, "Received uncompressed data for stream %ld (%s_%s)\n",
	  pKey, sta, chan);
  gen_log(0,2, "  length: %d, nsamp: %d, samprate: %d, time: %f\n",
	  length, pNSamp, pSampRate, pTime);
  
  free(sta);
}

/**************************************************************************

  Function:  flushBytes

  Purpose:  receives and discards some bytes from the server

 ------------------------------------------------------------------------*/
extern int
flushBytes(int isock, int length)
{
  int rc = 0;

  if (length > 0)
  {
    char* buffer = (char*) malloc(length);
    rc = s_recv(isock, buffer, length);
    free(buffer);
  }
  return rc;
}

/**************************************************************************

  Function:  lookupChannelKey

  Purpose:  looks up a channel key in the ChannelList using the name

 ------------------------------------------------------------------------*/
extern int
lookupChannelKey(char* name)
{
  int length = channelList.length;
  int ich = 0;

  for (ich = 0; ich < length; ich++)
  {
    if (strcasecmp(name, channelList.channel[ich].name) == 0)
      return channelList.channel[ich].key;
  }

  return KEY_NOT_FOUND;
}

/**************************************************************************

  Function:  lookupChannelName

  Purpose:  looks up a channel name in the ChannelList using a key

 ------------------------------------------------------------------------*/
extern char *
lookupChannelName(int key)
{
  int length = channelList.length;
  int ich = 0;

  for (ich = 0; ich < length; ich++)
  {
    if ( key == channelList.channel[ich].key )
      return &channelList.channel[ich].name[0];
  }

  return NAME_NOT_FOUND;
}

/* A generic logging/printing routine
   This function works in two modes:
   1 - Initialization, expecting 2 arguments with the first (level)
       being -1 and the second being verbosity.  This will set the
       verbosity for all future calls, the default is 0.  Can be used
       to change the verbosity at any time. I.e. 'sl_log(-1,2);'

   2 - expecting 3+ arguments, log level, verbosity level, printf
       format, and printf arguments.  If the verbosity level is less
       than or equal to the set verbosity (see mode 1), the printf
       format and arguments will be printed at the appropriate log
       level. I.e. 'sl_log(0, 0, "error: %s", result);'

   Returns the new verbosity if using mode 1.
   Returns the number of characters formatted on success, and a
     a negative value on error if using mode 2.
*/
extern int
gen_log(int level, int verb, ... )
{
  static int staticverb = 0;
  int retvalue;

  if ( level == -1 ) {
    staticverb = verb;
    retvalue = staticverb;
  }
  else if (verb <= staticverb) {
    char message[100];
    char timestr[100];
    char *format;
    va_list listptr;
    time_t loc_time;

    va_start(listptr, verb);
    format = va_arg(listptr, char *);

    /* Build local time string and cut off the newline */
    time(&loc_time);
    strcpy(timestr, asctime(localtime(&loc_time)));
    timestr[strlen(timestr) - 1] = '\0';

    retvalue = vsnprintf(message, 100, format, listptr);

    if ( level == 1 ) {
      printf("%s - naqs_plugin: error: %s",timestr, message);
    }
    else {
      printf("%s - naqs_plugin: %s", timestr, message);
    }

    fflush(stdout);
    va_end(listptr);
  }

  return retvalue;
} /* End of gen_log() */


/* the swab8() function by Doug Neuhauser, stolen from qlib2 */
void
swab8 (double *in)    /* ptr to double to byteswap */
{
    unsigned char *p = (unsigned char *)in;
    unsigned char tmp;
    tmp = *p;
    *p = *(p+7);
    *(p+7) = tmp;
    tmp = *(p+1);
    *(p+1) = *(p+6);
    *(p+6) = tmp;
    tmp = *(p+2);
    *(p+2) = *(p+5);
    *(p+5) = tmp;
    tmp = *(p+3);
    *(p+3) = *(p+4);
    *(p+4) = tmp;
}
