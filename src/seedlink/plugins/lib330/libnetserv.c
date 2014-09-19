/*   Lib330 Netserver (LISS) definitions
     Copyright 2006 Certified Software Corporation

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
    1 2006-10-29 rdr Remove "addr" function when passing thread address. Fix posix
                     thread function return type. Hal's solaris and linux changes
                     put back in.
    2 2007-03-07 rdr pbuf declaration fixed for lib_ns_send. Don't generate client
                     disconnected message in send_next_buffer, this can cause corruption
                     of the miniseed callback structure.
*/
#ifndef OMIT_SEED /* Can't use without seed generation */
#ifndef libnetserv_h
#include "libnetserv.h"
#endif

#ifndef libmsgs_h
#include "libmsgs.h"
#endif
#ifndef libclient_h
#include "libclient.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif

typedef struct {
#ifdef X86_WIN32
  HANDLE mutex ;
  HANDLE threadhandle ;
  longword threadid ;
#else
  pthread_mutex_t mutex ;
  pthread_t threadid ;
#endif
  boolean running ;
  boolean haveclient ; /* accepted a client */
  boolean sockopen ;
  boolean sockfull ; /* last send failed */
  boolean terminate ;
  boolean report_discon ; /* report disconnetion */
  tns_par ns_par ; /* creation parameters */
#ifdef X86_WIN32
  SOCKET npath ; /* netserv socket */
  struct sockaddr nsockin, nsockout ; /* netserv address descriptors */
  SOCKET sockpath ;
  struct sockaddr client ;
#else
  integer npath ; /* commands socket */
  struct sockaddr nsockin, nsockout ; /* netserv address descriptors */
  integer sockpath ;
  struct sockaddr client ;
#endif
  integer nsq_in, nsq_out ;
  double last_sent ;
  completed_record sync_record ;
} tnsstr ;
typedef tnsstr *pnsstr ;

#ifdef X86_WIN32
static void create_mutex (pnsstr nsstr)
begin

  nsstr->mutex = CreateMutex(NIL, FALSE, NIL) ;
end

static void destroy_mutex (pnsstr nsstr)
begin

  CloseHandle (nsstr->mutex) ;
end

static void qlock (pnsstr nsstr)
begin

  WaitForSingleObject (nsstr->mutex, INFINITE) ;
end

static void qunlock (pnsstr nsstr)
begin

  ReleaseMutex (nsstr->mutex) ;
end

#else

static void create_mutex (pnsstr nsstr)
begin

  pthread_mutex_init (addr(nsstr->mutex), NULL) ;
end

static void destroy_mutex (pnsstr nsstr)
begin

  pthread_mutex_destroy (addr(nsstr->mutex)) ;
end

static void qlock (pnsstr nsstr)
begin

  pthread_mutex_lock (addr(nsstr->mutex)) ;
end

static void qunlock (pnsstr nsstr)
begin

  pthread_mutex_unlock (addr(nsstr->mutex)) ;
end

#endif

static void close_socket (pnsstr nsstr)
begin

  nsstr->sockopen = FALSE ;
  if (nsstr->npath != INVALID_SOCKET)
    then
      begin
#ifdef X86_WIN32
        closesocket (nsstr->npath) ;
#else
        close (nsstr->npath) ;
#endif
        nsstr->npath = INVALID_SOCKET ;
      end
  if (nsstr->sockpath != INVALID_SOCKET)
    then
      begin
#ifdef X86_WIN32
        closesocket (nsstr->sockpath) ;
#else
        close (nsstr->sockpath) ;
#endif
        nsstr->sockpath = INVALID_SOCKET ;
      end
end

static void open_socket (pnsstr nsstr)
begin
  integer lth, j ;
  integer err ;
  integer flag ;
  word host_port ;
  struct sockaddr xyz ;
  struct sockaddr_in *psock ;
  string63 s ;
#ifdef X86_WIN32
  BOOL flag2 ;
#else
  int flag2 ;
#endif
  struct linger lingeropt ;

  close_socket (nsstr) ;
  nsstr->npath = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP) ;
  if (nsstr->npath == INVALID_SOCKET)
    then
      begin
        err =
#ifdef X86_WIN32
               WSAGetLastError() ;
#else
               errno ;
#endif
        sprintf(s, "%d on netserv[%d] port", err, nsstr->ns_par.server_number) ;
        lib_msg_add(nsstr->ns_par.stnctx, AUXMSG_SOCKETERR, 0, addr(s)) ;
        return ;
      end
  psock = (pointer) addr(nsstr->nsockin) ;
  memset(psock, 0, sizeof(struct sockaddr)) ;
  psock->sin_family = AF_INET ;
  if (nsstr->ns_par.ns_port == PORT_OS)
    then
      psock->sin_port = 0 ;
    else
      psock->sin_port = htons(nsstr->ns_par.ns_port) ;
  psock->sin_addr.s_addr = INADDR_ANY ;
  flag2 = 1 ;
#ifdef X86_WIN32
  j = sizeof(BOOL) ;
  setsockopt (nsstr->npath, SOL_SOCKET, SO_REUSEADDR, addr(flag2), j) ;
#else
  j = sizeof(int) ;
  setsockopt (nsstr->npath, SOL_SOCKET, SO_REUSEADDR, addr(flag2), j) ;
#endif
  flag = sizeof(struct linger) ;
#ifdef X86_WIN32
  getsockopt (nsstr->npath, SOL_SOCKET, SO_LINGER, addr(lingeropt), addr(flag)) ;
#else
  getsockopt (nsstr->npath, SOL_SOCKET, SO_LINGER, addr(lingeropt), addr(flag)) ;
#endif
  if (lingeropt.l_onoff)
    then
      begin
        lingeropt.l_onoff = 0 ;
        lingeropt.l_linger = 0 ;
        flag = sizeof(struct linger) ;
#ifdef X86_WIN32
        setsockopt (nsstr->npath, SOL_SOCKET, SO_LINGER, addr(lingeropt), flag) ;
#else
        setsockopt (nsstr->npath, SOL_SOCKET, SO_LINGER, addr(lingeropt), flag) ;
#endif
      end
#ifdef X86_WIN32
  err = bind(nsstr->npath, addr(nsstr->nsockin), sizeof(struct sockaddr)) ;
  if (err)
#else
  err = bind(nsstr->npath, addr(nsstr->nsockin), sizeof(struct sockaddr)) ;
  if (err)
#endif
    then
      begin
        err =
#ifdef X86_WIN32
               WSAGetLastError() ;
        closesocket (nsstr->npath) ;
#else
               errno ;
        close (nsstr->npath) ;
#endif
        nsstr->npath = INVALID_SOCKET ;
        sprintf(s, "%d on netserv[%d] port", err, nsstr->ns_par.server_number) ;
        lib_msg_add(nsstr->ns_par.stnctx, AUXMSG_BINDERR, 0, addr(s)) ;
        return ;
      end
  flag = 1 ;
#ifdef X86_WIN32
  ioctlsocket (nsstr->npath, FIONBIO, addr(flag)) ;
  err = listen (nsstr->npath, 1) ;
#else
  ioctl (nsstr->npath, FIONBIO, addr(flag)) ;
  err = listen (nsstr->npath, 1) ;
#endif
  if (err)
    then
      begin
        err =
#ifdef X86_WIN32
               WSAGetLastError() ;
        closesocket (nsstr->npath) ;
#else
               errno ;
        close (nsstr->npath) ;
#endif
        nsstr->npath = INVALID_SOCKET ;
        sprintf(s, "%d on netserv[%d] port", err, nsstr->ns_par.server_number) ;
        lib_msg_add(nsstr->ns_par.stnctx, AUXMSG_LISTENERR, 0, addr(s)) ;
        return ;
      end
  lth = sizeof(struct sockaddr) ;
  getsockname (nsstr->npath, addr(xyz), addr(lth)) ;
  psock = (pointer) addr(xyz) ;
  host_port = ntohs(psock->sin_port) ;
  sprintf(s, "on netserv[%d] port %d", nsstr->ns_par.server_number, host_port) ;
  lib_msg_add(nsstr->ns_par.stnctx, AUXMSG_SOCKETOPEN, 0, addr(s)) ;
  nsstr->sockopen = TRUE ;
  nsstr->report_discon = FALSE ;
end

static void accept_ns_socket (pnsstr nsstr)
begin
  integer i, lth, err, err2 ;
  integer flag ;
  integer bufsize ;
  longword client_ip ;
  word client_port ;
  boolean found ;
  string15 hostname ;
  string63 s ;
  struct sockaddr_in *psock ;
  twhitelist *pwhite ;

  lth = sizeof(struct sockaddr) ;
  if (nsstr->npath == INVALID_SOCKET)
    then
      return ;
#ifdef X86_WIN32
  nsstr->sockpath = accept (nsstr->npath, addr(nsstr->client), addr(lth)) ;
#else
  nsstr->sockpath = accept (nsstr->npath, addr(nsstr->client), addr(lth)) ;
#endif
  if (nsstr->sockpath == INVALID_SOCKET)
    then
      begin
        err =
#ifdef X86_WIN32
               WSAGetLastError() ;
#else
               errno ;
#endif
        if ((err != EWOULDBLOCK) land (err != EINPROGRESS))
          then
            begin
#ifdef X86_WIN32
              err2 = closesocket (nsstr->npath) ;
#else
              err2 = close (nsstr->npath) ;
#endif
              nsstr->npath = INVALID_SOCKET ;
              sprintf(s, "%d on netserv[%d] port", err2, nsstr->ns_par.server_number) ;
              lib_msg_add(nsstr->ns_par.stnctx, AUXMSG_ACCERR, 0, addr(s)) ;
            end
      end
    else
      begin
        flag = 1 ;
#ifdef X86_WIN32
        ioctlsocket (nsstr->npath, FIONBIO, addr(flag)) ;
#else
        ioctl (nsstr->npath, FIONBIO, addr(flag)) ;
#endif
        psock = (pointer) addr(nsstr->client) ;
        showdot (ntohl(psock->sin_addr.s_addr), addr(hostname)) ;
        client_ip = ntohl(psock->sin_addr.s_addr) ;
        client_port = ntohs(psock->sin_port) ;
        if (nsstr->ns_par.whitecount > 0)
          then
            begin
              found = FALSE ;
              for (i = 1 ; i <= nsstr->ns_par.whitecount ; i++)
                begin
                  pwhite = addr(nsstr->ns_par.whitelist[i]) ;
                  if ((client_ip >= pwhite->lowip) land (client_ip <= pwhite->highip))
                    then
                      begin
                        found = TRUE ;
                        break ;
                      end
                end
              if (lnot found)
                then
                  begin
#ifdef X86_WIN32
                    closesocket (nsstr->npath) ;
#else
                    close (nsstr->npath) ;
#endif
                    nsstr->sockpath = INVALID_SOCKET ;
                    return ;
                  end
            end
        sprintf(s, "\"%s:%d\" to netserv[%d] port", addr(hostname), client_port, nsstr->ns_par.server_number) ;
        lib_msg_add (nsstr->ns_par.stnctx, AUXMSG_CONN, 0, addr(s)) ;
        lth = sizeof(integer) ;
#ifdef X86_WIN32
        err = getsockopt (nsstr->sockpath, SOL_SOCKET, SO_SNDBUF, addr(bufsize), addr(lth)) ;
#else
        err = getsockopt (nsstr->sockpath, SOL_SOCKET, SO_SNDBUF, addr(bufsize), addr(lth)) ;
#endif
        if ((err == 0) land (bufsize < 30000))
          then
            begin
              bufsize = 30000 ;
#ifdef X86_WIN32
              setsockopt (nsstr->sockpath, SOL_SOCKET, SO_SNDBUF, addr(bufsize), lth) ;
#else
              setsockopt (nsstr->sockpath, SOL_SOCKET, SO_SNDBUF, addr(bufsize), lth) ;
#endif
            end
#ifndef X86_WIN32
        flag = 1 ;
        lth = sizeof(integer) ;
#if defined(linux) || defined(solaris)
        signal (SIGPIPE, SIG_IGN) ;
#else
        setsockopt (nsstr->sockpath, SOL_SOCKET, SO_NOSIGPIPE, addr(flag), lth) ;
#endif
#endif
        nsstr->haveclient = TRUE ;
        nsstr->last_sent = now () ;
        nsstr->sockfull = FALSE ;
        nsstr->report_discon = FALSE ;
      end
end

static void read_from_client (pnsstr nsstr)
begin
#define RBUFSIZE 100
  integer err ;
  byte buf[RBUFSIZE] ;
  string63 s ;

  repeat
    if (lnot nsstr->haveclient)
      then
        return ;
    err = recv(nsstr->sockpath, addr(buf), RBUFSIZE, 0) ;
    if (err == SOCKET_ERROR)
      then
        begin
#ifdef X86_WIN32
               WSAGetLastError() ;
#else
               errno ;
#endif
          if ((err == ECONNRESET) lor (err == ECONNABORTED))
            then
              begin
#ifdef X86_WIN32
                closesocket (nsstr->sockpath) ;
#else
                close (nsstr->sockpath) ;
#endif
                sprintf(s, "netserv[%d] port", nsstr->ns_par.server_number) ;
                lib_msg_add(nsstr->ns_par.stnctx, AUXMSG_DISCON, 0, addr(s)) ;
                nsstr->haveclient = FALSE ;
                nsstr->sockfull = FALSE ;
                return ;
              end
        end
  until (err == 0)) ; /* nothing left in buffer */
end

static integer wrap_buffer (integer max, integer i)
begin
  integer j ;

  j = i + 1 ;
  if (j >= max)
    then
      j = 0 ;
  return j ;
end

/* returns TRUE if error */
static boolean send_netserv_packet (pnsstr nsstr, pointer buf)
begin
  integer err ;

  if (lnot nsstr->haveclient)
    then
      return TRUE ;
#if defined(linux)
  err = send(nsstr->sockpath, buf, LIB_REC_SIZE, MSG_NOSIGNAL) ;
#else
  err = send(nsstr->sockpath, buf, LIB_REC_SIZE, 0) ;
#endif
  if (err == SOCKET_ERROR)
    then
      begin
        err =
#ifdef X86_WIN32
               WSAGetLastError() ;
#else
               errno ;
#endif
        if (err == EWOULDBLOCK)
          then
            begin
              nsstr->sockfull = TRUE ;
              return TRUE ;
            end
#ifdef X86_WIN32
        else if ((err == ECONNRESET) lor (err == ECONNABORTED))
#else
        else if (err = EPIPE)
#endif
          then
            begin
#ifdef X86_WIN32
              closesocket (nsstr->sockpath) ;
#else
              close (nsstr->sockpath) ;
#endif
              nsstr->report_discon = TRUE ;
              nsstr->haveclient = FALSE ;
              nsstr->sockfull = FALSE ;
              return TRUE ;
            end
      end
    else
      nsstr->sockfull = FALSE ;
  nsstr->last_sent = now () ;
  return FALSE ;
end

static void send_next_buffer (pnsstr nsstr)
begin
  boolean failed ;

  if (lnot nsstr->haveclient)
    then
      return ;
  failed = send_netserv_packet (nsstr, addr((*(nsstr->ns_par.nsbuf))[nsstr->nsq_out])) ;
  if (lnot failed)
    then
      nsstr->nsq_out = wrap_buffer(nsstr->ns_par.record_count, nsstr->nsq_out) ; /* sucessful */
end

void lib_ns_send (pointer ct, pcompleted_record pbuf)
begin
  pnsstr nsstr ;
  integer nq ;
  boolean wasempty ;

  nsstr = ct ;
  qlock (nsstr) ;
  wasempty = (nsstr->nsq_in == nsstr->nsq_out) ;
  nq = wrap_buffer (nsstr->ns_par.record_count, nsstr->nsq_in) ; /* next pointer after we insert new record */
  if (nq == nsstr->nsq_out)
    then
      nsstr->nsq_out = wrap_buffer(nsstr->ns_par.record_count, nsstr->nsq_out) ; /* throw away oldest */
  memcpy(addr((*(nsstr->ns_par.nsbuf))[nsstr->nsq_in]), pbuf, LIB_REC_SIZE) ;
  nsstr->nsq_in = nq ;
  if (wasempty)
    then
      send_next_buffer (nsstr) ;
  qunlock (nsstr) ;
end

#ifdef X86_WIN32
unsigned long  __stdcall nsthread (pointer p)
begin
  pnsstr nsstr ;
  fd_set readfds, writefds, exceptfds ;
  struct timeval timeout ;
  integer res ;
  boolean sent ;
  string95 s ;

  nsstr = p ;
  repeat
    if (nsstr->sockopen)
      then
        begin /* wait for socket input or timeout */
          FD_ZERO (addr(readfds)) ;
          FD_ZERO (addr(writefds)) ;
          FD_ZERO (addr(exceptfds)) ;
          if (lnot nsstr->haveclient)
            then
              FD_SET (nsstr->npath, addr(readfds)) ; /* waiting for accept */
            else
              begin
                FD_SET (nsstr->sockpath, addr(readfds)) ; /* client might try to send me something */
                if (nsstr->sockfull)
                  then
                    FD_SET (nsstr->sockpath, addr(writefds)) ; /* buffer was full */
              end
          timeout.tv_sec = 0 ;
          timeout.tv_usec = 25000 ; /* 25ms timeout */
          res = select (0, addr(readfds), addr(writefds), addr(exceptfds), addr(timeout)) ;
          if (res > 0)
            then
              begin
                if (FD_ISSET (nsstr->npath, addr(readfds)))
                  then
                    accept_ns_socket (nsstr) ;
                else if (nsstr->haveclient)
                  then
                    begin
                      if (FD_ISSET (nsstr->sockpath, addr(readfds)))
                        then
                          read_from_client (nsstr) ;
                      if ((nsstr->sockfull) land (FD_ISSET (nsstr->sockpath, addr(writefds))))
                        then
                          nsstr->sockfull = FALSE ;
                    end
              end
          if (nsstr->haveclient)
            then
              begin
                sent = FALSE ;
                if (nsstr->sockfull)
                  then
                    sleepms (25) ;
                  else
                    begin
                      qlock (nsstr) ;
                      if (nsstr->nsq_in != nsstr->nsq_out)
                        then
                          begin
                            sent = TRUE ;
                            send_next_buffer (nsstr) ;
                          end
                      qunlock (nsstr) ;
                      if (lnot sent)
                        then
                          begin
                            if ((nsstr->ns_par.sync_time) land ((now () - nsstr->last_sent) >= nsstr->ns_par.sync_time))
                              then
                                send_netserv_packet (nsstr, addr(nsstr->sync_record)) ;
                              else
                                sleepms (25) ;
                          end
                    end
              end
        end
      else
        sleepms (25) ;
    if (nsstr->report_discon)
      then
        begin
          nsstr->report_discon = FALSE ;
          sprintf(s, "netserv[%d] port", nsstr->ns_par.server_number) ;
          lib_msg_add(nsstr->ns_par.stnctx, AUXMSG_DISCON, 0, addr(s)) ;
        end
  until nsstr->terminate) ;
  nsstr->running = FALSE ;
  ExitThread (0) ;
  return 0 ;
end
#else
void *nsthread (pointer p)
begin
  pnsstr nsstr ;
  fd_set readfds, writefds, exceptfds ;
  struct timeval timeout ;
  integer res ;
  boolean sent ;
  string95 s ;

  nsstr = p ;
  repeat
    if (nsstr->sockopen)
      then
        begin /* wait for socket input or timeout */
          FD_ZERO (addr(readfds)) ;
          FD_ZERO (addr(writefds)) ;
          FD_ZERO (addr(exceptfds)) ;
          if (lnot nsstr->haveclient)
            then
              FD_SET (nsstr->npath, addr(readfds)) ; /* waiting for accept */
            else
              begin
                FD_SET (nsstr->sockpath, addr(readfds)) ; /* client might try to send me something */
                if (nsstr->sockfull)
                  then
                    FD_SET (nsstr->sockpath, addr(writefds)) ; /* buffer was full */
              end
          timeout.tv_sec = 0 ;
          timeout.tv_usec = 25000 ; /* 25ms timeout */
          res = select (getdtablesize(), addr(readfds), addr(writefds), addr(exceptfds), addr(timeout)) ;
          if (res > 0)
            then
              begin
                if (FD_ISSET (nsstr->npath, addr(readfds)))
                  then
                    accept_ns_socket (nsstr) ;
                else if (nsstr->haveclient)
                  then
                    begin
                      if (FD_ISSET (nsstr->sockpath, addr(readfds)))
                        then
                          read_from_client (nsstr) ;
                      if ((nsstr->sockfull) land (FD_ISSET (nsstr->sockpath, addr(writefds))))
                        then
                          nsstr->sockfull = FALSE ;
                    end
              end
          if (nsstr->haveclient)
            then
              begin
                sent = FALSE ;
                if (nsstr->sockfull)
                  then
                    sleepms (25) ;
                  else
                    begin
                      qlock (nsstr) ;
                      if (nsstr->nsq_in != nsstr->nsq_out)
                        then
                          begin
                            sent = TRUE ;
                            send_next_buffer (nsstr) ;
                          end
                      qunlock (nsstr) ;
                      if (lnot sent)
                        then
                          begin
                            if ((nsstr->ns_par.sync_time) land ((now () - nsstr->last_sent) >= nsstr->ns_par.sync_time))
                              then
                                send_netserv_packet (nsstr, addr(nsstr->sync_record)) ;
                              else
                                sleepms (25) ;
                          end
                    end
              end
        end
      else
        sleepms (25) ;
    if (nsstr->report_discon)
      then
        begin
          nsstr->report_discon = FALSE ;
          sprintf(s, "netserv[%d] port", nsstr->ns_par.server_number) ;
          lib_msg_add(nsstr->ns_par.stnctx, AUXMSG_DISCON, 0, addr(s)) ;
        end
  until nsstr->terminate) ;
  nsstr->running = FALSE ;
  pthread_exit (0) ;
end
#endif

pointer lib_ns_start (tns_par *nspar)
begin
  pnsstr nsstr ;
  char *p ;
  integer i ;
#ifndef X86_WIN32
  integer err ;
#endif

  nsstr = malloc (sizeof(tnsstr)) ;
  memset (nsstr, 0, sizeof(tnsstr)) ;
  memcpy (addr(nsstr->ns_par), nspar, sizeof(tns_par)) ;
  memset (nsstr->sync_record,' ', LIB_REC_SIZE) ;
  p = addr(nsstr->sync_record) ;
  for (i = 1 ; i <= 6 ; i++)
    begin
      *p = '0' ;
      inc(p) ;
    end
  create_mutex (nsstr) ;
  nsstr->npath = INVALID_SOCKET ;
  nsstr->sockpath = INVALID_SOCKET ;
  open_socket (nsstr) ;
  if (lnot nsstr->sockopen)
    then
      begin
        free (nsstr) ;
        return NIL ;
      end
#ifdef X86_WIN32
  nsstr->threadhandle = CreateThread (NIL, 0, nsthread, nsstr, 0, addr(nsstr->threadid)) ;
  if (nsstr->threadhandle == NIL)
#else
  err = pthread_create(addr(nsstr->threadid), NULL, nsthread, nsstr) ;
  if (err)
#endif
    then
      begin
        free (nsstr) ;
        return NIL ;
      end
  nsstr->running = TRUE ;
  return nsstr ; /* return running context */
end

void lib_ns_stop (pointer ct)
begin
  pnsstr nsstr ;

  nsstr = ct ;
  nsstr->terminate = TRUE ;
  repeat
    sleepms (25) ;
  until (lnot nsstr->running)) ;
  close_socket (nsstr) ;
  destroy_mutex (nsstr) ;
end

#endif
