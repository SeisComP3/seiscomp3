/* 
 * scream2slink module  
 *
 *    This plugin is able to receive version 4 data-packets 
 *    from SCREAM through UDP and to request missing version 
 *    4 data-packets from SCREAM through TCP.
 *
 */

/* Mar 2004  - Developed by Reinoud Sleeman  (ORFEUS/KNMI)
 *             for the SCREAM plugin in SeedLink
 * Oct 2008  - Improved by Jan Becker (gempa.de) to support
 *             stream mapping using the sysid of a stream
 *             optionally.
 *
 * Nov 2010  - This plugin opens a UDP connection to receive GCF data blocks
 *             from a SCREAM server. GCF data-packets are stored in a central
 *             rinbuffer using a thread. Whenever a data record is missing -
 *             based on record numbers, not on time ! - a TCP request is send
 *             out to the SCREAM server to have the missing record being re-send.
 *
 *             A second thread takes the records from the central ringbuffer
 *             and distributes these over ringbuffers per channel. In each
 *             channel ringbuffer the blocks are sorted on time and send to
 *             seedlink when these are in time order. Whenever a gap is
 *             detected the record(s) in the stack will be put on hold to allow
 *             delayed data to flow in. The stack will be released when the
 *             delayed data has arrived (and filled the gap), or when the stack is full.
 *             The longer the stack the longer is wait for delayed data, hence the
 *             latency can be lagre.

 *             Reinoud Sleeman (ORFEUS/KNMI)
 *
 * Oct 2014  - Added support for TCP connections to receive GCF data blocks.
 *             The connection code has been ported from scream_plugin.
 *             Additionally some includes and compiler warnings were removed
 *             though there are still lots of remaining.
 *
 *             Jan Becker (gempa GmbH)
 */


#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "project.h"
#include "config.h"
#include "map.h"
#include "gcf.h"
#include "plugin.h"


#define ORBSIZE      2000        // size of the first ringbuffer to store incoming GCF records
                                 // this size does not affect latency

extern  uint8_t      *request_block_tcp_mode ( uint16_t );


int                  pthread_create ();
void                 *listen_to_scream (void *in);
void                 *distribute_forever (void *in);
double               get_dtime (void);
pthread_t            thread_scream;
pthread_t            thread_distribute;
uint8_t              *cbuf[ORBSIZE];
char                 *mapfile;
Map                  *rootmap;
struct config_struct config;
int                  previousblocknr;
char                 *dumpfile;
int                  TCP_REQUEST=0;
int                  NO_SEND=0;
int                  DEBUG=0;
int                  RSIZE;

// these variables are used in threads and should not be made static
pthread_mutex_t      mutex1 = PTHREAD_MUTEX_INITIALIZER;
int                  wptr;
int                  optrr;
int                  wcflag;



void my_exit (int sig) {
        switch  (sig)
        {
           case SIGABRT:
                printf("\nSIGABRT - Abnormal termination.\n");
                break;
           case SIGFPE:
                printf("\nSIGFPE - Divide by 0 or overflow. (Floating point exception.)\n");
                break;
           case SIGILL:
                printf("\nSIGILL - An illegal instruction in the program.\n");
                break;
           case SIGINT:
                printf("\nSIGINT - Interactive interrupt.\n");
                break;
           case SIGSEGV:
                printf("\nSIGSEGV - Segmentation violation.\n");
                break;
           case SIGTERM:
                printf("\nSIGTERM - Termination request made to the program.\n");
                break;
        }
        fflush (stdout);
        fflush (stderr);
        printf("\n");
        exit(sig);
}


void parse_mapfile (char *mapfile);
void gcf_byte_swap(uint8_t* buf);


int
main (int argc, char **argv)
{
  int       optind, i;
  Map       *mp;
  char      scream_arg[60];
  char      gcf_arg[60];

  //printf("SCREAM2EW_DATATYPE = %s\n", SCREAM2EW_DATATYPE);


  (void) signal(SIGABRT, my_exit);
  (void) signal(SIGFPE, my_exit);
  (void) signal(SIGILL, my_exit);
  (void) signal(SIGINT, my_exit);
  (void) signal(SIGSEGV, my_exit);
  (void) signal(SIGTERM, my_exit);

  RSIZE = -1;

  // UDP is used by default

  config.protocol = SCM_PROTO_UDP;

  /* Process command line argument */

  for(optind=1 ; optind < argc ; optind++)
  {
    if (strcmp(argv[optind], "-h") == 0) 
    {
         config.server = (char *) malloc (100);
         strcpy ( config.server, argv[++optind] );
    }
    else if (strcmp(argv[optind], "-p") == 0)          // port (UDP or TCP)
    {
         config.port = atoi(argv[++optind]);
    }
    else if (strcmp(argv[optind], "-r") == 0)          // TCP port
    {
         config.reqport = (uint16_t) atoi(argv[++optind]);
    }
    else if (strcmp(argv[optind], "-f") == 0)          // dump GCF records to output file (only for testing)
    {
         dumpfile = strdup( argv[++optind] );
    }
    else if (strcmp(argv[optind], "-i") == 0)          // no tcp requesting done (testing only)
    {
         TCP_REQUEST = 1;
    }
    else if (strcmp(argv[optind], "-rsize") == 0)      // size of the stream ringbuffers (small values are useful for RT systems;
    {                                                  // large values for decreasing gaps)
         RSIZE = (uint16_t) atoi(argv[++optind]);
    }
    else if (strcmp(argv[optind], "-nosend") == 0)     // do not send to seedlink (testing only)
    {
         NO_SEND = 1;
    }
    else if (strcmp(argv[optind], "-debug") == 0)     // do not send to seedlink (testing only)
    {
         DEBUG = 1;
    }
    else if (strcmp(argv[optind], "-m") == 0)         // mapfile
    {
         mapfile = (char *) malloc (140);
         strcpy ( mapfile, argv[++optind] );
    }
    else if (strcmp(argv[optind], "-tcp") == 0)
    {
         config.protocol = SCM_PROTO_TCP;
    }
  }
  if ( config.server == NULL || config.port == 0 || config.reqport == 0 ) {
       printf("No server or port(s) defined\n");
       printf("Usage:  program  -h ipaddress  -p udpport  -r tcpport -m mapfile\n");
       return(0);
  }

  if(DEBUG==1) printf("server: %s  udp port: %d  mapfile: %s   reqport: %d\n", config.server, config.port, mapfile, config.reqport);

  parse_mapfile (mapfile);

  for ( mp = rootmap; mp != NULL; mp=mp->next ) {
        printf("found mapping: %5s  %s ->  %s  %s  %s  %d\n",
        mp->sysid != NULL ? mp->sysid : "",
        mp->stream,
        mp->network,
        mp->station,
        mp->channel,
        mp->id );
  }

  fflush(stdout);

  for(i=0;i<ORBSIZE; i++) cbuf[i] = (uint8_t *) malloc(SCREAM_V40_LENGTH);

  sprintf ( scream_arg, "%s:%d", config.server, config.port);

  pthread_create ( &thread_scream, NULL, listen_to_scream, (void *) scream_arg);
  system ("sleep 1s");

  pthread_create ( &thread_distribute, NULL, distribute_forever, (void *) gcf_arg);
  system ("sleep 1s");

  //pthread_join( &thread_scream, NULL );
  //pthread_join( &thread_distribute, NULL );

  do {
    system ("sleep 1s");
  } while (1);


  printf("Should never come here ... so you are not lucky to read this !\n");

  exit(0);
  
}


void *listen_to_scream (void *in)             // thread
{
     int                     thisblocknr;
     int                     np;
     uint8_t                 *gcfbuf;

     previousblocknr = -1;

     // allocate data packet for version 4.0  (total 1077 bytes)
     // -  GCF data record is 1024 bytes
     // -  53 tailbytes

     // initialize UDP connection to receive UDP packets from SCREAM


     scream_init_socket (config.protocol, config.server, config.port);

     gcfbuf = (uint8_t *) malloc ( sizeof(uint8_t) * SCREAM_MAX_LENGTH );

     for (;;)
     {
         scream_receive (&thisblocknr, gcfbuf, sizeof(uint8_t) * SCREAM_MAX_LENGTH);
         if(DEBUG==1)printf("got record %d\n", thisblocknr);
               // now throw the data packet into the central ring buffer
               // ----- lock variables
               do {} while ( (np=pthread_mutex_trylock(&mutex1 )) != 0 );
               memcpy ( (uint8_t *) cbuf[wptr], (uint8_t *) gcfbuf, SCREAM_V40_LENGTH);
               // update pointer for writing
               wptr++;
               if(wptr>=ORBSIZE){ wptr=0; wcflag = 1;}
               pthread_mutex_unlock( &mutex1 );
               //-----  unlocked variables
               usleep(1);  // otherwise this loop will 'eat' CPU time ...
     }
     exit(0);
}



void *distribute_forever (void *in)             // thread
{
     int                     i, thisblocknr;
     int                     blocknr;
     int                     np;
     int                     iwptr, iwcflag;
     /*int                     reset; */
     /*double                  lasttime;*/
     uint8_t                 *gcfbuf;
     uint8_t                 *req_gcfbuf;
     uint8_t                 byteorder;
     //struct gcf_block_struct *gp;

     previousblocknr = -1;
     /*reset = 0;*/

     // initialize UDP connection to receive UDP packets from the ringbuffer
     // and send it out to be distributed over different ring buffers
     // this port must be the same as in thread 'release_gcf'

     gcfbuf = (uint8_t *) malloc ( sizeof(uint8_t) * SCREAM_MAX_LENGTH );

     for (;;) 
     {
            // receive data packet for version 4.0  (total 1077 bytes)
            // -  GCF data record is 1024 bytes
            // -  53 tailbytes

            memset(gcfbuf,0,sizeof(uint8_t) * SCREAM_MAX_LENGTH);

            // try to get 1077 bytes here

//======================================================

            // ----- lock variable
            do {} while ( (np=pthread_mutex_trylock(&mutex1)) != 0 );
            iwptr = wptr;
            iwcflag = wcflag;
            pthread_mutex_unlock( &mutex1 );
            // ----- unlocked variable

            while ( (optrr < iwptr && iwcflag == 0) || (optrr > iwptr && iwcflag == 1) )
            {
                   do {} while ( (np=pthread_mutex_trylock(&mutex1 )) != 0 );
                   memcpy ( (uint8_t *) gcfbuf, (uint8_t *) cbuf[optrr], SCREAM_V40_LENGTH);
                   pthread_mutex_unlock( &mutex1 );
                   optrr++;
                   if(optrr>=ORBSIZE) {optrr=0; wcflag = 0;}
                   usleep(1);  // otherwise this loops will 'eat' CPU time ...

//==================================================

            switch (gcfbuf[GCF_BLOCK_LEN])
            {
            case 31:
                  blocknr = gcfbuf[GCF_BLOCK_LEN+34]*256 + gcfbuf[GCF_BLOCK_LEN+35];
                  break;
            case 40:
                  blocknr = gcfbuf[GCF_BLOCK_LEN+2]*256 + gcfbuf[GCF_BLOCK_LEN+3];
                  break;
            case 45:
                  blocknr = gcfbuf[GCF_BLOCK_LEN+2]*256 + gcfbuf[GCF_BLOCK_LEN+3];
                  break;
            default:
                  fprintf(stderr, "Scream version ID = %d\n", gcfbuf[GCF_BLOCK_LEN]);
                  fatal (("Unknown version of scream protocol at remote end"));
                  break;
            }
            if (DEBUG==1) printf("Got UDP blocknr = %d\n", blocknr);

            thisblocknr = blocknr;

            byteorder = gcfbuf[GCF_BLOCK_LEN+1];
            if ( byteorder == 2 ) {
                    gcf_byte_swap ( gcfbuf );
            }

            // now dispatch the data packet
            gcf_dispatch ((uint8_t *) gcfbuf, SCREAM_V40_LENGTH);

            usleep(1);  // otherwise this loop will 'eat' CPU time ...

            if ( TCP_REQUEST == 0 ) {

              // now check for missing records
              if (  previousblocknr != -1 && thisblocknr - previousblocknr > 1 ) {

/*
// does not yet overcome the reset of the UDP counter
              if ( (reset == 0 && previousblocknr != -1 && thisblocknr - previousblocknr > 1) ||
                   (reset == 1  ) {
*/
                   for ( i=previousblocknr+1; i<thisblocknr; i++ ) {

                         printf("%c[%d;%dm  Block %d is missing and requested now by TCP %c[%d;%dm\n", 27,7,31, i, 27,0,30);
                         req_gcfbuf = (uint8_t *) request_block_tcp_mode ( (uint16_t) i);

                         // byte-order has been checked and applied if needed in 'request_block_tcp_mode' 

                         gcf_dispatch ((uint8_t *) req_gcfbuf, SCREAM_V40_LENGTH);
                         usleep(1);
                         if ( req_gcfbuf != NULL ) free (req_gcfbuf);

                   }
                }
              }

              /*
              // flag the resetting of the UDP counter from 65536 to 0
              if ( previousblocknr <= 65536 && thisblocknr >= 0 && thisblocknr < previousblocknr ) reset = 1;
              else                                                                                 reset = 0;
              */

              previousblocknr = thisblocknr;

              // test for TCP re-requesting
              //previousblocknr -= 5;
	      //
            }    // end of while

            usleep(1);  // otherwise this loops will 'eat' CPU time ...

     } // end of for
}

double get_dtime (void)
{
  struct timeval tp;
  struct timezone tzp;

  gettimeofday (&tp, &tzp);
  return ((double) tp.tv_sec + ((double) tp.tv_usec / 1000000.0));

}

