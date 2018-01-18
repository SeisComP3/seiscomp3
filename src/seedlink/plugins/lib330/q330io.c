/*   Lib330 Q330 I/O Communications routine
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
    0 2006-09-29 rdr Created
    1 2006-10-26 rdr Add setting of current_ip and current_port.
    2 2006-10-29 rdr Add include for q330io's header file as well as defining local
                     routines as static.
    3 2006-10-31 rdr Add setting of current_ip and current_port for serial operation.
    4 2006-12-18 rdr Use ctrlport and dataport to store current port numbers instead of
                     host_ctrlport and host_dataport.
    5 2007-06-26 rdr Fix mask conversion in function decode for base-96.
    6 2007-06-27 rdr Fix return type of cksum function. Change method of setting serial baud.
                     Initialize control character array. Add conditional for flow control
                     constants.
    7 2007-07-16 rdr Add support for baler simulation on serial port.
    8 2007-08-04 rdr Add conditionals for omitting network code. Add CMEX32 support.
                     Add baler callback for foreign received packets along with packet
                     injection routine.
    9 2007-08-21 rdr Add baler socket setup callback.
   10 2007-10-03 rdr Remove setting access_timer.
   11 2008-08-20 rdr Add TCP support.
   12 2010-01-04 rdr Use fcntl instead of ioctl to set socket non-blocking.
   13 2010-03-27 rdr Add Q335 support.
   14 2010-05-13 rdr Add detection of 127.0.0.1 as additional baler port.
*/
#ifdef CMEX32
#include "cmexserial.h"
#endif
#ifndef q330io_h
#include "q330io.h"
#endif
#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif
#ifndef libmsgs_h
#include "libmsgs.h"
#endif
#ifndef libcmds_h
#include "libcmds.h"
#endif
#ifndef libclient_h
#include "libclient.h"
#endif
#ifndef q330cvrt_h
#include "q330cvrt.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif
#ifndef libcvrt_h
#include "libcvrt.h"
#endif
#ifndef libslider_h
#include "libslider.h"
#endif
#ifndef libstats_h
#include "libstats.h"
#endif

#ifndef OMIT_SERIAL
#define INQSIZE 20000
#define OUTQSIZE 5000
  /* IP constants */
#define IPT_ICMP 1 /* protocol type for ICMP packets */
#define IP_VERSION 4 /* current version value */
#define CIP_TTL 255 /* Initial time-to-live value */
#define IPP_NORMAL 0x00 /* normal */
#define IP_MF 0x2000 /* more fragments bit */
#define IP_DF 0x4000 /* don't fragment bit */
#define IP_FRAGMASK 0x1fff /* fragment offset mask */
  /* SLIP */
#define SLIP_FRM 0xC0 /* Framing character */
#define SLIP_ESC 0xDB /* Escape character, special follows */
#define ESC_FRM 0xDC /* SLIP_ESC|ESC_FRM = 0xC0 */
#define ESC_ESC 0xDD /* SLIP_ESC|ESC_ESC = 0xDB */
#endif

#define LOOPBACK_PORT 2066 /* For getting C2_BACK */

void close_sockets (pq330 q330)
begin

#ifndef OMIT_NETWORK
  if (q330->usesock)
    then
      begin
        if (q330->cpath != INVALID_SOCKET)
          then
            begin
#ifdef X86_WIN32
              closesocket (q330->cpath) ;
#else
              close (q330->cpath) ;
#endif
              q330->cpath = INVALID_SOCKET ;
            end
        if (q330->dpath != INVALID_SOCKET)
          then
            begin
#ifdef X86_WIN32
              closesocket (q330->dpath) ;
#else
              close (q330->dpath) ;
#endif
              q330->dpath = INVALID_SOCKET ;
            end
      end
#endif
#ifndef OMIT_SERIAL
  if (q330->usesock == 0)
    then
      begin
        if (q330->comid != INVALID_IO_HANDLE)
          then
            begin
#ifdef X86_WIN32
              CloseHandle(q330->comid) ;
#else
              close(q330->comid) ;
#endif
              q330->comid = INVALID_IO_HANDLE ;
            end
      end
#endif
end

#ifndef OMIT_NETWORK
static longword baler_socket (pq330 q330, pntrint sockpath, enum tbaler_socket socktype)
begin

  if (q330->par_create.call_baler)
    then
      begin
        q330->baler_call.context = q330 ;
        q330->baler_call.baler_type = BT_SOCKET ;
        memcpy(addr(q330->baler_call.station_name), addr(q330->station_ident), sizeof(string9)) ;
        q330->baler_call.info = socktype ;
        q330->baler_call.info2 = (void *) sockpath ;
        q330->baler_call.response = 0 ;
        q330->par_create.call_baler (addr(q330->baler_call)) ;
        return q330->baler_call.response ;
      end
    else
      return 0 ;
end

void tcp_error (pq330 q330, string95 *msgsuf)
begin

  lib_change_state (q330, LIBSTATE_WAIT, LIBERR_NOTR) ;
  close_sockets (q330) ;
  q330->reg_wait_timer = 60 * 10 ;
  q330->registered = FALSE ;
  libmsgadd (q330, LIBMSG_TCPTUN, msgsuf) ;
end

boolean open_sockets (pq330 q330, boolean both, boolean fromback)
begin
  integer lth, j, err ;
  integer flag ;
  integer bufsize ;
  struct sockaddr xyz ;
  boolean isd ;
#ifdef X86_WIN32
  BOOL flag2 ;
#else
  int flag2 ;
#endif
  struct sockaddr_in *psock ;
  string95 msg ;

  if (lnot q330->usesock)
    then
      return FALSE ;
  close_sockets (q330) ;
  q330->got_connected = TRUE ;
  if (lnot fromback)
    then
      begin /* C2_BACK sets q330ip and ports */
        if (strcmp(q330->par_register.q330id_address, "255.255.255.255") == 0)
          then
            begin
              q330->q330ip = 0xFFFFFFFF ;
              q330->q330cport = 0xFFFF ;
              both = FALSE ; /* don't both with data port for announcement */
              q330->balesim = TRUE ;
              q330->tcp = FALSE ; /* TCP doesn't do broadcasts */
            end
        else if (strcmp(q330->par_register.q330id_address, "127.0.0.1") == 0)
          then
            begin
              q330->q330ip = 0x7F000001 ;
              q330->q330cport = LOOPBACK_PORT ;
              both = FALSE ; /* don't both with data port for announcement */
              q330->balesim = TRUE ;
              q330->tcp = FALSE ; /* TCP doesn't do broadcasts */
            end
          else
            begin
              q330->balesim = FALSE ;
              q330->q330ip = getip (q330->par_register.q330id_address, addr(isd)) ;
              if (q330->q330ip == INADDR_NONE)
                then
                  begin
                    libmsgadd(q330, LIBMSG_BADIPADDR, "") ;
                    return TRUE ;
                  end
            end
      end
  if (q330->tcp)
    then
      q330->cpath = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP) ;
    else
      q330->cpath = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP) ;
  if (q330->cpath == INVALID_SOCKET)
    then
      begin
        err =
#ifdef X86_WIN32
               WSAGetLastError() ;
#else
               errno ;
#endif
        if (q330->tcp)
          then
            sprintf(addr(msg), "%d on tcp port", err) ;
          else
            sprintf(addr(msg), "%d on control port", err) ;
        libmsgadd(q330, LIBMSG_SOCKETERR, addr(msg)) ;
        return TRUE ;
      end
#ifndef X86_WIN32
  if (q330->cpath > q330->high_socket)
    then
      q330->high_socket = q330->cpath ;
#endif
  psock = (pointer) addr(q330->csockout) ;
  memset(psock, 0, sizeof(struct sockaddr)) ;
  psock->sin_family = AF_INET ;
  psock->sin_port = htons(q330->q330cport) ;
  psock->sin_addr.s_addr = htonl(q330->q330ip) ;
  flag = 1 ;
#ifdef X86_WIN32
  ioctlsocket (q330->cpath, FIONBIO, addr(flag)) ;
#else
  flag = fcntl (q330->cpath, F_GETFL, 0) ;
  fcntl (q330->cpath, F_SETFL, flag or O_NONBLOCK) ;
#endif
  if (q330->tcp)
    then
      begin
        q330->tcpidx = 0 ;
        flag2 = 1 ;
#ifdef X86_WIN32
        j = sizeof(BOOL) ;
        setsockopt (q330->cpath, SOL_SOCKET, SO_REUSEADDR, addr(flag2), j) ;
#else
        j = sizeof(int) ;
        setsockopt (q330->cpath, SOL_SOCKET, SO_REUSEADDR, addr(flag2), j) ;
#endif
        err = connect (q330->cpath, addr(q330->csockout), sizeof(struct sockaddr)) ;
        if (err)
          then
            begin
              err =
#ifdef X86_WIN32
                     WSAGetLastError() ;
#else
                     errno ;
#endif
              if (err != EWOULDBLOCK)
                then
                  begin
                    close_sockets (q330) ;
                    sprintf(addr(msg), "%d on tcp port", err) ;
                    libmsgadd(q330, LIBMSG_SOCKETERR, addr(msg)) ;
                    return TRUE ;
                  end
              q330->got_connected = FALSE ; /* OK, but not yet connected */
            end
      end
    else
      begin
        psock = (pointer) addr(q330->csockin) ;
        memset(psock, 0, sizeof(struct sockaddr)) ;
        psock->sin_family = AF_INET ;
        psock->sin_port = htons(q330->par_register.host_ctrlport) ;
        psock->sin_addr.s_addr = INADDR_ANY ;
#ifdef X86_WIN32
        err = bind(q330->cpath, addr(q330->csockin), sizeof(struct sockaddr)) ;
        if (err)
#else
        err = bind(q330->cpath, addr(q330->csockin), sizeof(struct sockaddr)) ;
        if (err)
#endif
          then
            begin
              err =
#ifdef X86_WIN32
                     WSAGetLastError() ;
              closesocket (q330->cpath) ;
#else
                     errno ;
              close (q330->cpath) ;
#endif
              q330->cpath = INVALID_SOCKET ;
              sprintf(addr(msg), "%d on control port", err) ;
              libmsgadd(q330, LIBMSG_BINDERR, addr(msg)) ;
              return TRUE ;
            end
        if (q330->q330ip == 0xFFFFFFFF)
          then /* Enable sending broadcasts */
            baler_socket (q330, q330->cpath, BS_BCASTCTRL) ;
          else
            baler_socket (q330, q330->cpath, BS_CONTROL) ;
      end
  lth = sizeof(struct sockaddr) ;
#ifdef X86_WIN32
  getsockname (q330->cpath, addr(xyz), addr(lth)) ;
#else
  getsockname (q330->cpath, addr(xyz), addr(lth)) ;
#endif
  psock = (pointer) addr(xyz) ;
  q330->ctrlport = ntohs(psock->sin_port) ;
  if (q330->tcp)
    then
      sprintf(addr(msg), "on tcp port %d", q330->ctrlport) ;
    else
      sprintf(addr(msg), "on control port %d", q330->ctrlport) ;
  libmsgadd(q330, LIBMSG_SOCKETOPEN, addr(msg)) ;
  q330->share.opstat.current_ip = q330->q330ip ;
  q330->share.opstat.current_port = q330->q330cport ;
  if ((both) land (lnot q330->tcp))
    then
      begin
        q330->dpath = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP) ;
        if (q330->dpath == INVALID_SOCKET)
          then
            begin
              err =
#ifdef X86_WIN32
                     WSAGetLastError() ;
#else
                     errno ;
#endif
              sprintf(addr(msg), "%d on data port", err) ;
              libmsgadd(q330, LIBMSG_SOCKETERR, addr(msg)) ;
              return TRUE ;
            end
        lth = sizeof(longint) ;
#ifdef X86_WIN32
        err = getsockopt (q330->dpath, SOL_SOCKET, SO_RCVBUF, addr(bufsize), addr(lth)) ;
#else
        if (q330->dpath > q330->high_socket)
          then
            q330->high_socket = q330->dpath ;
        err = getsockopt (q330->dpath, SOL_SOCKET, SO_RCVBUF, addr(bufsize), addr(lth)) ;
#endif
        if ((err == 0) land (bufsize < 30000))
          then
            begin
              bufsize = 30000 ;
#ifdef X86_WIN32
              setsockopt (q330->dpath, SOL_SOCKET, SO_RCVBUF, addr(bufsize), lth) ;
#else
              setsockopt (q330->dpath, SOL_SOCKET, SO_RCVBUF, addr(bufsize), lth) ;
#endif
            end
        psock = (pointer) addr(q330->dsockin) ;
        memset(psock, 0, sizeof(struct sockaddr)) ;
        psock->sin_family = AF_INET ;
        psock->sin_port = htons(q330->par_register.host_dataport) ;
        psock->sin_addr.s_addr = INADDR_ANY ;
#ifdef X86_WIN32
        err = bind(q330->dpath, addr(q330->dsockin), sizeof(struct sockaddr)) ;
        if (err)
#else
        err = bind(q330->dpath, addr(q330->dsockin), sizeof(struct sockaddr)) ;
        if (err)
#endif
          then
            begin
              err =
#ifdef X86_WIN32
                     WSAGetLastError() ;
              closesocket (q330->dpath) ;
#else
                     errno ;
              close (q330->dpath) ;
#endif
              q330->dpath = INVALID_SOCKET ;
              sprintf(addr(msg), "%d on data port", err) ;
              libmsgadd(q330, LIBMSG_BINDERR, addr(msg)) ;
              return TRUE ;
            end
        baler_socket (q330, q330->dpath, BS_DATA) ;
        psock = (pointer) addr(q330->dsockout) ;
        memset(psock, 0, sizeof(struct sockaddr)) ;
        psock->sin_family = AF_INET ;
        psock->sin_port = htons(q330->q330dport) ;
        psock->sin_addr.s_addr = htonl(q330->q330ip) ;
        flag = 1 ;
        lth = sizeof(struct sockaddr) ;
#ifdef X86_WIN32
        ioctlsocket (q330->dpath, FIONBIO, addr(flag)) ;
        getsockname (q330->dpath, addr(xyz), addr(lth)) ;
#else
        flag = fcntl (q330->dpath, F_GETFL, 0) ;
        fcntl (q330->dpath, F_SETFL, flag or O_NONBLOCK) ;
        getsockname (q330->dpath, addr(xyz), addr(lth)) ;
#endif
        psock = (pointer) addr(xyz) ;
        q330->dataport = ntohs(psock->sin_port) ;
        sprintf(addr(msg), "on data port %d", q330->dataport) ;
        libmsgadd(q330, LIBMSG_SOCKETOPEN, addr(msg)) ;
      end
  return FALSE ;
end
#endif

/* because of the original encoding, lth will always be a multiple of
  4 bytes (1 group) plus 2. Returns -1 if not valid */
static integer decode (pq330 q330, integer lth)
begin
typedef byte tenc[4] ;
  integer groups, actual, diff ;
  pbyte p, psave, pdest ;
  tenc enc ;
  tenc *psrc ;
  byte m ;
  integer mask ;
  char *pmask ;
  longint thiscrc ;

  psave = (pointer) addr(q330->datain.qdp) ;
  pmask = (pointer) psave ;
  /* NOTE: I was unable to find a C routine that would convert hexadecimal string
     to binary AND clearly indicate that the input was not valid, so do the hard way */
  mask = 0 ;
  m = *pmask++ ;
  if ((m >= '0') land (m <= '9'))
    then
      mask = m - 0x30 ;
  else if ((m >= 'A') land (m <= 'F'))
    then
      mask = m - 0x37 ;
    else
      return -1 ; /* not valid */
  m = *pmask++ ;
  if ((m >= '0') land (m <= '9'))
    then
      mask = (mask shl 4) + (m - 0x30) ;
  else if ((m >= 'A') land (m <= 'F'))
    then
      mask = (mask shl 4) + (m - 0x37) ;
    else
      return -1 ; /* not valid */
  psrc = (pointer) pmask ; /* skipped over encoding */
  pdest = psave ;
  groups = (lth - 2) shr 2 ;
  actual = groups * 3 ; /* decoded byte count */
  while (groups > 0)
    begin
      memcpy(addr(enc), psrc, 4) ;
      psrc++ ;
      m = enc[3] - 0x20 ;
      *pdest++ = (enc[0] - 0x20 + ((m and 0x30) shl 2)) xor mask ;
      *pdest++ = (enc[1] - 0x20 + ((m and 0xc) shl 4)) xor mask ;
      *pdest++ = (enc[2] - 0x20 + ((m and 3) shl 6)) xor mask ;
      dec(groups) ;
    end
  p = psave ;
  loadqdphdr (addr(p), addr(q330->recvhdr)) ;
  diff = actual - (q330->recvhdr.datalength + QDP_HDR_LTH) ;
  if ((diff >= 0) land (diff <= 2))
    then
      actual = q330->recvhdr.datalength + QDP_HDR_LTH ; /* replace with exact length */
    else
      return -1 ; /* no good, return */
  thiscrc = gcrccalc (addr(q330->crc_table), (pointer)((pntrint)psave + 4), actual - 4) ;
  if (thiscrc == q330->recvhdr.crc)
    then
      return actual ; /* good crc, return actual length */
    else
      return -1 ; /* no good */
end

static integer check_crc (pq330 q330, integer lth)
begin
  longint thiscrc ;
  pbyte p ;

  p = addr(q330->datain.qdp) ;
  thiscrc = gcrccalc (addr(q330->crc_table), (pointer)((pntrint)p + 4), lth - 4) ;
  loadqdphdr (addr(p), addr(q330->recvhdr)) ;
  if (thiscrc == q330->recvhdr.crc)
    then
      return lth ; /* good crc, return actual length */
    else
      return -1 ; /* bad crc */
end

static void check_for_encoded (pq330 q330, integer plth)
begin
  integer actual ;
  word flgs ;

  lock (q330) ;
  flgs = q330->share.log.flags ;
  unlock (q330) ;
  if (flgs and LNKFLG_BASE96)
    then
      begin /* am expecting encoded */
        memcpy (addr(q330->datasave.qdp), addr(q330->datain.qdp), plth) ;
        actual = decode(q330, plth) ; /* convert to binary */
        if (actual < 0)
          then
            begin
              memcpy (addr(q330->datain.qdp), addr(q330->datasave.qdp), plth) ;
              actual = check_crc (q330, plth) ;
            end
      end
    else
      begin /* am expecting binary */
        actual = check_crc (q330, plth) ;
        if (actual < 0)
          then
            actual = decode(q330, plth) ; /* convert to binary */
      end
  if (actual < 0)
    then
      add_status (q330, AC_CHECK, 1) ;
    else
      process_data (q330) ;
end

#ifndef OMIT_NETWORK
void read_data_socket (pq330 q330)
begin
  integer lth, err ;
  string95 msg ;

  if (q330->dpath == INVALID_SOCKET)
    then
      return ;
  lth = sizeof(struct sockaddr) ;
  err = recvfrom (q330->dpath, addr(q330->datain.qdp), QDP_HDR_LTH + MAXDATA96, 0, addr(q330->dsockin), addr(lth)) ;
  if (err == SOCKET_ERROR)
    then
      begin
        err =
#ifdef X86_WIN32
               WSAGetLastError() ;
#else
               errno ;
#endif
        if (err != EWOULDBLOCK)
          then
            if (err == ECONNRESET)
              then
                begin
                  purge_cmdq (q330) ;
                  set_liberr (q330, LIBERR_NOTR) ;
                  close_sockets (q330) ;
                  q330->reg_wait_timer = 60 * 10 ;
                  if ((q330->libstate == LIBSTATE_RUNWAIT) lor (q330->libstate == LIBSTATE_RUN))
                    then
                      begin
                        start_deallocation (q330) ;
                        libmsgadd (q330, LIBMSG_ROUTEFAULT, "Deallocating and waiting 10 minutes") ;
                      end
                    else
                      begin
                        new_state (q330, LIBSTATE_WAIT) ;
                        q330->registered = FALSE ;
                        libmsgadd (q330, LIBMSG_ROUTEFAULT, "Waiting 10 minutes") ;
                      end
                end
              else
                begin
                  sprintf(msg, "%d", err) ;
                  libmsgadd(q330, LIBMSG_RECVERR, addr(msg)) ;
                  add_status (q330, AC_IOERR, 1) ; /* add one I/O error */
                end
      end
  else if (err > 0)
    then
      begin
        add_status (q330, AC_READ, err + IP_HDR_LTH + UDP_HDR_LTH) ;
        check_for_encoded (q330, err) ;
      end
end

static void process_cmd_socket (pq330 q330, integer msglth, boolean isdata)
begin
  longint thiscrc ;
  pbyte p ;

  add_status (q330, AC_READ, msglth + IP_HDR_LTH + UDP_HDR_LTH) ;
  p = addr(q330->commands.cmsgin.qdp) ;
  thiscrc = gcrccalc (addr(q330->crc_table), (pointer)((pntrint)p + 4), msglth - 4) ;
  loadqdphdr (addr(p), addr(q330->recvhdr)) ;
  if (thiscrc != q330->recvhdr.crc)
    then
      add_status (q330, AC_CHECK, 1) ;
  else if ((q330->tcp) land (isdata))
    then
      begin /* this is actually a data packet */
        memcpy (addr(q330->datain.qdp), addr(q330->commands.cmsgin.qdp), msglth) ; /* where it's expected */
        check_for_encoded (q330, msglth) ;
      end
    else
      lib_command_response (q330, p) ;
end

void read_cmd_socket (pq330 q330)
begin
  integer lth ;
  pbyte p ;
  integer err ;
  word poff, qdplth ;
  string95 msg ;

  if (q330->cpath == INVALID_SOCKET)
    then
      return ;
  if (q330->tcp)
    then
      begin
        err = recv (q330->cpath, (pchar) addr((q330->tcpbuf)[q330->tcpidx]), TCPBUFSZ - q330->tcpidx, 0) ;
        if (err == SOCKET_ERROR)
          then
            begin
              err =
#ifdef X86_WIN32
                    WSAGetLastError() ;
#else
                    errno ;
#endif
              if (err != EWOULDBLOCK)
                then
                  begin
                    sprintf(msg, "%d, Waiting 10 minutes", err) ;
                    tcp_error (q330, addr(msg)) ;
                  end
            end
        else if (err == 0)
          then
            tcp_error (q330, "Connection Closed, Waiting 10 minutes") ; /* connection closed */
        else if (err > 0)
          then
            begin
              q330->tcpidx = q330->tcpidx + err ;
              while (q330->tcpidx >= 4)
                begin
                  p = addr(q330->tcpbuf) ;
                  poff = loadword (addr(p)) ;
                  qdplth = loadword (addr(p)) ;
                  if ((poff > 1) lor (qdplth < sizeof(tqdp)) lor
                      (qdplth > MAXMTU))
                    then
                      begin
                        tcp_error (q330, "Invalid header, Waiting 10 minutes") ;
                        return ;
                      end
                  if (q330->tcpidx >= (qdplth + 4))
                    then
                      begin /* have a full packet, or more */
                        err = qdplth ; /* actual size */
                        memcpy (addr(q330->commands.cmsgin.qdp), addr((q330->tcpbuf)[4]), err) ;
                        q330->tcpidx = q330->tcpidx - (qdplth + 4) ; /* amount left over */
                        if (q330->tcpidx > 0)
                          then
                            memcpy (addr(q330->tcpbuf), addr((q330->tcpbuf)[qdplth + 4]), q330->tcpidx) ; /* move to start of buffer */
                        process_cmd_socket (q330, err, (poff != 0)) ;
                      end
                    else
                      break ; /* not enough to process */
                end
            end
      end
    else
      begin
        lth = sizeof(struct sockaddr) ;
        err = recvfrom (q330->cpath, addr(q330->commands.cmsgin.qdp), QDP_HDR_LTH + MAXDATA96, 0, addr(q330->csockin), addr(lth)) ;
        if (err == SOCKET_ERROR)
          then
            begin
              err =
#ifdef X86_WIN32
                     WSAGetLastError() ;
#else
                     errno ;
#endif
              if (err != EWOULDBLOCK)
                then
                  if (err == ECONNRESET)
                    then
                      begin
                        purge_cmdq (q330) ;
                        set_liberr (q330, LIBERR_NOTR) ;
                        close_sockets (q330) ;
                        q330->reg_wait_timer = 60 * 10 ;
                        if ((q330->libstate == LIBSTATE_RUNWAIT) lor (q330->libstate == LIBSTATE_RUN))
                          then
                            begin
                              start_deallocation (q330) ;
                              libmsgadd (q330, LIBMSG_ROUTEFAULT, "Deallocating and waiting 10 minutes") ;
                            end
                          else
                            begin
                              new_state (q330, LIBSTATE_WAIT) ;
                              q330->registered = FALSE ;
                              q330->share.target_state = LIBSTATE_WAIT ;
                              libmsgadd (q330, LIBMSG_ROUTEFAULT, "Waiting 10 minutes") ;
                            end
                      end
                    else
                      begin
                        sprintf(msg, "%d", err) ;
                        libmsgadd(q330, LIBMSG_RECVERR, addr(msg)) ;
                        add_status (q330, AC_IOERR, 1) ; /* add one I/O error */
                      end
            end
        else if (err > 0)
          then
            process_cmd_socket (q330, err, FALSE) ;
      end
end
#endif

#ifndef OMIT_SERIAL
static word cksum(ppsuedo psuedo, pointer data, integer count)
begin
  word *pw ;
  longint csum ;
  integer loop ;
  pbyte p, pb ;
  byte psuedobuf[12] ;

  if (count and 1)
    then
      begin
        pb = data ;
        incn(pb, count) ; /* skip past valid data */
        *pb = 0 ; /* pad with zero to make even */
        inc(count) ;
      end
  csum = 0 ;
  if (psuedo)
    then
      begin
        p = addr(psuedobuf) ;
        storelongword(addr(p), psuedo->up_src) ;
        storelongword(addr(p), psuedo->up_dst) ;
        storebyte(addr(p), psuedo->up_zero) ;
        storebyte(addr(p), psuedo->up_proto) ;
        storeword(addr(p), psuedo->up_length) ;
        pw = addr(psuedobuf) ;
        for (loop = 0 ; loop <= 5 ; loop++)
          csum = csum + *pw++ ;
      end
  pw = (pointer) data ;
  for (loop = 0 ; loop <= (count shr 1) - 1 ; loop++)
    csum = csum + *pw++ ;
  csum = (csum shr 16) + (csum and 0xFFFF) ;
  csum = csum + (csum shr 16) ;
#ifdef ENDIAN_LITTLE
  return htons(not (word)csum) ;
#else
  return not (word)csum ;
#endif
end

void baler_ip_callback (pq330 q330, enum tbaler_type btype, word datalength,
      void *iphdr, void *udptcphdr, void *dataptr)
begin

  if (q330->par_create.call_baler)
    then
      begin
        q330->baler_call.context = q330 ;
        q330->baler_call.baler_type = btype ;
        memcpy(addr(q330->baler_call.station_name), addr(q330->station_ident), sizeof(string9)) ;
        q330->baler_call.info = datalength ;
        q330->baler_call.info2 = iphdr ;
        q330->baler_call.info3 = udptcphdr ;
        q330->baler_call.info4 = dataptr ;
        q330->baler_call.response = 0 ;
        q330->par_create.call_baler (addr(q330->baler_call)) ;
      end
end

static void proc_udp (pq330 q330, pbyte *p, boolean foreign)
begin
  tpsuedo psuedo ;
  pbyte psave ;

  psave = *p ;
  q330->recvudp.u_src = loadword (p) ;
  q330->recvudp.u_dst = loadword (p) ;
  q330->recvudp.u_len = loadword (p) ;
  q330->recvudp.u_cksum = loadword (p) ;
  if ((q330->recvudp.u_len < QDP_HDR_LTH) land (not foreign))
    then
      return ;
  psuedo.up_src = q330->recvip.ip_src ;
  psuedo.up_dst = q330->recvip.ip_dst ;
  psuedo.up_zero = 0 ;
  psuedo.up_proto = IPT_UDP ;
  psuedo.up_length = q330->recvudp.u_len ;
  if (cksum(addr(psuedo), psave, q330->recvudp.u_len))
    then
      begin
        add_status (q330, AC_CHECK, 1) ;
        return ;
      end
  psave = *p ; /* start of QDP header or foreign data */
  if (foreign)
    then
      begin
        baler_ip_callback (q330, BT_UDPRECV, q330->recvudp.u_len - UDP_HDR_LTH,
                           addr(q330->recvip), addr(q330->recvudp), *p) ;
        return ;
      end
  if (q330->recvudp.u_dst == q330->dataport)
    then
      begin
        memcpy (addr(q330->datain.qdp), psave, q330->recvudp.u_len - UDP_HDR_LTH) ;
        check_for_encoded (q330, q330->recvudp.u_len - UDP_HDR_LTH) ;
        return ;
      end
  if (q330->recvudp.u_dst != q330->ctrlport)
    then
      begin
        baler_ip_callback (q330, BT_UDPRECV, q330->recvudp.u_len - UDP_HDR_LTH,
                           addr(q330->recvip), addr(q330->recvudp), *p) ;
        return ;
      end
  p = psave ;
  loadqdphdr (addr(p), addr(q330->recvhdr)) ;
  if (q330->recvhdr.datalength > MAXDATA)
    then
      return ;
  if (q330->recvhdr.crc != gcrccalc (addr(q330->crc_table), (pointer)((pntrint)psave + 4),
                       q330->recvhdr.datalength + QDP_HDR_LTH - 4))
    then
      begin
        add_status (q330, AC_CHECK, 1) ;
        return ;
      end
  lib_command_response (q330, p) ;
end

static void proc_tcp (pq330 q330, pbyte *p)
begin
  tpsuedo psuedo ;
  pbyte psave ;
  ttcp recvtcp ;
  word hlth, tlth, dlth ;

  psave = *p ; /* start of TCP header */
  recvtcp.t_src = loadword (p) ;
  recvtcp.t_dst = loadword (p) ;
  recvtcp.t_seq = loadlongword (p) ;
  recvtcp.t_ack = loadlongword (p) ;
  recvtcp.t_offset = loadbyte (p) ;
  recvtcp.t_code = loadbyte (p) ;
  recvtcp.t_window = loadword (p) ;
  recvtcp.t_cksum = loadword (p) ;
  recvtcp.t_urgptr = loadword (p) ;
  hlth = (q330->recvip.ip_verlen and 0xf) shl 2 ;
  tlth = ((recvtcp.t_offset and 0xF0) shr 2) ; /* size of tcp header */
  dlth = q330->recvip.ip_len - hlth - tlth ; /* size of data */
  while (tlth > TCP_HDR_LTH)
    begin
      loadbyte (p) ; /* skip over extra TCP header fields if any */
      tlth-- ;
    end
  psuedo.up_src = q330->recvip.ip_src ;
  psuedo.up_dst = q330->recvip.ip_dst ;
  psuedo.up_zero = 0 ;
  psuedo.up_proto = IPT_TCP ;
  psuedo.up_length = q330->recvip.ip_len - hlth ;
  if (cksum(addr(psuedo), psave, psuedo.up_length))
    then
      begin
        add_status (q330, AC_CHECK, 1) ;
        return ;
      end
  baler_ip_callback (q330, BT_TCPRECV, dlth, addr(q330->recvip), addr(recvtcp), *p) ;
end

static boolean proc_ip (pq330 q330, integer lth)
begin
  pbyte p, pudp ;
  integer hlth ;
  boolean foreign ;

  p = addr(q330->commands.cmsgin.headers) ;
  q330->recvip.ip_verlen = loadbyte (addr(p)) ;
  q330->recvip.ip_tos = loadbyte (addr(p)) ;
  q330->recvip.ip_len = loadword (addr(p)) ;
  if (q330->recvip.ip_len != lth)
    then
      return FALSE ;
  q330->recvip.ip_id = loadword (addr(p)) ;
  q330->recvip.ip_fragoff = loadword (addr(p)) ;
  q330->recvip.ip_ttl = loadbyte (addr(p)) ;
  q330->recvip.ip_proto = loadbyte (addr(p)) ;
  q330->recvip.ip_cksum = loadword (addr(p)) ;
  q330->recvip.ip_src = loadlongword (addr(p)) ;
  q330->recvip.ip_dst = loadlongword (addr(p)) ;
  p = addr(q330->commands.cmsgin.headers) ;
  if (cksum (NIL, p, IP_HDR_LTH))
    then
      begin
        add_status (q330, AC_CHECK, 1) ;
        return TRUE ;
      end
  if ((q330->recvip.ip_verlen shr 4) != IP_VERSION)
    then
      return TRUE ;
  foreign = ((q330->recvip.ip_dst != q330->serial_ip) land (q330->recvip.ip_dst != 0xFFFFFFFF)) ;
  switch (q330->recvip.ip_proto) begin
    case IPT_UDP :
      hlth = (q330->recvip.ip_verlen and 0xf) shl 2 ;
      pudp = addr(q330->commands.cmsgin.headers) ;
      incn(pudp, hlth) ; /* skip header */
      proc_udp (q330, addr(pudp), foreign) ;
      break ;
    case IPT_TCP :
      hlth = (q330->recvip.ip_verlen and 0xf) shl 2 ;
      pudp = addr(q330->commands.cmsgin.headers) ;
      incn(pudp, hlth) ; /* skip header */
      proc_tcp (q330, addr(pudp)) ;
      break ;
  end
  return TRUE ;
end

void read_from_serial (pq330 q330)
begin
#define IBSIZE 700
  pbyte pin, pout ;
  pbyte maxp ;
  byte c ;
  longword i ;
#ifdef X86_WIN32
  COMSTAT comstat ;
  longword numread ;
  longword errs ;
#else
  ssize_t numread ;
#endif
  byte inbuf[IBSIZE] ;
  integer bufidx ;

#ifdef X86_WIN32
  ClearCommError(q330->comid, addr(errs), addr(comstat)) ;
  if (errs)
    then
      begin
        add_status (q330, AC_IOERR, 1) ;
        PurgeComm(q330->comid, PURGE_TXCLEAR or PURGE_RXCLEAR) ;
      end
  if (lnot ReadFile(q330->comid, addr(inbuf), IBSIZE, addr(numread), NIL))
    then
      return ;
#else
  numread = read(q330->comid, addr(inbuf), IBSIZE) ;
  if (numread <= 0)
    then
      begin
        sleepms (25) ;
        return ;
      end
#endif
  pin = addr(inbuf) ;
  pout = q330->bufptr ;
  maxp = addr((q330->commands.cmsgin.qdp_data)[MAXDATA96 - 1]) ;
  for (i = 1 ; i <= numread ; i++)
    begin
      c = *pin++ ;
      if ((c == SLIP_FRM) lor (lnot q330->needframe))
        then
          switch (c) begin
            case SLIP_FRM :
              q330->escpend = FALSE ;
              q330->needframe = FALSE ;
              bufidx = (integer)((pntrint)pout - (pntrint)addr(q330->commands.cmsgin.headers)) ;
              if (bufidx)
                then
                  begin
                    if (proc_ip (q330, bufidx))
                      then
                        begin /* is actually a packet */
                          q330->needframe = TRUE ;
                          add_status (q330, AC_READ, bufidx) ;
                        end
                  end
              pout = addr(q330->commands.cmsgin.headers) ;
              break ;
            case SLIP_ESC :
              q330->escpend = TRUE ;
              break ;
            case ESC_FRM :
              if (q330->escpend)
                then
                  begin
                    q330->escpend = FALSE ;
                    *pout++ = SLIP_FRM ;
                  end
                else
                  *pout++ = c ;
              break ;
            case ESC_ESC :
              if (q330->escpend)
                then
                  begin
                    q330->escpend = FALSE ;
                    *pout++ = SLIP_ESC ;
                  end
                else
                  *pout++ = c ;
              break ;
            default :
              *pout++ = c ;
              break ;
          end
      if ((pntrint)pout > ((pntrint)maxp + 1))
        then
          begin
            pout = addr(q330->commands.cmsgin.headers) ;
            add_status (q330, AC_CHECK, 1) ;
          end
    end
  q330->bufptr = pout ;
end

static void build_ip (pq330 q330, pbyte p, longword src, longword dest, byte protocol, integer datalength)
begin
  pbyte psave ;
  integer ck ;

  decn (p, IP_HDR_LTH + UDP_HDR_LTH) ;
  psave = p ; /* start of IP header */
  storebyte (addr(p), (IP_VERSION shl 4) + (IP_HDR_LTH shr 2)) ;
  storebyte (addr(p), IPP_NORMAL) ;
  storeword (addr(p), datalength + IP_HDR_LTH) ;
  storeword (addr(p), q330->ipid) ;
  inc(q330->ipid) ;
  storeword (addr(p), IP_DF) ; /* don't fragment */
  storebyte (addr(p), CIP_TTL) ;
  storebyte (addr(p), protocol) ;
  storeword (addr(p), 0) ; /* zero checksum for now */
  storelongword (addr(p), src) ;
  storelongword (addr(p), dest) ;
  ck = cksum(NIL, psave, IP_HDR_LTH) ;
  incn(psave, 10) ; /* point at checksum field */
  storeword (addr(psave), ck) ;
end

static void build_udp (pbyte p, longword src, longword dest,
                     word p_src, word p_dest, integer datalength)
begin
  tpsuedo psuedo ;
  pbyte psave ;
  integer ck ;

  decn (p, UDP_HDR_LTH) ;
  psave = p ; /* start of UDP header */
  storeword (addr(p), p_src) ;
  storeword (addr(p), p_dest) ;
  storeword (addr(p), datalength + UDP_HDR_LTH) ;
  storeword (addr(p), 0) ; /* for now */
  psuedo.up_src = src ;
  psuedo.up_dst = dest ;
  psuedo.up_zero = 0 ;
  psuedo.up_proto = IPT_UDP ;
  psuedo.up_length = datalength + UDP_HDR_LTH ;
  ck = cksum(addr(psuedo), psave, datalength + UDP_HDR_LTH) ;
  if (ck == 0)
    then
      ck = -1 ;
  incn(psave, 6) ; /* point at checksum field */
  storeword (addr(psave), ck) ;
end

static void build_tcp (pbyte p, longword src, longword dest,
                       word p_src, word p_dest, integer datalength,
                       longword seq, longword ack, word window, byte flags)
begin
  tpsuedo psuedo ;
  pbyte psave ;
  integer ck ;
  byte t_offset ;

  decn (p, TCP_HDR_LTH) ;
  psave = p ; /* start of TCP header */
  storeword (addr(p), p_src) ;
  storeword (addr(p), p_dest) ;
  storelongword (addr(p), seq) ;
  storelongword (addr(p), ack) ;
  if (datalength == -4) /* option field set */
    then
      t_offset = 0x60 ; /* option is in data area */
    else
      t_offset = 0x50 ; /* no options */
  datalength = abs(datalength) ;
  storebyte (addr(p), t_offset) ;
  storebyte (addr(p), flags) ;
  storeword (addr(p), window) ;
  storeword (addr(p), 0) ; /* for now */
  storeword (addr(p), 0) ; /* no urgency */
  psuedo.up_src = src ;
  psuedo.up_dst = dest ;
  psuedo.up_zero = 0 ;
  psuedo.up_proto = IPT_TCP ;
  psuedo.up_length = datalength + TCP_HDR_LTH ;
  ck = cksum(addr(psuedo), psave, datalength + TCP_HDR_LTH) ;
  incn(psave, 16) ; /* point at checksum field */
  storeword (addr(psave), ck) ;
end

static void flushob (pq330 q330, pointer buf, integer *cnt)
begin
#ifdef X86_WIN32
  longword numwrite ;
#else
  ssize_t numwrite ;
#endif

#ifdef X86_WIN32
  if (WriteFile(q330->comid, buf, *cnt, addr(numwrite), NIL) == 0)
#else
  numwrite = write(q330->comid, buf, *cnt) ;
  if (numwrite != *cnt)
#endif
    then
      add_status (q330, AC_IOERR, 1) ;
    else
      add_status (q330, AC_WRITE, *cnt) ;
  *cnt = 0 ;
end

static void sendpkt (pq330 q330, pbyte pin)
begin
#define OBSIZE 1500 /* should handle almost any packet */
  pbyte p, pout ;
  integer i ;
  byte c ;
  integer outcnt ;
  byte outbuf[OBSIZE] ;
  integer lth ;

  if (q330->comid == INVALID_IO_HANDLE)
    then
      return ;
  decn(pin, IP_HDR_LTH + UDP_HDR_LTH) ; /* IP header */
  p = pin ;
  incn(p, 2) ; /* point at IP length */
  lth = loadword (addr(p)) ;
  pout = addr(outbuf) ;
  outcnt = 1 ;
  *pout++ = SLIP_FRM ;
  for (i = 1 ; i <= lth ; i++)
    begin
      c = *pin++ ;
      if (c == SLIP_FRM)
        then
          begin
            *pout++ = SLIP_ESC ;
            *pout++ = ESC_FRM ;
            incn(outcnt, 2) ;
          end
      else if (c == SLIP_ESC)
        then
          begin
            *pout++ = SLIP_ESC ;
            *pout++ = ESC_ESC ;
            incn(outcnt, 2) ;
          end
        else
          begin
            *pout++ = c ;
            inc(outcnt) ;
          end
      if (outcnt >= (OBSIZE - 2))
        then
          begin
            pout = addr(outbuf) ;
            flushob (q330, addr(outbuf), addr(outcnt)) ;
          end
    end
  *pout = SLIP_FRM ;
  inc(outcnt) ;
  flushob (q330, addr(outbuf), addr(outcnt)) ;
end

void send_packet (pq330 q330, integer lth, word toport, word fromport)
begin

  build_udp (addr(q330->commands.cmsgout.qdp), q330->serial_ip, q330->q330ip, fromport, toport, lth) ;
  build_ip (q330, addr(q330->commands.cmsgout.qdp), q330->serial_ip, q330->q330ip, IPT_UDP, lth + UDP_HDR_LTH) ;
  sendpkt (q330, addr(q330->commands.cmsgout.qdp)) ;
end

void inject_packet (pq330 q330, pbyte payload, byte protocol, longword srcaddr,
                    longword destaddr, word srcport, word destport, word datalength,
                    longword seq, longword ack, word window, byte flags)
begin

  if (protocol == IPT_UDP)
    then
      begin
        build_udp (payload, srcaddr, destaddr, srcport, destport, datalength) ;
        build_ip (q330, payload, srcaddr, destaddr, IPT_UDP, datalength + UDP_HDR_LTH) ;
      end
    else
      begin
        build_tcp (payload, srcaddr, destaddr, srcport, destport, datalength,
                   seq, ack, window, flags) ;
        build_ip (q330, payload, srcaddr, destaddr, IPT_TCP, abs(datalength) + TCP_HDR_LTH) ;
      end
  sendpkt (q330, payload) ;
end

boolean open_serial (pq330 q330)
begin
#ifndef CMEX32
#ifdef X86_WIN32
  longword errs ;
  DCB dcb ;
  COMSTAT comstat ;
  COMMTIMEOUTS timeouts ;
#else
  struct termios sttynew ;
  struct termios sttyold ;
  longword cflag ;
  integer err ;
#endif
#endif
  word port ;
  boolean isd ;

  if (q330->comid != INVALID_IO_HANDLE)
    then
      begin
#ifdef X86_WIN32
        CloseHandle(q330->comid) ;
#else
        close(q330->comid) ;
#endif
        q330->comid = INVALID_IO_HANDLE ;
      end
  if (strcmp(q330->par_register.q330id_address, "255.255.255.255") == 0)
    then
      begin
        q330->balesim = TRUE ; /* simulate baler */
        q330->q330ip = 0xFFFFFFFF ;
        q330->q330cport = 0xFFFF ;
      end
    else
      begin
        q330->balesim = FALSE ;
        q330->q330ip = getip (q330->par_register.q330id_address, addr(isd)) ;
        if (q330->q330ip == INADDR_NONE)
          then
            begin
              libmsgadd(q330, LIBMSG_BADIPADDR, "") ;
              return TRUE ;
            end
      end
#ifdef X86_WIN32
  q330->comid = CreateFile(q330->par_register.host_interface, GENERIC_READ or GENERIC_WRITE, 0, NIL,
                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0) ;
  if (q330->comid != INVALID_IO_HANDLE)
    then
      begin
        SetupComm (q330->comid, INQSIZE, OUTQSIZE) ;
        GetCommState(q330->comid, addr(dcb)) ;
        dcb.DCBlength = sizeof(DCB) ;
        dcb.BaudRate = q330->par_register.serial_baud ;
        dcb.ByteSize = 8 ;
        dcb.Parity = NOPARITY ;
        dcb.StopBits = ONESTOPBIT ;
        dcb.fBinary = 1 ;
        dcb.fDtrControl = 1 ; /* DTR on */
        if (q330->par_register.serial_flow)
          then
            begin
              dcb.fOutxCtsFlow = 1 ;
              dcb.fRtsControl = 1 ;
            end
        dcb.XoffLim = INQSIZE div 3 ;
        dcb.XonLim = INQSIZE div 3 ;
        SetCommState(q330->comid, addr(dcb)) ;
        SetCommMask(q330->comid, 0) ;
        timeouts.ReadIntervalTimeout = MAXDWORD ;
        timeouts.ReadTotalTimeoutMultiplier = MAXDWORD ;
        timeouts.ReadTotalTimeoutConstant = 25 ; /* wait for 25ms if nothing there */
        timeouts.WriteTotalTimeoutMultiplier = (10000 div q330->par_register.serial_baud) + 1 ;
        timeouts.WriteTotalTimeoutConstant = (30000 div q330->par_register.serial_baud) + 10 ;
        SetCommTimeouts(q330->comid, addr(timeouts)) ;
        ClearCommError(q330->comid, addr(errs), addr(comstat)) ;
#elif defined (CMEX32)
  q330->comid = serial_open (DATA_UART, q330->par_register.serial_baud,
                            (q330->par_register.serial_flow != 0), OUTQSIZE, INQSIZE) ;
  if (q330->comid != INVALID_IO_HANDLE)
    then
      begin /* nothing else to do */
#else
  q330->comid = open (q330->par_register.host_interface, O_RDWR or O_NONBLOCK) ;
  if (q330->comid != INVALID_IO_HANDLE)
    then
      begin
        tcgetattr(q330->comid, addr(sttyold)) ;
        memcpy (addr(sttynew), addr(sttyold), sizeof(struct termios)) ;
        sttynew.c_iflag = IGNBRK ;
        sttynew.c_oflag = 0 ;
        cflag = 0 ;
        if (q330->par_register.serial_flow)
          then
#ifdef CCTS_OFLOW
            cflag = cflag or CCTS_OFLOW or CRTS_IFLOW ;
#else
            cflag = cflag or CRTSCTS ;
#endif
        sttynew.c_cflag = CS8 or CREAD or CLOCAL or cflag ;
        sttynew.c_lflag = NOFLSH ;
        memset (addr(sttynew.c_cc), _POSIX_VDISABLE, NCCS) ;
        sttynew.c_cc[VMIN] = 0 ;
        sttynew.c_cc[VTIME] = 0 ;
        err = cfsetspeed(addr(sttynew), q330->par_register.serial_baud) ;
/* Configure port using new settings */
        err = tcsetattr(q330->comid, TCSANOW, addr(sttynew)) ;
#endif
        q330->escpend = FALSE ;
        q330->bufptr = addr(q330->commands.cmsgin.headers) ;
        if (q330->par_register.host_ctrlport == 0)
          then
            begin
              port = newrand(addr(q330->rsum)) and 4095 ;
              q330->ctrlport = port + 1024 ;
            end
          else
            q330->ctrlport = q330->par_register.host_ctrlport ;
        if (q330->par_register.host_dataport == 0)
          then
            q330->dataport = q330->ctrlport + 1 ;
          else
            q330->dataport = q330->par_register.host_dataport ;
        q330->serial_ip = q330->par_register.serial_hostip ; /* starting value */
        q330->share.opstat.current_ip = q330->q330ip ;
        q330->share.opstat.current_port = q330->q330cport ;
        q330->got_connected = TRUE ;
      end
    else
      begin
        libmsgadd(q330, LIBMSG_SEROPEN, q330->par_register.host_interface) ;
        q330->comid = INVALID_IO_HANDLE ;
        return TRUE ;
      end
#ifdef CMEX32
  enable_data_serial_thread () ;
#endif
  return FALSE ;
end
#endif
