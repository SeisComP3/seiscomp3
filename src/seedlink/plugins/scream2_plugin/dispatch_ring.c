
/* Author:     Reinoud Sleeman (ORFEUS)
 *
 * Nov 2010
 *
 *
 * based on original software by Guralp Systems Limited  
 *
 *
 */

#include "project.h"
#include "plugin.h"
#include "map.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "project.h"
#include <sys/socket.h>
#include <netdb.h>

#define MAXSTREAMS            	500        // max number of streams accepted from SCREAM

#define G_OK          0
#define G_TOO_LATE    1
#define G_GAP         2
#define G_DUPLICATE   3

typedef struct {
int idum;
        time_t       		epochstart;
        time_t       		epochend;
        char         		*stat;
        char         		*chan;
        char         		*netw;
        int          		flag;          // 0 = gcf block follows previous block; 1 = gcf block follows a gap 
        int          		recno;
        int                     samples;
        int                     sample_rate;
        int                     ttl;
        int                     data[2048];
} GCF_STACK;

GCF_STACK  		        **stackpp[MAXSTREAMS];

extern int                      NO_SEND;
extern int                      DEBUG;
extern Map 			*rootmap;
extern struct config_struct 	config;
static int        		s_index[MAXSTREAMS];
static long int 		time_of_next_expected_sample[MAXSTREAMS];
static int                      numstreams;

extern int RSIZE;
int        RINGBUFFER_LENGTH;  	// container for GCF blocks; the larger this number the larger
                                // the delay can be for sending out data 

extern double get_dtime (void);

time_t epochtime (int year, int mon, int day, int hour, int min, int sec);

int debug;

void dispatch (struct gcf_block_struct *b, int recno)
{
  int 	chid;
  int 	b_status;
  Map 	*mp;
  char 	buffer[70];
  char 	buffer2[100];
  //char	*timestring;

  debug = DEBUG;

  // We don't handle status information
  if (!b->sample_rate) {
    //printf("no valid sample_rate %d\n",  b->sample_rate);
    return;
  }

  if ( RSIZE == -1 )
       RINGBUFFER_LENGTH = 1000;
  else
       RINGBUFFER_LENGTH = RSIZE;

//printf("RINGBUFFER_LENGTH = %d\n", RINGBUFFER_LENGTH);


  //------ Check if the incoming stream is defined in the map file

  chid = -1;
  for ( mp = rootmap; mp != NULL; mp=mp->next ) {

        // hope this works well for the different configuartiosn ...

        if (  mp->sysid == NULL ) {
              if (strncmp (b->strid, mp->stream, strlen(mp->stream)) == 0 ) {
                   chid=mp->id;
                   break;
              }
        }
        else {
              if (strncmp (b->sysid, mp->sysid, strlen(mp->sysid)) == 0 && strncmp (b->strid, mp->stream, strlen(mp->stream)) == 0 ) {
                   chid=mp->id;
                   break;
              }
        }
  }

   if ( chid < 0 || chid >= MAXSTREAMS )
   {
        //printf("Unable to handle stream %s\n", b->strid);
        if ( chid >= MAXSTREAMS ) {printf("INCREAS MAXSTREAMS in dispatch-ring.c\n"); exit(0);}
	return;
   }



   if ( time_of_next_expected_sample[chid] < 0 ) {
        printf("This should never happen ...\n");
        return;
   }


/* useful for debugging
*/

   if ( debug == 1 ) {
        strftime ( buffer,50,"%d-%B-%Y  (%j) %T  %Z ",localtime(&b->estart));
        printf("----------------------------------------------------------------------------------------\n");
        printf("RECEIVED record:  %s %s [%s] [%s]   %s  ns=%d  sr=%d recno=%d\n", 
                 b->strid, b->sysid,  mp->station, mp->channel, buffer, b->samples, b->sample_rate, recno);
        printf("----------------------------------------------------------------------------------------\n");
   }

   if ( time_of_next_expected_sample[chid] == 0 ) {

        // initial data received at start-up for this channel ID: initialize the stack
        // older datat than this record is not handled, therefore this initial
        // data is send directly to seedlink and is not stored in stack

        numstreams++;

        init_stackp (chid);

        s_index[chid] = 0;
        time_of_next_expected_sample[chid] = 0;

        strftime ( buffer,50,"%d-%B-%Y  (%j) %T  %Z ",localtime(&b->estart));

        printf("Got INITIAL data for [%s]-[%s] - send now  %s  %d samples  sr=%d  recno=%d    Got %d streams now\n", 
                mp->station, mp->channel, buffer, b->samples, b->sample_rate, recno, numstreams);

        if ( NO_SEND == 0 )
             send_raw_depoch ( mp->station, mp->channel, b->estart, 0, -1, b->data, b->samples);

        time_of_next_expected_sample[chid] = b->estart + (b->samples)/b->sample_rate;

        return;
   }

   b_status = -1;

   if      ( time_of_next_expected_sample[chid] > 0 && (b->estart > time_of_next_expected_sample[chid] )  ) 
             b_status = G_GAP; 
   else if ( time_of_next_expected_sample[chid] > 0 && ((b->estart + (b->samples)/b->sample_rate) == time_of_next_expected_sample[chid]) ) 
             b_status = G_DUPLICATE; 
   else if ( time_of_next_expected_sample[chid] > 0 && (b->estart < time_of_next_expected_sample[chid]) ) 
             b_status = G_TOO_LATE; 
   else if ( time_of_next_expected_sample[chid] > 0 && (b->estart == time_of_next_expected_sample[chid]) ) 
             b_status = G_OK; 
   else
             b_status = -1;

   //endt = b->estart + (b->samples)/b->sample_rate;
   //strftime(timestr1,50,"%T",localtime(&b->estart));
   //strftime(timestr2,50,"%T",localtime(&endt));
   //strftime(timestr3,50,"%T",localtime(&time_of_next_expected_sample[chid]));


   if ( b_status == G_GAP ) {

        // data gap detected; this data record is stored in the stack now as to allow
        // delayed data to flow in

        if ( debug == 1 ) {
             strftime(buffer2,50,"%T",localtime(&b->estart));
             printf("GAP detected of %ld seconds for %s  -> add to stack %s\n", b->estart - time_of_next_expected_sample[chid], b->strid, buffer2);
        }
        add_to_stack ( b, chid, mp, recno, 1);

   }

   else if ( b_status == G_DUPLICATE ) {

        if ( debug == 1 ) {
             strftime(buffer2,50,"%T",localtime(&b->estart));
             printf("Duplicate DATA arrived for [%s]-[%s] - skipped  (%s)  recno=%d\n", mp->station, mp->channel, buffer2, recno);
        }
   }

   else if ( b_status == G_TOO_LATE ) {

        // delayed data arrived too late ....
        if ( debug == 1 ) {
             strftime(buffer2,50,"%T",localtime(&b->estart));
             printf("Earlier DATA arrived too late for [%s]-[%s] - skipped  (%s)\n", mp->station, mp->channel, buffer2);
        }
   }

   else if ( b_status == G_OK ) {

        // data in time order received
        if ( debug == 1 ) {
             strftime(buffer,50,"%d-%B-%Y  (%j) %T  %Z ",localtime(&b->estart));
             printf("DATA IN TIME ORDER for [%s]-[%s] - send now ()  %ld     %s recno=%d  ttl=%d\n", 
                mp->station, mp->channel, b->estart, buffer, recno, b->ttl);
        }


        add_to_stack ( b, chid, mp, recno, 1);

        return;
   }
   else {
        printf(".... no status ?? \n");
        return;
   }

}

init_stackp (int chid)
{
    int i;

    stackpp[chid] = (GCF_STACK **) calloc ( 1, RINGBUFFER_LENGTH * sizeof(GCF_STACK *));

    for(i=0; i < RINGBUFFER_LENGTH; i++) {

        stackpp[chid][i] = (GCF_STACK *) calloc ( 1, sizeof(GCF_STACK));
        stackpp[chid][i]->epochstart =  epochtime ( 2030, 1, 1, 0, 0, 0);
        stackpp[chid][i]->epochend   =  0;
        stackpp[chid][i]->flag       =  0;
        stackpp[chid][i]->idum       =  i;
        stackpp[chid][i]->stat       =  (char *) malloc ( 8);
        stackpp[chid][i]->chan       =  (char *) malloc ( 8);
        stackpp[chid][i]->netw       =  (char *) malloc ( 8);
    }

    s_index[chid] = 0;

    time_of_next_expected_sample[chid] = 0;
}



fifo_ring (int chid)
{
    int i;
    char buffer[100];

    // update time of last sample here to enable the release of other blocks in the stack
    // then send the oldest block to seedlink

   time_of_next_expected_sample[chid] = stackpp[chid][0]->epochend;

   if ( debug == 1 ) {
        strftime(buffer,50,"%d-%B-%Y  (%j) %T  %Z ",localtime(&stackpp[chid][0]->epochstart));
        printf("in fifo[1] Release NOW from stack this record:  [%s]-[%s]  %s      ns=%d  ttl=%d\n", 
            stackpp[chid][0]->stat, stackpp[chid][0]->chan, buffer, stackpp[chid][0]->samples, stackpp[chid][0]->ttl);
   }

   if ( NO_SEND == 0 )
        send_raw_depoch ( stackpp[chid][0]->stat, stackpp[chid][0]->chan, stackpp[chid][0]->epochstart, 0, 100, 
                     stackpp[chid][0]->data, stackpp[chid][0]->samples);

   for(i=0; i <= RINGBUFFER_LENGTH-2; i++) {
            memcpy ( (GCF_STACK *)stackpp[chid][i], (GCF_STACK *)stackpp[chid][i+1], sizeof(GCF_STACK ));
   }

   stackpp[chid][RINGBUFFER_LENGTH-1]->samples=0;
   stackpp[chid][RINGBUFFER_LENGTH-1]->epochstart =  epochtime ( 2030, 1, 1, 0, 0, 0);
   stackpp[chid][RINGBUFFER_LENGTH-1]->epochend =  0;

   s_index[chid] = s_index[chid] -1;
}

add_to_stack (struct gcf_block_struct *b, int chid, Map *mp, int recno, int flag)
{
   int index, i, j, cnt;
   char tbuf[100], tbuf2[100];

   //list_stack (0,chid);

   s_index[chid]= s_index[chid] + 1;                  // number of entries in the stack
   index = s_index[chid] - 1;        // index starts at '0'


   if ( s_index[chid] >= RINGBUFFER_LENGTH ) {

        printf("STACK REACHED LIMIT for [%s]-[%s] - send and release oldest data now\n", mp->station, mp->channel);

        sort_stack (chid);
        set_stack_flag (chid);

        fifo_ring ( chid );
   }

   index = s_index[chid] - 1;        // index starts at '0'

   stackpp[chid][index]->epochstart = b->estart;
   stackpp[chid][index]->epochend   = b->estart + (b->samples)/b->sample_rate;
   stackpp[chid][index]->sample_rate= b->sample_rate;
   stackpp[chid][index]->samples    = b->samples;
   stackpp[chid][index]->ttl        = b->ttl;
   stackpp[chid][index]->flag       = flag;
   stackpp[chid][index]->recno      = recno;
   memcpy ( &stackpp[chid][index]->data[0], &b->data[0], sizeof(int)*2048);

   sprintf (stackpp[chid][index]->stat,"%s", mp->station);
   sprintf (stackpp[chid][index]->chan,"%s", mp->channel);


   //list_stack (1,chid);

   sort_stack (chid);
   set_stack_flag (chid);

   //list_stack (2, chid);

   i=0;
   while (  i< s_index[chid] && stackpp[chid][i]->flag == 0 ) {

         strftime(tbuf,50,"%d-%m-%Y  (%j) %T",localtime(&stackpp[chid][i]->epochstart));
         strftime(tbuf2,50," %T",localtime(&stackpp[chid][i]->epochend));

/*
        printf("in add to stack[4] release stack block  %s  %s  %s %s  %d samples   \n", 
              stackpp[chid][i]->stat, stackpp[chid][i]->chan, tbuf, tbuf2,stackpp[chid][i]->gcfb->samples  );
*/
        i++;
   }

   for (j=0; j<i; j++) fifo_ring (chid);

   //list_stack (3, chid);

}

sort_on_begtime ( el1, el2 )
GCF_STACK **el1, **el2;
//GCF_STACK *el1, *el2;
{
/*
printf("in sort time ============================================  %ld  %ld  %d  %d   + %d %d\n", 
              el1->epochstart, el2->epochstart, el1->recno, el2->recno, el1->idum, el2->idum);
*/
     //if ( el1->idum <  el2->idum ) return (-1);
     //if ( el1->idum == el2->idum ) return (0);
     //if ( el1->idum >  el2->idum ) return (1);
     if ( (*el1)->epochstart <  (*el2)->epochstart ) return (-1);
     if ( (*el1)->epochstart == (*el2)->epochstart ) return (0);
     if ( (*el1)->epochstart >  (*el2)->epochstart ) return (1);
     else return(0);
}

printf_gcf (GCF_STACK *el)
{
printf("recno = %d  epoch = %ld  sta=%s  cha=%s  flag=%d  ns=%d  sr=%d idu=%d\n", el->recno, el->epochstart, el->stat, el->chan, el->flag, el->samples, el->sample_rate, el->idum);
}

sort_stack (chid)
int chid;
{
    // sort stack based on start-time
    int ns;

    ns = s_index[chid];

    //for(i=0;i<ns;i++) printf_gcf (stackpp[chid][i]);
    qsort ((GCF_STACK **) stackpp[chid], ns, sizeof(GCF_STACK **), sort_on_begtime);
    //for(i=0;i<ns;i++) printf_gcf (stackpp[chid][i]);
}

print_stack (int chid,  int index)
{
    char buffer[70];
    char buffer2[70];

    strftime(buffer,50,"%d-%m-%Y  (%j) %T",localtime(&time_of_next_expected_sample[chid]));

    strftime(buffer,50,"%d-%m-%Y  (%j) %T",localtime(&stackpp[chid][index]->epochstart));
    strftime(buffer2,50,"%T",localtime(&stackpp[chid][index]->epochend));

    printf("stack[%d] = %ld  %s %s ns=%d\n",  index, stackpp[chid][index]->epochstart, buffer, buffer2, stackpp[chid][index]->samples );

}

list_stack (int id,  int chid)
{
    int i;
    char buffer[70];
    char buffer2[70];

    //printf("list stack------------- chid=%d  s_index = %d --------\n", chid, s_index[chid]);
    strftime(buffer,50,"%d-%m-%Y  (%j) %T",localtime(&time_of_next_expected_sample[chid]));
    //printf("last send sample time  %s\n", buffer);

 
    for (i=0; i<s_index[chid]; i++) {

         strftime(buffer,50,"%d-%m-%Y  (%j) %T",localtime(&stackpp[chid][i]->epochstart));
         strftime(buffer2,50,"%T",localtime(&stackpp[chid][i]->epochend));

         printf("[id=%d] stack[%d] = %ld  %s %s ns=%d\n",  id, i, stackpp[chid][i]->epochstart, buffer, buffer2, stackpp[chid][i]->samples );
/*

         if ( stack[chid][i].flag == 1 )  // this block follows after a gap
          printf("Stack %d  %d begtime %s  end: %s s_index: %d %c[%d;%dm %d %c[%d;%dm   %d     sps=%d   ns=%d         begtime: %ld  diff: %ld\n", 
                  chid, i, buffer, buffer2, s_index[chid], 27,7,31, stack[chid][i].flag, 27,0,30,  stack[chid][i].recno,
                  stack[chid][i].gcfb->sample_rate, stack[chid][i].gcfb->samples, stack[chid][i].epochstart,
                  stack[chid][i].epochstart - time_of_next_expected_sample[chid]);
         else if ( stack[chid][i].flag == 0 )  // ok
          printf("Stack %d  %d begtime %s  end: %s s_index: %d %c[%d;%dm %d %c[%d;%dm   %d     sps=%d   ns=%d         begtime: %ld  diff: %ld\n", 
                  chid, i, buffer, buffer2, s_index[chid],  27,7,32, stack[chid][i].flag, 27,0,30, stack[chid][i].recno,
                  stack[chid][i].gcfb->sample_rate, stack[chid][i].gcfb->samples, stack[chid][i].epochstart,
                  stack[chid][i].epochstart - time_of_next_expected_sample[chid]);
         else // should never come here
          printf("Stack %d  %d begtime %s  end: %s s_index: %d  %d    %d     sps=%d   ns=%d       begtime: %ld  diff: %ld\n", 
                  chid, i, buffer, buffer2, s_index[chid],   stack[chid][i].flag,  stack[chid][i].recno, stack[chid][i].epochstart,
                  stack[chid][i].gcfb->sample_rate, stack[chid][i].gcfb->samples, stack[chid][i].epochstart);
*/

    }
}

/*
check_stack (int chid )
{
    int i, cnt=0;

    if ( stack[chid][0].flag = 1 ) {
              printf("WAIT for more data to fill the gap for %s %s\n", stack[chid][i].stat, stack[chid][i].chan);
              return(0);
    }
    else {

        for (i=0; i<s_index[chid]; i++) {

             if ( stack[chid][0].flag == 0 ) {

                   printf("QUEUE IN ORDER for [%s]-[%s] - send now     sr=%d\n",  stack[chid][i].stat, stack[chid][i].chan, stack[chid][i].gcfb->sample_rate);

                   time_of_next_expected_sample[chid] = stack[chid][i].epochend;
                   cnt++;

            }
        }

        for (i=0; i<cnt; i++) fifo_ring (chid);
        
    }

    sort_stack (chid);
}
*/

set_stack_flag (int chid)
{

    int i;
    //char buffer[70];
    //char buffer2[70];

/*
strftime(buffer,50,"%d-%m-%Y  (%j) %T",localtime(&time_of_next_expected_sample[chid]));
printf("set_stack_flag: last send sample time  %s\n", buffer);
strftime(buffer,50,"%d-%m-%Y  (%j) %T",localtime(&stack[chid][0].epochstart));
printf("set_stack_flag: stack0 last first sample time  %s\n", buffer);
*/


    // update 'gap' flag
    if ( stackpp[chid][0]->epochstart > time_of_next_expected_sample[chid]  )
                 stackpp[chid][0]->flag = 1;
    else
                 stackpp[chid][0]->flag = 0;

    for (i=0; i<s_index[chid]-1; i++) {
            if ( stackpp[chid][i+1]->epochstart > stackpp[chid][i]->epochend  ) {
                 stackpp[chid][i+1]->flag = 1;
            }
            else
                 stackpp[chid][i+1]->flag = 0;
    }



}


time_t epochtime (int year, int mon, int day, int hour, int min, int sec)
{
        struct tm *timestruct = (struct tm *)malloc(sizeof(struct tm));

        timestruct->tm_year = year-1900;
        timestruct->tm_mon = mon-1;
        timestruct->tm_mday = day;
        timestruct->tm_hour = hour;
        timestruct->tm_min = min;
        timestruct->tm_sec = sec;
        timestruct->tm_isdst = 0;

        setenv ("TZ", "UTC", 0);
        tzset ();
        return ( mktime(timestruct));

}



uint8_t *request_block_tcp_mode ( uint16_t recnr)
{
      int sockid, byteorder;
      int blocknr, n;
      //struct sockaddr_in sin;
      struct sockaddr_in local, remote;
      struct hostent *he;
      uint8_t *buf;
      uint16_t hrecnr;
      char server[100];
      unsigned char uc[3], *up;


      // command: 0xFF (to request block by 16-bit sequence number)
      // followed by sequence number in network byte order (big endian)

      //printf("request for recnr:  %d  %x\n", recnr, recnr);
      uc[0]  = (uint8_t) SCREAM_CMD_RESEND;
      hrecnr = htons(recnr);
      up     = &hrecnr;
      uc[1]  = *up;
      uc[2]  = *(up+1);


      sockid = socket (PF_INET, SOCK_STREAM, 0);
      local.sin_family = AF_INET;
      local.sin_port = htons(0);
      local.sin_addr.s_addr = htonl(INADDR_ANY);

      if (bind (sockid, (struct sockaddr *) &local, sizeof (local)))
          fatal (("bind failed "));
      remote.sin_port =  htons((uint16_t) config.reqport);         // port for gap retrieval using TCP;
      remote.sin_family = AF_INET;
      strcpy ( server, config.server);
      he = gethostbyname (server);
      if (!he) fatal (("gethostbyname(%s) failed: %m", server));
      if (he->h_addrtype != AF_INET)
          fatal (("gethostbyname returned a non-IP address"));
      memcpy (&remote.sin_addr.s_addr, he->h_addr, he->h_length);

      if (connect (sockid, (struct sockaddr *) &remote, sizeof (remote))) {
          printf("connect failed\n");
      }
      else
      {
        if (send (sockid, &uc[0], 3,  MSG_CONFIRM) != 3) {
            printf("Send TCP failed\n");
            exit(0);
        }

        buf = (uint8_t *) malloc ( sizeof(uint8_t) * SCREAM_MAX_LENGTH );

        n = complete_read (sockid, (uint8_t *) buf, SCREAM_V40_LENGTH);

        if (n!=SCREAM_V40_LENGTH) printf("read complete failed.....n=%d\n", n);

        //if(n==4) printf("block no longer available %d\n", (uint16_t) recnr); 
        
        byteorder = buf[GCF_BLOCK_LEN+1];

        //blocknr = buf[GCF_BLOCK_LEN+2]*256 + buf[GCF_BLOCK_LEN+3];
        //printf("got block %d  over TCP \n", (blocknr));

        if ( byteorder == 2 ) {
             //printf("TCP swap buffer\n"); 
             gcf_byte_swap ( buf );
        }

        blocknr = buf[GCF_BLOCK_LEN+2]*256 + buf[GCF_BLOCK_LEN+3];

        //printf("got block %d  over TCP \n", (blocknr));

      }
      close(sockid);

      return ( (uint8_t *) buf );
                          

}

