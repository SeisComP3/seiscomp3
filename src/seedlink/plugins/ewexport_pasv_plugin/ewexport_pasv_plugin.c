
/*
 * ewexport_plugin.c: A SeedLink plugin to collect data from an
 *                    Earthworm passive export_? process via TCP/IP.
 *
 * The version of import_pasv shipped with Earthworm v7.4 was used
 * as a starting point and shoe-horned into a SeedLink plugin.
 * Instead of passing received messages into a local shared memory
 * area, this program gives received waveform data to a controlling
 * SeedLink server.
 *
 * Jan Becker
 * gempa GmbH
 * fork of ewexport_plugin written by:
 * Chad Trabant
 * ORFEUS Data Center
 * IRIS Data Managment Center
 *
 */

#define VERSION "0.93 (2011.137)"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* This file includes structures and defines for Earthworm */
#include "ewdefs.h" 

#include "swap.h"
#include "network.h"
#include "plugin.h"
#include "util.h"
#include "trheadconv.h"

/* Functions in this source file */
static void   *MessageReceiver( void * );
static void   *Heartbeat( void * );
static void    import_filter( char *, int);
static int     WriteToSocket( int, char *, MSG_LOGO * );
static void    dummy_handler(int sig);
static void    term_handler(int sig);
static void    ThreadSignalHandler( int sig );
static int     ThreadStart( pthread_t *tid, pthread_attr_t *attr, void *func(void *), void *arg );
static void    config_params( int argcount, char **argvec );
static void    usage(void);

/* Things to read or derive from configuration file */
static int     verbose = 0;
static int     MaxMsgSize;          /* max size for input/output msgs    */
static int     MyAliveInt;          /* Seconds between sending alive     */
                                    /* message to foreign sender         */
static char    MyAliveString[MAX_ALIVE_STR];  /* Text of above alive message       */

static char    ListenerBindAddr[20];     /* Bind server address, in dot notation */
static char    ListenerPortStr[10];      /* Sender server port number   */
static int     ListenerPort;
static int     SenderHeartRate;     /* Expect alive messages this often from foreign sender */
static char    SenderHeartText[MAX_ALIVE_STR];/* Text making up the sender's heart beat message */

static int     PassiveSocket;
static struct sockaddr_in snd_socket;
static socklen_t          snd_socket_len;

static unsigned int SocketTimeoutLength; /* Length of timeouts on SOCKET_ew calls */

/* Globals: timers, etc, used by both threads */
#define CONNECT_WAIT_DT 10           /* Seconds wait between connect attempts            */
#define THEAD_STACK_SIZE 8192        /* Implies different things on different systems !! */
       /* on os2, the stack grows dynamically beyond this number if needed, but likey    */
       /* at the expesene of performance. Solaris: not sure, but overflow might be fatal  */
volatile time_t LastServerBeat;      /* times of heartbeats from the server machine      */
volatile time_t MyLastInternalBeat;  /* time of last heartbeat into local Earthworm ring */
volatile time_t MyLastSocketBeat;    /* time of last heartbeat to server - via socket    */
volatile int HeartThreadStatus = 0;  /* < 0 if server croaks. Set by heartbeat thread    */
volatile int MessageReceiverStatus =0; /* status of message receiving thread: -1 means bad news */
volatile int Sd = 0;            /* Socket descriptor                                */
char     *MsgBuf;               /* incoming message buffer; used by receiver thread */
pthread_t TidHeart = 0;         /* thread id */
pthread_t TidMsgRcv = 0;        /* thread id */

unsigned char tracebuf_type;
MSG_LOGO      heartlogo;

int main( int argc, char **argv )
{
  int    tmp;
  time_t now;
  int    quit;

  struct sockaddr_in rcv_socket;

  pthread_attr_t heart_attr;
  pthread_attr_t msgrecv_attr;
  
  /* Signal handling, use POSIX calls with standardized semantics */
  struct sigaction sa;
  
  sa.sa_handler = dummy_handler;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGALRM, &sa, NULL);
  
  sa.sa_handler = term_handler;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGQUIT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  
  sa.sa_handler = SIG_IGN;
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGPIPE, &sa, NULL);

  /* Signal-handling function that needs to be inherited by threads */
  sa.sa_handler = ThreadSignalHandler;
  sigaction(SIGUSR1, &sa, NULL);
  
  /* Process command line arguments */
  config_params(argc, argv);
  
  /* Initialize thread attributes */
  pthread_attr_init( &heart_attr );
  pthread_attr_init( &msgrecv_attr );
  pthread_attr_setdetachstate( &heart_attr, PTHREAD_CREATE_DETACHED );
  pthread_attr_setdetachstate( &msgrecv_attr, PTHREAD_CREATE_DETACHED );

  ListenerPort = atoi(ListenerPortStr);
  if ( ListenerPort <= 0 )
  {
    gen_log (1,0, "Wrong passive port given: %s\n", ListenerPortStr);
    return -1;
  }

  /* Heartbeat parameters sanity checks */
  if( 1000 * SenderHeartRate >= SocketTimeoutLength )
    {
      gen_log (0,0, "Socket timeout (%d ms) is less than incoming heartrate (%d sec)",
	       SocketTimeoutLength, SenderHeartRate);
      SocketTimeoutLength = 1000 * SenderHeartRate;
      gen_log (0,0, "Setting socket timeout to %d ms\n", SocketTimeoutLength);
    }
  
  /* Allocate the message buffer */
  if ( ( MsgBuf = (char *) malloc(MaxMsgSize) ) == (char *) NULL )
    {
      gen_log (1,0, "Can't allocate buffer of size %ld bytes\n", MaxMsgSize);
      return -1;
    }

  /* Setup socket listener */
  if ( (PassiveSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
  {
    gen_log (1,0, "Can't create listener socket\n");
    free(MsgBuf);
    return -1;
  }

  if ( setsockopt(PassiveSocket, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(tmp)) != 0 )
  {
    gen_log (1,0, "Can't set socket option SO_REUSEADDR\n");
    free(MsgBuf);
    close(PassiveSocket);
    return -1;
  }

  memset(&rcv_socket, 0, sizeof(rcv_socket));
  rcv_socket.sin_family = AF_INET;
  rcv_socket.sin_port = htons((short)ListenerPort);
  /*rcv_socket.sin_addr.s_addr = INADDR_ANY;*/
  if ( (int)(rcv_socket.sin_addr.s_addr = inet_addr(ListenerBindAddr)) == -1 )
  {
    gen_log (1,0, "inet_addr failed for <%s>\n", ListenerBindAddr);
    free(MsgBuf);
    close(PassiveSocket);
    return -1;
  }

  if ( bind(PassiveSocket, (const struct sockaddr*)&rcv_socket, sizeof(rcv_socket)) == -1 )
  {
    gen_log (1,0, "Unable to bind to port %d\n", ListenerPort);
    free(MsgBuf);
    close(PassiveSocket);
    return -1;
  }

  /* Start the heartbeat thread */
  HeartThreadStatus=0;                /* set it's status flag to ok */
  time((time_t*)&MyLastInternalBeat); /* initialize our last heartbeat time */
  time((time_t*)&MyLastSocketBeat);   /* initialize time of our heartbeat over socket */
  time((time_t*)&LastServerBeat);     /* initialize time of last heartbeat from 
                                           serving machine */
  if ( ThreadStart( &TidHeart, &heart_attr, Heartbeat, NULL ) == -1 )
    {
      gen_log (1,0, "cannot start Heartbeat thread. Exiting.\n");
      close(PassiveSocket);
      free(MsgBuf);
      return -1;
    }

  /* Start the message receiver thread */
  MessageReceiverStatus =0; /* set it's status flag to ok */
  if ( ThreadStart( &TidMsgRcv , &msgrecv_attr, MessageReceiver, NULL ) == -1 )
    {
      gen_log (1,0, "cannot start MessageReceiver thread. Exiting.\n");
      close(PassiveSocket);
      free(MsgBuf);
      return -1;
    }

   /* Working loop: check on server heartbeat status, check on receive thread health.
      check for shutdown requests. If things go wrong, kill all  threads and restart */
  quit=0; /* to restart or not to restart */
  while (1)
  {
    safe_usleep (1000000); /* sleep one second. Remember, the receieve thread is awake */
    time(&now);

    /* How's the server's heart? */
    if (Sd > 0 && difftime(now,LastServerBeat) > (double)SenderHeartRate && SenderHeartRate !=0)
  {
    gen_log (1,0, "No heartbeat received for %d seconds. Restarting connection\n",SenderHeartRate);
    close(Sd); Sd = 0;
  }

    /* How's the receive thread feeling ? */
    if ( MessageReceiverStatus == -1)
  {
    gen_log (1,0, "Receiver thread has quit. Restarting\n");
    quit=1;
  }

    /* How's the heartbeat thread feeling ? */
    if ( HeartThreadStatus == -1)
  {
    gen_log (1,0, "Heartbeat thread unhappy. Restarting\n");
    quit=1;
  }


    /* Any other shutdown conditions here */
    /* blah blah blah */

    /* restart preparations */
    if (quit == 1)
  {
    pthread_kill( TidMsgRcv, SIGUSR1 );
    pthread_kill( TidHeart, SIGUSR1 );
    close (PassiveSocket);
    close (Sd);
    quit=0;
    gen_log (1, 0, "Shutdown plugin due to quit request\n");
    break;
  }

  }  /* end of working loop */

  return(0);
} /* End of main() */


/************************Messge Receiver Thread ***********************
 *          Listen for client heartbeats, and set global variable     *
 *          showing time of last heartbeat. Main thread will take     *
 *          it from there                                             *
 **********************************************************************/
/*
  Modified to read binary messages, alex 10/10/96: The scheme (I got it from Carl) is define
  some sacred characters. Sacred characters are the start-of-message and end-of-message
  framing characters, and an escape character. The sender's job is to cloak unfortunate bit
  patters in the data which look like sacred characters by inserting before them an 'escape'
  character.  Our problem here is to recognize, and use, the 'real' start- and end-of-
  messge characters, and to 'decloak' any unfortunate look-alikes within the message body.
*/
static void *
MessageReceiver( void *dummy )
{
  static int  state;
  static char chr, lastChr;
  static int  nr;
  static int  nchar;                     /* counter for msg buffer (esc's removed) */
  static char startCharacter=STX;        /* ASCII STX characer                     */
  static char endCharacter=ETX;          /* ASCII ETX character */
  static char escape=ESC;                /* our escape character */
  static char inBuffer[INBUFFERSIZE];
  static int  inchar;                    /* counter for raw socket buffer (w/esc) */

  /* Tell the main thread we're ok */
  MessageReceiverStatus=0;

  while(1)
  {
    if ( listen(PassiveSocket, 0) )
    {
      gen_log (1,0, "Set port to listen state failed.\n");
      close(PassiveSocket);
      pthread_exit( (void *)NULL );
    }

    Sd = accept(PassiveSocket, (struct sockaddr*)&snd_socket, &snd_socket_len);
    if ( Sd < 0 ) {
      gen_log (1,0, "Client connection could not be accepted\n");
      continue;
    }

    /* Reset heartbeat */
    time((time_t*)&LastServerBeat);
    gen_log (0,1, "Accepted new client connection\n");
    state=SEARCHING_FOR_MESSAGE_START; /* we're initializing */

    /* Multi-byte Read
       Set inchar to be nr-1, so that when inchar is incremented, they will be
       the same and a new read from socket will take place.
       Set chr to 0 to indicate a null character, since we haven't read any yet.
       Set nchar to 0 to begin building a new MsgBuffer (with escapes removed).
    */
    inchar = -1;
    nr     =  0;
    chr    =  0;
    nchar  =  0;

  /* Working loop: receive and process messages */
  /* We are either (0) initializing: searching for message start
                   (1) expecting a message start: error if not
                   (2) assembling a message.
     The variable "state' determines our mood */

    while(1) /* loop over bytes read from socket */
    {
      /* Get next char operation */
      if (++inchar == nr) {
        /* Read from socket operation */
        nr = my_recv (Sd, inBuffer, INBUFFERSIZE-1, 0, SocketTimeoutLength);

        if (nr<=0)  /* Connection Closed */
        {
          close(Sd); Sd = 0;
          break;
        }
        /*else*/
        inchar=0;
        /* End of Read from socket operation */
      }

      lastChr=chr;
      chr=inBuffer[inchar];
      /* End of Get next char operation */
      
      /* Initialization */
      if (state==SEARCHING_FOR_MESSAGE_START)   /* throw all away until we see a naked start character */
      {
        /* Do we have a real start character? */
        if ( lastChr!=escape && chr==startCharacter)
        {
          state=ASSEMBLING_MESSAGE;  /*from now on, assemble message */
          continue;
        }
      }
      
      /* Confirm message start */
      if (state==EXPECTING_MESSAGE_START)  /* the next char had better be a start character - naked, that is */
      {
        /* Is it a naked start character? */
        if ( chr==startCharacter &&  lastChr != escape) /* This is it: message start!! */
        {
          nchar=0; /* start with firsts char position */
          state=ASSEMBLING_MESSAGE; /* go into assembly mode */
          continue;
        }
	    else   /* we're eating garbage */
	    {
	      gen_log (1,0, "Unexpected character from client. Re-synching\n");
	      state=SEARCHING_FOR_MESSAGE_START; /* search for next start sequence */
	      continue;
	    }
	  }
      
      /* In the throes of assembling a message */
      if (state==ASSEMBLING_MESSAGE)
	  {
	    /* Is this the end? */
	    if (chr==endCharacter)   /* naked end character: end of the message is at hand */
	    {
	      /* We have a complete message */
	      MsgBuf[nchar]=0; /* terminate as a string */
	      
	      if(strcmp(&MsgBuf[9],SenderHeartText)==0) /* Server's heartbeat */
		  {
		    gen_log (0,2, "Received heartbeat from server\n");
		    time((time_t*)&LastServerBeat); /* note time of heartbeat */
		    state=EXPECTING_MESSAGE_START; /* reset for next message */
		    MsgBuf[0]=' '; MsgBuf[9]=' ';
		  }
	      else
		  {
		    /* got a non-heartbeat message */
		    gen_log (0,3, "Received non-heartbeat from server\n");
		    time((time_t*)&LastServerBeat); /* note time of (implied) heartbeat */
		  
		    import_filter(MsgBuf,nchar); /* process the message via user-routine */
		  }
	      state=EXPECTING_MESSAGE_START;
	      continue;
	    }
	    else
	      {
	        /* process the message byte we just read: we know it's not a naked end character*/
	      
	        /* Escape sequence? */
	        if (chr==escape)
		    {  /*  process the escape sequence */
		  
		      /* Read from buffer */

		      /* Get next char operation */
		      if (++inchar == nr)
		      {
		        /* Read from socket operation */
		        nr = my_recv (Sd, inBuffer, INBUFFERSIZE-1, 0, SocketTimeoutLength);

		        if (nr<=0)  /* Connection Closed */
		        {
                  close(Sd); Sd = 0;
		          break;
		        }
		        /*else*/
		        inchar=0;
		      /* End of Read from socket operation */
		      }
		      lastChr=chr;
		      chr=inBuffer[inchar];
		      /* End of Get next char operation */
		  
		      if( chr != startCharacter && chr != endCharacter && chr != escape)
		      {   /* bad news: unknown escape sequence */
		        gen_log (1,0, " Unknown escape sequence in message. Re-synching\n");
		        state=SEARCHING_FOR_MESSAGE_START; /* search for next start sequence */
		        continue;
		      }
		      else   /* it's ok: it's a legit escape sequence, and we save the escaped byte */
		      {
		        MsgBuf[nchar++]=chr; if(nchar>MaxMsgSize) goto freak; /*save character */
		        continue;
		      }
		    }

	        /*  Naked start character? */
	        if (chr==startCharacter)
		    {   /* bad news: unescaped start character */
		      gen_log (1,0, "Unescaped start character in message. Re-synching\n");
		      state=SEARCHING_FOR_MESSAGE_START; /* search for next start sequence */
		      continue;
		    }
	      
	        /* So it's not a naked start, escape, or a naked end: Hey, it's just a normal byte */
	        MsgBuf[nchar++]=chr; if(nchar>MaxMsgSize) goto freak; /*save character */
	        continue;
	      
	        freak:  /* freakout: message won't fit */
	        {
		      gen_log (1,0, "receive buffer overflow after %ld bytes\n", MaxMsgSize);
		      state=SEARCHING_FOR_MESSAGE_START; /* initialize again */
		      nchar=0;
		      continue;
	        }
	    } /* end of not-an-endCharacter processing */
	  } /* end of state==ASSEMBLING_MESSAGE processing */
    }  /* end of loop over characters */
  }
}  /* end of MessageReceiver thread */


/***************************** Heartbeat **************************
 *           Send a heartbeat to the server via socket            *
 *                 Check on our server's hearbeat                 *
 *           Slam socket shut if no Server heartbeat: that        *
 *               really shakes up the main thread                 *
 ******************************************************************/
static void *
Heartbeat( void *dummy )
{
  time_t    now;
  
  /* once a second, do the rounds. If anything looks bad, set HeartThreadStatus to -1
     and go into a long sleep. The main thread should note that our status is -1,
     and launch into re-start procedure, which includes killing and restarting us. */
  while ( 1 )
    {
      safe_usleep (1000000);
      time(&now);

      /* Beat our heart (over the socket) to our server */
      if (Sd > 0 && difftime(now,MyLastSocketBeat) > (double)MyAliveInt && MyAliveInt != 0)
	{
	  if ( WriteToSocket( Sd, MyAliveString, &heartlogo ) != 0 )
            {
	      /* If we get an error, simply quit */
	      gen_log (1,0, "problem sending alive msg to socket\n");
	      /*
	      HeartThreadStatus=-1;
	      pthread_exit( (void *)NULL );
	      gen_log (1,0, "Fatal system error: Heart thread could not pthread_exit()\n");
	      exit(-1);
	      */
            }
	  else
	      gen_log (0,2, "Heartbeat sent to export server\n");
	  MyLastSocketBeat=now;
	}
    }

  pthread_exit( (void *)NULL );
} /* End of Heartbeat() */


/************************** import_filter *************************
 *                   Process received messages
 *
 * We assume thaq the first 9 characters are three three-character
 * fields giving IstallationId, ModuleId, and MessageType as this is
 * expected from an export_(generic,scn,etc.) on the other end.
 *
 ******************************************************************/
static void
import_filter( char *msg, int msgLen )
{
  static char *msgptr = NULL;
  static int *intconv = NULL;

  int idx;
  int *int_samples     = NULL;  /* Assuming int type is 32-bit integers */
  short *short_samples = NULL;  /* Assuming short type is 16-bit integers */
  char origdatatype[2];
  char cInst[4], cMod[4], cType[4];
  MSG_LOGO logo;
  TRACE2_HEADER *trh2;
  
  /* We assume that this message was created by our bretheren "export_generic",
     which attaches the logo as three groups of three characters at the front of
     the message */
  /* Peel off the logo chacters */
  strncpy(cInst,msg,    3);  cInst[3]=0;  logo.instid =(unsigned char)atoi(cInst);
  strncpy(cMod ,&msg[3],3);  cMod[3] =0;  logo.mod    =(unsigned char)atoi(cMod);
  strncpy(cType,&msg[6],3);  cType[3]=0;  logo.type   =(unsigned char)atoi(cType);
  
  /* We need an aligned buffer to access structures (especially for SPARC) */
  msgptr = (char *) realloc (msgptr, msgLen - 9);
  if ( msgptr == NULL )
    {
      gen_log (1,0, "cannot allocate message alignment buffer\n");
      return;
    }
  memcpy (msgptr, &msg[9], msgLen - 9);

  if ( logo.type == TYPE_TRACEBUF )
    {
      TrHeadConv((TRACE_HEADER *) msgptr);
      logo.type = TYPE_TRACEBUF2;
    }

  if ( logo.type == TYPE_TRACEBUF2 )
  {
    trh2 = (TRACE2_HEADER *) msgptr;

    /* Copy the original data type as it might be changed in WaveMsgMakeLocal() */
    *(origdatatype) = *(trh2->datatype);  *(origdatatype+1) = *(trh2->datatype+1);

    /* Swap TRACEBUF to local byte order if needed */
    if ( WaveMsg2MakeLocal( trh2 ) == -1 )
    {
      gen_log (1,0, "WaveMsgMakeLocal: unknown datatype %s\n", trh2->datatype);
      return;
    }

    gen_log (0,2, "Received TRACEBUF2 message(%d): %s_%s_%s_%s, %d samps of %2.2s at %.2f sps starting %f\n",
         msgLen-9, trh2->net, trh2->sta, trh2->loc, trh2->chan, trh2->nsamp, origdatatype, trh2->samprate, trh2->starttime);

    /* Check if 16-bit samples were received and convert to 32-bit if so */
    if ( (strcmp (trh2->datatype, "s2")==0) ||
     (strcmp (trh2->datatype, "i2")==0) )
    {
      intconv = (int *) realloc (intconv, sizeof(int) * trh2->nsamp);
      if ( intconv == NULL ) {
        gen_log (1,0, "cannot allocate 32-bit integer conversion buffer\n");
        return;
      }

      short_samples = (short *) (msgptr + sizeof(TRACE_HEADER));

      for ( idx = 0; idx < trh2->nsamp; idx++ ) {
        *(intconv+idx) = *(short_samples+idx);
      }

      int_samples = intconv;
    }
    else if ( (strcmp (trh2->datatype, "s4")==0) ||
	  (strcmp (trh2->datatype, "i4")==0) )
    {
      int_samples = (int *) (msgptr + sizeof(TRACE2_HEADER));
    }
    else
    {
      gen_log (1,0, "Unknown datatype %s\n", trh2->datatype);
      return;
    }

    /* Send the data to the controlling SeedLink server */
    send_raw_depoch (trh2->sta, trh2->chan, trh2->starttime, 0, 100, int_samples, trh2->nsamp);

    /* Print samples to STDERR if verbosity is 3 or higher */
    if ( verbose >= 3 && int_samples != NULL )
    {
      for ( idx = 0; idx < trh2->nsamp; idx++ ) {
        fprintf ( stderr, "%6d ", *(int_samples+idx) );
        if(idx%10==9) fprintf ( stderr, "\n" );
      }
    }
  }
  else
    {
      gen_log (0,2, "Received non-TRACEBUF message(%d): instid %d, modid %d, type %d\n",
	       msgLen-9, logo.instid, logo.mod, logo.type);
    }      
  
  return;
} /* End of import_filter() */


/*************************** WriteToSocket ************************
 *    send a message logo and message to the socket               *
 *    returns  0 if there are no errors                           *
 *            -1 if any errors are detected                       *
 ******************************************************************/
static int
WriteToSocket( int sock, char *msg, MSG_LOGO *logo )
{
  char asciilogo[11];       /* ascii version of outgoing logo */
  char startmsg = STX;      /* flag for beginning of message  */
  char endmsg   = ETX;      /* flag for end of message        */
  int  msglength;           /* total length of msg to be sent */
  int  rc;
  
  msglength=strlen(msg);    /* assumes msg is null terminated */
    
  /* Send "start of transmission" flag & ascii representation of logo */
  sprintf( asciilogo, "%c%3d%3d%3d",startmsg,
           (int) logo->instid, (int) logo->mod, (int) logo->type );
  rc = my_send( sock, asciilogo, 10, 0, SocketTimeoutLength);
  if( rc != 10 )
    {
      gen_log (1,0, "my_send() %s\n", strerror(errno));
      return( -1 );
    }
  
  /* Send message; break it into chunks if it's big! */
  rc = my_send( sock, msg, msglength, 0, SocketTimeoutLength );
  if ( rc == -1 )
    {
      gen_log (1,0, "my_send() %s\n", strerror(errno));
      return( -1 );
    }

  /* Send "end of transmission" flag */
  rc = my_send( sock, &endmsg, 1, 0, SocketTimeoutLength);
  if( rc != 1 ) 
    {
      gen_log (1,0, "my_send() %s\n", strerror(errno));
      return( -1 );
    }
  
  return( 0 );
} /* End of WriteToSocket() */


/* Signal handling routines */
static void
dummy_handler(int sig) { }

static void
term_handler(int sig)
{
  if ( TidHeart )
    pthread_kill (TidHeart, SIGUSR1);
  
  if ( TidMsgRcv )
    pthread_kill (TidMsgRcv, SIGUSR1);
  if ( PassiveSocket )
    close (PassiveSocket);
  if ( Sd )
    close (Sd);

  exit( sig );
} /* End of term_handler() */

static void
ThreadSignalHandler( int sig )
{
  switch (sig)
    {
    case SIGUSR1:      
      pthread_exit( (void *)NULL );
    }
}


/***************************************************************************
 * ThreadStart():
 *
 * Return -1 on error and 0 on success.
 ***************************************************************************/
static int
ThreadStart( pthread_t *tid, pthread_attr_t *attr,
	     void *func(void *), void *arg )
{
  int rc;
    
  /* Start the thread */
  rc = pthread_create( tid, attr, func, arg );
  
  if ( rc != 0 )
    return -1;
  
  return 0;
} /* End of ThreadStart() */


/*****************************************************************************
 *  config_params() processes command line arguments and set defaults        *
 *****************************************************************************/
static void
config_params( int argcount, char **argvec )
{
  int error = 0;
  int optind;
  char *heartlogostr = NULL;
 
  /* Setup defaults */
  MaxMsgSize = 4096;
  SocketTimeoutLength = 80000;

  MyAliveInt = 120;
  strncpy (MyAliveString, "alive", MAX_ALIVE_STR-1);
  MyAliveString[MAX_ALIVE_STR-1] = '\0';
  
  SenderHeartRate = 60;
  strncpy (SenderHeartText, "alive", MAX_ALIVE_STR-1);
  SenderHeartText[MAX_ALIVE_STR-1] = '\0';
  
  ListenerPortStr[0] = '\0';
  
  heartlogo.instid = 0;
  heartlogo.mod    = 0;
  heartlogo.type   = TYPE_HEARTBEAT;

  tracebuf_type = TYPE_TRACEBUF;

  if (argcount < 5) error++;

  /* Process all command line arguments */
  for(optind=1 ; optind < argcount ; optind++) {
    if (strncmp(argvec[optind], "-v", 2) == 0) {
      verbose += strspn (&argvec[optind][1], "v");
    }
    else if (strcmp(argvec[optind], "-u") == 0) {
      usage();
    }
    else if (strcmp(argvec[optind], "-s") == 0) {
      if (++optind < argcount ) {
	strncpy (ListenerBindAddr, argvec[optind], 19);
	ListenerBindAddr[19] = '\0';
      }
      else error++;
    }
    else if (strcmp(argvec[optind], "-p") == 0) {
      if (++optind < argcount ) {
	strncpy (ListenerPortStr, argvec[optind], 9);
	ListenerPortStr[9] = '\0';
      }
      else error++;
    }
    else if (strcmp(argvec[optind], "-Ar") == 0) {
      if (++optind < argcount ) {
	MyAliveInt = atoi(argvec[optind]);
      }
      else error++;
    }
    else if (strcmp(argvec[optind], "-At") == 0) {
      if (++optind < argcount ) {
	strncpy (MyAliveString, argvec[optind], MAX_ALIVE_STR-1);
	MyAliveString[MAX_ALIVE_STR-1] = '\0';
      }
      else error++;
    }
    else if (strcmp(argvec[optind], "-Sr") == 0) {
      if (++optind < argcount ) {
	SenderHeartRate = atoi(argvec[optind]);
      }
      else error++;
    }
    else if (strcmp(argvec[optind], "-St") == 0) {
      if (++optind < argcount ) {
	strncpy (SenderHeartText, argvec[optind], MAX_ALIVE_STR-1);
	SenderHeartText[MAX_ALIVE_STR-1] = '\0';
      }
      else error++;
    }
    else if (strcmp(argvec[optind], "-m") == 0) {
      if (++optind < argcount ) {
	MaxMsgSize = atoi(argvec[optind]);
      }
      else error++;
    }
    else if (strcmp(argvec[optind], "-t") == 0) {
      if (++optind < argcount ) {
	SocketTimeoutLength = atoi(argvec[optind]);
      }
      else error++;
    }
    else if (strcmp(argvec[optind], "-Hl") == 0) {
      if (++optind < argcount ) {
	heartlogostr = argvec[optind];
      }
      else error++;
    }
    else if (strcmp(argvec[optind], "-Tt") == 0) {
      if (++optind < argcount ) {
	tracebuf_type = atoi(argvec[optind]);
      }
      else error++;
    }
  }

  /* Check for required host and port assignments */
  if ( ListenerPortStr[0] == '\0' )
    error++;
  
  /* Initialize the verbosity for the gen_log function */
  gen_log(-1, verbose);
  
  /* Report the program version */
  gen_log(0,0, "ewexport_pasv_plugin version: %s\n", VERSION);
  
  /* Parse the heartbeat logo if specified */
  if ( heartlogostr )
    {
      char *cptr, *dptr, *nptr;
      
      cptr = strchr (heartlogostr, '/');
      dptr = strrchr (heartlogostr, '/');
      
      if ( cptr == NULL || dptr == NULL || cptr == dptr )
	{
	  gen_log (1,0, "Heartbeat logo specified incorrectly: %s\n", heartlogostr);
	  error++;
	}
      else
	{
	  nptr = cptr++; nptr = '\0';
	  nptr = dptr++; nptr = '\0';
	  
	  heartlogo.instid = (unsigned char) atoi(heartlogostr);
	  heartlogo.mod    = (unsigned char) atoi(cptr);
	  heartlogo.type   = (unsigned char) atoi(dptr);
	}
    }

  /* If errors then report the usage message and quit */
  if ( error )
    usage();
  
  return;
} /* End of config_params() */


/***************************************************************************
 * usage():
 *
 * Print usage message and exit.
 ***************************************************************************/
static void
usage(void)
{
  fprintf(stderr,"\n"
	  "Usage: ewexport_pasv_plugin [options] <-s address> <-p port>\n"
	  "\n"
	  "  -u            Print this usage message\n"
	  "  -v            Be more verbose, mutiple flags can be used\n"
	  "  -s address    Bind address of export server\n"
	  "  -p port       Port to connect to\n"
	  "\n"
	  "  -Ar rate      Rate at which to send hearbeats to server, default=120\n"
	  "  -At text      Text for heartbeat to server, default='alive'\n"
	  "  -Sr rate      Rate at which to expect heartbeats from server, default=60\n"
	  "  -St text      Text which is expected in heartbeats from server, default='alive'\n"
	  "  -m maxmsg     Maximum message size that can be received, default=4096\n"
	  "  -t timeout    Socket timeout in milliseconds, default=80000\n"
	  "  -Hl #/#/#     Specify the logo to use for heartbeats, default=0/0/3\n"
	  "                   #/#/# is the <inst id>/<mod id>/<heartbeat type>\n"
	  "  -Tt type      Specify the message type for TRACE_BUF messages, default=20\n"
	  "\n\n");
  
  exit (1);
} /* End of usage() */
