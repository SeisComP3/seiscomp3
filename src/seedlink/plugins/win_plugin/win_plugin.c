/* $Id: win_plugin.c 1904 2009-09-30 14:08:31Z andres $ */
/* "recvt.c"      4/10/93 - 6/2/93,7/2/93,1/25/94    urabe */
/*                2/3/93,5/25/94,6/16/94 */
/*                1/6/95 bug in adj_time fixed (tm[0]--) */
/*                2/17/95 "illegal time" message stopped */
/*                3/15-16/95 write_log changed */
/*                3/26/95 check_packet_no; port# */
/*                10/16/95 added processing of "host table full" */
/*                5/22/96 support merged packet with ID="0x0A" */
/*                5/22/96 widen time window to +/-30 min */
/*                5/28/96 bcopy -> memcpy */
/*                6/6/96,7/9/96 discard duplicated resent packets & fix bug */ 
/*                12/29/96 "next" */
/*                97.6.23 RCVBUF->65535 */
/*                97.9.4        & 50000 */
/*                97.12.18 channel selection by file */
/*                97.12.23 ch no. in "illegal time" & LITTLE_ENDIAN */
/*                98.4.23 b2d[] etc. */
/*                98.6.26 yo2000 */
/*                99.2.4  moved signal(HUP) to read_chfile() by urabe */
/*                99.4.19 byte-order-free */
/*                2000.3.13 >=A1 format */
/*                2000.4.24 added SS & SR check, check_time +/-60m */
/*                2000.4.24 strerror() */
/*                2000.4.25 added check_time() in >=A1 format */
/*                2000.4.26 host control, statistics, -a, -m, -p options */
/*                2001.2.20 wincpy() improved */
/*                2001.3.9 debugged for sh->r */
/*                2001.11.14 strerror(),ntohs() */
/*                2002.1.7 implemented multicasting (options -g, -i) */
/*                2002.1.7 option -n to suppress info on abnormal packets */
/*                2002.1.8 MAXMESG increased to 32768 bytes */
/*                2002.1.12 trivial fixes on 'usage' */
/*                2002.1.15 option '-M' necessary for receiving mon data */
/*                2002.3.2 wincpy2() discard duplicated data (TS+CH) */
/*                2002.3.2 option -B ; write blksize at EOB */
/*                2002.3.18 host control debugged */
/*                2002.5.3 N_PACKET 64->128, 'no request resend' log */
/*                2002.5.3 maximize RCVBUF size */
/*                2002.5.3,7 maximize RCVBUF size ( option '-s' )*/
/*                2002.5.14 -n debugged / -d to set ddd length */
/*                2002.5.23 stronger to destructed packet */
/*                2002.11.29 corrected byte-order of port no. in log */
/*                2002.12.21 disable resend request if -r */
/*                2003.3.24-25 -N (no pno) and -A (no TS) options */
/*                2004.8.9 fixed bug introduced in 2002.5.23 */
/*                2004.10.26 daemon mode (Uehira) */
/*                2004.11.15 corrected byte-order of port no. in log */
/*                2005.2.17 option -o [source host]:[port] for send request */
/*                2005.2.20 option -f [ch_file] for additional ch files */
/*                2005.6.24 don't change optarg's content (-o) */
/*                2005.7.18 converted to win_plugin -AH */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>

#include <time.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>

#include "plugin.h"

#define MYVERSION "1.0 (2010.256)"

#ifndef SYSLOG_FACILITY
#define SYSLOG_FACILITY LOG_LOCAL0
#endif

#ifndef CHANNEL_MAP_FILE
#define CHANNEL_MAP_FILE "/home/sysop/config/win2sl.map"
#endif

#define LOGMSGLEN  150
#define NQUEUE     65536
#define QUEUE_LEN  16
#define QUEUE_MASK 15

#define DEBUG     0
#define DEBUG0    0
#define DEBUG1    0
#define DEBUG2    0
#define DEBUG3    0     /* -o */
#define BELL      0
#define MAXMESG   32768
#define N_PACKET  128   /* N of old packets to be requested */  
#define N_HOST    100   /* max N of hosts */  
#define N_HIST    10    /*  length of packet history */
#define N_CHFILE  30    /*  length of packet history */

static const char *const ident_str = "SeedLink WIN Plugin v" MYVERSION;
static const char *const opterr_message = "Try `%s -h' for more information\n";
static const char *const help_message =
    "Usage: %s [options] UDP_port ctl_file plugin_name\n"
    "\n"
    "'plugin_name' is used as a signature in log messages\n"
    "\n"
    "-A  Don't check time stamps\n"
    "-a  Accept >=A1 packets\n"
    "-D  Daemon mode\n"
    "-d  Length of packet history in sec\n"
    "-i  Multicast interface (IP address)\n"
    "-f  Channel list file\n"
    "-F  SeedLink channel map file\n"
    "-g  Multicast group (IP address)\n"
    "-h  Show this help message\n"
    "-m  Time limit before RT in minutes\n"
    "-M  Mon-format data\n"
    "-N  No packet numbers\n"
    "-n  Suppress info on abnormal packets\n"
    "-o  Host and port for request\n"
    "-p  Time limit after RT in minutes\n"
    "-r  Disable resend request\n"
    "-s  Preferred socket buffer size in KB\n"
    "-v  Increase verbosity level\n"
    "-V  Show version information\n";

static unsigned char rbuf[MAXMESG],ch_table[65536];
static char *progname,chfile[N_CHFILE][256];
static int n_ch,negate_channel,hostlist[N_HOST][2],n_host,no_pinfo,n_chfile;
static const char *plugin_name = NULL;
static int daemon_mode = 0, daemon_init = 0;
static int verbosity = 0;

struct {
    int host;
    int port;
    int no;
    unsigned char nos[256/8];
    unsigned int n_bytes;
    unsigned int n_packets;
    } ht[N_HOST];

struct CHHIST
  {
    int n; time_t (*ts)[65536];
    int p[65536];
  };

struct Packet
  {
    struct ptime pt;
    int usec_correction;
    int timing_quality;
    int number_of_samples;
    int32_t *data;
  };

struct Queue
  {
    char *station;
    char *channel;
    int start;
    int end;
    struct Packet pkt[QUEUE_LEN];
  };

static struct Queue queue[NQUEUE];

void log_print(char *msg)
  {
    if(daemon_init)
      {
        syslog(LOG_INFO, "%s", msg);
      }
    else
      {
        time_t t = time(NULL);
        char *p = asctime(localtime(&t));
        printf("%.*s - %s: %s\n", strlen(p) - 1, p, plugin_name, msg);
        fflush(stdout);
      }
  }

int log_printf(const char *fmt, ...)
  {
    char buf[LOGMSGLEN];
    int r;
    va_list ap;

    va_start(ap, fmt);
    r = vsnprintf(buf, LOGMSGLEN, fmt, ap);
    va_end(ap);

    log_print(buf);

    return r;
  }

int mklong(ptr)
  unsigned char *ptr;
  {
  unsigned long a;
  a=((ptr[0]<<24)&0xff000000)+((ptr[1]<<16)&0xff0000)+
    ((ptr[2]<<8)&0xff00)+(ptr[3]&0xff);
  return a;
  }

int mkshort(ptr)
  unsigned char *ptr;
  {
  unsigned short a;
  a=((ptr[0]<<8)&0xff00)+(ptr[1]&0xff);
  return a;
  }

void get_time(rt)
  int *rt;
  {
  struct tm *nt;
  time_t ltime;
  time(&ltime);
  nt=localtime(&ltime);
  rt[0]=nt->tm_year%100;
  rt[1]=nt->tm_mon+1;
  rt[2]=nt->tm_mday;
  rt[3]=nt->tm_hour;
  rt[4]=nt->tm_min;
  rt[5]=nt->tm_sec;
  }

int bcd_dec(dest,sour)
  unsigned char *sour;
  int *dest;
  {
  static int b2d[]={
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,  /* 0x00 - 0x0F */  
    10,11,12,13,14,15,16,17,18,19,-1,-1,-1,-1,-1,-1,
    20,21,22,23,24,25,26,27,28,29,-1,-1,-1,-1,-1,-1,
    30,31,32,33,34,35,36,37,38,39,-1,-1,-1,-1,-1,-1,
    40,41,42,43,44,45,46,47,48,49,-1,-1,-1,-1,-1,-1,
    50,51,52,53,54,55,56,57,58,59,-1,-1,-1,-1,-1,-1,
    60,61,62,63,64,65,66,67,68,69,-1,-1,-1,-1,-1,-1,
    70,71,72,73,74,75,76,77,78,79,-1,-1,-1,-1,-1,-1,
    80,81,82,83,84,85,86,87,88,89,-1,-1,-1,-1,-1,-1,
    90,91,92,93,94,95,96,97,98,99,-1,-1,-1,-1,-1,-1,  /* 0x90 - 0x9F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
  int i;
  i=b2d[sour[0]];
  if(i>=0 && i<=99) dest[0]=i; else return 0;
  i=b2d[sour[1]];
  if(i>=1 && i<=12) dest[1]=i; else return 0;
  i=b2d[sour[2]];
  if(i>=1 && i<=31) dest[2]=i; else return 0;
  i=b2d[sour[3]];
  if(i>=0 && i<=23) dest[3]=i; else return 0;
  i=b2d[sour[4]];
  if(i>=0 && i<=59) dest[4]=i; else return 0;
  i=b2d[sour[5]];
  if(i>=0 && i<=60) dest[5]=i; else return 0;
  return 1;
  }

void ctrlc()
  {
  log_print("end");
  if (daemon_mode)
    closelog();
  exit(0);
  }

void err_sys(ptr)
  char *ptr;
{
  log_printf("%s: %s", ptr, strerror(errno));
  ctrlc();
}

void send_data(const char *station, const char *channel,
  const struct ptime *pt, int usec_correction, int timing_quality,
  const int32_t *dataptr, int number_of_samples)
  {
    int r;

    r = send_raw3(station, channel, pt, usec_correction, timing_quality,
      dataptr, number_of_samples);

    if(r < 0)
      {
        log_printf("error sending data to SeedLink: %s", strerror(errno));
        ctrlc();
      }
    else if(r == 0)
      {
        log_printf("error sending data to SeedLink");
        ctrlc();
      }
  }

void queue_data(int chid, time_t qidx, const struct ptime *pt,
  int usec_correction, int timing_quality, const int32_t *dataptr,
  int number_of_samples)
  {
    struct Packet *p;
    struct Queue *q;
    
    if(chid >= NQUEUE)
      {
        log_printf("invalid channel ID %04x", chid);
        return;
      }
    
    q = queue + chid;
    
    if(q->station == NULL)
      {
        if(verbosity > 1)
            log_printf("unknown channel ID %04x", chid);

        return;
      }
    
    if(qidx < q->start)
      {
        if(verbosity > 0)
            log_printf("send out-of-sequence %s %s %d", q->station, q->channel,
              q->start);

        send_data(q->station, q->channel, pt, usec_correction, timing_quality,
          dataptr, number_of_samples);

        return;
      }
    
    if(q->start == 0)
        q->start = q->end = qidx;
    
    while(qidx - q->start >= QUEUE_LEN)
      {
        p = q->pkt + (q->start & QUEUE_MASK);
        
        if(p->data != NULL)
          {
            if(verbosity > 1)
                log_printf("send forced %s %s %d", q->station, q->channel,
                  q->start);

            send_data(q->station, q->channel, &p->pt, p->usec_correction,
              p->timing_quality, p->data, p->number_of_samples);

            free(p->data);
            p->data = NULL;
          }

        ++q->start;
      }

    if(qidx >= q->end)
        q->end = qidx + 1;

    if(q->end - q->start == 1)
      {
        if(verbosity > 1)
            log_printf("send direct %s %s %d", q->station, q->channel, qidx);

        send_data(q->station, q->channel, pt, usec_correction, timing_quality,
          dataptr, number_of_samples);

        ++q->start;
        return;
      }

    if(verbosity > 1)
        log_printf("queue %s %s %d", q->station, q->channel, qidx);

    p = q->pkt + (qidx & QUEUE_MASK);
    p->pt = *pt;
    p->usec_correction = usec_correction;
    p->timing_quality = timing_quality;
    p->number_of_samples = number_of_samples;
    p->data = (int32_t *)malloc(number_of_samples * 4);
    memcpy(p->data, dataptr, number_of_samples * 4);

    while(q->start < q->end && q->pkt[q->start & QUEUE_MASK].data != NULL)
      {
        p = q->pkt + (q->start & QUEUE_MASK);
        
        if(verbosity > 1)
            log_printf("send queue %s %s %d", q->station, q->channel, q->start);

        send_data(q->station, q->channel, &p->pt, p->usec_correction,
          p->timing_quality, p->data, p->number_of_samples);

        free(p->data);
        p->data = NULL;
        ++q->start;
      }
  }

int decode_data(int chid, time_t qidx, const struct ptime *pt,
  int smprate, int smpsize, unsigned char *ptr, int maxlen)
  {
    int nbytes = 4, nsamp = 1, i = 0;
    int32_t sample_val, sample_buf[4096];

    if(verbosity > 1)
        log_printf("chid = %04x, smprate = %d, smpsize = %d, maxlen = %d",
          chid, smprate, smpsize, maxlen);
    
    sample_val = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
    sample_buf[0] = sample_val;
    
    while(nsamp < smprate)
      {
        if(nbytes >= maxlen || nbytes + smpsize > maxlen)
          {
            log_printf("error decoding data!");
            return -1;
          }
        
        switch(smpsize)
          {
          case 0:
            sample_val += ((int8_t)ptr[nbytes]) >> 4;
            sample_buf[nsamp++] = sample_val;

            if(nsamp < smprate)
              {
                sample_val += ((int8_t)(ptr[nbytes] << 4)) >> 4;
                sample_buf[nsamp++] = sample_val;
              }

            ++nbytes;
            break;

          case 1:
            sample_val += (int8_t)ptr[nbytes];
            sample_buf[nsamp++] = sample_val;
            ++nbytes;
            break;

          case 2:
            sample_val += (int16_t)((ptr[nbytes] << 8) | ptr[nbytes + 1]);
            sample_buf[nsamp++] = sample_val;
            nbytes += 2;
            break;

          case 3:
            sample_val += ((int32_t)((ptr[nbytes] << 24) | 
              (ptr[nbytes + 1] << 16) | (ptr[nbytes + 2] << 8))) >> 8;
            sample_buf[nsamp++] = sample_val;
            nbytes += 3;
            break;

          case 4:
            sample_val += (ptr[nbytes] << 24) | (ptr[nbytes + 1] << 16) |
              (ptr[nbytes + 2] << 8) | ptr[nbytes + 3];
            sample_buf[i++] = sample_val;
            nbytes += 4;
            break;

          default:
            log_printf("wrong sample size!");
            return -1;
          }
      }
            
    queue_data(chid, qidx, pt, 0, -1, sample_buf, nsamp);
    return nbytes;
  }

void decode_second_block(unsigned char *ptr, int len)
  {
    int chid, smpsize, smprate, r, n = 6;
    unsigned int t[6];
    time_t qidx;
    struct tm t_parts;
    struct ptime pt;
    
    if(!bcd_dec(t, ptr))
      {
        log_printf("cannot convert time from bcd to dec!");
        return;
      }
    
    if(verbosity > 1)
        log_printf("time stamp %02d%02d%02d %02d%02d%02d", t[0], t[1], t[2],
          t[3], t[4], t[5]);

    t_parts.tm_year = t[0] + 100;
    t_parts.tm_mon = t[1] - 1;
    t_parts.tm_mday = t[2];
    t_parts.tm_hour = t[3];
    t_parts.tm_min = t[4];
    t_parts.tm_sec = t[5];

    if((qidx = mktime(&t_parts)) == -1)
      {
        log_printf("invalid time!");
        return;
      }

    pt.year = t_parts.tm_year + 1900;
    pt.yday = t_parts.tm_yday + 1;
    pt.hour = t_parts.tm_hour;
    pt.minute = t_parts.tm_min;
    pt.second = t_parts.tm_sec;
    pt.usec = 0;
    
    while(n < len)
      {
        chid = (ptr[n] << 8) | ptr[n + 1];
        smpsize = (ptr[n + 2] >> 4) & 0xf;
        smprate = ((ptr[n + 2] & 0xf) << 8) | ptr[n + 3];
        n += 4;

        r = decode_data(chid, qidx, &pt, smprate, smpsize, ptr + n, len - n);
        
        if(r < 0) break;

        n += r;
      }
  }

void decode_packet(unsigned char *ptr)
  {
    unsigned int len;

    len = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];

    if(len < 22)
      {
        if(verbosity > 0)
            log_printf("packet is too short!");
        return;
      }
    
    decode_second_block(ptr + 8, len - 8);
  }

time_t check_ts(ptr,pre,post)
  char *ptr;
  int pre,post;
  {
  int diff,tm[6];
  time_t ts,rt;
  struct tm mt;
  if(!bcd_dec(tm,ptr)) return 0; /* out of range */
  memset((char *)&mt,0,sizeof(mt));
  if((mt.tm_year=tm[0])<50) mt.tm_year+=100;
  mt.tm_mon=tm[1]-1;
  mt.tm_mday=tm[2];
  mt.tm_hour=tm[3];
  mt.tm_min=tm[4];
  mt.tm_sec=tm[5];
  mt.tm_isdst=0;
  ts=mktime(&mt);
  /* compare time with real time */
  time(&rt);
  diff=ts-rt;
  if((pre==0 || pre<diff) && (post==0 || diff<post)) return ts;
#if DEBUG1
  printf("diff %d s out of range (%ds - %ds)\n",diff,pre,post);
#endif
  return 0;
  }

void read_chfile()
  {
  FILE *fp;
  int i,j,k,ii,i_chfile;
  char tbuf[1024],host_name[80];
  struct hostent *h;
  static time_t ltime,ltime_p;

  n_host=0;
  if(*chfile[0])
    {
    if((fp=fopen(chfile[0],"r"))!=NULL)
      {
#if DEBUG
      fprintf(stderr,"ch_file=%s\n",chfile[0]);
#endif
      if(negate_channel) for(i=0;i<65536;i++) ch_table[i]=1;
      else for(i=0;i<65536;i++) ch_table[i]=0;
      ii=0;
      while(fgets(tbuf,1024,fp))
        {
        *host_name=0;
        if(sscanf(tbuf,"%s",host_name)==0) continue;
        if(*host_name==0 || *host_name=='#') continue;
        if(*tbuf=='*') /* match any channel */
          {
          if(negate_channel) for(i=0;i<65536;i++) ch_table[i]=0;
          else for(i=0;i<65536;i++) ch_table[i]=1;
          }
        else if(n_host==0 && (*tbuf=='+' || *tbuf=='-'))
          {
          if(*tbuf=='+') hostlist[ii][0]=1;   /* allow */
          else hostlist[ii][0]=(-1);          /* deny */
          if(tbuf[1]=='\r' || tbuf[1]=='\n' || tbuf[1]==' ' || tbuf[1]=='\t')
            {
            hostlist[ii][1]=0;                  /* any host */
            if(*tbuf=='+') log_print("allow from the rest");
            else log_print("deny from the rest");
            }
          else
            {
            if(sscanf(tbuf+1,"%s",host_name)>0) /* hostname */
              {
              if(!(h=gethostbyname(host_name)))
                {
                log_printf("host '%s' not resolved",host_name);
                continue;
                }
              memcpy((char *)&hostlist[ii][1],h->h_addr,4);
              if(*tbuf=='+')
                  log_printf("allow from host %d.%d.%d.%d",
                    ((unsigned char *)&hostlist[ii][1])[0],
                    ((unsigned char *)&hostlist[ii][1])[1],
                    ((unsigned char *)&hostlist[ii][1])[2],
                    ((unsigned char *)&hostlist[ii][1])[3]);
              else
                  log_printf("deny from host %d.%d.%d.%d",
                    ((unsigned char *)&hostlist[ii][1])[0],
                    ((unsigned char *)&hostlist[ii][1])[1],
                    ((unsigned char *)&hostlist[ii][1])[2],
                    ((unsigned char *)&hostlist[ii][1])[3]);
              }
            }
          if(++ii==N_HOST)
            {
            n_host=ii;
            log_print("host control table full"); 
            }
          }
        else
          {
          sscanf(tbuf,"%x",&k);
          k&=0xffff;
#if DEBUG
          fprintf(stderr," %04X",k);
#endif
          if(negate_channel) ch_table[k]=0;
          else ch_table[k]=1;
          }
        }
#if DEBUG
      fprintf(stderr,"\n");
#endif
      if(ii>0 && n_host==0) n_host=ii;
      log_printf("%d host rules",n_host);
      fclose(fp);
      }
    else
      {
#if DEBUG
      fprintf(stderr,"ch_file '%s' not open\n",chfile[0]);
#endif
      log_printf("channel list file '%s' not open",chfile[0]);
      log_print("end");
      exit(1);
      }
    }
  else
    {
    for(i=0;i<65536;i++) ch_table[i]=1;
    n_host=0;
    }

  for(i_chfile=1;i_chfile<n_chfile;i_chfile++)
    {
    if((fp=fopen(chfile[i_chfile],"r"))!=NULL)
      {
#if DEBUG
      fprintf(stderr,"ch_file=%s\n",chfile[i_chfile]);
#endif
      while(fgets(tbuf,1024,fp))
        {
        if(*tbuf==0 || *tbuf=='#') continue;
        sscanf(tbuf,"%x",&k);
        k&=0xffff;
#if DEBUG
        fprintf(stderr," %04X",k);
#endif
        ch_table[k]=1;
        }
      fclose(fp);
      }
    else
      {
#if DEBUG
      fprintf(stderr,"ch_file '%s' not open\n",chfile[i_chfile]);
#endif
      log_printf("channel list file '%s' not open",chfile[i_chfile]);
      }
    }

  n_ch=0;
  for(i=0;i<65536;i++) if(ch_table[i]==1) n_ch++;
  if(n_ch==65536) log_printf("all channels");
  else log_printf("%d (-%d) channels",n_ch,65536-n_ch);

  time(&ltime);
  j=ltime-ltime_p;
  k=j/2;
  if(ht[0].host)
    {
    log_printf("statistics in %d s (pkts, B, pkts/s, B/s)",j);
    }
  for(i=0;i<N_HOST;i++) /* print statistics for hosts */
    {
    if(ht[i].host==0) break;
    log_printf("  src %d.%d.%d.%d:%d   %d %d %d %d",
      ((unsigned char *)&ht[i].host)[0],((unsigned char *)&ht[i].host)[1],
      ((unsigned char *)&ht[i].host)[2],((unsigned char *)&ht[i].host)[3],
      ntohs(ht[i].port),ht[i].n_packets,ht[i].n_bytes,(ht[i].n_packets+k)/j,
      (ht[i].n_bytes+k)/j);
    ht[i].n_packets=ht[i].n_bytes=0;
    }
  ltime_p=ltime;
  signal(SIGHUP,(void *)read_chfile);
  }

int check_pno(from_addr,pn,pn_f,sock,fromlen,n,nr) /* returns -1 if duplicated */
  struct sockaddr_in *from_addr;  /* sender address */
  unsigned int pn,pn_f;           /* present and former packet Nos. */
  int sock;                       /* socket */
  int fromlen;                    /* length of from_addr */
  int n;                          /* size of packet */
  int nr;                         /* no resend request if 1 */
  {
  int i,j;
  int host_;  /* 32-bit-long host address in network byte-order */
  int port_;  /* port No. in network byte-order */
  unsigned int pn_1;
  static unsigned int mask[8]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
  unsigned char pnc;

  j=(-1);
  host_=from_addr->sin_addr.s_addr;
  port_=from_addr->sin_port;
  for(i=0;i<n_host;i++)
    {
    if(hostlist[i][0]==1 && (hostlist[i][1]==0 || hostlist[i][1]==host_)) break;
    if(hostlist[i][0]==(-1) && (hostlist[i][1]==0 || hostlist[i][1]==host_))
      {
      if(!no_pinfo)
        {
        log_printf("deny packet from host %d.%d.%d.%d:%d",
          ((unsigned char *)&host_)[0],((unsigned char *)&host_)[1],
          ((unsigned char *)&host_)[2],((unsigned char *)&host_)[3],ntohs(port_));
        }
      return -1;
      }
    }
  for(i=0;i<N_HOST;i++)
    {
    if(ht[i].host==0) break;
    if(ht[i].host==host_ && ht[i].port==port_)
      {
      j=ht[i].no;
      ht[i].no=pn;
      ht[i].nos[pn>>3]|=mask[pn&0x07]; /* set bit for the packet no */
      ht[i].n_bytes+=n;
      ht[i].n_packets++;
      break;
      }
    }
  if(i==N_HOST)   /* table is full */
    {
    for(i=0;i<N_HOST;i++) ht[i].host=0;
    log_print("host table full - flushed.");
    i=0;
    }
  if(j<0)
    {
    ht[i].host=host_;
    ht[i].port=port_;
    ht[i].no=pn;
    ht[i].nos[pn>>3]|=mask[pn&0x07]; /* set bit for the packet no */
    log_printf("registered host %d.%d.%d.%d:%d (%d)",
      ((unsigned char *)&host_)[0],((unsigned char *)&host_)[1],
      ((unsigned char *)&host_)[2],((unsigned char *)&host_)[3],ntohs(port_),i);
    ht[i].n_bytes=n;
    ht[i].n_packets=1;
    }
  else /* check packet no */
    {
    pn_1=(j+1)&0xff;
    if(!nr && (pn!=pn_1))
      {
      if(((pn-pn_1)&0xff)<N_PACKET) do
        { /* send request-resend packet(s) */
        pnc=pn_1;
        sendto(sock,&pnc,1,0,(struct sockaddr *)from_addr,fromlen);
        log_printf("request resend %s:%d #%d",
          inet_ntoa(from_addr->sin_addr),ntohs(from_addr->sin_port),pn_1);
#if DEBUG1
        printf("<%d ",pn_1);
#endif
        ht[i].nos[pn_1>>3]&=~mask[pn_1&0x07]; /* reset bit for the packet no */
        } while((pn_1=(++pn_1&0xff))!=pn);
      else
        {
        log_printf("no request resend %s:%d #%d-#%d",
          inet_ntoa(from_addr->sin_addr),ntohs(from_addr->sin_port),pn_1,pn);
        }
      }
    }
  if(pn!=pn_f && ht[i].nos[pn_f>>3]&mask[pn_f&0x07])
    {   /* if the resent packet is duplicated, return with -1 */
    if(!no_pinfo)
      {
      log_printf("discard duplicated resent packet #%d for #%d",pn,pn_f);
      }
    return -1;
    }
  return 0;
  }

int wincpy2(ptw,ts,ptr,size,mon,chhist,from_addr)
  unsigned char *ptw,*ptr;
  time_t ts;
  int size,mon;
  struct CHHIST *chhist;
  struct sockaddr_in *from_addr;
  {
#define MAX_SR 500
#define MAX_SS 4
#define SR_MON 5
  int sr=0,n,ss;
  unsigned char *ptr_lim,*ptr1;
  unsigned short ch;
  int gs,i,aa,bb,k;
  unsigned long gh;

  ptr_lim=ptr+size;
  n=0;
  do    /* loop for ch's */
    {
    if(!mon)
      {
      gh=mklong(ptr);
      ch=(gh>>16)&0xffff;
      sr=gh&0xfff;
      ss=(gh>>12)&0xf;
      if(ss) gs=ss*(sr-1)+8;
      else gs=(sr>>1)+8;
      if(sr>MAX_SR || ss>MAX_SS || ptr+gs>ptr_lim)
        {
        if(!no_pinfo)
          {
          log_printf(
"ill ch hdr %02X%02X%02X%02X %02X%02X%02X%02X psiz=%d sr=%d ss=%d gs=%d rest=%d from %s:%d",
            ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7],
            size,sr,ss,gs,ptr_lim-ptr,
            inet_ntoa(from_addr->sin_addr),ntohs(from_addr->sin_port));
          }
        return n;
        }
      }
    else /* mon format */
      {
      ch=mkshort(ptr1=ptr);
      ptr1+=2;
      for(i=0;i<SR_MON;i++)
        {
        aa=(*(ptr1++));
        bb=aa&3;
        if(bb) for(k=0;k<bb*2;k++) ptr1++;
        else ptr1++;
        }
      gs=ptr1-ptr;
      if(ptr+gs>ptr_lim)
        {
        if(!no_pinfo)
          {
          log_printf(
"ill ch blk %02X%02X%02X%02X %02X%02X%02X%02X psiz=%d sr=%d gs=%d rest=%d from %s:%d",
            ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7],
            size,sr,gs,ptr_lim-ptr,
            inet_ntoa(from_addr->sin_addr),ntohs(from_addr->sin_port));
          }
        return n;
        }
      }
    if(ch_table[ch] && ptr+gs<=ptr_lim)
      {
      for(i=0;i<chhist->n;i++) if(chhist->ts[i][ch]==ts) break;
      if(i==chhist->n) /* TS not found in last chhist->n packets */
        {
        if(chhist->n>0)
          {
          chhist->ts[chhist->p[ch]][ch]=ts;
          if(++chhist->p[ch]==chhist->n) chhist->p[ch]=0;
          }
#if DEBUG1
        fprintf(stderr,"%5d",gs);
#endif
        memcpy(ptw,ptr,gs);
        ptw+=gs;
        n+=gs;
        }
#if DEBUG1
      else
        fprintf(stderr,"%5d(!%04X)",gs,ch);
#endif
      }
    ptr+=gs;
    } while(ptr<ptr_lim);
  return n;
  }

void send_req(sock,host_addr)
  struct sockaddr_in *host_addr;  /* sender address */
  int sock;                       /* socket */
  {
#define SWAPS(a) ((((a)<<8)&0xff00)|(((a)>>8)&0xff))
  int i,j;
/*
send list of chs : 2B/ch,1024B/packet->512ch/packet max 128packets
header: magic number,seq,n,list...
if all channels, seq=n=0 and no list.
*/
  unsigned seq,n_seq;
  struct { char mn[4]; unsigned short seq[2]; unsigned short chlist[512];}
    sendbuf;
  strcpy(sendbuf.mn,"REQ");
  if(n_ch<65536)
    {
    seq=1;
    if(n_ch==0) n_seq=1;
    else n_seq=(n_ch-1)/512+1;
    sendbuf.seq[1]=SWAPS(n_seq);
    j=0;
    for(i=0;i<65536;i++)
      {
      sendbuf.seq[0]=SWAPS(seq);
      if(ch_table[i]) sendbuf.chlist[j++]=SWAPS(i);
      if(j==512)
        {
        sendto(sock,&sendbuf,8+2*j,0,(struct sockaddr *)host_addr,
          sizeof(*host_addr));
#if DEBUG3
        printf("send channel list to %s:%d (%d): %s %d/%d %04X %04X %04X ...\n",
          inet_ntoa(host_addr->sin_addr),ntohs(host_addr->sin_port),
          n_ch,sendbuf.mn,SWAPS(sendbuf.seq[0]),SWAPS(sendbuf.seq[1]),
          SWAPS(sendbuf.chlist[0]),SWAPS(sendbuf.chlist[1]),SWAPS(sendbuf.chlist[2]));
#endif
        j=0;
        seq++;
        }
      }
    if(j>0)
      {
      sendto(sock,&sendbuf,8+2*j,0,(struct sockaddr *)host_addr,
        sizeof(*host_addr));
#if DEBUG3
        printf("send channel list to %s:%d (%d): %s %d/%d %04X %04X %04X ...\n",
          inet_ntoa(host_addr->sin_addr),ntohs(host_addr->sin_port),
          n_ch,sendbuf.mn,SWAPS(sendbuf.seq[0]),SWAPS(sendbuf.seq[1]),
          SWAPS(sendbuf.chlist[0]),SWAPS(sendbuf.chlist[1]),SWAPS(sendbuf.chlist[2]));
#endif
      seq++;
      }
    }
  else /* all channels */
    {
    sendbuf.seq[0]=sendbuf.seq[1]=0;
    sendto(sock,&sendbuf,8,0,(struct sockaddr *)host_addr,sizeof(*host_addr));
#if DEBUG3
    printf("send channel list to %s:%d (%d): %s %d/%d\n",
      inet_ntoa(host_addr->sin_addr),ntohs(host_addr->sin_port),
      n_ch,sendbuf.mn,SWAPS(sendbuf.seq[0]),SWAPS(sendbuf.seq[1]));
#endif
    }
  }

int main(argc,argv)
  int argc;
  char *argv[];
  {
  unsigned long uni;
  unsigned char *ptr,tm[6],*ptr_size,*ptr_size2,host_name[1024];
  int i,j,size,fromlen,n,nlen,sock,nn,all,pre,post,c,mon,pl,eobsize,
    sbuf,noreq,no_ts,no_pno;
  struct sockaddr_in to_addr,from_addr,host_addr;
  unsigned short to_port,host_port=0;
  extern int optind;
  extern char *optarg;
  struct Shm {
    unsigned long p;    /* write point */
    unsigned long pl;   /* write limit */
    unsigned long r;    /* latest */
    unsigned long c;    /* counter */
    unsigned char d[1];   /* data buffer */
    } *sh;
  char tb2[256];
  struct ip_mreq stMreq;
  char mcastgroup[256]; /* multicast address */
  char interface[256]; /* multicast interface */
  time_t ts=0,sec,sec_p;
  struct CHHIST chhist;
  struct hostent *h;
  struct timeval timeout;
  fd_set fds;
  FILE *fp;
  int r, chid;
  const char *channel_map_file = CHANNEL_MAP_FILE;
  char station[PLUGIN_SIDLEN+1], channel[PLUGIN_CIDLEN+1];

  if((progname=strrchr(argv[0],'/'))) progname++;
  else progname=argv[0];

  all=no_pinfo=mon=eobsize=noreq=no_ts=no_pno=0;
  pre=post=0;
  *interface=(*mcastgroup)=(*host_name)=0;
  sbuf=256;
  chhist.n=N_HIST;
  n_chfile=1;
  while((c=getopt(argc,argv,"AaDd:f:F:g:i:m:MNno:p:rs:vVh"))!=EOF)
    {
    switch(c)
      {
      case 'A':   /* don't check time stamps */
        no_ts=1;
        break;
      case 'a':   /* accept >=A1 packets */
        all=1;
        break;
      case 'D':
        daemon_mode = 1;  /* daemon mode */
        break;   
      case 'd':   /* length of packet history in sec */
        chhist.n=atoi(optarg);
        break;
      case 'f':   /* channel list file */
        strcpy(chfile[n_chfile++],optarg);
        break;
      case 'F':   /* channel map file: chid -> {station, channel} */
        if((channel_map_file = strdup(optarg)) == NULL)
          {
            perror("strdup");
            exit(1);
          }
        break;
      case 'g':   /* multicast group (multicast IP address) */
        strcpy(mcastgroup,optarg);
        break;
      case 'i':   /* interface (ordinary IP address) which receive mcast */
        strcpy(interface,optarg);
        break;
      case 'm':   /* time limit before RT in minutes */
        pre=atoi(optarg);
        if(pre<0) pre=(-pre);
        break;
      case 'M':   /* 'mon' format data */
        mon=1;
        break;
      case 'N':   /* no packet nos */
        no_pno=no_ts=1;
        break;
      case 'n':   /* supress info on abnormal packets */
        no_pinfo=1;
        break;
      case 'o':   /* host and port for request */
        strcpy(tb2,optarg);
        if((ptr=(unsigned char *)strchr(tb2,':')))
          {
          *ptr=0;
          host_port=atoi(ptr+1);
          }
        else
          {
          fprintf(stderr," option -o requires '[host]:[port]' pair !\n");
          fprintf(stderr, opterr_message, progname);
          exit(1);
          }
        strcpy(host_name,tb2);
        break;
      case 'p':   /* time limit after RT in minutes */
        post=atoi(optarg);
        break;
      case 'r':   /* disable resend request */
        noreq=1;
        break;
      case 's':   /* preferred socket buffer size (KB) */
        sbuf=atoi(optarg);
        break;
      case 'v':
        ++verbosity;
        break;
      case 'V':
        fprintf(stdout, "%s\n", ident_str);
        exit(0);
      case 'h':
        fprintf(stdout, help_message, progname);
        exit(0);
      case '?':
        fprintf(stderr, opterr_message, progname);
        exit(1);
      }
    }

    if(optind != argc - 3)
      {
        fprintf(stderr, help_message, progname);
        exit(1);
      }

  pre=(-pre*60);
  post*=60;

  to_port=atoi(argv[optind]);

  *chfile[0]=0;
  if(strcmp("-",argv[optind + 1])==0) *chfile[0]=0;
  else
    {
    if(argv[4+optind][0]=='-')
      {
      strcpy(chfile[0],argv[optind + 1]+1);
      negate_channel=1;
      }
    else
      {
      strcpy(chfile[0],argv[optind + 1]);
      negate_channel=0;
      }
    }

  if(n_chfile==1 && (*chfile[0])==0) n_chfile=0;

  if((plugin_name = strdup(argv[optind + 2])) == NULL)
    {
      perror("strdup");
      exit(1);
    }
  
  if((fp = fopen(channel_map_file, "r")) == NULL)
    {
      fprintf(stderr, "cannot open %s: %s\n", channel_map_file,
        strerror(errno));
      exit(1);
    }

  while((r = fscanf(fp, "%x %10s %10s\n", &chid, station, channel)) == 3)
    {
      if(chid >= NQUEUE) break;

      if((queue[chid].station = strdup(station)) == NULL)
        {
          perror("strdup");
          exit(1);
        }
  
      if((queue[chid].channel = strdup(channel)) == NULL)
        {
          perror("strdup");
          exit(1);
        }
    }

  fclose(fp);
  
  if(r != EOF)
    {
      fprintf(stderr, "error parsing %s\n", channel_map_file);
      exit(1);
    }

  if(daemon_mode)
    {
      log_printf("%s started", ident_str);
      log_printf("take a look into syslog files for more messages");
      openlog(plugin_name, 0, SYSLOG_FACILITY);
      daemon_init = 1;
    }
  
  if((chhist.ts=
      (time_t (*)[65536])malloc(65536*chhist.n*sizeof(time_t)))==NULL)
    {
    chhist.n=N_HIST;
    if((chhist.ts=
        (time_t (*)[65536])malloc(65536*chhist.n*sizeof(time_t)))==NULL)
      {
      fprintf(stderr,"malloc failed (chhist.ts)\n");
      exit(1);
      }
    }

  log_printf("%s started", ident_str);

  size = 10000;
  sh = (struct Shm *)malloc(size);
  
  /* initialize buffer */
  sh->c=0;
  sh->pl=pl=2000; /* (size-sizeof(*sh))/10*9; */
  sh->p=0;
  sh->r=(-1);

  log_printf("n_hist=%d", chhist.n);

  if(all)
      log_printf("accept >=A1 packets");

  log_printf("TS window %ds - +%ds",pre,post);
  
  if((sock=socket(AF_INET,SOCK_DGRAM,0))<0) err_sys("socket");
  for(j=sbuf;j>=16;j-=4)
    {
    i=j*1024;
    if(setsockopt(sock,SOL_SOCKET,SO_RCVBUF,(char *)&i,sizeof(i))>=0)
      break;
    }
  if(j<16) err_sys("SO_RCVBUF setsockopt error\n");
  log_printf("RCVBUF size=%d",j*1024);

  memset((char *)&to_addr,0,sizeof(to_addr));
  to_addr.sin_family=AF_INET;
  to_addr.sin_addr.s_addr=htonl(INADDR_ANY);
  to_addr.sin_port=htons(to_port);
  if(bind(sock,(struct sockaddr *)&to_addr,sizeof(to_addr))<0) err_sys("bind");

  if(*mcastgroup){
    stMreq.imr_multiaddr.s_addr=inet_addr(mcastgroup);
    if(*interface) stMreq.imr_interface.s_addr=inet_addr(interface);
    else stMreq.imr_interface.s_addr=INADDR_ANY;
    if(setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char *)&stMreq,
      sizeof(stMreq))<0) err_sys("IP_ADD_MEMBERSHIP setsockopt error\n");
  }

  if(*host_name) /* host_name and host_port specified */
    {
    /* source host/port */
    if(!(h=gethostbyname(host_name))) err_sys("can't find host");
    memset((char *)&host_addr,0,sizeof(host_addr));
    host_addr.sin_family=AF_INET;
    memcpy((caddr_t)&host_addr.sin_addr,h->h_addr,h->h_length);
    host_addr.sin_port=htons(host_port);
    for(j=sbuf;j>=16;j-=4)
      {
      i=j*1024;
      if(setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(char *)&i,sizeof(i))>=0)
        break;
      }
    if(j<16) err_sys("SO_SNDBUF setsockopt error\n");
    log_printf("SNDBUF size=%d",j*1024);
    }

  if(noreq) log_print("resend request disabled");
  if(no_ts) log_print("time-stamps not interpreted");
  if(no_pno) log_print("packet numbers not interpreted");
  if(*host_name)
    {
    log_printf("send channel list to %s:%d",
      inet_ntoa(host_addr.sin_addr),ntohs(host_addr.sin_port));
    }

  signal(SIGTERM,(void *)ctrlc);
  signal(SIGINT,(void *)ctrlc);
  signal(SIGPIPE,(void *)ctrlc);

  for(i=0;i<6;i++) tm[i]=(-1);
  ptr=ptr_size=sh->d+sh->p;
  read_chfile();
  time(&sec);
  sec_p=sec-1;

  while(1)
    {
    if(*host_name) /* send request */
      {
      time(&sec);
      if(sec!=sec_p)
        {
        send_req(sock,&host_addr);
        sec_p=sec;
        }
      }

    timeout.tv_sec=0;
    timeout.tv_usec=500000;
    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    if(select(sock+1,&fds,NULL,NULL,&timeout)<=0) continue;

    fromlen=sizeof(from_addr);
    n=recvfrom(sock,rbuf,MAXMESG,0,(struct sockaddr *)&from_addr,&fromlen);
#if DEBUG0
    if(rbuf[0]==rbuf[1]) printf("%d ",rbuf[0]);
    else printf("%d(%d) ",rbuf[0],rbuf[1]);
    for(i=0;i<16;i++) printf("%02X",rbuf[i]);
    printf(" (%d)\n",n);
    fflush(stdout);
#endif

    if(no_ts) /* no time stamp */
      {
      ptr+=4;   /* size */
      ptr+=4;   /* time of write */
      if(no_pno) memcpy(ptr,rbuf,nn=n);
      else
        {
        if(check_pno(&from_addr,rbuf[0],rbuf[1],sock,fromlen,n,noreq)<0) continue;
        memcpy(ptr,rbuf+2,nn=n-2);
        }
      ptr+=nn;
      uni=time(0);
      ptr_size[4]=uni>>24;  /* tow (H) */
      ptr_size[5]=uni>>16;
      ptr_size[6]=uni>>8;
      ptr_size[7]=uni;      /* tow (L) */
      ptr_size2=ptr;
      if(eobsize) ptr+=4; /* size(2) */
      uni=ptr-ptr_size;
      ptr_size[0]=uni>>24;  /* size (H) */
      ptr_size[1]=uni>>16;
      ptr_size[2]=uni>>8;
      ptr_size[3]=uni;      /* size (L) */
      if(eobsize)
        {
        ptr_size2[0]=ptr_size[0];  /* size (H) */
        ptr_size2[1]=ptr_size[1];
        ptr_size2[2]=ptr_size[2];
        ptr_size2[3]=ptr_size[3];  /* size (L) */
        }
      sh->r=sh->p;      /* latest */
      if(eobsize && ptr>sh->d+pl) {sh->pl=ptr-sh->d-4;ptr=sh->d;}
      if(!eobsize && ptr>sh->d+sh->pl) ptr=sh->d;
      sh->p=ptr-sh->d;
      sh->c++;
      ptr_size=ptr;
#if DEBUG1
      printf("%s:%d(%d)>",inet_ntoa(from_addr.sin_addr),ntohs(from_addr.sin_port),n);
      printf("%d(%d) ",rbuf[0],rbuf[1]);
      rbuf[n]=0;
      printf("%s\n",rbuf);
#endif
      
      if(verbosity > 1)
          log_printf("decode packet (1)");

      decode_packet(sh->d + sh->r);
      continue;
      }

    if(check_pno(&from_addr,rbuf[0],rbuf[1],sock,fromlen,n,noreq)<0) continue;

  /* check packet ID */
    if(rbuf[2]<0xA0)
      {
      for(i=0;i<6;i++) if(rbuf[i+2]!=tm[i]) break;
      if(i==6)  /* same time */
        {
        nn=wincpy2(ptr,ts,rbuf+8,n-8,mon,&chhist,&from_addr);
        ptr+=nn;
        uni=time(0);
        ptr_size[4]=uni>>24;  /* tow (H) */
        ptr_size[5]=uni>>16;
        ptr_size[6]=uni>>8;
        ptr_size[7]=uni;      /* tow (L) */
        }
      else
        {
        if((ptr-ptr_size)>14)
          {
          ptr_size2=ptr;
          if(eobsize) ptr+=4; /* size(2) */
          uni=ptr-ptr_size;
          ptr_size[0]=uni>>24;  /* size (H) */
          ptr_size[1]=uni>>16;
          ptr_size[2]=uni>>8;
          ptr_size[3]=uni;      /* size (L) */
          if(eobsize)
            {
            ptr_size2[0]=ptr_size[0];  /* size (H) */
            ptr_size2[1]=ptr_size[1];
            ptr_size2[2]=ptr_size[2];
            ptr_size2[3]=ptr_size[3];  /* size (L) */
            }
#if DEBUG2
          printf("%d - %d (%d) %d / %d\n",ptr_size-sh->d,uni,ptr_size2-sh->d,
            sh->pl,pl);
#endif
#if DEBUG1
          printf("(%d)",time(0));
          for(i=8;i<14;i++) printf("%02X",ptr_size[i]);
          printf(" : %d > %d\n",uni,ptr_size-sh->d);
#endif
          }
        else ptr=ptr_size;
        if(!(ts=check_ts(rbuf+2,pre,post)))
          {
          if(!no_pinfo)
            {
            log_printf("ill time %02X:%02X %02X%02X%02X.%02X%02X%02X:%02X%02X%02X%02X%02X%02X%02X%02X from %s:%d",
              rbuf[0],rbuf[1],rbuf[2],rbuf[3],rbuf[4],rbuf[5],rbuf[6],rbuf[7],
              rbuf[8],rbuf[9],rbuf[10],rbuf[11],rbuf[12],rbuf[13],rbuf[14],
              rbuf[15],inet_ntoa(from_addr.sin_addr),ntohs(from_addr.sin_port));
            }
          for(i=0;i<6;i++) tm[i]=(-1);
          continue;
          }
        else
          {
          sh->r=sh->p;      /* latest */
          if(eobsize && ptr>sh->d+pl) {sh->pl=ptr-sh->d-4;ptr=sh->d;}
          if(!eobsize && ptr>sh->d+sh->pl) ptr=sh->d;
          sh->p=ptr-sh->d;
          sh->c++;
          ptr_size=ptr;
          ptr+=4;   /* size */
          ptr+=4;   /* time of write */
          memcpy(ptr,rbuf+2,6);
          ptr+=6;
          nn=wincpy2(ptr,ts,rbuf+8,n-8,mon,&chhist,&from_addr);
          ptr+=nn;
          memcpy(tm,rbuf+2,6);
          uni=time(0);
          ptr_size[4]=uni>>24;  /* tow (H) */
          ptr_size[5]=uni>>16;
          ptr_size[6]=uni>>8;
          ptr_size[7]=uni;      /* tow (L) */
          }
        }

      if(verbosity > 1)
          printf("decode packet (2)\n");

      decode_packet(sh->d + sh->r);
      }
    else if(rbuf[2]==0xA0) /* merged packet */
      {
      nlen=n-3;
      j=3;
      while(nlen>0)
        {
        n=(rbuf[j]<<8)+rbuf[j+1];
        if(n<8 || n>nlen)
          {
          if(!no_pinfo)
            {
log_printf("ill blk n=%d(%02X%02X) %02X%02X%02X.%02X%02X%02X:%02X%02X%02X%02X%02X%02X%02X%02X from %s:%d #%d(#%d) at %d",
              n,rbuf[j],rbuf[j+1],rbuf[j+2],rbuf[j+3],rbuf[j+4],rbuf[j+5],
              rbuf[j+6],rbuf[j+7],rbuf[j+8],rbuf[j+9],rbuf[j+10],rbuf[j+11],
              rbuf[j+12],rbuf[j+13],rbuf[j+14],rbuf[j+15],
              inet_ntoa(from_addr.sin_addr),ntohs(from_addr.sin_port),
              rbuf[0],rbuf[1],j);
            }
          for(i=0;i<6;i++) tm[i]=(-1);
          break;
          }
        j+=2;
        for(i=0;i<6;i++) if(rbuf[j+i]!=tm[i]) break;
        if(i==6)  /* same time */
          {
          nn=wincpy2(ptr,ts,rbuf+j+6,n-8,mon,&chhist,&from_addr);
          ptr+=nn;
          uni=time(0);
          ptr_size[4]=uni>>24;  /* tow (H) */
          ptr_size[5]=uni>>16;
          ptr_size[6]=uni>>8;
          ptr_size[7]=uni;      /* tow (L) */
          }
        else
          {
          if((ptr-ptr_size)>14) /* data copied - close the sec block */
            {
            ptr_size2=ptr;
            if(eobsize) ptr+=4; /* size(2) */
            uni=ptr-ptr_size;
            ptr_size[0]=uni>>24;  /* size (H) */
            ptr_size[1]=uni>>16;
            ptr_size[2]=uni>>8;
            ptr_size[3]=uni;      /* size (L) */
            if(eobsize)
              {
              ptr_size2[0]=ptr_size[0];  /* size (H) */
              ptr_size2[1]=ptr_size[1];
              ptr_size2[2]=ptr_size[2];
              ptr_size2[3]=ptr_size[3];  /* size (L) */
              }
#if DEBUG2
          printf("%d - %d (%d) %d / %d\n",ptr_size-sh->d,uni,ptr_size2-sh->d,
            sh->pl,pl);
#endif
#if DEBUG1
            printf("(%d)",time(0));
            for(i=8;i<14;i++) printf("%02X",ptr_size[i]);
            printf(" : %d > %d\n",uni,ptr_size-sh->d);
#endif
            sh->r=sh->p;      /* latest */
            if(eobsize && ptr>sh->d+pl) {sh->pl=ptr-sh->d-4;ptr=sh->d;}
            if(!eobsize && ptr>sh->d+sh->pl) ptr=sh->d;
            sh->p=ptr-sh->d;
            sh->c++;
            ptr_size=ptr;

            if(verbosity > 1)
                log_printf("decode packet (3)");

            decode_packet(sh->d + sh->r);
            }
          else /* sec block empty - reuse the space */
            ptr=ptr_size;
          if(!(ts=check_ts(rbuf+j,pre,post))) /* illegal time */
            {
            if(!no_pinfo)
              {
              log_printf("ill time %02X%02X%02X.%02X%02X%02X:%02X%02X%02X%02X%02X%02X%02X%02X from %s:%d",
                rbuf[j],rbuf[j+1],rbuf[j+2],rbuf[j+3],rbuf[j+4],rbuf[j+5],
                rbuf[j+6],rbuf[j+7],rbuf[j+8],rbuf[j+9],rbuf[j+10],rbuf[j+11],
                rbuf[j+12],rbuf[j+13],
                inet_ntoa(from_addr.sin_addr),ntohs(from_addr.sin_port));
              }
            for(i=0;i<6;i++) tm[i]=(-1);
            }
          else /* valid time stamp */
            {
            ptr+=4;   /* size */
            ptr+=4;   /* time of write */
            memcpy(ptr,rbuf+j,6);
            ptr+=6;
            nn=wincpy2(ptr,ts,rbuf+j+6,n-8,mon,&chhist,&from_addr);
            ptr+=nn;
            memcpy(tm,rbuf+j,6);
            uni=time(0);
            ptr_size[4]=uni>>24;  /* tow (H) */
            ptr_size[5]=uni>>16;
            ptr_size[6]=uni>>8;
            ptr_size[7]=uni;      /* tow (L) */
            }
          }
        nlen-=n;
        j+=n-2;
        }
      }
    else if(all) /* rbuf[2]>=0xA1 with packet ID */
      {
      if(!(ts=check_ts(rbuf+3,pre,post)))
        {
        if(!no_pinfo)
          {
          log_printf("ill time %02X:%02X %02X %02X%02X%02X.%02X%02X%02X:%02X%02X%02X%02X%02X%02X%02X%02X from %s:%d",
            rbuf[0],rbuf[1],rbuf[2],rbuf[3],rbuf[4],rbuf[5],rbuf[6],rbuf[7],
            rbuf[8],rbuf[9],rbuf[10],rbuf[11],rbuf[12],rbuf[13],rbuf[14],
            rbuf[15],rbuf[16],inet_ntoa(from_addr.sin_addr),
            ntohs(from_addr.sin_port));
          }
        for(i=0;i<6;i++) tm[i]=(-1);
        continue;
        }
      else
        {
        ptr_size=ptr;
        ptr+=4;   /* size */
        ptr+=4;   /* time of write */
        memcpy(ptr,rbuf+2,n-2);
        ptr+=n-2;
        ptr_size2=ptr;
        if(eobsize) ptr+=4; /* size(2) */
        uni=ptr-ptr_size;
        ptr_size[0]=uni>>24;  /* size (H) */
        ptr_size[1]=uni>>16;
        ptr_size[2]=uni>>8;
        ptr_size[3]=uni;      /* size (L) */
        if(eobsize)
          {
          ptr_size2[0]=ptr_size[0];  /* size (H) */
          ptr_size2[1]=ptr_size[1];
          ptr_size2[2]=ptr_size[2];
          ptr_size2[3]=ptr_size[3];  /* size (L) */
          }
        memcpy(tm,rbuf+3,6);
        uni=time(0);
        ptr_size[4]=uni>>24;  /* tow (H) */
        ptr_size[5]=uni>>16;
        ptr_size[6]=uni>>8;
        ptr_size[7]=uni;      /* tow (L) */

        sh->r=sh->p;      /* latest */
        if(eobsize && ptr>sh->d+pl) {sh->pl=ptr-sh->d-4;ptr=sh->d;}
        if(!eobsize && ptr>sh->d+sh->pl) ptr=sh->d;
        sh->p=ptr-sh->d;
        sh->c++;
        }

      if(verbosity > 1)
          log_printf("decode packet (4)");

      decode_packet(sh->d + sh->r);
      }

#if BELL
    fprintf(stderr,"\007");
    fflush(stderr);
#endif
    }

    return 0;
  }

