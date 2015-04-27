/*   Lib330 DSS Routines
     Copyright 2009 Certified Software Corporation

    This file is part of Lib330

    Lib330 is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Lib330 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Lib330; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

Edit History:
   Ed Date       By  Changes
   -- ---------- --- ---------------------------------------------------
    0 2009-07-28 rdr Created.
    1 2009-08-03 rdr Fix handling lowest priority password.
    2 2009-08-04 rdr Make memory requests honor actual memory limits. Use dss_gate_on flag
                     instead of data_timer in check_data.
    3 2009-08-05 rdr Fix store_long for 16 bit integer format.
    4 2009-09-17 jms fix timeout msg. add BALER44 condx for ARM double storage.
                     report secs since Q330 reboot, not dss server.
    5 2010-01-04 rdr Use fcntl instead of ioctl to set socket non-blocking.
    6 2013-02-02 rdr Set high_socket.
*/
#ifndef OMIT_SEED /* Can't use without seed generation */
#ifndef OMIT_NETWORK /* or without network */

#ifndef libdss_h
#include "libdss.h"
#endif
#ifndef libsampglob_h
#include "libsampglob.h"
#endif
#ifndef libcvrt_h
#include "libcvrt.h"
#endif
#ifndef libmsgs_h
#include "libmsgs.h"
#endif
#ifndef libclient_h
#include "libclient.h"
#endif
#ifndef libdetect_h
#include "libdetect.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif

#define RPT_CLIENTS 1 /* verbosity >= shows new clients */
#define RPT_REPORTS 2 /* shows reports */
#define RPT_ALLOC 3 /* show memory allocations */
#define RPT_ALLMEM 4 /* all memory allocations */
#define DEF_CHUNKSIZE 16384 /* how much to get using getmem */
#define MIN_FRAG 32 /* minimum size of left memory segment to create a new segment */
#define BYTE_INTERVAL 10 /* interval over which to measure bytes transmitted */
#define FLAG_INT 1 /* just get data at the interval */
#define FLAG_CON 2 /* continuously get new data from server */
#define FLAG_CNTDN 4 /* Sending data not available packets */
#define FLAG_WAIT 8 /* Waiting for reconnection timeout */
#define FLAG_GOOD 16 /* Have good data */
#define FLAG_MASK (not (FLAG_WAIT or FLAG_CNTDN)) /* to turn these off */
#define YEAR_ADJUST (86400.0L * 5844.0L) /* Difference between 2000 and 1984 references */

#define DSS_REQ 0xF0     /* Request new report */
#define DSS_DEL 0xF1     /* Delete report(s) */
#define DSS_TOR 0xF2     /* Timeout reset */
#define DSS_REG 0xF3     /* Register new client */
#define DSS_LRQ 0xF4     /* List of reports request */
#define DSS_REM 0xF5     /* Remove client */
#define DSS_RQL 0xFF     /* Requested reports list */
#define DSS_REF 0xFE     /* Refusal */
#define DSS_DAT 0xFD     /* Data Packet */
#define DSS_ACK 0xFC     /* Acknowledgement */
#define DSS_PRG 0xFB     /* Report purged */

#define REF_TO 1        /* Timeout */
#define REF_IDS 2       /* Invalid data source */
#define REF_SEI 3       /* Seconds exceeds Interval */
#define REF_INP 4       /* Invalid Password */
#define REF_URC 5       /* Unregistered Client */
#define REF_TMR 6       /* Too Many Requests for this client */
#define REF_URQ 7       /* Unknown Request */
#define REF_DUP 8       /* Duplicate Data ID */
#define REF_OOM 9       /* Server out of memory */
#define REF_IDF 10      /* Invalid data format */

#define PRG_ESL 1       /* Excessive Server Loading */
#define PRG_BPS 2       /* Too many bytes per second */

#define REQ_SIMP 1      /* Simple data request */
#define REQ_GPS 2       /* GPS location request */
#define REQ_DEAD 3      /* Dead channel request */
#define REQ_DET 4       /* Detection count request */
#define REQ_REC 5       /* Record count request */
#define REQ_BOOT 6      /* Time since reboot request */
#define REQ_MMA 7       /* Min, Max, & Avg request */
#define REQ_SQR 8       /* Sum of squares & Avg request */

#define FLG_NEVER 0     /* never report loss of data source */
#define FLG_ONCE 1      /* report data source loss 1 time */
                      /* 2 .. 249 = 2 .. 249 times */
#define FLG_CONT 250    /* report data source loss continuously */

#define FMT_I16 1       /* 16 bit int16 */
#define FMT_I32 3       /* 32 bit int16 */
#define FMT_F32 4       /* 32 bit Float */
#define FMT_F64 5       /* 64 bit Float */

#define OFF_MASK 0x1FF   /* Mask for offset fields */
#define OFF_DNA 0x8000   /* Indicates data not available */

typedef struct {
  word data_id ;     /* data identification word */
  word rep_prio ;    /* report priority */
  longint interval ; /* reporting interval */
  byte reqcode ;     /* request code REQ_XXX */
  byte repflag ;     /* report flag */
  tseed_stn station ; /* after conversion */
} tdss_req ;
typedef struct {
  tqdp qdp ;
  byte buffer[MAXDATA] ; /* maximum length */
} tdss_msg ;
typedef tdss_msg *pdss_msg ;

typedef struct {
  int16 count ;    /* number of reports */
  word reports[(MAXDATA - 2) div 2] ; /* actual reports */
} tdss_del ;
typedef tdss_del *pdss_del ;

typedef struct {
  word data_id ; /* report number */
  word status ; /* data availability status */
} tdss_repstat ;
typedef struct {
  int16 count ;    /* number of reports */
  tdss_repstat reports[(MAXDATA - 2) div 4] ;
} tdss_rql ;
typedef tdss_rql *pdss_rql ;

typedef struct { /* Note - original arrays started at index 1, not 0 */
  char latitude[10] ; /* -xx.xxxxxx degrees */
  char longitude[11] ; /* -xxx.xxxxxx degrees */
  char elevation[7] ; /* -xxxx.x meters */
} gps_struc ;

typedef struct tmemory {
  struct tmemory *next ;     /* pointer to next memory segment */
  struct tmemory *prev ;     /* pointer to previous memory segment */
  integer size ;     /* size of this segment, including header */
} tmemory ;
typedef tmemory *pmemory ;

typedef struct {
  tmemory memory ;
  pmemory access ;   /* start of access structure */
  longint counter ;  /* Interval counter */
  byte flag ;        /* FLAG_XXX */
  byte repcount ;    /* Used to countdown loss of data reports */
  int16 priority ;   /* Priority of my client */
  word data_id ;     /* data identification word */
  word rep_prio ;    /* report priority */
  longint interval ; /* reporting interval */
  byte reqcode ;     /* request code REQ_XXX */
  byte repflag ;     /* Report flags */
  tseed_stn station ;
  tlocation location ; /* from aqstruc */
  tseed_name channel ; /* from aqstruc */
  byte format ;      /* FMT_XXX */
  longint seconds ;  /* seconds of data */
} dss_report ;
typedef dss_report *pdss_report ;

typedef struct {
  tmemory memory ;
  pdss_report head ; /* head of report list for this client */
  longint timer ;    /* timeout counter */
  longword ipaddr ;   /* IP address */
  word port ;        /* IP port number */
  byte qdpver ;
  string79 name ; /* client name, variable length */
} dss_client ;
typedef dss_client *pdss_client ;

typedef struct {
  tmemory memory ;
  plcq lcq ;
  longint current ;
} treccnt ;

typedef struct {
  tmemory memory ;
  pdet_packet det ;
  longint current ;
} tdetcnt ;

typedef struct {
  tmemory memory ;
  longint seccount ; /* number of seconds of data so far */
  double rettime ;   /* return time */
  plcq lcq ;         /* LCQ providing data */
  boolean acc ;      /* TRUE if actually accumulating samples */
  longint minimum ;
  longint maximum ;
  double average ;
} tmma ;

typedef struct {
  tmemory memory ;
  longint seccount ; /* number of seconds of data so far */
  double rettime ;   /* return time */
  plcq lcq ;         /* LCQ providing data */
  boolean acc ;      /* TRUE if actually accumulating samples */
  double sum ;
  double average ;
} tsqr ;

typedef struct {
  pq330 q330 ; /* local copy */
  paqstruc paqs ;
  boolean running ;
  boolean sockopen ;
  boolean sockfull ; /* last send failed */
  tdss dss_par ; /* creation parameters */
#ifdef X86_WIN32
  struct sockaddr dsockin, sock ; /* dss address descriptors */
#else
  struct sockaddr dsockin, sock ; /* dss address descriptors */
#endif
  pmemory free ;       /* head of free memory list */
  integer total ;      /* total memory obtained from system */
  integer mem_allowed ; /* memory allowed to be used */
  longint client_timeout ;  /* number of seconds without DSS_TOR */
  longint maxbps ;     /* maximum bytes per second */
  longint bytecount ;  /* bytes transmitted so far */
  int16 client_priority ; /* Current priority we are servicing */
  integer bytesec ;    /* number of seconds so far (for BPS) */
  integer dss_time_sec ; /* used to get 1 second timer from 100ms */
  boolean refused ; /* set if current request was refused */
  word reply_sequence ; /* only ls 8 bits for early qdp */
  word host_port ; /* actual socket number */
  pdss_client clients[3] ; /* clients based on priority */
  string7 passwords[3] ; /* passwords for clients */
  tdss_msg msgin, msgout ;
  double last_sec ;
  pdss_client cur_client ;
  tqdp recvhdr ;
  tqdp sendhdr ;
  byte cur_ver ;
  byte verbosity ;
  double last_interval ;
  string63 dss_server_display ;
  string detname ;    /* even length as required */
  pbyte pb, lastp ;
} tdssstr ;
typedef tdssstr *pdssstr ;

/* try to find smallest free segment that is big enough and
   return, else return NIL */
static pmemory scan (pdssstr dssstr, integer sz, integer *smallest)
begin
  pmemory mscan, smaller ;

  *smallest = 0 ;
  smaller = NIL ;
  mscan = dssstr->free ;
  while (mscan)
    begin
      if ((mscan->size >= sz) land ((smaller == NIL) lor (mscan->size < *smallest)))
        then
          begin
            *smallest = mscan->size ;
            smaller = mscan ;
          end
      mscan = mscan->next ;
    end
  return smaller ;
end

/* Return pointer to memory segment at least sz bytes long, or NIL
  if not available */
static pmemory memreq (pdssstr dssstr, integer sz)
begin
  pmemory mscan, oldnext, oldprev, mend ;
  integer oldsize ;
  pmemory smallpt, mpt, ret ;
  integer smallest, msize;
  string63 s ;

  sz = (sz + 3) and 0xFFFFFFFC ;
  if (dssstr->verbosity >= RPT_ALLMEM)
    then
      begin
        sprintf(s, "DSS Memory Request for %d  Bytes", sz) ;
        lib_msg_add(dssstr->q330, AUXMSG_DSS, 0, addr(s)) ;
      end
  smallpt = scan (dssstr, sz, addr(smallest)) ; /* check for memory in free list */
  if (smallpt == NIL)
    then
      begin /* nothing found there */
        msize = DEF_CHUNKSIZE ;
        if (msize > (dssstr->mem_allowed - dssstr->total))
          then
            msize = dssstr->mem_allowed - dssstr->total ;
        if (msize < 1024)
          then
            return NIL ; /* can't allocate a usable amount */
        getbuf (dssstr->q330, addr(mpt), msize) ;
        incn(dssstr->total, msize) ;
        if (dssstr->verbosity >= RPT_ALLOC)
          then
            begin
              sprintf(s, "Total DSS Memory=%d", dssstr->total) ;
              lib_msg_add(dssstr->q330, AUXMSG_DSS, 0, addr(s)) ;
            end
        mscan = dssstr->free ;
        while (mscan) /* check for contiguous with other free memory */
          begin
            mend = (pointer)((uninteger)mscan + mscan->size) ;
            if (mend == mpt)
              then
                begin /* mpt follows mscan */
                  mscan->size = mscan->size + msize ;
                  mpt = NIL ; /* used up */
                  break ;
                end
            else if ((pointer)((uninteger)mpt + msize) == mscan)
              then
                begin /* mpt preceeds mscan */
                  if (mscan->prev)
                    then
                      mscan->prev->next = mpt ;
                  if (mscan->next)
                    then
                      mscan->next->prev = mpt ;
                  mpt->size = msize + mscan->size ;
                  mpt->next = mscan->next ;
                  mpt->prev = mscan->prev ;
                  mpt = NIL ;
                  break ;
                end
            mscan = mscan->next ;
          end
        if (mpt)
          then /* not contiguous, add to beginning of free list */
            begin
              if (dssstr->free == NIL)
                then
                  mpt->next = NIL ;
                else
                  begin
                    dssstr->free->prev = mpt ;
                    mpt->next = dssstr->free ;
                  end
              dssstr->free = mpt ;
              mpt->size = msize ;
              mpt->prev = NIL ;
            end
        smallpt = scan (dssstr, sz, addr(smallest)) ; /* now check, should be enough now */
      end
  if (smallpt == NIL)
    then
      return NIL ; /* guess not */
  ret = smallpt ; /* before we skip what we just allocated */
  oldnext = smallpt->next ;
  oldprev = smallpt->prev ;
  oldsize = smallpt->size ;
  memset (smallpt, 0, sz) ; /* clear header and block */
  if ((smallest - sz) >= MIN_FRAG)
    then
      begin /* keep leftover fragment */
        smallpt->size = sz ; /* new size of this segment */
        smallpt = (pointer)((uninteger)smallpt + sz) ; /* memory left over */
        smallpt->size = oldsize - sz ;
        smallpt->next = oldnext ;
        smallpt->prev = oldprev ;
        if (oldprev)
          then
            oldprev->next = smallpt ;
          else
            dssstr->free = smallpt ;
        if (oldnext)
          then
            oldnext->prev = smallpt ;
      end
    else
      begin /* use all of memory fragment */
        smallpt->size = oldsize ; /* restore size after clearing block */
        if (oldprev)
          then
            oldprev->next = oldnext ;
          else
            dssstr->free = oldnext ;
        if (oldnext)
          then
            oldnext->prev = oldprev ;
      end
  return ret ;
end

static void count_blocks (pdssstr dssstr)
begin
  pmemory mscan ;
  integer count, total ;
  string95 s ;

  count = 0 ;
  total = 0 ;
  mscan = dssstr->free ;
  while (mscan)
    begin
      inc(count) ;
      total = total + mscan->size ;
      mscan = mscan->next ;
    end
  sprintf(s, "%d DSS Free Blocks with size of %d", count, total) ;
  lib_msg_add(dssstr->q330, AUXMSG_DSS, 0, addr(s)) ;
end

/* return memory segment to free list, merging if possible */
static void mem_free (pdssstr dssstr, pmemory pt)
begin
  pmemory mscan, mend, previous ;
  string63 s ;

  if (dssstr->verbosity >= RPT_ALLMEM)
    then
      begin
        sprintf(s, "DSS Memory Release for %d  Bytes", pt->size) ;
        lib_msg_add(dssstr->q330, AUXMSG_DSS, 0, addr(s)) ;
      end
 /* contract linked list */
  if (pt->prev) /* this should always be non-nil */
    then
      pt->prev->next = pt->next ;
  if (pt->next)
    then
      pt->next->prev = pt->prev ;
  previous = NIL ;
 /* see if adjoins existing segment */
  mend = (pointer)((uninteger)pt + pt->size) ; /* end of this segment */
  mscan = dssstr->free ;
  while (mscan)
    begin
      if (mscan == mend)
        then
          begin /* returned segment just before scanned segment */
            if (mscan->prev)
              then
                mscan->prev->next = pt ;
              else
                dssstr->free = pt ;
            if (mscan->next)
              then
                mscan->next->prev = pt ;
            pt->size = pt->size + mscan->size ;
            pt->next = mscan->next ;
            pt->prev = mscan->prev ;
            return ;
          end
      else if ((pointer)((uninteger)mscan + mscan->size) == pt)
        then
          begin /* returned segment follows scanned segment */
            mscan->size = mscan->size + pt->size ;
            return ;
          end
      else if ((uninteger)mscan < (uninteger)pt)
        then
          previous = mscan ;
      mscan = mscan->next ;
    end
  /* if it gets here, it is not contiguous with another segment,
    add it to the free list */
  if (previous)
    then
      begin
        pt->next = previous->next ;
        if (previous->next)
          then
            previous->next->prev = pt ;
        previous->next = pt ;
        pt->prev = previous ;
      end
    else
      begin /* beginning of list */
        if (dssstr->free == NIL)
          then
            pt->next = NIL ;
          else
            begin
              dssstr->free->prev = pt ;
              pt->next = dssstr->free ;
            end
        dssstr->free = pt ;
        pt->prev = NIL ;
      end
  if (dssstr->verbosity >= RPT_ALLMEM)
    then
      count_blocks (dssstr) ;
end

static void compact_memory (pdssstr dssstr)
begin
  pmemory mscan, mend, mnext ;

  mscan = dssstr->free ;
  while ((mscan) land (mscan->next))
    begin
      mend = (pointer)((uninteger)mscan + mscan->size) ;
      mnext = mscan->next ;
      if (mend == mnext)
        then
          begin /* merge them */
            mscan->next = mnext->next ;
            mscan->size = mscan->size + mnext->size ;
          end
        else
          mscan = mnext ; /* try next pair */
    end
  if (dssstr->verbosity >= RPT_ALLMEM)
    then
      count_blocks (dssstr) ;
end

static void close_socket (pdssstr dssstr)
begin

  dssstr->dss_server_display[0] = 0 ;
  dssstr->sockopen = FALSE ;
  if (dssstr->q330->dsspath != INVALID_SOCKET)
    then
#ifdef X86_WIN32
      closesocket (dssstr->q330->dsspath) ;
#else
      close (dssstr->q330->dsspath) ;
#endif
  dssstr->q330->dsspath = INVALID_SOCKET ;
end

static void open_socket (pdssstr dssstr)
begin
  integer lth ;
  integer err, flags ;
  longint flag ;
  string63 s ;
  struct sockaddr xyz ;
  struct sockaddr_in *psock ;

  close_socket (dssstr) ;
  dssstr->q330->dsspath = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP) ;
  if (dssstr->q330->dsspath == INVALID_SOCKET)
    then
      begin
        err =
#ifdef X86_WIN32
               WSAGetLastError() ;
#else
               errno ;
#endif
        sprintf(s, "%d on DSS", err) ;
        lib_msg_add(dssstr->q330, AUXMSG_SOCKETERR, 0, addr(s)) ;
        sprintf(s, "DSS Socket Error: %d", err) ;
        strcpy (addr(dssstr->dss_server_display), addr(s)) ;
        return ;
      end
#ifndef X86_WIN32
  if (dssstr->q330->dsspath > dssstr->q330->high_socket)
    then
      dssstr->q330->high_socket = dssstr->q330->dsspath ;
#endif
  psock = (pointer) addr(dssstr->dsockin) ;
  memset(psock, 0, sizeof(struct sockaddr)) ;
  psock->sin_family = AF_INET ;
  if (dssstr->dss_par.port_number == PORT_OS)
    then
      psock->sin_port = 0 ;
    else
      psock->sin_port = htons(dssstr->dss_par.port_number) ;
  psock->sin_addr.s_addr = INADDR_ANY ;
#ifdef X86_WIN32
  err = bind(dssstr->q330->dsspath, addr(dssstr->dsockin), sizeof(struct sockaddr)) ;
  if (err)
#else
  err = bind(dssstr->q330->dsspath, addr(dssstr->dsockin), sizeof(struct sockaddr)) ;
  if (err)
#endif
    then
      begin
        err =
#ifdef X86_WIN32
               WSAGetLastError() ;
        closesocket (dssstr->q330->dsspath) ;
#else
               errno ;
        close (dssstr->q330->dsspath) ;
#endif
        dssstr->q330->dsspath = INVALID_SOCKET ;
        sprintf(s, "%d on DSS", err) ;
        lib_msg_add(dssstr->q330, AUXMSG_BINDERR, 0, addr(s)) ;
        sprintf(s, "DSS Bind Error: %d", err) ;
        strcpy (addr(dssstr->dss_server_display), addr(s)) ;
        return ;
      end
#ifdef X86_WIN32
  flag = 1 ;
  ioctlsocket (dssstr->q330->dsspath, FIONBIO, addr(flag)) ;
#else
  flags = fcntl (dssstr->q330->dsspath, F_GETFL, 0) ;
  fcntl (dssstr->q330->dsspath, F_SETFL, flags or O_NONBLOCK) ;  
#endif
  lth = sizeof(struct sockaddr) ;
  getsockname (dssstr->q330->dsspath, addr(xyz), addr(lth)) ;
  psock = (pointer) addr(xyz) ;
  dssstr->host_port = ntohs(psock->sin_port) ;
  sprintf(s, "on DSS port %d", dssstr->host_port) ;
  lib_msg_add(dssstr->q330, AUXMSG_SOCKETOPEN, 0, addr(s)) ;
  sprintf(s, "DSS Open on Port: %d", dssstr->host_port) ;
  strcpy(addr(dssstr->dss_server_display), addr(s)) ;
  dssstr->sockopen = TRUE ;
end

static void remove_report (pdssstr dssstr, pdss_report prep)
begin

  if (prep->access)
    then
      mem_free (dssstr, prep->access) ;
  mem_free (dssstr, addr(prep->memory)) ;
end

static void remove_client (pdssstr dssstr, pdss_client pcli)
begin
  pdss_report prep1, prep2 ;
  integer cp ;
  string63 s ;

  /* remove all reports */
  prep1 = pcli->head ;
  while (prep1)
    begin
      prep2 = prep1 ;
      prep1 = (pdss_report)(prep1->memory.next) ;
      remove_report (dssstr, prep2) ;
    end
  if (dssstr->verbosity >= RPT_CLIENTS)
    then
      begin
        sprintf(s, "Removing Client ""%s""", addr(pcli->name)) ;
        lib_msg_add(dssstr->q330, AUXMSG_DSS, 0, addr(s)) ;
      end
  /* if head of list, change head */
  for (cp = 0 ; cp <= 2 ; cp++)
    if (dssstr->clients[cp] == pcli)
      then
        begin
          dssstr->clients[cp] = (pdss_client)((dssstr->clients[cp])->memory.next) ;
          break ;
        end
  /* remove memory */
  mem_free (dssstr, addr(pcli->memory)) ;
  compact_memory (dssstr) ; /* good time to clean up free list */
end

static void storedsshdr (pbyte *p, tqdp *hdr)
begin

  storelongint (p, 0) ; /* for now */
  storebyte (p, hdr->command) ;
  storebyte (p, hdr->version) ;
  storeword (p, hdr->datalength) ;
  if (hdr->version < QDP_VERSION)
    then
      begin /* old format */
        storebyte (p, hdr->sequence) ;
        storebyte (p, hdr->acknowledge) ;
        storeword (p, 0) ;
      end
    else
      begin
        storeword (p, hdr->sequence) ;
        storeword (p, hdr->acknowledge) ;
      end
end

static void loaddsshdr (pbyte *p, tqdp *hdr)
begin

  hdr->crc = loadlongint (p) ;
  hdr->command = loadbyte (p) ;
  hdr->version = loadbyte (p) ;
  hdr->datalength = loadword (p) ;
  if (hdr->version < QDP_VERSION)
    then
      begin /* old format */
        hdr->sequence = loadbyte (p) ;
        hdr->acknowledge = loadbyte (p) ;
        loadword (p) ; /* skip spare */
      end
    else
      begin
        hdr->sequence = loadword (p) ;
        hdr->acknowledge = loadword (p) ;
      end
end

static void send_msg (pdssstr dssstr)
begin
  word msglth ;
  pbyte p ;
  integer err ;
  string63 s ;

  dssstr->sendhdr.version = dssstr->cur_ver ;
  if (dssstr->sendhdr.command == DSS_DAT)
    then
      begin
        dssstr->sendhdr.sequence = 0 ;
        dssstr->sendhdr.acknowledge = 0 ;
      end
    else
      begin
        dssstr->sendhdr.sequence = dssstr->reply_sequence ;
        inc(dssstr->reply_sequence) ;
        dssstr->sendhdr.acknowledge = dssstr->recvhdr.sequence ;
      end
  msglth = 12 + dssstr->sendhdr.datalength ;
  p = addr(dssstr->msgout) ;
  storedsshdr (addr(p), addr(dssstr->sendhdr)) ;
  dssstr->sendhdr.crc = gcrccalc (addr(dssstr->q330->crc_table), (pointer)((integer)addr(dssstr->msgout) + 4), msglth - 4) ;
  p = addr(dssstr->msgout) ;
  storelongint (addr(p), dssstr->sendhdr.crc) ;
  incn(dssstr->bytecount, (longint)msglth) ;
  err = sendto(dssstr->q330->dsspath, addr(dssstr->msgout), msglth, 0, addr(dssstr->sock), sizeof(struct sockaddr)) ;
  if (err == SOCKET_ERROR)
    then
      begin
        err =
#ifdef X86_WIN32
              WSAGetLastError() ;
#else
              errno ;
#endif
        sprintf (s, "ERROR: DSS Cannot send, removing client, error code %d", err) ;
        lib_msg_add(dssstr->q330, AUXMSG_DSS, 0, addr(s)) ;
        if (dssstr->cur_client)
          then
            remove_client (dssstr, dssstr->cur_client) ;
      end
end

static void ack (pdssstr dssstr)
begin

  dssstr->sendhdr.command = DSS_ACK ;
  dssstr->sendhdr.datalength = 0 ;
  send_msg (dssstr) ;
end

static void refuse (pdssstr dssstr, int16 errnum)
begin
  pbyte p ;

  dssstr->refused = TRUE ;
  p = addr(dssstr->msgout.buffer) ;
  dssstr->sendhdr.command = DSS_REF ;
  dssstr->sendhdr.datalength = 2 ;
  storeword (addr(p), errnum) ;
  send_msg (dssstr) ;
end

static char decode_character (byte w)
begin
  char dc ;

  w = w and 0x3F ; /* 6 bits only */
  if (w <= 9)
    then
      dc = (char)(w + 0x30) ; /* map 0x0 - 0x9 to '0' to '9' */
  else if (w <= 0x23)
    then
      dc = (char)(w + 0x37) ; /* map 0xA - 0x23 to 'A' to 'Z' */
  else if (w <= 0x3D)
    then
      dc = (char)(w + 0x3D) ; /* map 0x24 - 0x3D to 'a' to 'z' */
  else if (w == 0x3E)
    then
      dc = '-' ;
    else
      dc = '_' ;
  return dc ;
end

static void load_req (pbyte *p, tdss_req *req)
begin
  longword lw ;
  integer i ;

  req->data_id = loadword (p) ;
  req->rep_prio = loadword (p) ;
  req->interval = loadlongint (p) ;
  req->reqcode = loadbyte (p) ;
  req->repflag = loadbyte (p) ;
  lw = loadlongword (p) ;
  if (lw and 0x80000000)
    then
      begin /* encoded */
        for (i = 4 ; i >= 0 ; i--)
          begin
            req->station[i] = decode_character(lw) ;
            lw = lw shr 6 ;
          end
      end
    else
      begin /* 4 character station */
        decn(*p, 4) ; /* backup to station name */
        memcpy (addr(req->station), *p, 4) ; /* first 4 */
        incn(*p, 4) ; /* no endian conversion */
        req->station[4] = ' ' ; /* last character always space */
      end
end

static void load_locseed (pbyte *p, tlocation *loc, tseed_name *sname)
begin

  loadblock (p, 2, loc) ;
  loadblock (p, 3, sname) ;
end

static pdss_client find_client (pdssstr dssstr)
begin
  pdss_client pcli ;
  integer cp ;
  struct sockaddr_in *psock ;

  for (cp = 2 ; cp >= 0 ; cp--)
    begin
      pcli = dssstr->clients[cp] ;
      dssstr->client_priority = cp ;
      psock = addr(dssstr->sock) ;
      while (pcli)
        begin
          if ((pcli->ipaddr == psock->sin_addr.s_addr) land (pcli->port == psock->sin_port))
            then
              begin
                dssstr->cur_client = pcli ;
                dssstr->cur_ver = pcli->qdpver ;
                return pcli ;
              end
          pcli = (pdss_client)(pcli->memory.next) ;
        end
    end
  return NIL ; /* not found */
end

static void register_client (pdssstr dssstr)
begin
  integer i, prio ;
  string7 pass ;
  string250 s ;
  string63 s2 ;
  string15 s3 ;
  pdss_client pcli ;
  pbyte p ;
  struct sockaddr_in *psock ;

  dssstr->cur_ver = dssstr->msgin.qdp.version ;
  dssstr->cur_client = NIL ;
  prio = -1 ;
  p = addr(dssstr->msgin.buffer) ;
  loadstring (addr(p), 8, addr(pass)) ;
  if (strlen(pass) > 7)
    then
      pass[7] = 0 ;
  lib330_upper(addr(pass)) ;
  loadstring (addr(p), 250, addr(s)) ;
  if (strlen(s) > 79)
    then
      s[79] = 0 ;
  for (i = 0 ; i <= 2 ; i++)
    if (strcmp(addr(pass), addr(dssstr->passwords[i])) == 0)
      then
        begin
          prio = i ;
          break ;
        end
  if (prio < 0)
    then
      begin
        refuse (dssstr, REF_INP) ; /* invalid password */
        return ;
      end
  pcli = find_client (dssstr) ;
  psock = addr(dssstr->sock) ;
  if (pcli == NIL)
    then
      begin /* new client */
        if (dssstr->verbosity >= RPT_CLIENTS)
          then
            begin
              showdot(ntohl(psock->sin_addr.s_addr), addr(s3)) ;
              sprintf(s2, "New Client ""%s"" from %s:%d", s, s3, (integer)ntohs(psock->sin_port)) ;
              lib_msg_add(dssstr->q330, AUXMSG_DSS, 0, addr(s2)) ;
            end
        pcli = (pdss_client)(memreq(dssstr, sizeof(dss_client))) ;
        if (pcli == NIL)
          then
            begin
              refuse (dssstr, REF_OOM) ;
              return ;
            end
        pcli->ipaddr = psock->sin_addr.s_addr ;
        pcli->port = psock->sin_port ;
        pcli->timer = 0 ;
        strcpy(addr(pcli->name), s) ;
        pcli->qdpver = dssstr->recvhdr.version ;
        pcli->head = NIL ;
        if (dssstr->clients[prio])
          then
            begin
              dssstr->clients[prio]->memory.prev = (pointer)pcli ;
              pcli->memory.next = (pmemory)(dssstr->clients[prio]) ;
            end
        dssstr->clients[prio] = pcli ; /* new head of list */
        pcli->memory.prev = (pmemory)pcli ;
        dssstr->client_priority = prio ;
        dssstr->cur_client = pcli ;
        dssstr->cur_ver = pcli->qdpver ;
        ack (dssstr) ;
      end
    else
      ack (dssstr) ; /* already there */
end

static void show_ident (pdss_report prep, string63 *res)
begin
  string7 s, s2 ;
  integer i ;

  s[0] = 0 ;
  for (i = 0 ; i <= 4 ; i++)
    if (prep->station[i] != ' ')
      then
        begin
          s[i] = prep->station[i] ;
          s[i + 1] = 0 ;
        end
      else
        break ;
  seed2string (prep->location, prep->channel, addr(s2)) ;
  sprintf(res, "%s.%s", s, s2) ;
end

static void list_requests (pdssstr dssstr, pdss_client pcli)
begin
  pdss_report prep ;
  pbyte p ;
  word cnt ;

  p = addr(dssstr->msgout.buffer) ;
  dssstr->sendhdr.command = DSS_RQL ;
  cnt = 0 ;
  storeword (addr(p), 0) ; /* temporary count */
  prep = pcli->head ;
  while (prep)
    begin
      storeword (addr(p), prep->data_id) ;
      if (prep->flag and FLAG_GOOD)
        then
          storeword (addr(p), 0) ;
        else
          storeword (addr(p), OFF_DNA) ;
      inc(cnt) ;
      prep = (pdss_report)(prep->memory.next) ;
    end
  p = addr(dssstr->msgout.buffer) ;
  storeword (addr(p), cnt) ; /* actual count */
  dssstr->sendhdr.datalength = 2 + cnt * 4 ;
  send_msg (dssstr) ;
end

static void del_requests (pdssstr dssstr, pdss_client pcli)
begin
  int16 i, count ;
  pdss_report prep ;
  word id ;
  pbyte p ;

  p = addr(dssstr->msgin.buffer) ;
  count = loadint16 (addr(p)) ;
  for (i = 0 ; i <= count - 1 ; i++)
    begin
      id = loadword (addr(p)) ;
      prep = pcli->head ;
      while (prep)
        if (prep->data_id == id)
          then
            begin
              if (prep == pcli->head)
                then
                  pcli->head = (pdss_report)(prep->memory.next) ;
              remove_report (dssstr, prep) ;
              break ;
            end
          else
            prep = (pdss_report)(prep->memory.next) ;
    end
  ack (dssstr) ;
end

static plcq find_lcq (pdssstr dssstr, pdss_report prep)
begin
  plcq q ;
  paqstruc paqs ;

  paqs = dssstr->q330->aqstruc ;
  if (paqs == NIL)
    then
      return NIL ; /* not connected */
  q = paqs->lcqs ;
  while (q)
    if ((memcmp(addr(prep->location), addr(q->location), sizeof(tlocation)) == 0) land
        (memcmp(addr(prep->channel), addr(q->seedname), sizeof(tseed_name)) == 0))
      then
        return q ;
      else
        q = q->link ;
  return NIL ;
end

static void setbad (pdss_report prep)
begin

  prep->flag = (prep->flag or FLAG_WAIT) and not FLAG_GOOD ;
  if (((prep->flag and FLAG_CNTDN) == 0) land (prep->repflag != FLG_NEVER))
    then
      begin
        prep->repcount = prep->repflag ;
        prep->flag = prep->flag or FLAG_CNTDN ;
      end
end

static void setgood (pdss_report prep, pmemory acc)
begin

  prep->access = acc ;
  prep->flag = (prep->flag or FLAG_GOOD) and FLAG_MASK ;
  if (prep->interval == 0)
    then
      prep->counter = 1 ;
    else
      prep->counter = prep->interval ;
end

static void recon_det (pdssstr dssstr, pdss_report prep)
begin
  pdet_packet pdet ;
  tdetcnt *pdcnt ;
  plcq q ;
  con_common *pcom ;
  string63 s ;

  setbad (prep) ;
  q = find_lcq (dssstr, prep) ;
  if (q == NIL)
    then
      begin
        refuse (dssstr, REF_IDS) ;
        remove_report (dssstr, prep) ;
        return ;
      end
  pdet = q->det ;
  while (pdet)
    begin
      strcpy(s, addr(pdet->detector_def->detname)) ;
      lib330_upper(addr(s)) ;
      if (strcmp(s, addr(dssstr->detname)) == 0)
      then
        break ;
      else
        pdet = pdet->link ;
    end
  if (pdet == NIL)
    then
      begin
        refuse (dssstr, REF_IDS) ;
        remove_report (dssstr, prep) ;
        return ;
      end
  pdcnt = (pointer)(memreq(dssstr, sizeof(tdetcnt))) ;
  if (pdcnt == NIL)
    then
      begin
        refuse (dssstr, REF_OOM) ;
        remove_report (dssstr, prep) ;
        return ;
      end
  pdcnt->det = pdet ;
  pcom = pdet->cont ;
  pdcnt->current = pcom->total_detections ;
  setgood (prep, addr(pdcnt->memory)) ;
end

static void recon_rec (pdssstr dssstr, pdss_report prep)
begin
  treccnt *prcnt ;
  plcq q ;

  setbad (prep) ;
  q = find_lcq (dssstr, prep) ;
  if (q == NIL)
    then
      begin
        refuse (dssstr, REF_IDS) ;
        remove_report (dssstr, prep) ;
        return ;
      end
  prcnt = (pointer)(memreq(dssstr, sizeof(treccnt))) ;
  if (prcnt == NIL)
    then
      begin
        refuse (dssstr, REF_OOM) ;
        remove_report (dssstr, prep) ;
        return ;
      end
  prcnt->lcq = q ;
  prcnt->current = q->com->records_written ;
  setgood (prep, addr(prcnt->memory)) ;
end

static boolean format_good (byte fmt)
begin

  switch (fmt and 7) begin
    case FMT_I16 :
    case FMT_I32 :
    case FMT_F32 :
    case FMT_F64 :
      return TRUE ;
    default :
      return FALSE ;
  end
end

void recon_simp (pdssstr dssstr, pdss_report prep)
begin
  plcq q ;
  treccnt *pl ;

  setbad (prep) ;
  if (lnot format_good(prep->format))
    then
      begin
        refuse (dssstr, REF_IDF) ;
        remove_report (dssstr, prep) ;
        return ;
      end
  q = find_lcq (dssstr, prep) ;
  if (q == NIL)
    then
      begin
        refuse (dssstr, REF_IDS) ;
        remove_report (dssstr, prep) ;
        return ;
      end
  pl = (pointer)(memreq(dssstr, sizeof(treccnt))) ;
  if (pl == NIL)
    then
      begin
        refuse (dssstr, REF_OOM) ;
        remove_report (dssstr, prep) ;
        return ;
      end
  pl->lcq = q ;
  setgood (prep, addr(pl->memory)) ;
end

static void recon_mma (pdssstr dssstr, pdss_report prep)
begin
  plcq q ;
  tmma *pmma ;

  prep->flag = (prep->flag and not FLAG_INT) or FLAG_CON ; /* not interval report */
  setbad (prep) ;
  if (lnot format_good(prep->format and 7))
    then
      begin
        refuse (dssstr, REF_IDF) ;
        remove_report (dssstr, prep) ;
        return ;
      end
  if (prep->seconds > prep->interval)
    then
      begin
        refuse (dssstr, REF_SEI) ;
        remove_report (dssstr, prep) ;
        return ;
      end
  q = find_lcq (dssstr, prep) ;
  if (q == NIL)
    then
      begin
        refuse (dssstr, REF_IDS) ;
        remove_report (dssstr, prep) ;
        return ;
      end
  pmma = (pointer)(memreq(dssstr, sizeof(tmma))) ;
  if (pmma == NIL)
    then
      begin
        refuse (dssstr, REF_OOM) ;
        remove_report (dssstr, prep) ;
        return ;
      end
  pmma->minimum = MAXLINT ;
  pmma->maximum = -MAXLINT ;
  pmma->average = 0.0 ;
  pmma->seccount = 0 ;
  pmma->acc = TRUE ;
  pmma->lcq = q ;
  setgood (prep, addr(pmma->memory)) ;
end

static void recon_sqr (pdssstr dssstr, pdss_report prep)
begin
  plcq q ;
  tsqr *psqr ;

  prep->flag = (prep->flag and not FLAG_INT) or FLAG_CON ; /* not interval report */
  setbad (prep) ;
  if (lnot format_good(prep->format and 7))
    then
      begin
        refuse (dssstr, REF_IDF) ;
        remove_report (dssstr, prep) ;
        return ;
      end ;
  if (prep->seconds > prep->interval)
    then
      begin
        refuse (dssstr, REF_SEI) ;
        remove_report (dssstr, prep) ;
        return ;
      end
  q = find_lcq (dssstr, prep) ;
  if (q == NIL)
    then
      begin
        refuse (dssstr, REF_IDS) ;
        remove_report (dssstr, prep) ;
        return ;
      end
  psqr = (pointer)(memreq(dssstr, sizeof(tsqr))) ;
  if (psqr == NIL)
    then
      begin
        refuse (dssstr, REF_OOM) ;
        remove_report (dssstr, prep) ;
        return ;
      end
  psqr->sum = 0.0 ;
  psqr->average = 0.0 ;
  psqr->seccount = 0 ;
  psqr->acc = TRUE ;
  psqr->lcq = q ;
  setgood (prep, addr(psqr->memory)) ;
end

static void add_request (pdssstr dssstr, pdss_client pcli)
begin
  pdss_report prep, newreq, lastpt ;
  string63 s, s2 ;
  pbyte p ;
  tdss_req req ;
  boolean locseed ;

  p = addr(dssstr->msgin.buffer) ;
  load_req (addr(p), addr(req)) ;
  if ((req.reqcode != REQ_BOOT) land
      (memcmp(addr(req.station), addr(dssstr->q330->station), sizeof(tseed_stn))))
    then
      begin /* not for this station */
        refuse (dssstr, REF_IDS) ;
        return ;
      end
  prep = pcli->head ;
  lastpt = pcli->head ;
  /* check for duplicate first, noting next highest priority */
  while (prep)
    begin
      if (prep->data_id == req.data_id)
        then
          begin /* duplicate */
            refuse (dssstr, REF_DUP) ;
            return ;
          end
      if (prep->rep_prio > req.rep_prio)
        then
          lastpt = prep ;
      prep = (pdss_report)(prep->memory.next) ;
    end
  newreq = (pdss_report)(memreq(dssstr, sizeof(dss_report))) ;
  if (newreq == NIL)
    then
      begin
        refuse (dssstr, REF_OOM) ;
        return ;
      end
  newreq->priority = dssstr->client_priority ;
  newreq->flag = FLAG_INT ; /* assume everything good */ ;
  /* insert into list */
  newreq->memory.prev = (pmemory)lastpt ;
  if (lastpt)
    then
      begin
        newreq->memory.next = lastpt->memory.next ;
        if (lastpt->memory.next)
          then
            lastpt->memory.next->prev = (pmemory)newreq ;
        lastpt->memory.next = (pmemory)newreq ;
      end
    else
      begin /* beginning of list */
        newreq->memory.next = (pmemory)(pcli->head) ;
        if (pcli->head)
          then
            pcli->head->memory.prev = (pmemory)newreq ;
        pcli->head = newreq ;
      end
  /* copy in actions */
  if (req.interval == 0)
    then
      newreq->counter = 1 ;
    else
      newreq->counter = req.interval ;
  newreq->data_id = req.data_id ;
  newreq->rep_prio = req.rep_prio ;
  newreq->interval = req.interval ;
  newreq->reqcode = req.reqcode ;
  newreq->repflag = req.repflag ;
  memcpy(addr(newreq->station), addr(req.station), sizeof(tseed_stn)) ;
  switch (req.reqcode) begin
    case REQ_SIMP :
    case REQ_DET :
    case REQ_REC :
    case REQ_MMA :
    case REQ_SQR :
      locseed = TRUE ;
      break ;
    default :
      locseed = FALSE ;
      break ;
  end
  if (locseed)
    then
      begin /* need location and seedname */
        load_locseed (addr(p), addr(newreq->location), addr(newreq->channel)) ;
        newreq->format = loadbyte (addr(p)) ;
        if (req.reqcode == REQ_DET)
          then
            begin
              loadstring (addr(p), 250, addr(dssstr->detname)) ;
              lib330_upper(addr(dssstr->detname)) ;
            end
        else if ((req.reqcode == REQ_MMA) lor (req.reqcode == REQ_SQR))
          then
            newreq->seconds = loadlongint (addr(p)) ;
      end
  if (dssstr->verbosity >= RPT_REPORTS)
    then
      begin
        switch (newreq->reqcode) begin
          case REQ_SIMP : strcpy (s, "Simple Data Report") ; break ;
          case REQ_GPS : strcpy (s, "GPS Location Report") ; break ;
          case REQ_DEAD : strcpy (s, "Dead Channels Report") ; break ;
          case REQ_DET : strcpy (s, "Detection Count Report") ; break ;
          case REQ_REC : strcpy (s, "Record Count Report") ; break ;
          case REQ_BOOT : strcpy (s, "Seconds since Boot Report") ; break ;
          case REQ_MMA : strcpy (s, "Min/Max/Avg Report") ; break ;
          case REQ_SQR : strcpy (s, "Squares/Avg Report") ; break ;
          default : strcpy (s, "") ; break ;
        end
        if (locseed)
          then
            begin
              show_ident (newreq, addr(s2)) ;
              strcat(s, " on ") ;
              strcat(s, s2) ;
            end
        if (newreq->reqcode == REQ_DET)
          then
            begin
              strcat(s, ":") ;
              strcat(s, addr(dssstr->detname)) ;
            end
        sprintf(s2, "Adding %s for client %s", s, addr(pcli->name)) ;
        lib_msg_add(dssstr->q330, AUXMSG_DSS, 0, addr(s2)) ;
      end
  /* try to reconnect now */
  switch (newreq->reqcode) begin
    case REQ_BOOT :
    case REQ_GPS :
    case REQ_DEAD :
      newreq->flag = FLAG_INT or FLAG_GOOD ;
      break ;
    case REQ_DET :
      recon_det (dssstr, newreq) ;
      break ;
    case REQ_REC :
      recon_rec (dssstr, newreq) ;
      break ;
    case REQ_SIMP :
      recon_simp (dssstr, newreq) ;
      break ;
    case REQ_MMA :
      recon_mma (dssstr, newreq) ;
      break ;
    case REQ_SQR :
      recon_sqr (dssstr, newreq) ;
      break ;
  end
  if (lnot dssstr->refused)
    then
      ack (dssstr) ; /* if it got here, it must have been ok */
end

static void store_long (pbyte *p, longint *dlth, byte format, longint l)
begin
  int16 i ;
  single r ;
  double d ;
  longword *plw ;

  switch (format and 7) begin
    case FMT_I16 :
      if (l > 32767)
        then
          i = 32767 ;
      else if (l < -32767)
        then
          i = -32767 ;
        else
          i = l ;
      storeint16 (p, i) ;
      incn(*dlth, 2) ;
      break ;
    case FMT_I32 :
      storelongint (p, l) ;
      incn(*dlth, 4) ;
      break ;
    case FMT_F32 :
      r = l ; /* make float */
      storesingle (p, r) ;
      incn(*dlth, 4) ;
      break ;
    case FMT_F64 :
      d = l ; /* make longreal */
      plw = addr(d) ;
#ifdef DOUBLE_HYBRID_ENDIAN
      storelongword (p, *plw) ;
      inc(plw) ;
      storelongword (p, *plw) ;
#else
#ifdef ENDIAN_LITTLE
      inc(plw) ;
      storelongword (p, *plw) ;
      plw = addr(d) ;
      storelongword (p, *plw) ;
#else
      storelongword (p, *plw) ;
      inc(plw) ;
      storelongword (p, *plw) ;
#endif
#endif
      incn(*dlth, 8) ;
      break ;
  end
end

void store_single (pbyte *p, longint *dlth, byte format, single r)
begin
  int16 i ;
  longint l ;
  double d ;
  longword *plw ;

  switch (format and 7) begin
    case FMT_I16 :
      if (r > 32767.0)
        then
          i = 32767 ;
      else if (r < -32767.0)
        then
          i = -32767 ;
        else
          i = lib_round(r) ;
      storeint16 (p, i) ;
      incn(*dlth, 2) ;
      break ;
    case FMT_I32 :
      if (r > 2147483647.0)
        then
          l = MAXLINT ;
      else if (r < -2147483647.0)
        then
          l = -MAXLINT ;
        else
          l = lib_round(r) ;
      storelongint (p, l) ;
      incn(*dlth, 4) ;
      break ;
    case FMT_F32 :
      storesingle (p, r) ;
      incn(*dlth, 4) ;
      break ;
    case FMT_F64 :
      d = r ; /* make longreal */
      plw = addr(d) ;
#ifdef DOUBLE_HYBRID_ENDIAN
      storelongword (p, *plw) ;
      inc(plw) ;
      storelongword (p, *plw) ;
#else
#ifdef ENDIAN_LITTLE
      inc(plw) ;
      storelongword (p, *plw) ;
      plw = addr(d) ;
      storelongword (p, *plw) ;
#else
      storelongword (p, *plw) ;
      inc(plw) ;
      storelongword (p, *plw) ;
#endif
#endif
      incn(*dlth, 8) ;
      break ;
  end
end

static void store_double (pbyte *p, longint *dlth, byte format, double r)
begin
  int16 i ;
  longint l ;
  single s ;
  longword *plw ;

  switch (format and 7) begin
    case FMT_I16 :
      if (r > 32767.0)
        then
          i = 32767 ;
      else if (r < -32767.0)
        then
          i = -32767 ;
        else
          i = lib_round(r) ;
      storeint16 (p, i) ;
      incn(*dlth, 2) ;
      break ;
    case FMT_I32 :
      if (r > 2147483647.0)
        then
          l = MAXLINT ;
      else if (r < -2147483647.0)
        then
          l = -MAXLINT ;
        else
          l = lib_round(r) ;
      storelongint (p, l) ;
      incn(*dlth, 4) ;
      break ;
    case FMT_F32 :
      if (r > 3.4E38)
        then
          s = 3.4E38 ;
      else if (r < -3.4E38)
        then
          s = -3.4E38 ;
        else
          s = r ; /* convert to single precision */
      storesingle (p, s) ;
      incn(*dlth, 4) ;
      break ;
    case FMT_F64 :
      plw = addr(r) ;
#ifdef DOUBLE_HYBRID_ENDIAN
      storelongword (p, *plw) ;
      inc(plw) ;
      storelongword (p, *plw) ;
#else
#ifdef ENDIAN_LITTLE
      inc(plw) ;
      storelongword (p, *plw) ;
      plw = addr(r) ;
      storelongword (p, *plw) ;
#else
      storelongword (p, *plw) ;
      inc(plw) ;
      storelongword (p, *plw) ;
#endif
#endif
      incn(*dlth, 8) ;
      break ;
  end
end

void lib_dss_read (pointer ct)
begin
  pdssstr dssstr ;
  longint expcrc ;
  pdss_client pcli ;
  integer err, lth ;
  pbyte p ;
  string63 s ;

  dssstr = ct ;
  lth = sizeof(struct sockaddr) ;
  dssstr->refused = FALSE ;
  err = recvfrom (dssstr->q330->dsspath, addr(dssstr->msgin), sizeof(tdss_msg), 0, addr(dssstr->sock), addr(lth)) ;
  if (err == SOCKET_ERROR)
    then
      begin
        err =
#ifdef X86_WIN32
              WSAGetLastError() ;
#else
              errno ;
#endif
        if ((err != EWOULDBLOCK) land (err != ECONNRESET))
          then
            begin
              sprintf (s, "ERROR: DSS Receive Error %d", err) ;
              lib_msg_add(dssstr->q330, AUXMSG_DSS, 0, addr(s)) ;
            end
        return ;
      end
  else if (err > 0)
    then
      begin
        p = addr(dssstr->msgin.qdp) ;
        expcrc = gcrccalc (addr(dssstr->q330->crc_table), (pointer)((integer)addr(dssstr->msgin) + 4), err - 4) ;
        loaddsshdr (addr(p), addr(dssstr->recvhdr)) ; /* handle both formats */
        if (expcrc != dssstr->recvhdr.crc)
          then
            begin
              /* Anything else I need to do?? */
              return ;
            end
      end
  if (dssstr->recvhdr.command == DSS_REG)
    then
      register_client (dssstr) ;
    else
      begin
        pcli = find_client (dssstr) ;
        if (pcli == NIL)
          then
            begin
              refuse (dssstr, REF_URC) ;
              return ;
            end
        switch (dssstr->recvhdr.command) begin
          case DSS_TOR :
            pcli->timer = 0 ;
            break ;
          case DSS_LRQ :
            list_requests (dssstr, pcli) ;
            break ;
          case DSS_DEL :
            del_requests (dssstr, pcli) ;
            break ;
          case DSS_REQ :
            add_request (dssstr, pcli) ;
            break ;
          case DSS_REM :
            remove_client (dssstr, pcli) ;
            break ;
        end
      end
end

static void link_rep (pdssstr dssstr, longint size)
begin
  word w ;
  pbyte p, pbeg ;

  incn(dssstr->sendhdr.datalength, size) ;
  pbeg = (pointer)((uninteger)(dssstr->pb) - size) ; /* back to beginning of record */
  if (dssstr->lastp)
    then
      begin
        p = dssstr->lastp ;
        w = loadword (addr(p)) ; /* current flags */
        p = dssstr->lastp ;
        storeword (addr(p), w or (word)((uninteger)pbeg - (uninteger)(addr(dssstr->msgout.buffer)))) ;
      end
  dssstr->lastp = (pointer)((uninteger)pbeg + 2) ; /* point at offset */
end

static void checksize (pdssstr dssstr, longint size)
begin

  if ((dssstr->sendhdr.datalength + size) > 512)
    then
      begin
        send_msg (dssstr) ;
        dssstr->sendhdr.datalength = 0 ;
        dssstr->lastp = NIL ;
        dssstr->pb = addr(dssstr->msgout.buffer) ;
      end
end

static void offline (pdssstr dssstr, pdss_report prep)
begin

  if (prep->flag and FLAG_GOOD) /* just went bad */
    then
      begin
        prep->flag = (prep->flag and not FLAG_GOOD) or FLAG_WAIT ;
        prep->repcount = prep->repflag ;
        if (prep->repflag != FLG_NEVER)
          then
            prep->flag = prep->flag or FLAG_CNTDN ;
      end
  if ((prep->flag and FLAG_CNTDN) land (prep->repcount > 0))
    then
      begin
        storeword (addr(dssstr->pb), prep->data_id) ;
        storeword (addr(dssstr->pb), OFF_DNA) ;
        link_rep(dssstr, 4) ;
        if ((prep->repflag < FLG_CONT) land (prep->repcount > 0))
          then
            dec(prep->repcount) ;
      end
end

static boolean check_data (pdssstr dssstr, pdss_report prep)
begin

  if (dssstr->q330->aqstruc)
    then
      begin
        if ((prep->flag and FLAG_GOOD) == 0)
          then /* had died */
            begin
              if (dssstr->q330->dss_gate_on)
                then
                  begin /* make good again */
                    prep->flag = (prep->flag or FLAG_GOOD) and FLAG_MASK ;
                    return TRUE ;
                  end
                else
                  return FALSE ; /* still bad */
            end
          else
            return dssstr->q330->dss_gate_on ; /* Current status */
      end
    else
      return FALSE ; /* can't get data */
end

static boolean check_status (pdssstr dssstr, pdss_report prep)
begin

  if (dssstr->q330->aqstruc)
    then
      begin
        if ((prep->flag and FLAG_GOOD) == 0)
          then /* had died */
            if (dssstr->q330->status_timer <= (prep->interval shl 1))
              then
                begin /* make good again */
                  prep->flag = (prep->flag or FLAG_GOOD) and FLAG_MASK ;
                  return TRUE ;
                end
              else
                return FALSE ; /* still bad */
        else if (dssstr->q330->status_timer <= (prep->interval shl 1)) /* Current status */
          then
            return TRUE ;
          else
            return FALSE ;
      end
    else
      return FALSE ; /* can't get status */
end

static void rep_boot (pdssstr dssstr, pdss_report prep)
begin
  longint dlth ;

  dlth = 4 ; /* id and offset */
  checksize (dssstr, 10) ;
  storeword (addr(dssstr->pb), prep->data_id) ;
  storeword (addr(dssstr->pb), 0) ; /* status and offset */
  store_double (addr(dssstr->pb), addr(dlth), FMT_F64, now () - dssstr->q330->share.fixed.last_reboot) ;
  link_rep (dssstr, dlth) ;
end

static void rep_gps (pdssstr dssstr, pdss_report prep)
begin
  string31 s ;
  topstat *pop ;

  checksize(dssstr, 32) ;
  if (check_status (dssstr, prep))
    then
      begin
        pop = addr(dssstr->q330->share.opstat) ;
        storeword (addr(dssstr->pb), prep->data_id) ;
        storeword (addr(dssstr->pb), 0) ; /* status and offset */
        if (pop->gps_lat == 0.0)
          then
            s[0] = 0 ;
          else
            sprintf(s, "%10.6f", pop->gps_lat) ;
        while (strlen(s) < 10)
          strcat(s, " ") ;
        storeblock (addr(dssstr->pb), 10, addr(s)) ;
        if (pop->gps_long == 0.0)
          then
            s[0] = 0 ;
          else
            sprintf(s, "%11.6f", pop->gps_long) ;
        while (strlen(s) < 11)
          strcat(s, " ") ;
        storeblock (addr(dssstr->pb), 11, addr(s)) ;
        if (pop->gps_elev == 0.0)
          then
            s[0] = 0 ;
          else
            sprintf(s, "%7.1f", pop->gps_elev) ;
        while (strlen(s) < 7)
          strcat(s, " ") ;
        storeblock (addr(dssstr->pb), 7, addr(s)) ;
        link_rep(dssstr, 32) ;
      end
    else
      offline (dssstr, prep) ;
end

static void rep_dead (pdssstr dssstr, pdss_report prep)
begin
  longint k ;
  byte j ;

  checksize(dssstr, 8) ;
  storeword (addr(dssstr->pb), prep->data_id) ;
  storeword (addr(dssstr->pb), 0) ; /* status and offset */
  k = 0 ;
  for (j = 0 ; j <= 5 ; j++)
    if (dssstr->paqs->calerr_bitmap and (1 shl j))
      then
        inc(k) ;
  storelongint (addr(dssstr->pb), k) ;
  link_rep(dssstr, 8) ;
end

static void rep_det (pdssstr dssstr, pdss_report prep)
begin
  tdetcnt *pdcnt ;
  con_common *pcom ;

  pdcnt = (pointer)(prep->access) ;
  checksize(dssstr, 8) ;
  if (check_data (dssstr, prep))
    then
      begin
        storeword (addr(dssstr->pb), prep->data_id) ;
        storeword (addr(dssstr->pb), 0) ; /* status and offset */
        pcom = pdcnt->det->cont ;
        storelongint (addr(dssstr->pb), pcom->total_detections - pdcnt->current) ;
        pdcnt->current = pcom->total_detections ;
        link_rep (dssstr, 8) ;
      end
    else
      offline (dssstr, prep) ;
end

static void rep_rec (pdssstr dssstr, pdss_report prep)
begin
  treccnt *prcnt ;

  prcnt = (pointer)(prep->access) ;
  checksize(dssstr, 8) ;
  if (check_data (dssstr, prep))
    then
      begin
        storeword (addr(dssstr->pb), prep->data_id) ;
        storeword (addr(dssstr->pb), 0) ; /* status and offset */
        storelongint (addr(dssstr->pb), prcnt->lcq->com->records_written - prcnt->current) ;
        prcnt->current = prcnt->lcq->com->records_written ;
        link_rep (dssstr, 8) ;
      end
    else
      offline (dssstr, prep) ;
end

static void rep_simp (pdssstr dssstr, pdss_report prep)
begin
  treccnt *pl ;
  longint l ;
  longint dlth ;
  plcq q ;

  dlth = 4 ; /* id and status */
  checksize(dssstr, 20) ; /* worst case size */
  pl = (pointer)(prep->access) ;
  if (check_data (dssstr, prep))
    then
      begin
        q = pl->lcq ;
        l = (*(q->databuf))[0] ;
        storeword (addr(dssstr->pb), prep->data_id) ;
        storeword (addr(dssstr->pb), 0) ; /* status and offset */
        store_double (addr(dssstr->pb), addr(dlth), FMT_F64, YEAR_ADJUST + dssstr->paqs->data_timetag - q->delay) ;
        store_long (addr(dssstr->pb), addr(dlth), prep->format, l) ;
        link_rep (dssstr, dlth) ;
      end
    else
      offline (dssstr, prep) ;
end

static void interval_reports (pdssstr dssstr, pdss_client pcli)
begin
  pdss_report savrep, prep ;

  dssstr->cur_client = pcli ;
  dssstr->cur_ver = pcli->qdpver ;
  prep = pcli->head ;
  dssstr->sendhdr.datalength = 0 ;
  dssstr->lastp = NIL ;
  dssstr->pb = addr(dssstr->msgout.buffer) ;
  while (prep)
    begin
      if (prep->flag and FLAG_WAIT)
        then
          begin /* Guess FLAG_WAIT isn't used in this case */
          end
      if (prep->flag and (FLAG_INT or FLAG_CNTDN))
        then
          begin
            dec(prep->counter) ;
            if (prep->counter == 0)
              then
                begin
                  savrep = (pdss_report)(prep->memory.next) ;
                  if (prep->flag and (FLAG_GOOD or FLAG_CNTDN))
                    then
                      begin
                        switch (prep->reqcode) begin
                          case REQ_BOOT : rep_boot (dssstr, prep) ; break ;
                          case REQ_GPS : rep_gps (dssstr, prep) ; break ;
                          case REQ_DEAD : rep_dead (dssstr, prep) ; break ;
                          case REQ_DET : rep_det (dssstr, prep) ; break ;
                          case REQ_REC : rep_rec (dssstr, prep) ; break ;
                          case REQ_SIMP : rep_simp (dssstr, prep) ; break ;
                          case REQ_MMA : offline (dssstr, prep) ; break ;
                          case REQ_SQR : offline (dssstr, prep) ; break ;
                        end
                        dssstr->pb = (pbyte)((integer)(addr(dssstr->msgout.buffer)) + dssstr->sendhdr.datalength) ;
                      end
                  if (prep->interval)
                    then
                      prep->counter = prep->interval ;
                    else
                      remove_report (dssstr, prep) ;
                  prep = savrep ;
                end
              else
                prep = (pdss_report)(prep->memory.next) ;
          end
        else
          prep = (pdss_report)(prep->memory.next) ;
    end
end

static void purge_last (pdssstr dssstr, pdss_client pcli, word code)
begin
  pdss_report prep ;
  pbyte p ;

  prep = pcli->head ;
  while (prep->memory.next)
    prep = (pdss_report)(prep->memory.next) ;
  p = addr(dssstr->msgout.buffer) ;
  dssstr->sendhdr.command = DSS_PRG ;
  dssstr->sendhdr.datalength = 4 ;
  storeword (addr(p), prep->data_id) ;
  storeword (addr(p), code) ;
  send_msg (dssstr) ;
  remove_report (dssstr, prep) ;
end

void lib_dss_timer (pointer ct)
begin
  pdssstr dssstr ;
  integer cp, loops ;
  pdss_client pcli ;
  struct sockaddr_in *psock ;
  string63 s ;

  dssstr = ct ;
  inc(dssstr->dss_time_sec) ;
  if (dssstr->dss_time_sec < 10)
    then
      return ; /* hasn't been one second yet */
  dssstr->dss_time_sec = 0 ;
  psock = addr(dssstr->sock) ;
  for (cp = 2 ; cp >= 0 ; cp--)
    begin
      dssstr->client_priority = cp ;
      dssstr->cur_client = dssstr->clients[cp] ;
      while (dssstr->cur_client)
        begin
          pcli = dssstr->cur_client ;
          dssstr->cur_ver = pcli->qdpver ;
          inc(pcli->timer) ;
          if (pcli->timer > dssstr->client_timeout)
            then
              begin
                psock->sin_addr.s_addr = pcli->ipaddr ;
                psock->sin_port = pcli->port ;
                refuse (dssstr, REF_TO) ; /* tell client tough luck */
                if (dssstr->verbosity >= RPT_CLIENTS)
                  then
                    begin
                      sprintf(s, "Timeout on Client ""%s""", addr(pcli->name)) ;
                      lib_msg_add(dssstr->q330, AUXMSG_DSS, 0, addr(s)) ;
                    end
                dssstr->cur_client = (pdss_client)(pcli->memory.next) ;
                remove_client (dssstr, pcli) ;
              end
            else
              begin
                psock->sin_addr.s_addr = pcli->ipaddr ;
                psock->sin_port = pcli->port ;
                interval_reports (dssstr, dssstr->cur_client) ;
                if (dssstr->sendhdr.datalength)
                  then
                    begin /* everything setup except command */
                      dssstr->sendhdr.command = DSS_DAT ;
                      send_msg (dssstr) ;
                    end
                dssstr->cur_client = (pdss_client)(pcli->memory.next) ;
              end
        end
    end
  inc(dssstr->bytesec) ;
  if (dssstr->bytesec >= BYTE_INTERVAL)
    then
      begin
        dssstr->bytecount = dssstr->bytecount div dssstr->bytesec ;
        dssstr->bytecount = dssstr->bytecount - dssstr->maxbps ;
        loops = 0 ;
        while ((dssstr->bytecount > 10) land (++loops < 1000))
          for (cp = 0 ; cp <= 2 ; cp++)
            begin
              dssstr->client_priority = cp ;
              dssstr->cur_client = dssstr->clients[cp] ;
              while (dssstr->cur_client)
                begin
                  pcli = dssstr->cur_client ;
                  if (pcli->head)
                    then
                      begin
                        dssstr->cur_ver = pcli->qdpver ;
                        purge_last(dssstr, pcli, PRG_BPS) ;
                        dssstr->bytesec = 0 ;
                        dssstr->bytecount = 0 ;
                        return ;
                      end
                    else
                      dssstr->cur_client = (pdss_client)(pcli->memory.next) ;
                  end
              end
        dssstr->bytesec = 0 ;
        dssstr->bytecount = 0 ;
      end
end

static void rep_mma (pdssstr dssstr, pdss_report prep)
begin
  integer k, points ;
  longint v ;
  longint dlth ;
  tmma *pmma ;
  plcq q ;

  pmma = (pointer)(prep->access) ;
  if (check_data (dssstr, prep))
    then
      begin
        q = pmma->lcq ;
        if (pmma->acc)
          then
            begin
              if (q->rate >= 1)
                then
                  points = q->rate ;
                else
                  points = 1 ;
              for (k = 0 ; k <= points - 1 ; k++)
                begin
                  v = (*(q->databuf))[k] ;
                  if (v < pmma->minimum)
                    then
                      pmma->minimum = v ;
                  if (v > pmma->maximum)
                    then
                      pmma->maximum = v ;
                  pmma->average = pmma->average + v ;
                end
              inc(pmma->seccount) ;
              if (pmma->seccount == 1)
                then
                  pmma->rettime = dssstr->paqs->data_timetag ;
              if (pmma->seccount >= prep->seconds)
                then
                  begin
                    checksize(dssstr, 36) ;
                    dlth = 4 ; /* id and status */
                    storeword (addr(dssstr->pb), prep->data_id) ;
                    storeword (addr(dssstr->pb), 0) ; /* offset */
                    store_double (addr(dssstr->pb), addr(dlth), FMT_F64, YEAR_ADJUST + pmma->rettime - q->delay) ;
                    store_long (addr(dssstr->pb), addr(dlth), prep->format, pmma->minimum) ;
                    store_long (addr(dssstr->pb), addr(dlth), prep->format, pmma->maximum) ;
                    store_double (addr(dssstr->pb), addr(dlth), prep->format, pmma->average / (points * pmma->seccount)) ;
                    link_rep (dssstr, dlth) ;
                    if (prep->seconds < prep->interval)
                      then
                        pmma->acc = FALSE ; /* gap */
                      else
                        pmma->seccount = 0 ;
                    pmma->minimum = MAXLINT ;
                    pmma->maximum = -MAXLINT ;
                    pmma->average = 0.0 ;
                  end
            end
          else
            begin
              inc(pmma->seccount) ;
              if (pmma->seccount >= prep->interval)
                then
                  begin
                    pmma->seccount = 0 ;
                    pmma->acc = TRUE ;
                  end
            end
      end
    else
      offline (dssstr, prep) ;
end

static void rep_sqr (pdssstr dssstr, pdss_report prep)
begin
  integer k, points ;
  double v ;
  longint dlth ;
  tsqr *psqr ;
  plcq q ;

  psqr = (pointer)(prep->access) ;
  if (check_data (dssstr, prep))
    then
      begin
        q = psqr->lcq ;
        if (psqr->acc)
          then
            begin
              if (q->rate >= 1)
                then
                  points = q->rate ;
                else
                  points = 1 ;
              for (k = 0 ; k <= points - 1 ; k++)
                begin
                  v = (*(q->databuf))[k] ;
                  psqr->sum = psqr->sum + (v * v) ;
                  psqr->average = psqr->average + v ;
                end
              inc(psqr->seccount) ;
              if (psqr->seccount == 1)
                then
                  psqr->rettime = dssstr->paqs->data_timetag ;
              if (psqr->seccount >= prep->seconds)
                then
                  begin
                    checksize(dssstr, 28) ;
                    dlth = 4 ; /* id and status */
                    storeword (addr(dssstr->pb), prep->data_id) ;
                    storeword (addr(dssstr->pb), 0) ; /* offset */
                    store_double (addr(dssstr->pb), addr(dlth), FMT_F64, YEAR_ADJUST + psqr->rettime - q->delay) ;
                    store_double (addr(dssstr->pb), addr(dlth), prep->format, psqr->sum / (points * psqr->seccount)) ;
                    store_double (addr(dssstr->pb), addr(dlth), prep->format, psqr->average / (points * psqr->seccount)) ;
                    link_rep (dssstr, dlth) ;
                    if (prep->seconds < prep->interval)
                      then
                        psqr->acc = FALSE ; /* gap */
                      else
                        psqr->seccount = 0 ;
                    psqr->sum = 0.0 ;
                    psqr->average = 0.0 ;
                  end
            end
          else
            begin
              inc(psqr->seccount) ;
              if (psqr->seccount >= prep->interval)
                then
                  begin
                    psqr->seccount = 0 ;
                    psqr->acc = TRUE ;
                  end
            end
      end
    else
      offline (dssstr, prep) ;
end

void lib_dss_continuous (pointer ct)
begin
  pdssstr dssstr ;
  integer cp ;
  pdss_report prep ;
  pdss_client pcli ;
  struct sockaddr_in *psock ;

  dssstr = ct ;
  dssstr->sendhdr.datalength = 0 ;
  dssstr->lastp = NIL ;
  dssstr->pb = addr(dssstr->msgout.buffer) ;
  psock = addr(dssstr->sock) ;
  for (cp = 2 ; cp >= 0 ; cp--)
    begin
      dssstr->client_priority = cp ;
      dssstr->cur_client = dssstr->clients[cp] ;
      while (dssstr->cur_client)
        begin
          pcli = dssstr->cur_client ;
          dssstr->cur_ver = pcli->qdpver ;
          psock->sin_addr.s_addr = pcli->ipaddr ;
          psock->sin_port = pcli->port ;
          prep = pcli->head ;
          dssstr->sendhdr.datalength = 0 ;
          dssstr->lastp = NIL ;
          dssstr->pb = addr(dssstr->msgout.buffer) ;
          while (prep)
            begin
              if (prep->flag and FLAG_CON)
                then
                  begin
                    switch (prep->reqcode) begin
                      case REQ_MMA : rep_mma (dssstr, prep) ; break ;
                      case REQ_SQR : rep_sqr (dssstr, prep) ; break ;
                    end
                    dssstr->pb = (pbyte)((integer)(addr(dssstr->msgout.buffer)) + dssstr->sendhdr.datalength) ;
                  end
              prep = (pdss_report)(prep->memory.next) ;
            end
          if (dssstr->sendhdr.datalength)
            then
              begin /* everything setup except command */
                dssstr->sendhdr.command = DSS_DAT ;
                send_msg (dssstr) ;
              end
          dssstr->cur_client = (pdss_client)(pcli->memory.next) ;
        end
    end
end

void lib_dss_start (tdss *dss, pointer ct, word host_size_limit)
begin
  pdssstr dssstr ;
  pq330 q330a ;
  word limit ;

  q330a = ct ; /* need this before allocating dssstr */
  getbuf (q330a, addr(dssstr), sizeof(tdssstr)) ;
  memcpy (addr(dssstr->dss_par), dss, sizeof(tdss)) ;
  dssstr->q330 = ct ;
  q330a->dsspath = INVALID_SOCKET ;
  q330a->dssstruc = NIL ;
  dssstr->paqs = q330a->aqstruc ;
  open_socket (dssstr) ;
  if (lnot dssstr->sockopen)
    then
      return ;
  dssstr->client_timeout = dss->timeout ;
  dssstr->maxbps = dss->max_bps ;
  if ((host_size_limit) land (host_size_limit < dss->max_mem))
    then
      limit = host_size_limit ;
    else
      limit = dss->max_mem ;
  dssstr->mem_allowed = (longword)limit * 1024 ;
  strcpy (addr(dssstr->passwords[0]), addr(dss->low_pass)) ;
  lib330_upper (addr(dssstr->passwords[0])) ;
  strcpy (addr(dssstr->passwords[1]), addr(dss->mid_pass)) ;
  lib330_upper (addr(dssstr->passwords[1])) ;
  strcpy (addr(dssstr->passwords[2]), addr(dss->high_pass)) ;
  lib330_upper (addr(dssstr->passwords[2])) ;
  dssstr->reply_sequence = 0 ;
  dssstr->verbosity = dss->verbosity ;
  dssstr->running = TRUE ;
  q330a->dssstruc = dssstr ; /* Tell libstruc we are alive */
end

void lib_dss_stop (pointer ct)
begin
  pdssstr dssstr ;

  dssstr = ct ;
  dssstr->dss_server_display[0] = 0 ;
  close_socket (dssstr) ;
  dssstr->q330->dssstruc = NIL ;
end

void get_dss_server_display (pointer ct, string63 *result)
begin
  pq330 q330 ;
  pdssstr dssstr ;

  q330 = ct ;
  if (q330 == NIL)
    then
      begin
        (*result)[0] = 0 ;
        return ;
      end
  dssstr = q330->dssstruc ;
  if (dssstr)
    then
      strcpy (result, addr(dssstr->dss_server_display)) ;
    else
      (*result)[0] = 0 ;
end

#endif
#endif
