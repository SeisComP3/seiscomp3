/*   Lib330 internal core routines
     Copyright 2006-2010 Certified Software Corporation

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
    0 2006-09-10 rdr Created
    1 2006-10-29 rdr Remove "addr" function when passing thread function. Fix posix
                     thread function return type.
    2 2006-12-09 rdr Add non-monotonic clock protection to 0.1hz dp statistics generation.
    3 2007-07-15 rdr Add baler simulation definitions.
    4 2007-08-01 rdr Add baler callback. Add conditionals for omitting network code.
                     Add conditionals for CMEX32.
    5 2007-08-07 rdr Add memory management for structures that live throughout
                     the duration of the thread.
    6 2007-09-04 rdr Add back in baler timer callback.
    7 2008-01-19 rdr Remove BT_TIMER call since it's allready done in lib_timer.
    8 2008-08-20 rdr Add TCP support.
    9 2009-09-28 rdr Add DSS support.
   10 2009-09-15 rdr Add DSS support for serial connection to Q330.
   11 2010-03-27 rdr Add Q335 support.
   12 2010-08-21 rdr In lib_destroy_330 clear ct before doing any deallocations.
   13 2015-09-25 dsn Configure pthread for libthread as detached thread, 
                     libthread calls pthread_detach() and pthread_exit().
*/
/* Make sure libstrucs.h is included */
#ifndef libstrucs_h
#include "libstrucs.h"
#endif
#ifndef libmsgs_h
#include "libmsgs.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif
#ifndef libcmds_h
#include "libcmds.h"
#endif
#ifndef libstats_h
#include "libstats.h"
#endif
#ifndef q330io_h
#include "q330io.h"
#endif
#ifndef libsampcfg_h
#include "libsampcfg.h"
#endif
#ifndef libcont_h
#include "libcont.h"
#endif
#ifndef libmd5_h
#include "libmd5.h"
#endif

#ifndef OMIT_SEED
#ifndef libfilters_h
#include "libfilters.h"
#endif
#ifndef libdss_h
#include "libdss.h"
#endif
#endif

#define MS100 (0.1)

#ifdef X86_WIN32
static void create_mutex (pq330 q330)
begin

  q330->mutex = CreateMutex(NIL, FALSE, NIL) ;
  q330->msgmutex = CreateMutex(NIL, FALSE, NIL) ;
end

static void destroy_mutex (pq330 q330)
begin

  CloseHandle (q330->mutex) ;
  CloseHandle (q330->msgmutex) ;
end

void lock (pq330 q330)
begin

  WaitForSingleObject (q330->mutex, INFINITE) ;
end

void unlock (pq330 q330)
begin

  ReleaseMutex (q330->mutex) ;
end

void msglock (pq330 q330)
begin

  WaitForSingleObject (q330->msgmutex, INFINITE) ;
end

void msgunlock (pq330 q330)
begin

  ReleaseMutex (q330->msgmutex) ;
end

void sleepms (integer ms)
begin

  Sleep (ms) ;
end

#else
#ifdef CMEX32
static void create_mutex (pq330 q330) begin end
static void destroy_mutex (pq330 q330) begin end
void lock (pq330 q330) begin end
void unlock (pq330 q330) begin end
void msglock (pq330 q330) begin end
void msgunlock (pq330 q330) begin end
void sleepms (integer ms) begin end

#else
static void create_mutex (pq330 q330)
begin

  pthread_mutex_init (addr(q330->mutex), NULL) ;
  pthread_mutex_init (addr(q330->msgmutex), NULL) ;
end

static void destroy_mutex (pq330 q330)
begin

  pthread_mutex_destroy (addr(q330->mutex)) ;
  pthread_mutex_destroy (addr(q330->msgmutex)) ;
end

void lock (pq330 q330)
begin

  pthread_mutex_lock (addr(q330->mutex)) ;
end

void unlock (pq330 q330)
begin

  pthread_mutex_unlock (addr(q330->mutex)) ;
end

void msglock (pq330 q330)
begin

  pthread_mutex_lock (addr(q330->msgmutex)) ;
end

void msgunlock (pq330 q330)
begin

  pthread_mutex_unlock (addr(q330->msgmutex)) ;
end

void sleepms (integer ms)
begin
  struct timespec dly ;

  dly.tv_sec = 0 ;
  dly.tv_nsec = ms * 1000000 ; /* convert to nanoseconds */
  nanosleep (addr(dly), NULL) ;
end
#endif
#endif

void getbuf (pq330 q330, pointer *p, integer size)
begin
  pbyte newblock ;
  pmem_manager pm ;

  pm = q330->cur_memory ;

  size = (size + 3) and 0xFFFFFFFC ; /* make multiple of longword */
  if ((pm->sofar + size) > pm->alloc_size)
    then
      begin /* need a new block of memory */
        if (q330->cur_memory->next)
          then
            begin /* already available from before */
              q330->cur_memory = q330->cur_memory->next ;
              q330->cur_memory->sofar = 0 ;
            end
          else
            begin /* need new allocation */
              pm->next = malloc (sizeof(tmem_manager)) ;
              pm = pm->next ;
              pm->next = NIL ;
              if (size > DEFAULT_MEM_INC)
                then
                  pm->alloc_size = size ;
                else
                  pm->alloc_size = DEFAULT_MEM_INC ;
              pm->sofar = 0 ;
              pm->base = malloc (pm->alloc_size) ;
              q330->cur_memory = pm ;
            end
      end
  newblock = q330->cur_memory->base ;
  incn(newblock, q330->cur_memory->sofar) ;
  q330->cur_memory->sofar = q330->cur_memory->sofar + size ;
  memset (newblock, 0, size) ; /* make sure is zeroed out */
  *p = newblock ;
end

void mem_release (pq330 q330)
begin
  pmem_manager pm ;

  pm = q330->memory_head ;
  while (pm)
    begin /* release  blocks */
      pm->sofar = 0 ;
      pm = pm->next ;
    end
end

void getthrbuf (pq330 q330, pointer *p, integer size)
begin
  pbyte newblock ;

  size = (size + 3) and 0xFFFFFFFC ; /* make multiple of longword */
  if ((q330->cur_thrmem->sofar + size) > q330->cur_thrmem->alloc_size)
    then
      begin /* need a new block of memory */
        q330->cur_thrmem->next = malloc (sizeof(tmem_manager)) ;
        q330->cur_thrmem = q330->cur_thrmem->next ;
        q330->cur_thrmem->next = NIL ;
        if (size > DEFAULT_THR_INC)
          then
            q330->cur_thrmem->alloc_size = size ;
          else
            q330->cur_thrmem->alloc_size = DEFAULT_THR_INC ;
        q330->cur_thrmem->sofar = 0 ;
        q330->cur_thrmem->base = malloc (q330->cur_thrmem->alloc_size) ;
      end
  newblock = q330->cur_thrmem->base ;
  incn(newblock, q330->cur_thrmem->sofar) ;
  q330->cur_thrmem->sofar = q330->cur_thrmem->sofar + size ;
  memset (newblock, 0, size) ; /* make sure is zeroed out */
  *p = newblock ;
end

void gcrcinit (crc_table_type *crctable)
begin
  integer count, bits ;
  longint tdata, accum ;

  for (count = 0 ; count <= 255 ; count++)
    begin
      tdata = (longint)count shl 24 ;
      accum = 0 ;
      for (bits = 1 ; bits <= 8 ; bits++)
        begin
          if ((tdata xor accum) < 0)
            then
              accum = (accum shl 1) xor CRC_POLYNOMIAL ;
            else
              accum = (accum shl 1) ;
          tdata = tdata shl 1 ;
        end
      (*crctable)[count] = accum ;
    end
end

longint gcrccalc (crc_table_type *crctable, pbyte p, longint len)
begin
  longint crc ;
  integer temp ;

  crc = 0 ;
  while (len > 0)
    begin
      temp = ((crc shr 24) xor *p++) and 255 ;
      crc = (crc shl 8) xor (*crctable)[temp] ;
      dec(len) ;
    end
  return crc ;
end

longword baler_callback (pq330 q330, enum tbaler_type btype, longword val)
begin

  if (q330->par_create.call_baler)
    then
      begin
        q330->baler_call.context = q330 ;
        q330->baler_call.baler_type = btype ;
        memcpy(addr(q330->baler_call.station_name), addr(q330->station_ident), sizeof(string9)) ;
        q330->baler_call.info = val ;
        q330->baler_call.response = 0 ;
        q330->par_create.call_baler (addr(q330->baler_call)) ;
        return q330->baler_call.response ;
      end
    else
      return 0 ;
end

#ifdef X86_WIN32
unsigned long  __stdcall libthread (pointer p)
begin
  pq330 q330 ;
#ifndef OMIT_NETWORK
  fd_set readfds, writefds, exceptfds ;
  struct timeval timeout ;
#endif
  integer res ;
  double now_, diff ;
  longint new_ten_sec ;

  q330 = p ;
  repeat
    switch (q330->libstate) begin
      case LIBSTATE_PING :
      case LIBSTATE_CONN :
      case LIBSTATE_ANNC :
      case LIBSTATE_REG :
      case LIBSTATE_READCFG :
      case LIBSTATE_READTOK :
      case LIBSTATE_DECTOK :
      case LIBSTATE_RUNWAIT :
      case LIBSTATE_RUN :
      case LIBSTATE_DEALLOC :
      case LIBSTATE_DEREG :
#ifndef OMIT_NETWORK
        if (q330->usesock)
          then
            begin /* wait for socket input or timeout */
              FD_ZERO (addr(readfds)) ;
              FD_ZERO (addr(writefds)) ;
              FD_ZERO (addr(exceptfds)) ;
              if (q330->libstate == LIBSTATE_CONN)
                then /* waiting for connection */
                  FD_SET (q330->cpath, addr(writefds)) ;
                else
                  begin
                    FD_SET (q330->cpath, addr(readfds)) ;
                    if ((q330->dpath != INVALID_SOCKET) land (q330->libstate != LIBSTATE_PING))
                      then
                        FD_SET (q330->dpath, addr(readfds)) ;
                  end
#ifndef OMIT_SEED
              if ((q330->dssstruc) land (q330->dsspath != INVALID_SOCKET))
                then
                  FD_SET (q330->dsspath, addr(readfds)) ;
#endif
              timeout.tv_sec = 0 ;
              timeout.tv_usec = 25000 ; /* 25ms timeout */
              res = select (0, addr(readfds), addr(writefds), addr(exceptfds), addr(timeout)) ;
              if (res > 0)
                then
                  begin
                    if (q330->libstate == LIBSTATE_CONN)
                      then
                        begin
                          if ((q330->cpath != INVALID_SOCKET) land (FD_ISSET (q330->cpath, addr(writefds))))
                            then
                              begin /* connected to tunnel330 */
                                q330->tcpidx = 0 ;
                                libmsgadd(q330, LIBMSG_CONN, "") ;
                                lib_continue_registration (q330) ;
                              end
                        end
                      else
                        begin
                          if ((q330->cpath != INVALID_SOCKET) land (FD_ISSET (q330->cpath, addr(readfds))))
                            then
                              read_cmd_socket (q330) ;
                          if ((q330->dpath != INVALID_SOCKET) land (q330->libstate != LIBSTATE_PING) land
                              (FD_ISSET (q330->dpath, addr(readfds))))
                            then
                              read_data_socket (q330) ;
#ifndef OMIT_SEED
                          if ((q330->dssstruc) land (q330->dsspath != INVALID_SOCKET) land
                              (FD_ISSET (q330->dsspath, addr(readfds))))
                            then
                              lib_dss_read (q330->dssstruc) ;
#endif
                        end
                  end
            end
#endif
#ifndef OMIT_SERIAL
        if (q330->usesock == 0)
          then
            begin
#ifndef OMIT_NETWORK
#ifndef OMIT_SEED
              if ((q330->dssstruc) land (q330->dsspath != INVALID_SOCKET))
                then
                  begin
                    FD_ZERO (addr(readfds)) ;
                    FD_ZERO (addr(writefds)) ;
                    FD_ZERO (addr(exceptfds)) ;
                    FD_SET (q330->dsspath, addr(readfds)) ;
                    timeout.tv_sec = 0 ;
                    timeout.tv_usec = 5000 ; /* 5ms timeout */
                    res = select (0, addr(readfds), addr(writefds), addr(exceptfds), addr(timeout)) ;
                    if ((res > 0) land (FD_ISSET (q330->dsspath, addr(readfds))))
                      then
                        lib_dss_read (q330->dssstruc) ;
                  end
#endif
#endif
              read_from_serial (q330) ;
            end
#endif
        break ;
      case LIBSTATE_TERM :
        break ; /* nothing to */
      case LIBSTATE_IDLE :
        if (q330->needtosayhello)
          then
            begin
              q330->needtosayhello = FALSE ;
              libmsgadd (q330, LIBMSG_CREATED, "") ;
            end
          else
            sleepms (25) ;
        break ;
      default :
        sleepms (25) ;
    end
    if (q330->terminate == FALSE)
      then
        begin
          now_ = now() ;
          if ((now_ < 200000000) lor (now_ > 219999999))
            then
              res = 5 ;
          diff = now_ - q330->last_100ms ;
          if (fabs(diff) > (MS100 * 20)) /* greater than 2 second spread */
            then
              q330->last_100ms = now_ + MS100 ; /* clock changed, reset interval */
          else if (diff >= MS100)
            then
              begin
                q330->last_100ms = q330->last_100ms + MS100 ;
                lib_timer (q330) ;
#ifndef OMIT_SEED
                if ((q330->dssstruc) land (q330->dsspath != INVALID_SOCKET))
                  then
                    lib_dss_timer (q330->dssstruc) ;
#endif
              end
          if (q330->terminate == FALSE) /* lib_timer may set terminate */
            then
              begin
                new_ten_sec = lib_round(now_ + q330->zone_adjust) ; /* rounded second */
                new_ten_sec = new_ten_sec div 10 ; /* integer 10 second value */
                if ((new_ten_sec > q330->last_ten_sec) lor
                    (new_ten_sec <= (q330->last_ten_sec - 12)))
                  then
                    begin
                      q330->last_ten_sec = new_ten_sec ;
                      q330->dpstat_timestamp = new_ten_sec * 10 ; /* into seconds since 2000 */
                      lib_stats_timer (q330) ;
                    end
              end
        end
  until q330->terminate) ;
  new_state (q330, LIBSTATE_TERM) ;
  ExitThread (0) ;
  return 0 ;
end

#else
#ifndef CMEX32

void *libthread (pointer p)
begin
  pq330 q330 ;
#ifndef OMIT_NETWORK
  fd_set readfds, writefds, exceptfds ;
  struct timeval timeout ;
#endif
  integer res, err ;
  double now_, diff ;
  longint new_ten_sec ;

  pthread_detach(pthread_self());
  q330 = p ;
  repeat
    switch (q330->libstate) begin
      case LIBSTATE_PING :
      case LIBSTATE_CONN :
      case LIBSTATE_ANNC :
      case LIBSTATE_REG :
      case LIBSTATE_READCFG :
      case LIBSTATE_READTOK :
      case LIBSTATE_DECTOK :
      case LIBSTATE_RUNWAIT :
      case LIBSTATE_RUN :
      case LIBSTATE_DEALLOC :
      case LIBSTATE_DEREG :
#ifndef OMIT_NETWORK
        if (q330->usesock)
          then
            begin /* wait for socket input or timeout */
              FD_ZERO (addr(readfds)) ;
              FD_ZERO (addr(writefds)) ;
              FD_ZERO (addr(exceptfds)) ;
              if (q330->cpath != INVALID_SOCKET)
                then
                  begin
                    if (q330->libstate == LIBSTATE_CONN)
                      then /* waiting for connection */
                        FD_SET (q330->cpath, addr(writefds)) ;
                      else
                        begin
                          FD_SET (q330->cpath, addr(readfds)) ;
                          if ((q330->dpath != INVALID_SOCKET) land (q330->libstate != LIBSTATE_PING))
                            then
                              FD_SET (q330->dpath, addr(readfds)) ;
                        end
                  end
#ifndef OMIT_SEED
              if ((q330->dssstruc) land (q330->dsspath != INVALID_SOCKET))
                then
                  FD_SET (q330->dsspath, addr(readfds)) ;
#endif
              timeout.tv_sec = 0 ;
              timeout.tv_usec = 25000 ; /* 25ms timeout */
              res = select (q330->high_socket + 1, addr(readfds), addr(writefds), addr(exceptfds), addr(timeout)) ;
              if ((q330->libstate != LIBSTATE_IDLE) land (res > 0))
                then
                  begin
                    if (q330->libstate == LIBSTATE_CONN)
                      then
                        begin
                          if ((q330->cpath != INVALID_SOCKET) land (FD_ISSET (q330->cpath, addr(writefds))))
                            then
                              begin /* connected to tunnel330 */
                                q330->tcpidx = 0 ;
                                libmsgadd(q330, LIBMSG_CONN, "") ;
                                lib_continue_registration (q330) ;
                              end
                        end
                      else
                        begin
                          if ((q330->cpath != INVALID_SOCKET) land (FD_ISSET (q330->cpath, addr(readfds))))
                            then
                              read_cmd_socket (q330) ;
                          if ((q330->dpath != INVALID_SOCKET) land (q330->libstate != LIBSTATE_PING) land (FD_ISSET (q330->dpath, addr(readfds))))
                            then
                              read_data_socket (q330) ;
#ifndef OMIT_SEED
                          if ((q330->dssstruc) land (q330->dsspath != INVALID_SOCKET) land
                              (FD_ISSET (q330->dsspath, addr(readfds))))
                            then
                              lib_dss_read (q330->dssstruc) ;
#endif
                        end
                  end
              else if (res < 0)
                then
                  begin
                    err = errno ;
                  end
            end
#endif
#ifndef OMIT_SERIAL
        if (q330->usesock == 0)
          then
            begin
#ifndef OMIT_NETWORK
#ifndef OMIT_SEED
              if ((q330->dssstruc) land (q330->dsspath != INVALID_SOCKET))
                then
                  begin
                    FD_ZERO (addr(readfds)) ;
                    FD_ZERO (addr(writefds)) ;
                    FD_ZERO (addr(exceptfds)) ;
                    FD_SET (q330->dsspath, addr(readfds)) ;
                    timeout.tv_sec = 0 ;
                    timeout.tv_usec = 5000 ; /* 5ms timeout */
                    res = select (q330->high_socket + 1, addr(readfds), addr(writefds), addr(exceptfds), addr(timeout)) ;
                    if ((res > 0) land (FD_ISSET (q330->dsspath, addr(readfds))))
                      then
                        lib_dss_read (q330->dssstruc) ;
                  end
#endif
#endif
              read_from_serial (q330) ;
            end
#endif
        break ;
      case LIBSTATE_TERM :
        break ; /* nothing to */
      case LIBSTATE_IDLE :
        if (q330->needtosayhello)
          then
            begin
              q330->needtosayhello = FALSE ;
              libmsgadd (q330, LIBMSG_CREATED, "") ;
            end
          else
            sleepms (25) ;
        break ;
      default :
        sleepms (25) ;
    end
    if (q330->terminate == FALSE)
      then
        begin
          now_ = now() ;
          if ((now_ < 200000000) lor (now_ > 219999999))
            then
              res = 5 ;
          diff = now_ - q330->last_100ms ;
          if (fabs(diff) > (MS100 * 20)) /* greater than 2 second spread */
            then
              q330->last_100ms = now_ + MS100 ; /* clock changed, reset interval */
          else if (diff >= MS100)
            then
              begin
                q330->last_100ms = q330->last_100ms + MS100 ;
                lib_timer (q330) ;
#ifndef OMIT_SEED
                if ((q330->dssstruc) land (q330->dsspath != INVALID_SOCKET))
                  then
                    lib_dss_timer (q330->dssstruc) ;
#endif
              end
          if (q330->terminate == FALSE) /* lib_timer may set terminate */
            then
              begin
                new_ten_sec = lib_round(now_ + q330->zone_adjust) ; /* rounded second */
                new_ten_sec = new_ten_sec div 10 ; /* integer 10 second value */
                if ((new_ten_sec > q330->last_ten_sec) lor
                    (new_ten_sec <= (q330->last_ten_sec - 12)))
                  then
                    begin
                      q330->last_ten_sec = new_ten_sec ;
                      q330->dpstat_timestamp = new_ten_sec * 10 ; /* into seconds since 2000 */
                      lib_stats_timer (q330) ;
                    end
              end
        end
  until q330->terminate) ;
  new_state (q330, LIBSTATE_TERM) ;
  pthread_exit (0) ;
end
#endif
#endif

void lib_create_330 (tcontext *ct, tpar_create *cfg)
begin
  pq330 q330 ;
#ifndef X86_WIN32
  integer err ;
  pthread_attr_t attr;
#endif

  *ct = malloc(sizeof(tq330)) ;
  q330 = *ct ;
  memset (q330, 0, sizeof(tq330)) ;
  create_mutex (q330) ;
  q330->libstate = LIBSTATE_IDLE ;
  q330->share.target_state = LIBSTATE_IDLE ;
  memcpy (addr(q330->par_create), cfg, sizeof(tpar_create)) ;
  gcrcinit (addr(q330->crc_table)) ;
  memcpy (addr(q330->qclock), addr(default_clock), sizeof(tclock)) ;
  q330->share.opstat.status_latency = INVALID_LATENCY ;
  q330->share.opstat.data_latency = INVALID_LATENCY ;
  q330->share.opstat.gps_age = -1 ;
  q330->zone_adjust = q330->par_create.host_timezone ; /* possibly overwritten by continuity. */
  q330->memory_head = malloc(sizeof(tmem_manager)) ;
  q330->memory_head->next = NIL ;
  q330->memory_head->alloc_size = DEFAULT_MEMORY ;
  q330->cur_memory_required = DEFAULT_MEMORY ;
  q330->thrmem_head = malloc(sizeof(tmem_manager)) ;
  q330->thrmem_head->next = NIL ;
  q330->thrmem_head->alloc_size = DEFAULT_THRMEM ;
  q330->cur_thrmem_required = DEFAULT_THRMEM ;
  restore_thread_continuity (q330, TRUE, addr(q330->contmsg)) ; /* get static storage and status */
  if (q330->cur_memory_required < DEFAULT_MEM_INC)
    then
      q330->cur_memory_required = DEFAULT_MEM_INC ;
  q330->memory_head->alloc_size = q330->cur_memory_required ;
  q330->memory_head->sofar = 0 ;
  q330->memory_head->base = malloc(q330->memory_head->alloc_size) ;
  q330->cur_memory = q330->memory_head ;
  if (q330->cur_thrmem_required < DEFAULT_THR_INC)
    then
      q330->cur_thrmem_required = DEFAULT_THR_INC ;
  q330->thrmem_head->alloc_size = q330->cur_thrmem_required ;
  q330->thrmem_head->sofar = 0 ;
  q330->thrmem_head->base = malloc(q330->thrmem_head->alloc_size) ;
  q330->cur_thrmem = q330->thrmem_head ;
  q330->last_100ms = now () ;
  q330->last_ten_sec = (q330->last_100ms + 0.5) div 10 ; /* integer 10 second value */
  q330->boot_time = now () ;
  q330->data_timeout = DEFAULT_DATA_TIMEOUT ;
  q330->data_timeout_retry = DEFAULT_DATA_TIMEOUT_RETRY ;
  q330->status_timeout = DEFAULT_STATUS_TIMEOUT ;
  q330->status_timeout_retry = DEFAULT_STATUS_TIMEOUT_RETRY ;
  q330->piu_retry = DEFAULT_PIU_RETRY ;
  getthrbuf (q330, (pointer *)addr(q330->cfgbuf), sizeof(tcfgbuf)) ;
  getthrbuf (q330, (pointer *)addr(q330->cbuf), sizeof(tcbuf)) ;
  init_md5_buffer (q330) ;
  q330->aqstruc = allocate_aqstruc (q330) ;
  allocate_packetbuffers (q330) ;
  q330->cur_verbosity = q330->par_create.opt_verbose ; /* until register anyway */
#ifndef OMIT_SEED
  load_firfilters (q330, q330->aqstruc) ; /* build standards */
  append_firfilters (q330, q330->aqstruc, q330->par_create.mini_firchain) ; /* add client defined */
#endif
  q330->rsum = now () ;
  q330->comid = INVALID_IO_HANDLE ;
#ifndef OMIT_NETWORK
  q330->cpath = INVALID_SOCKET ;
  q330->dpath = INVALID_SOCKET ;
  q330->dsspath = INVALID_SOCKET ;
#endif
#ifdef X86_WIN32
  q330->threadhandle = CreateThread (NIL, 0, libthread, q330, 0, addr(q330->threadid)) ;
  if (q330->threadhandle == NIL)
#else
#ifdef CMEX32
  err = 0 ;
#else
  err = pthread_attr_init(&attr);
  if (! err) err = pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
  if (! err) err = pthread_create(addr(q330->threadid), NULL, libthread, q330) ;
#endif
  if (err)
#endif
    then
      begin
        cfg->resp_err = LIBERR_THREADERR ;
        free (*ct) ; /* no context */
        *ct = NIL ;
      end
    else
      q330->needtosayhello = TRUE ;
end

enum tliberr lib_destroy_330 (tcontext *ct)
begin
  pq330 q330 ;
  pmem_manager pm, pmn ;

  q330 = *ct ;
  *ct = NIL ;
  destroy_mutex (q330) ;
  pm = q330->memory_head ;
  while (pm)
    begin
      pmn = pm->next ;
      free (pm->base) ;
      free (pm) ;
      pm = pmn ;
    end
  pm = q330->thrmem_head ;
  while (pm)
    begin
      pmn = pm->next ;
      free (pm->base) ;
      free (pm) ;
      pm = pmn ;
    end
  free (q330) ;
  return LIBERR_NOERR ;
end

enum tliberr lib_register_330 (pq330 q330, tpar_register *rpar)
begin

  memset (addr(q330->first_clear), 0, (pntrint)addr(q330->last_clear) - (pntrint)addr(q330->first_clear)) ;
  memset (addr(q330->share.first_share_clear), 0,
       (pntrint)addr(q330->share.last_share_clear) - (pntrint)addr(q330->share.first_share_clear)) ;
  memcpy (addr(q330->par_register), rpar, sizeof(tpar_register)) ;
  q330->share.opstat.gps_age = -1 ;
  q330->tcp = (q330->par_register.host_mode == HOST_TCP) ;
  q330->usesock = ((q330->par_register.host_mode == HOST_ETH) lor (q330->tcp)) ;
  q330->q330cport = q330->par_register.q330id_baseport + (2 * (q330->par_create.q330id_dataport + 1)) ;
  q330->q330dport = q330->q330cport + 1 ;
  memcpy (addr(q330->station_ident), addr(q330->par_create.q330id_station), sizeof(string9)) ; /* initial station name */
  q330->share.status_interval = 10 ;
  q330->reg_tries = 0 ;
  q330->dynip_age = 0 ;
  if ((q330->par_register.opt_dynamic_ip) land ((q330->par_register.q330id_address)[0] == 0))
    then
      q330->share.target_state = LIBSTATE_WAIT ;
    else
      q330->share.target_state = LIBSTATE_RUNWAIT ;
  return LIBERR_NOERR ;
end

enum tliberr lib_unregping_330 (pq330 q330, tpar_register *rpar)
begin

  lock (q330) ;
  memset (addr(q330->first_clear), 0, (pntrint)addr(q330->last_clear) - (pntrint)addr(q330->first_clear)) ;
  memset (addr(q330->share.first_share_clear), 0,
       (pntrint)addr(q330->share.last_share_clear) - (pntrint)addr(q330->share.first_share_clear)) ;
  memcpy (addr(q330->par_register), rpar, sizeof(tpar_register)) ;
  q330->share.opstat.gps_age = -1 ;
  q330->usesock = (q330->par_register.host_mode == HOST_ETH) ;
  q330->q330cport = q330->par_register.q330id_baseport + (2 * (q330->par_create.q330id_dataport + 1)) ;
  q330->q330dport = q330->q330cport + 1 ;
  memcpy (addr(q330->station_ident), addr(q330->par_create.q330id_station), sizeof(string9)) ; /* initial station name */
  q330->share.status_interval = 10 ;
  q330->share.target_state = LIBSTATE_PING ;
  unlock (q330) ;
  return LIBERR_NOERR ;
end

void new_state (pq330 q330, enum tlibstate newstate)
begin

  q330->libstate = newstate ;
  state_callback (q330, ST_STATE, (longword)newstate) ;
end

void new_cfg (pq330 q330, longword newbitmap)
begin

  lock (q330) ;
  if (q330->share.have_config != (q330->share.have_config or newbitmap))
    then
      begin /* something we didn't have before */
        q330->share.have_config = q330->share.have_config or newbitmap ;
        unlock (q330) ;
        state_callback (q330, ST_CFG, q330->share.have_config) ;
      end
    else
      unlock (q330) ;
end

void new_status (pq330 q330, longword newbitmap)
begin

  lock (q330) ;
  q330->share.have_status = q330->share.have_status or newbitmap ;
  unlock (q330) ;
  state_callback (q330, ST_STATUS, newbitmap) ;
end

void state_callback (pq330 q330, enum tstate_type stype, longword val)
begin

  if (q330->par_create.call_state)
    then
      begin
        q330->state_call.context = q330 ;
        q330->state_call.state_type = stype ;
        memcpy(addr(q330->state_call.station_name), addr(q330->station_ident), sizeof(string9)) ;
        q330->state_call.info = val ;
        if (q330->q335)
          then
            q330->state_call.subtype = SCS_Q335 ;
          else
            q330->state_call.subtype = SCS_Q330 ;
        q330->par_create.call_state (addr(q330->state_call)) ;
      end
end

longword make_bitmap (longword bit)
begin

  return (longword)1 shl bit ;
end

void set_liberr (pq330 q330, enum tliberr newerr)
begin

  lock (q330) ;
  q330->share.liberr = newerr ;
  unlock (q330) ;
end
