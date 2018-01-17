/*   Lib330 POC Receiver
     Copyright 2006, 2013 Certified Software Corporation

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
    0 2006-09-28 rdr Created
    1 2006-10-29 rdr Remove "addr" function when passing thread address. Fix posix
                     thread function return type.
    2 2007-08-04 rdr Add conditionals for omitting network code.
    3 2010-01-04 rdr Use fcntl instead of ioctl to set socket non-blocking.
    4 2013-02-02 rdr Use actual socket number for select.
*/
#ifndef OMIT_NETWORK

#ifndef q330types_h
#include "q330types.h"
#endif
#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif
#ifndef libpoc_h
#include "libpoc.h"
#endif
#ifndef libmsgs_h
#include "libmsgs.h"
#endif
#ifndef libclient_h
#include "libclient.h"
#endif
#ifndef q330cvrt_h
#include "q330cvrt.h"
#endif
#ifndef libcvrt_h
#include "libcvrt.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif

#define C2_POC 0xC5 /* Point of Contact */

typedef struct {
  t64 sernum ; /* Q330 serial number */
  longword q330_ip ; /* Q330 IP address */
  longword poc_ip ; /* POC IP address */
  longword router_ip ; /* Router IP Address */
  word bport ; /* Q330 base port */
  word lport ; /* Logical Port number */
  word webbps ; /* web bps / 10 */
  word flags ; /* baler/dial-out flags */
  longword spare2 ;
} tpoc ;
typedef struct {
#ifdef X86_WIN32
  HANDLE threadhandle ;
  longword threadid ;
#else
  pthread_t threadid ;
#endif
  boolean running ;
  tpoc_par poc_par ; /* creation parameters */
  tpoc_recvd poc_buf ; /* to build message for client */
  tpoc poc_recv ;
#ifdef X86_WIN32
  SOCKET cpath ; /* commands socket */
  struct sockaddr csockin, csockout ; /* commands socket address descriptors */
#else
  integer cpath ; /* commands socket */
  integer high_socket ;
  struct sockaddr csockin, csockout ; /* commands socket address descriptors */
#endif
  crc_table_type crc_table ;
  tqdp recvhdr ;
  boolean sockopen ;
  boolean terminate ;
  tany pkt ;
} tpocstr ;
typedef tpocstr *ppocstr ;

static void close_socket (ppocstr pocstr)
begin

  pocstr->sockopen = FALSE ;
  if (pocstr->cpath != INVALID_SOCKET)
    then
      begin
#ifdef X86_WIN32
        closesocket (pocstr->cpath) ;
#else
        close (pocstr->cpath) ;
#endif
        pocstr->cpath = INVALID_SOCKET ;
      end
#ifndef X86_WIN32
  pocstr->high_socket = 0 ;
#endif
end

static void process_poc (ppocstr pocstr, pbyte *p)
begin
  struct sockaddr_in *psock ;

#ifdef ENDIAN_LITTLE
  (pocstr->poc_recv.sernum)[1] = loadlongword (p) ;
  (pocstr->poc_recv.sernum)[0] = loadlongword (p) ;
#else
  (pocstr->poc_recv.sernum)[0] = loadlongword (p) ;
  (pocstr->poc_recv.sernum)[1] = loadlongword (p) ;
#endif
  pocstr->poc_recv.q330_ip = loadlongword (p) ;
  pocstr->poc_recv.poc_ip = loadlongword (p) ;
  pocstr->poc_recv.router_ip = loadlongword (p) ;
  pocstr->poc_recv.bport = loadword (p) ;
  pocstr->poc_recv.lport = loadword (p) ;
  pocstr->poc_recv.webbps = loadword (p) ;
  pocstr->poc_recv.flags = loadword (p) ;
  pocstr->poc_recv.spare2 = loadlongword (p) ;
  memcpy(addr(pocstr->poc_buf.serial_number), addr(pocstr->poc_recv.sernum), sizeof(t64)) ;
  pocstr->poc_buf.data_port = pocstr->poc_recv.lport ;
  psock = (pointer) addr(pocstr->csockin) ;
#ifdef ENDIAN_LITTLE
  pocstr->poc_buf.ip_address = ntohl(psock->sin_addr.s_addr) ;
#else
  pocstr->poc_buf.ip_address = psock->sin_addr.s_addr ;
#endif
  pocstr->poc_buf.base_port = pocstr->poc_recv.bport ;
  if (pocstr->poc_par.poc_callback)
    then
      pocstr->poc_par.poc_callback (PS_NEWPOC, addr(pocstr->poc_buf)) ;
end

static void read_poc_socket (ppocstr pocstr)
begin
  longint thiscrc ;
  integer lth ;
  pbyte p ;
  integer err ;

  if (pocstr->cpath == INVALID_SOCKET)
    then
      return ;
  lth = sizeof(struct sockaddr) ;
  err = recvfrom (pocstr->cpath, addr(pocstr->pkt.qdp), QDP_HDR_LTH + MAXDATA, 0, addr(pocstr->csockin), addr(lth)) ;
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
                  close_socket (pocstr) ;
                  pocstr->poc_par.poc_callback (PS_CONNRESET, addr(pocstr->poc_buf)) ;
                end
      end
  else if (err > 0)
    then
      begin
        p = addr(pocstr->pkt.qdp) ;
        thiscrc = gcrccalc (addr(pocstr->crc_table), (pointer)((pntrint)p + 4), err - 4) ;
        loadqdphdr (addr(p), addr(pocstr->recvhdr)) ;
        if ((thiscrc == pocstr->recvhdr.crc) land (pocstr->recvhdr.command == C2_POC))
          then
            process_poc (pocstr, addr(p)) ;
      end
end

static void open_socket (ppocstr pocstr)
begin
  integer err, flags ;
  longint flag ;
  struct sockaddr_in *psock ;

  close_socket (pocstr) ;
  pocstr->cpath = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP) ;
  if (pocstr->cpath == INVALID_SOCKET)
    then
      return ;
#ifndef X86_WIN32
  if (pocstr->cpath > pocstr->high_socket)
    then
      pocstr->high_socket = pocstr->cpath ;
#endif
  psock = (pointer) addr(pocstr->csockin) ;
  memset(psock, 0, sizeof(struct sockaddr)) ;
  psock->sin_family = AF_INET ;
  psock->sin_port = htons(pocstr->poc_par.poc_port) ;
  psock->sin_addr.s_addr = INADDR_ANY ;
#ifdef X86_WIN32
  err = bind(pocstr->cpath, addr(pocstr->csockin), sizeof(struct sockaddr)) ;
  if (err)
#else
  err = bind(pocstr->cpath, addr(pocstr->csockin), sizeof(struct sockaddr)) ;
  if (err)
#endif
    then
      begin
#ifdef X86_WIN32
        closesocket (pocstr->cpath) ;
#else
        close (pocstr->cpath) ;
#endif
        pocstr->cpath = INVALID_SOCKET ;
        return ;
      end
  flag = 1 ;
#ifdef X86_WIN32
  ioctlsocket (pocstr->cpath, FIONBIO, addr(flag)) ;
#else
  flags = fcntl (pocstr->cpath, F_GETFL, 0) ;
  fcntl (pocstr->cpath, F_SETFL, flags or O_NONBLOCK) ;
#endif
  pocstr->sockopen = TRUE ;
end

#ifdef X86_WIN32
unsigned long  __stdcall pocthread (pointer p)
begin
  ppocstr pocstr ;
  fd_set readfds, writefds, exceptfds ;
  struct timeval timeout ;
  integer res ;

  pocstr = p ;
  repeat
    if (pocstr->sockopen)
      then
        begin /* wait for socket input or timeout */
          FD_ZERO (addr(readfds)) ;
          FD_ZERO (addr(writefds)) ;
          FD_ZERO (addr(exceptfds)) ;
          FD_SET (pocstr->cpath, addr(readfds)) ;
          timeout.tv_sec = 0 ;
          timeout.tv_usec = 25000 ; /* 25ms timeout */
          res = select (0, addr(readfds), addr(writefds), addr(exceptfds), addr(timeout)) ;
          if (res > 0)
            then
              if (FD_ISSET (pocstr->cpath, addr(readfds)))
                then
                  read_poc_socket (pocstr) ;
        end
      else
        sleepms (25) ;
  until pocstr->terminate) ;
  pocstr->running = FALSE ;
  ExitThread (0) ;
  return 0 ;
end

#else
void *pocthread (pointer p)
begin
  ppocstr pocstr ;
  fd_set readfds, writefds, exceptfds ;
  struct timeval timeout ;
  integer res ;

  pocstr = p ;
  repeat
    if (pocstr->sockopen)
      then
        begin /* wait for socket input or timeout */
          FD_ZERO (addr(readfds)) ;
          FD_ZERO (addr(writefds)) ;
          FD_ZERO (addr(exceptfds)) ;
          FD_SET (pocstr->cpath, addr(readfds)) ;
          timeout.tv_sec = 0 ;
          timeout.tv_usec = 25000 ; /* 25ms timeout */
          res = select (pocstr->high_socket + 1, addr(readfds), addr(writefds), addr(exceptfds), addr(timeout)) ;
          if (res > 0)
            then
              if (FD_ISSET (pocstr->cpath, addr(readfds)))
                then
                  read_poc_socket (pocstr) ;
        end
      else
        sleepms (25) ;
  until pocstr->terminate) ;
  pocstr->running = FALSE ;
  pthread_exit (0) ;
end
#endif

pointer lib_poc_start (tpoc_par *pp)
begin
  ppocstr pocstr ;
#ifndef X86_WIN32
  integer err ;
#endif

  pocstr = malloc(sizeof(tpocstr)) ;
  memset (pocstr, 0, sizeof(tpocstr)) ;
  gcrcinit (addr(pocstr->crc_table)) ;
  memcpy (addr(pocstr->poc_par), pp, sizeof(tpoc_par)) ;
  pocstr->cpath = INVALID_SOCKET ;
  open_socket (pocstr) ;
  if (pocstr->sockopen == FALSE)
    then
      begin
        free (pocstr) ;
        return NIL ;
      end
#ifdef X86_WIN32
  pocstr->threadhandle = CreateThread (NIL, 0, pocthread, pocstr, 0, addr(pocstr->threadid)) ;
  if (pocstr->threadhandle == NIL)
#else
  err = pthread_create(addr(pocstr->threadid), NULL, pocthread, pocstr) ;
  if (err)
#endif
    then
      begin
        free (pocstr) ;
        return NIL ;
      end ;
  pocstr->running = TRUE ;
  return pocstr ; /* return running context */
end

void lib_poc_stop (pointer ct)
begin
  ppocstr pocstr ;

  pocstr = ct ;
  pocstr->terminate = TRUE ;
  while (pocstr->running)
    sleepms (25) ;
  close_socket (pocstr) ;
end

#endif
