/*
 * seedutils.c:
 * Routines to connect to a TCP port and collect/process
 * Mini-SEED records
 *
 * Based on the LISS-utils and seedsniff 2.0 source code
 *
 * Written by Chad Trabant, ORFEUS/EC-Project MEREDIAN
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <regex.h>
#include <unistd.h>

#include "seedutils.h"
#include "plugin.h"

int lprintf(int level, char *fmt, ...);

int process_telemetry(char *host, int port, unsigned char *buf,
		      char *match, char *netcode, int rec_size)
{
  struct sockaddr_in sin;
  struct hostent *hp;
  int sock, count, optval, i;
  int bufpop;

  /*
   * Translate the hostname into an internet address
   */
  hp = gethostbyname(host);
  if(hp == 0) {
    lprintf(0, "%s: unknown host",host);
    exit(1);
  }

  /*
   * We now create a socket which will be used for our connection.
   */
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock < 0) {
    lprintf(0, "Couldn't open local socket - %s",
	    strerror(errno));
    return 1;
  }

  /*
   *  Before we attempt a connect, we fill up the "sockaddr_in" structure
   */
  memcpy((char *)&sin.sin_addr, hp->h_addr , hp->h_length);
  sin.sin_family      = AF_INET;
  sin.sin_port        = htons(port);
  /*
   *  Now attempt to connect to the remote host .....
   */
  if(connect(sock, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
    lprintf(0, "Couldn't connect to server %s - %s",host,
	    strerror(errno));
    return 2;
  }

  /* if 1+ verbose flag(s) given report the connection */
  lprintf(1, "Connected to %s:%d", host,port);

  /* Turn on keepalive on socket so we won't hang if it gets stuck */
  optval = 1;
  i = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *) &optval, sizeof(int));
  
  if ( i == -1 ) 
    lprintf(1, "Couldn't set keepalive on socket %s",
	    strerror(errno));

  bufpop = 0;

  while ( 1 ) {

    /*
     * Read from the network
     */
    count = recv(sock, &buf[bufpop], 256, 0);

    if(count < 0) {
      lprintf(0, "Read error from server - %s", strerror(errno));
      return 3;
    }
    if (count==0) {
      lprintf(1, "EOF (zero byte count) received");
      break;
    }
    
    bufpop += count;

    while (bufpop>=rec_size) {

      process_record((char *) buf, match, netcode, rec_size);
      bufpop -= rec_size;
      if (bufpop>0)
	memcpy(buf,&buf[rec_size],bufpop);
    }

    /*
     * Continue the loop
     */
  }

  close(sock);

  return(0);
}

void process_record(char *inseed, char *match,
		    char *netcode, int rec_size)
{
  /* Define some structures to pass to parse_record() */
  static struct s_dev_id *dev_id = NULL;
  static struct s_blk_1000 *blk_1000 = NULL;

  char type;
  char prtnet[4], prtsta[7];
  char prtloc[4], prtchan[5];
  char *seedsta, *spcptr, *encoding;
  char order[4], sourcename[50];
  int  parsed = 0;
  int  seedreclen;
  int  i,p;

  /* Simple verification of a data record */
  type = inseed[6];
  if ( type != 'D' && type != 'R' && type != 'Q' ) {
    lprintf(1, "Record header/quality indicator unrecognized!");
    return;
  }

  /* Make sure there is room for these structs */
  dev_id = (struct s_dev_id *) realloc(dev_id, sizeof(struct s_dev_id));
  blk_1000  = (struct s_blk_1000 *) realloc(blk_1000, sizeof(struct s_blk_1000));
  
  parsed = parse_record( (struct s_dev_id *) dev_id,
			 (struct s_blk_1000 *) blk_1000,
			 (char *) inseed, rec_size);

  if ( !parsed ) {
    lprintf(1, "1000 blockette was NOT found!");
    return;
  }

  /* Most of this monkey business is so the data stream naming convention
     will be consistent even with oddly filled fields */
  strncpy(prtnet, dev_id->network_code, 2); prtnet[2] = '\0';
  strncpy(prtsta, dev_id->station_code, 5); prtsta[5] = '\0';
  strncpy(prtloc, dev_id->location_id, 2); prtloc[2] = '\0';
  strncpy(prtchan, dev_id->channel_id, 3); prtchan[3] = '\0';

  /* Cut trailing spaces. Assumes left justified fields */
  if ( (spcptr = strstr(prtnet, " ")) != NULL ) *spcptr = '\0';
  if ( (spcptr = strstr(prtsta, " ")) != NULL ) *spcptr = '\0';
  if ( (spcptr = strstr(prtloc, " ")) != NULL ) *spcptr = '\0';
  if ( (spcptr = strstr(prtchan, " ")) != NULL ) *spcptr = '\0';

  seedsta = strdup(prtsta);

  if (prtnet[0] != '\0') strcat(prtnet, "_");
  if (prtsta[0] != '\0') strcat(prtsta, "_");
  if (prtloc[0] != '\0') strcat(prtloc, "_");

  /* Build the source name string */
  sprintf( sourcename, "%.3s%.6s%.3s%.3s",
	   prtnet, prtsta, prtloc, prtchan);

  /* Calculate record size in bytes as 2^(blk_1000->rec_len) */
  for (p=1, i=1; i <= blk_1000->rec_len; i++) p *= 2;
  seedreclen = p;

  if (seedreclen != rec_size) {
    lprintf(1, "Record was not expected size: %d, dropping", rec_size);
    p = 0;   /* temporarily used, notifies the if/else below */
  }

  /* Big or little endian reported by the 1000 blockette? */
  if (blk_1000->word_swap == 0) strcpy(order, "LE");
  else if (blk_1000->word_swap == 1) strcpy(order, "BE");
  else strcpy(order, "??");

  /* Get a description of the encoding format */
  encoding = (char *) encoding_hash(blk_1000->encoding);

  /* Force network code if supplied */
  if ( netcode ) {
    strncpy( inseed+18, netcode, 2);
  }
  
  if ( matches(sourcename, match) && p ) {  /* send it off to the daemon */
    if ( send_mseed(seedsta, (void *) inseed, rec_size) < 0 ) {
      lprintf(0, "Error sending data to seedlink: %s", strerror(errno));
      exit(1);
    }
    if ( netcode ) {
      lprintf(2, "%s (Forced net: '%s'): %s, %d bytes, %s",
	      sourcename, netcode, order, seedreclen, encoding);
    }
    else {
      lprintf(2, "%s: %s, %d bytes, %s",
	      sourcename, order, seedreclen, encoding);
    }
  }
  else {
    lprintf(2, "DROPPED %s: %s, %d bytes, %s",
	    sourcename, order, seedreclen, encoding);
  }

  free (seedsta);

  return;
}

/* Fill the dev_id and blk_1000 structures from the given record (lptr)
   Returns 1 if 1000 blockette was found, 0 if not
*/
int parse_record( t_dev_id *dev_id,
		  t_blk_1000 *blk_1000,
		  char *lptr, int rec_size)
{
  
  static struct s_fsdh_data *fsdh_data = NULL;
  static struct s_blk_head *blk_head = NULL;

  int found_1000b = 0;            /* found the 1000 blockette? */
  char swap_flag = 0;             /* is swapping needed? */
  unsigned short begin_blockette; /* byte offset for next blockette */

  /* Make sure there is room for these structs */
  fsdh_data = (struct s_fsdh_data *) realloc(fsdh_data, sizeof(struct s_fsdh_data));
  blk_head  = (struct s_blk_head *) realloc(blk_head, sizeof(struct s_blk_head));

  /* copy fixed section data header data into memory */
  memcpy((void *)dev_id,lptr+8,sizeof(struct s_dev_id));
  memcpy((void *)fsdh_data,lptr+20,sizeof(struct s_fsdh_data));

  /* check to see if word swapping is needed (bogus year makes good test) */
  if ( (fsdh_data->start_time.year < 1960) || 
       (fsdh_data->start_time.year > 2050) )
    swap_flag = 1;

  /* change word order?  Most are unused, but leave it for now */
  swap_2bytes(&fsdh_data->start_time.year,swap_flag);
  swap_2bytes(&fsdh_data->start_time.day,swap_flag);
  swap_2bytes(&fsdh_data->start_time.fracts,swap_flag);
  swap_2bytes(&fsdh_data->num_samples,swap_flag);
  swap_2bytes((uint16_t *)&fsdh_data->sample_rate,swap_flag);
  swap_2bytes((uint16_t *)&fsdh_data->multiplier,swap_flag);
  swap_4bytes((uint32_t *)&fsdh_data->time_correct,swap_flag);
  swap_2bytes(&fsdh_data->begin_data,swap_flag);
  swap_2bytes(&fsdh_data->begin_blockette,swap_flag);

  begin_blockette = fsdh_data->begin_blockette;

  /* loop through blockettes as long as number is non-zero and viable */
  while ((begin_blockette != 0) &&
	 (begin_blockette <= (uint16_t) rec_size)) {

    memcpy((void *)blk_head,lptr+begin_blockette,sizeof(struct s_blk_head));
    swap_2bytes(&blk_head->blk_type,swap_flag);
    swap_2bytes(&blk_head->next_blk,swap_flag);

    if (blk_head->blk_type == 1000) { /* found the 1000 blockette */

      memcpy((void *)blk_1000,lptr+begin_blockette,sizeof(struct s_blk_1000));
      found_1000b = 1;
    }

    if (begin_blockette != 0)  /* check not useful now, but for the future */ 
      begin_blockette = blk_head->next_blk;
  }

  return found_1000b;
}

void swap_2bytes (uint16_t *a, char f)
{
  *a = ntohs(*a);

#if 0
  union {
    uint16_t i;
    char b[2];
  } word;
  char temp;

  if (f == 1){  /* f is the flag to trigger byte swapping */
  word.i = *a;
  temp = word.b[0];
  word.b[0] = word.b[1];
  word.b[1] = temp;
  memcpy((void *)a,(void *)&(word.i),sizeof(short));
  }
#endif
}

void swap_4bytes (uint32_t *a, char f)
{
  *a = ntohl(*a);

#if 0
  union {
    uint32_t l;
    char b[4];
  } word;
  char temp;

  if (f == 1){  /* f is the flag to trigger byte swapping */
  word.l = *a;
  temp = word.b[0];
  word.b[0] = word.b[3];
  word.b[3] = temp;
  temp = word.b[1];
  word.b[1] = word.b[2];
  word.b[2] = temp;
  memcpy((void *)a,(void *)&(word.l),sizeof(long));
  }
#endif
}

int matches( char *source, char *match)
{

    int retcode = 0;

    if ( match != '\0' ) {
        static regex_t *pgm = 0;
        if ( pgm == 0 ) {
	  pgm = (regex_t *) malloc(sizeof(*pgm));
            if ( regcomp(pgm, match, REG_EXTENDED ) != 0) {
                lprintf(0, "can't compile regular expression '%s'", match );
            }
        }
        if ( regexec ( pgm, source, 0, 0, 0) == 0 ) {
            retcode = 1;
        }
    } else {
        retcode = 1;
    }
    return retcode;
}

char *encoding_hash(char enc)
{

  static char encstr[100];
  char tmpstr[50];

  switch(enc) {
   case 0:
      strcpy(encstr,"ASCII text (val:0)");
      break;
    case 1:
      strcpy(encstr,"16 bit integers (val:1)");
      break;
    case 2:
      strcpy(encstr,"24 bit integers (val:2)");
      break;
    case 3:
      strcpy(encstr,"32 bit integers (val:3)");
      break;
    case 4:
      strcpy(encstr,"IEEE floating point (val:4)");
      break;
    case 5:
      strcpy(encstr,"IEEE double precision float (val:5)");
      break;
    case 10:
      strcpy(encstr,"STEIM 1 Compression (val:10)");
      break;
    case 11:
      strcpy(encstr,"STEIM 2 Compression (val:11)");
      break;
    case 12:
      strcpy(encstr,"GEOSCOPE Muxed 24 bit int (val:12)");
      break;
    case 13:
      strcpy(encstr,"GEOSCOPE Muxed 16/3 bit gain/exp (val:13)");
      break;
    case 14:
      strcpy(encstr,"GEOSCOPE Muxed 16/4 bit gain/exp (val:14)");
      break;
    case 15:
      strcpy(encstr,"US National Network compression (val:15)");
      break;
    case 16:
      strcpy(encstr,"CDSN 16 bit gain ranged (val:16)");
      break;
    case 17:
      strcpy(encstr,"Graefenberg 16 bit gain ranged (val:17)");
      break;
    case 18:
      strcpy(encstr,"IPG - Strasbourg 16 bit gain (val:18)");
      break;
    case 19:
      strcpy(encstr,"STEIM 3 Compression (val:19)");
      break;
    case 30:
      strcpy(encstr,"SRO Format (val:30)");
      break;
    case 31:
      strcpy(encstr,"HGLP Format (val:31)");
      break;
    case 32:
      strcpy(encstr,"DWWSSN Gain Ranged Format (val:32)");
      break;
    case 33:
      strcpy(encstr,"RSTN 16 bit gain ranged (val:33)");
      break;
    default:
      sprintf(tmpstr, "Unknown format code: (%d)", enc);
      strcpy(encstr, tmpstr);
  }  /* end switch */

  return &encstr[0];
}
