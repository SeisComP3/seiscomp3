/*   Lib330 Command Processing
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
    1 2006-10-29 rdr Some extra parenthesis and begin/end blocks to make compiler happy.
    2 2006-12-18 rdr Use ctrlport instead of host_ctrlport when sending using serial.
    3 2006-12-28 rdr Change registration timeout from 1 to 3 minutes.
    4 2006-12-30 rdr If cfg_timer decrements to zero then flush configuration blockettes.
    5 2007-01-18 rdr Add Status timeout checking, include for libopaque.
    6 2007-02-19 rdr Status timeout not getting cleared when first registering and also
                     not getting checked while running, fixed.
    7 2007-03-05 rdr Make calls to purge_thread_continuity.
    8 2007-07-16 rdr Add baler command support.
    9 2007-07-31 rdr Add BT_Q330TIME callback and conditionals for omitting network code.
   10 2007-08-07 rdr Add BT_BACK callback when C2_BRDY received. Add access timeout to
                     avoiding shutting down a baler when it's busy reloading.
   11 2007-09-04 rdr Do a BT_Q330TIME callback even if automatic timezone correction is disabled.
   12 2007-10-03 rdr Access timer changed to countdown to zero.
   13 2008-01-09 rdr Add processing for freeze_timer. Add locking of access time decrement.
                     Use settable timeouts instead of constants. Pass along Q330 serial number
                     in C2_BRDY call. Add flush_all support. Add additional info to LIBMSG_BACK message.
                     Add log_timer handling. Add setting changed Q330 IP address and port as
                     a result of receiving C2_BRDY.
   14 2008-02-27 rdr Add lib_round when calculating zone offset.
   15 2008-08-05 rdr Request baler configuration in C2_BRDY call.
                     If get a CERR_SNV go to wait state with a NR_TIME timeout. If get a socket
                     open error then it could be a failure to reach the DNS server
                     instead of a actual error in the address, keep retrying every 10 minutes.
   16 2008-08-20 rdr Add TCP support.
   17 2009-02-09 rdr Add EP support.
   18 2009-07-25 rdr Add DSS support.
   19 2009-09-05 rdr Fix LIBMSG_SNV parameter, message is already included in libmsgs.
   20 2010-03-27 rdr Add Q335 support by not doing SDUMP related structure requests until
                     the C1_FGLS packet is received. In start_dealocation, if not registered
                     go to LIBSTATE_WAIT instead of target state.
   21 2010-04-20 rdr Add processing for LPSF_PWROFF request.
   22 2010-05-07 rdr If opt_connwait is zero then use a value of ten minutes.
   23 2010-05-17 rdr Add sending Q335 Aware flag in C1_RQFGLS.
   24 2013-08-18 rdr Change reboot to lib330_reboot to avoid conflict with some nonsense.
*/
#ifndef libcmds_h
#include "libcmds.h"
#endif
#ifndef libcvrt_h
#include "libcvrt.h"
#endif
#ifndef q330cvrt_h
#include "q330cvrt.h"
#endif
#ifndef q330io_h
#include "q330io.h"
#endif
#ifndef libclient_h
#include "libclient.h"
#endif
#ifndef libmsgs_h
#include "libmsgs.h"
#endif
#ifndef libtokens_h
#include "libtokens.h"
#endif
#ifndef libsample_h
#include "libsample.h"
#endif
#ifndef libverbose_h
#include "libverbose.h"
#endif
#ifndef libsampcfg_h
#include "libsampcfg.h"
#endif
#ifndef libslider_h
#include "libslider.h"
#endif
#ifndef libsampglob_h
#include "libsampglob.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif
#ifndef libstats_h
#include "libstats.h"
#endif
#ifndef libcont_h
#include "libcont.h"
#endif
#ifndef libmd5_h
#include "libmd5.h"
#endif

#ifndef OMIT_SEED
#ifndef liblogs_h
#include "liblogs.h"
#endif
#ifndef lobopaque_h
#include "libopaque.h"
#endif
#ifndef libdss_h
#include "libdss.h"
#endif
#endif

#define TUNCMDFLAG 0xFFFF /* if new_cmd size uses this value then flag as tunnelled command */

void purge_cmdq (pq330 q330)
begin

  q330->commands.cphase = CP_IDLE ;
  q330->commands.cmdin = 0 ;
  q330->commands.cmdout = 0 ;
  if (q330->stalled_link)
    then
      begin
        q330->stalled_link = FALSE ;
        state_callback (q330, ST_STALL, 0) ;
      end
end

void clear_cmsg (pq330 q330)
begin
  double r, nw ;
  tcommands *pc ;
  tcmdq *pcmd ;

  pc = addr(q330->commands) ;
  pcmd = addr(pc->cmdq[pc->cmdout]) ;
  pcmd->retsz = pcmd->retsz + IP_HDR_LTH + UDP_HDR_LTH + QDP_HDR_LTH + q330->recvhdr.datalength ;
  nw = now () ;
  if (nw == pcmd->sent)
    then
      nw = nw + 0.001 ;
  r = (pcmd->retsz + pcmd->sendsz) / (nw - pcmd->sent) ;
  pc->histories[pc->history_idx] = r ;
  pc->history_idx = (pc->history_idx + 1) mod MAX_HISTORY ;
  if (pc->history_count < MAX_HISTORY)
    then
      inc(pc->history_count) ;
  pc->cphase = CP_IDLE ;
  pc->ctrl_retries = 0 ;
  if (pc->cmdin != pc->cmdout)
    then
      pc->cmdout = (pc->cmdout + 1) mod CMDQSZ ;
  if ((pc->cmdin == pc->cmdout) land (q330->stalled_link))
    then
      begin
        q330->stalled_link = FALSE ;
        state_callback (q330, ST_STALL, 0) ;
      end
end

static void initcmdhdr (pq330 q330, pbyte *p, byte cmd)
begin

  storeqdphdr (p, cmd, 0, q330->commands.ctrlseq, 0) ;
end

static void lib_send_next_cmd (pq330 q330)
begin
  pbyte p ;
  pbyte pref ;
  pbyte plth ;
  integer lth ;
  integer msglth ;
  integer i, err ;
  double r ;
  word sersz ;
#ifndef OMIT_SERIAL
#ifdef X86_WIN32
  longword errs ;
  COMSTAT comstat ;
#else
#endif
#endif
  string95 s, s1 ;
  tcommands *pc ;
  tcmdq *pcmd ;

  pc = addr(q330->commands) ;
  if ((pc->cphase == CP_IDLE) land (pc->cmdin != pc->cmdout))
    then
      pc->cphase = CP_NEED ;
  if (pc->cphase == CP_NEED)
    then
      begin
        p = addr(pc->cmsgout.qdp) ;
        pcmd = addr(pc->cmdq[pc->cmdout]) ;
        initcmdhdr (q330, addr(p), pcmd->cmd) ;
        pref = p ; /* pointer after header for length calculation */
        if (pcmd->tunneled)
          then
            memcpy (p, addr(q330->share.tunnel.payload), q330->share.tunnel.paysize) ;
          else
            switch (pcmd->cmd) begin
              case C1_RQSRV :
                storerqsrv (addr(p), addr(q330->par_create.q330id_serial)) ;
                break ;
              case C1_SRVRSP :
                storesrvrsp (addr(p), addr(q330->srvresp)) ;
                break ;
              case C1_DSRV :
                storerqsrv (addr(p), addr(q330->par_create.q330id_serial)) ;
                break ;
              case C1_RQSTAT :
                storerqstat (addr(p), q330->stat_request) ;
                break ;
              case C1_RQLOG :
                storeword (addr(p), q330->par_create.q330id_dataport) ;
                break ;
              case C1_RQFGLS :
                storeword (addr(p), q330->par_create.q330id_dataport) ;
                storeword (addr(p), 1) ;
                break ;
              case C1_POLLSN :
                storepollsn (addr(p), addr(q330->newpoll)) ;
                break ;
              case C1_PING :
                lock (q330) ;
                q330->pinghdr.ping_type = q330->share.pingreq.pingtype ;
                switch (q330->pinghdr.ping_type) begin
                  case 0 :
                  case 4 : /* normal ping or format request */
                    q330->pinghdr.ping_opt = q330->pingid ;
                    inc(q330->pingid) ;
                    storepinghdr (addr(p), addr(q330->pinghdr)) ;
                    break ;
                  case 2 : /* status request */
                    q330->pinghdr.ping_opt = q330->share.pingreq.pingopt ;
                    storepinghdr (addr(p), addr(q330->pinghdr)) ;
                    storepingstatreq (addr(p), q330->share.pingreq.pingreqmap) ;
                    break ;
                end
                unlock (q330) ;
                q330->ping_send = now () ;
                break ;
              case C1_RQMEM :
                storememhdr (addr(p), addr(q330->mem_req)) ;
                break ;
              case C1_SLOG :
                lock (q330) ;
                storeslog (addr(p), addr(q330->share.newlog)) ;
                unlock (q330) ;
                break ;
              case C1_UMSG :
                lock (q330) ;
                storeumsg (addr(p), addr(q330->share.newuser)) ;
                unlock (q330) ;
                break ;
              case C1_WEB :
                lock (q330) ;
                if (q330->share.fixed.flags and FF_NWEB)
                  then
                    storenewweb (addr(p), addr(q330->share.new_webadv)) ;
                  else
                    storeoldweb (addr(p), addr(q330->share.old_webadv)) ;
                unlock (q330) ;
                break ;
              case C2_BRDY :
                memset(addr(q330->brdy), 0, sizeof(tbrdy)) ;
                memcpy(addr(q330->brdy.sernum), addr(q330->par_create.q330id_serial), sizeof(t64)) ;
                (q330->brdy.stn)[5] = BR_RQCFG ; /* Request it if it is there */
                storebrdy (addr(p), addr(q330->brdy)) ;
                break ;
              case C2_BOFF :
                storeword (addr(p), 1) ;
                break ;
              case C2_REGCHK :
                lock (q330) ;
                storelongword (addr(p), q330->share.check_ip) ;
                unlock (q330) ;
                break ;
              case C2_SEPCFG :
                lock (q330) ;
                storeepcfg (addr(p), addr(q330->share.newepcfg)) ;
                unlock (q330) ;
                break ;
            end
        lth = (longint)p - (longint)pref ; /* length of data */
        p = addr(pc->cmsgout.qdp) ;
        plth = p ;
        incn(plth, 6) ; /* point at length */
        storeword (addr(plth), lth) ;
        pc->cphase = CP_WAIT ;
        pc->lastctrlseq = pc->ctrlseq ;
        inc(pc->ctrlseq) ;
        pcmd->sent = now() ;
        pcmd->sendsz = lth + IP_HDR_LTH + UDP_HDR_LTH + QDP_HDR_LTH ;
        if (q330->usesock)
          then
            sersz = 576 ;
#ifndef OMIT_SERIAL
#ifdef X86_WIN32
          else
            begin
              ClearCommError(q330->comid, addr(errs), addr(comstat)) ;
              sersz = comstat.cbOutQue + 600 ;
            end
#else
#endif
#endif
        if (pc->history_count < 3)
          then
            begin
              if (q330->usesock)
                then
                  pc->ctrlrecnt = (q330->par_register.host_maxcmdretry + q330->par_register.host_mincmdretry) div 2 ;
#ifndef OMIT_SERIAL
                else
                  begin
                    pc->ctrlrecnt = ((pcmd->sendsz + pcmd->estsz + sersz) div (q330->par_register.serial_baud div 10)) * 2 + 1 ;
                    if (pc->ctrlrecnt < ((q330->par_register.host_maxcmdretry + q330->par_register.host_mincmdretry) div 2))
                      then
                        pc->ctrlrecnt = (q330->par_register.host_maxcmdretry + q330->par_register.host_mincmdretry) div 2 ;
                  end
#endif
            end
          else
            begin
              r = 0 ;
              for (i = 0 ; i <= pc->history_count - 1 ; i++)
                r = r + pc->histories[i] ;
              r = r / pc->history_count ;
              if (r == 0.0)
                then
                  pc->ctrlrecnt = (q330->par_register.host_maxcmdretry + q330->par_register.host_mincmdretry) div 2 ;
                else
                  pc->ctrlrecnt = ((pcmd->sendsz + pcmd->estsz + sersz) / r) * 2 + 1.5 ;
              pc->ctrlrecnt = pc->ctrlrecnt * (pc->ctrl_retries + 1) ; /* backoff */
            end
        if (pc->ctrlrecnt < q330->par_register.host_mincmdretry)
          then
            pc->ctrlrecnt = q330->par_register.host_mincmdretry ;
        if (pc->ctrlrecnt > q330->par_register.host_maxcmdretry)
          then
            pc->ctrlrecnt = q330->par_register.host_maxcmdretry ;
        switch (pcmd->cmd) begin
          case C1_POLLSN :
            pc->ctrlrecnt = 3 ; /* 300ms, special timeout */
            break ;
          case C1_PING :
            pc->ctrlrecnt = 5 ; /* 5 seconds */
            break ;
        end
        msglth = QDP_HDR_LTH + lth ;
        p = addr(pc->cmsgout.qdp) ;
        storelongint (addr(p), gcrccalc (addr(q330->crc_table), (pointer)((integer)p + 4), msglth - 4)) ;
        if (q330->cur_verbosity and VERB_PACKET)
          then
            begin /* log the message sent */
              p = addr(pc->cmsgout.qdp) ;
              loadqdphdr (addr(p), addr(q330->recvhdr)) ; /* for display purposes */
              packet_time (now(), addr(s)) ;
              command_name (q330->recvhdr.command, addr(s1)) ;
              strcat (s, s1) ;
              sprintf(s1, ", Lth=%d Seq=%d Ack=%d", q330->recvhdr.datalength, q330->recvhdr.sequence,
                      q330->recvhdr.acknowledge) ;
              strcat(s, s1) ;
              libmsgadd(q330, LIBMSG_PKTOUT, addr(s)) ;
            end
#ifndef OMIT_NETWORK
        if (q330->usesock)
          then
            begin
              if (q330->cpath == INVALID_SOCKET)
                then
                  return ;
              if (q330->tcp)
                then
                  begin
                    p = (pointer)((integer)addr(pc->cmsgout.qdp) - 4) ;
                    pref = p ; /* save start of tcp packet */
                    storeword (addr(p), 0) ; /* control port */
                    storeword (addr(p), msglth) ; /* qdp length */
                    err = send(q330->cpath, (pchar) pref, msglth + 4, 0) ;
                  end
                else
                  err = sendto(q330->cpath, addr(pc->cmsgout.qdp), msglth, 0, addr(q330->csockout), sizeof(struct sockaddr)) ;
              if (err == SOCKET_ERROR)
                then
                  begin
                    err =
#ifdef X86_WIN32
                           WSAGetLastError() ;
#else
                           errno ;
#endif
                    if (err == ENOBUFS)
                      then
                        begin
                          purge_cmdq (q330) ;
                          lib_change_state (q330, LIBSTATE_WAIT, LIBERR_NOTR) ;
                          close_sockets (q330) ;
                          q330->reg_wait_timer = 60 * 10 ;
                          q330->registered = FALSE ;
                          libmsgadd (q330, LIBMSG_ROUTEFAULT, "Waiting 10 minutes") ;
                        end
                    else if (err != EWOULDBLOCK)
                      then
                        if (q330->tcp)
                          then
                            begin
                              purge_cmdq (q330) ;
                              sprintf(s1, "%d, Waiting 10 minutes", err) ;
                              tcp_error (q330, addr(s1)) ;
                            end
                          else
                            begin
                              sprintf(s1, "%d", err) ;
                              libmsgadd(q330, LIBMSG_CANTSEND, addr(s1)) ;
                              add_status (q330, AC_IOERR, 1) ; /* add one I/O error */
                              pc->ctrl_retries = 10 ;
                            end
                  end
                else
                  add_status (q330, AC_WRITE, msglth + IP_HDR_LTH + UDP_HDR_LTH) ;
            end
#endif
#ifndef OMIT_SERIAL
        if (q330->usesock == 0)
          then
            send_packet (q330, msglth, q330->q330cport, q330->ctrlport) ;
#endif
      end
end

static void abort_cmsg (pq330 q330)
begin

  if ((q330->commands.cphase == CP_WAIT) land (q330->commands.cmdin != q330->commands.cmdout))
    then
      begin
        libmsgadd (q330, LIBMSG_CMDABT, "") ;
        clear_cmsg (q330) ;
        q330->stat_request = 0 ;
      end
end

void new_cmd (pq330 q330, byte ncmd, word sz)
begin
  integer i ;
  tcommands *pc ;
  tcmdq *pcmd ;

  pc = addr(q330->commands) ;
  i = pc->cmdout ;
  while (i != pc->cmdin)
    if (pc->cmdq[i].cmd == ncmd)
      then
        return ; /* already one there */
      else
        i = (i + 1) mod CMDQSZ ;
  pcmd = addr(pc->cmdq[pc->cmdin]) ;
  pcmd->cmd = ncmd ;
  if (sz == TUNCMDFLAG)
    then
      begin
        pcmd->estsz = MAXMTU ;
        pcmd->tunneled = TRUE ;
      end
    else
      begin
        pcmd->estsz = sz + sizeof(tip) + sizeof(tudp) + sizeof(tqdp) ;
        pcmd->tunneled = FALSE ;
      end
  pcmd->retsz = 0 ;
  pc->cmdin = (pc->cmdin + 1) mod CMDQSZ ;
  lib_send_next_cmd (q330) ; /* in case are free to send */
end

static word stat_estimate (longword stat_rqst)
begin
  word w ;
  word sz ;

  sz = 0 ;
  for (w = SRB_GLB ; w <= SRB_FES ; w++)
    begin
      if (sz > MAXMTU)
        then
          return 0 ;
      if (stat_rqst and make_bitmap(w))
        then
          switch (w) begin
            case SRB_GLB :
              sz = sz + sizeof(tstat_global) ;
              break ;
            case SRB_GST :
              sz = sz + sizeof(tstat_gps) ;
              break ;
            case SRB_PWR :
              sz = sz + sizeof(tstat_pwr) ;
              break ;
            case SRB_BOOM :
              sz = sz + sizeof(tstat_boom) ;
              break ;
            case SRB_PLL :
              sz = sz + sizeof(tstat_pll) ;
              break ;
            case SRB_GSAT :
              sz = sz + sizeof(tstat_sathdr) + sizeof(tstat_sat1) * MAX_SAT ;
              break ;
            case SRB_ARP :
              sz = sz + sizeof(tstat_arphdr) + sizeof(tarp1) * MAXARP ;
              break ;
            case SRB_LOG1 :
            case SRB_LOG2 :
            case SRB_LOG3 :
            case SRB_LOG4 :
              sz = sz + sizeof(tstat_log) ;
              break ;
            case SRB_SER1 :
            case SRB_SER2 :
            case SRB_SER3 :
              sz = sz + sizeof(tstat_serial) ;
              break ;
            case SRB_ETH :
              sz = sz + sizeof(tstat_ether) ;
              break ;
            case SRB_BALER :
              sz = sz + sizeof(tstat_baler) ;
              break ;
            case SRB_DYN :
              sz = sz + sizeof(tdyn_ips) ;
              break ;
            case SRB_SS :
              sz = sz + sizeof(tsshdr) + (sizeof(tssstat) * 2) ;
              break ;
            case SRB_EP :
              sz = sz + sizeof(tstat_ep) ;
              break ;
            case SRB_FES :
              sz = sz + sizeof(tstat_fes) ;
              break ;
          end
    end
  return sz ;
end

static void lib330_reboot (pq330 q330, boolean keep_link)
begin

  q330->reboot_done = FALSE ;
  q330->share.opstat.runtime = 0 ;
  new_state (q330, LIBSTATE_READCFG) ;
  q330->link_recv = keep_link ;
  new_cmd (q330, C1_RQFGLS, sizeof(tlog) + sizeof(tfixed) +
           sizeof(tglobal) + sizeof(tsensctrl)) ;
  new_cmd (q330, C1_RQGID, sizeof(tgpsid)) ;
  q330->stat_request = make_bitmap(q330->par_create.q330id_dataport + SRB_LOG1) or make_bitmap(SRB_GLB) ;
#ifndef OMIT_SDUMP
  if (q330->cur_verbosity and VERB_SDUMP)
    then
      q330->stat_request = q330->stat_request or 0x2A ; /* add boom, gps, and pll status for status dump */
#endif
  new_cmd (q330, C1_RQSTAT, stat_estimate(q330->stat_request)) ;
end

static void start_deregistration (pq330 q330)
begin
  boolean have_msg ;

  new_state (q330, LIBSTATE_DEREG) ;
  purge_cmdq (q330) ;
  if (q330->cur_verbosity and VERB_REGMSG)
    then
      begin
        lock (q330) ;
        q330->share.pingreq.pingtype = 0 ; /* simple ping */
        have_msg = (q330->share.newuser.msg[0]) ;
        unlock (q330) ;
        new_cmd (q330, C1_PING, 0) ;
        if (have_msg)
          then
            new_cmd (q330, C1_UMSG, 0) ;
      end
  if (q330->balesim)
    then
      new_cmd (q330, C2_BOFF, 0) ;
    else
      new_cmd (q330, C1_DSRV, 0) ;
  q330->comtimer = 0 ;
  lock (q330) ;
  q330->share.have_config = 0 ;
  unlock (q330) ;
  libmsgadd (q330, LIBMSG_DEREGWAIT, "") ;
end

void start_deallocation (pq330 q330)
begin
  enum tliberr err ;

  libmsgadd (q330, LIBMSG_DEALLOC, "") ;
  new_state (q330, LIBSTATE_DEALLOC) ;
  deallocate_sg (q330->aqstruc) ;
  lock (q330) ;
  err = q330->share.liberr ;
  unlock (q330) ;
  switch (err) begin
    case LIBERR_TOKENS_CHANGE :
      cfg_start (q330) ;
      break ;
    case LIBERR_NOTR :
    case LIBERR_TMSERV :
    case LIBERR_DATATO :
      new_state (q330, LIBSTATE_WAIT) ;
      q330->registered = FALSE ;
      close_sockets (q330) ;
      break ;
    default :
      start_deregistration (q330) ;
  end
end

void lib_continue_registration (pq330 q330)
begin

  if (q330->balesim)
    then
      new_state (q330, LIBSTATE_ANNC) ;
    else
      new_state (q330, LIBSTATE_REG) ;
  q330->registered = FALSE ;
  lock (q330) ;
  q330->share.have_config = 0 ;
  q330->share.have_status = 0 ;
  q330->share.want_config = 0 ;
  q330->status_timer = 0 ;
  unlock (q330) ;
  q330->link_recv = FALSE ;
  q330->commands.cmdin = 0 ;
  q330->commands.cmdout = 0 ; /* purge command queue */
  if (q330->balesim)
    then
      new_cmd (q330, C2_BRDY, sizeof(t64)) ;
    else
      new_cmd (q330, C1_RQSRV, sizeof(t64)) ;
  q330->reg_timer = 0 ;
end

void lib_start_registration (pq330 q330)
begin

#ifndef OMIT_NETWORK
  if (q330->usesock)
    then
      begin
        if (open_sockets (q330, TRUE, FALSE))
          then
            begin
              lib_change_state (q330, LIBSTATE_WAIT, LIBERR_NETFAIL) ;
              q330->reg_wait_timer = 60 * 10 ;
              q330->registered = FALSE ;
              libmsgadd (q330, LIBMSG_SNR, "10 Minutes") ;
              return ;
            end
      end
#endif
#ifndef OMIT_SERIAL
  if (q330->usesock == 0)
    then
      if (open_serial (q330))
        then
          begin
            lib_change_state (q330, LIBSTATE_WAIT, LIBERR_NETFAIL) ;
            q330->reg_wait_timer = 60 * 10 ;
            q330->registered = FALSE ;
            libmsgadd (q330, LIBMSG_SNR, "10 Minutes") ;
            return ;
          end
#endif
  q330->reg_timer = 0 ;
  if (q330->got_connected)
    then
      lib_continue_registration (q330) ;
    else
      new_state (q330, LIBSTATE_CONN) ; /* must be TCP */
end

void lib_start_ping (pq330 q330)
begin

#ifndef OMIT_NETWORK
  if (q330->usesock)
    then
      begin
        if (open_sockets (q330, FALSE, FALSE))
          then
            begin
              lock (q330) ;
              q330->share.target_state = LIBSTATE_IDLE ;
              q330->share.liberr = LIBERR_NETFAIL ;
              unlock (q330) ;
              return ;
            end
      end
#endif
#ifndef OMIT_SERIAL
  if (q330->usesock == 0)
    then
      if (open_serial (q330))
        then
          begin
            lock (q330) ;
            q330->share.target_state = LIBSTATE_IDLE ;
            q330->share.liberr = LIBERR_NETFAIL ;
            unlock (q330) ;
            return ;
          end
#endif
  new_state (q330, LIBSTATE_PING) ;
  q330->registered = FALSE ;
  lock (q330) ;
  q330->share.have_config = 0 ;
  q330->share.have_status = 0 ;
  q330->share.pingreq.pingtype = 0 ; /* simple ping */
  q330->share.client_ping = CLP_REQ ; /* this will trigger a ping */
  unlock (q330) ;
  q330->link_recv = FALSE ;
  q330->commands.cmdin = 0 ;
  q330->commands.cmdout = 0 ; /* purge command queue */
  q330->reg_timer = 0 ;
end

/* pb has pointer to packet buffer */
void lib_command_response (pq330 q330, pbyte pb)
begin
  pbyte p, psave ;
  t64 md5equiv ;
  word errcode ;
  tfgl fgl ;
  string95 s, s2 ;
  string250 s1 ;
  string63 s3 ;
  longword req_mask, mask ;
  longword bitnum, check_resp ;
  byte errcmd ;
  double t ;
  single perc ;
  longint l ;
  integer i ;
  tcommands *pc ;
  tcmdq *pcmd ;
  tstat_global *pglob ;
  tstat_log *plog ;

  pc = addr(q330->commands) ;
  p = pb ;
  add_status (q330, AC_PACKETS, 1) ;
  if (q330->cur_verbosity and VERB_PACKET)
    then
      begin /* log the message received */
        packet_time (now(), addr(s)) ;
        command_name (q330->recvhdr.command, addr(s2)) ;
        strcat (s, s2) ;
        sprintf(s2, ", Lth=%d Seq=%d Ack=%d", q330->recvhdr.datalength, q330->recvhdr.sequence,
                q330->recvhdr.acknowledge) ;
        strcat(s, s2) ;
        libmsgadd(q330, LIBMSG_PKTIN, s) ;
      end
/*  with *q330, recvhdr, commands */
  pcmd = addr(pc->cmdq[pc->cmdout]) ;
  if (pc->cphase == CP_WAIT)
    then
      if (q330->recvhdr.acknowledge == pc->lastctrlseq)
        then
          if (q330->recvhdr.command == C1_CERR)
            then
              begin
                errcode = loadcerr (addr(p)) ;
                errcmd = pcmd->cmd ;
                q330->share.tunnel_state = TS_IDLE ; /* just in case */
                if (errcmd != C1_RQMEM)
                  then
                    clear_cmsg (q330) ;
                switch (errcode) begin
                  case CERR_PERM :
                    libmsgadd (q330, LIBMSG_PERM, "") ;
                    break ;
                  case CERR_TMSERV :
                    sprintf(s, "%d seconds", q330->piu_retry) ;
                    libmsgadd(q330, LIBMSG_PIU, addr(s)) ;
                    q330->registered = FALSE ;
                    q330->reg_wait_timer = q330->piu_retry ;
                    lib_change_state (q330, LIBSTATE_WAIT, LIBERR_TMSERV) ;
                    break ;
                  case CERR_NOTR :
                    lib_change_state (q330, LIBSTATE_WAIT, LIBERR_NOTR) ;
                    q330->registered = FALSE ;
                    sprintf(s, "%d seconds", NR_TIME) ;
                    libmsgadd(q330, LIBMSG_SNR, addr(s)) ;
                    q330->reg_wait_timer = NR_TIME ;
                    break ;
                  case CERR_INVREG :
                    lib_change_state (q330, LIBSTATE_IDLE, LIBERR_INVREG) ;
                    libmsgadd(q330, LIBMSG_INVREG, "") ;
                    q330->registered = FALSE ;
                    break ;
                  case CERR_PAR :
                    libmsgadd(q330, LIBMSG_PARERR, "") ;
                    break ;
                  case CERR_SNV :
                    libmsgadd(q330, LIBMSG_SNV, "") ;
                    switch (errcmd) begin
                      case C1_RQMEM :
                        if ((q330->libstate == LIBSTATE_READTOK) land
                           (q330->mem_req.memtype == q330->par_create.q330id_dataport + MT_CFG1))
                          then
                            begin
                              lib_change_state (q330, LIBSTATE_WAIT, LIBERR_INVAL_TOKENS) ;
                              sprintf(s, ", will retry in %d seconds", NR_TIME) ;
                              libmsgadd (q330, LIBMSG_INVTOK, addr(s)) ;
                              q330->reg_wait_timer = NR_TIME ;
                            end
                        break ;
                    end
                    break ;
                  case CERR_CTRL :
                    libmsgadd(q330, LIBMSG_CMDCTRL, "") ;
                    break ;
                  case CERR_SPEC :
                    libmsgadd(q330, LIBMSG_CMDSPEC, "") ;
                    break ;
                  case CERR_DB9 :
                    libmsgadd(q330, LIBMSG_CON, "") ;
                    break ;
                  case CERR_MEM :
                    libmsgadd(q330, LIBMSG_MEMOP, "") ;
                    break ;
                  case CERR_CIP :
                    libmsgadd(q330, LIBMSG_CALPROG, "") ;
                    break ;
                end
              end
          else if (pcmd->tunneled)
            then
              begin
                clear_cmsg (q330) ;
                if ((q330->share.tunnel_state == TS_SENT) land (q330->recvhdr.command == q330->share.tunnel.respcmd))
                  then
                    begin
                      lock (q330) ;
                      q330->share.tunnel_state = TS_READY ;
                      q330->share.tunnel.paysize = q330->recvhdr.datalength ;
                      memcpy (addr(q330->share.tunnel.payload), p, q330->share.tunnel.paysize) ;
                      if (q330->par_create.call_state)
                        then
                          begin
                            q330->state_call.context = q330 ;
                            q330->state_call.state_type = ST_TUNNEL ;
                            q330->state_call.subtype = 0 ;
                            memcpy(addr(q330->state_call.station_name), addr(q330->station_ident), sizeof(string9)) ;
                            q330->state_call.info = 0 ;
                            unlock (q330) ;
                            q330->par_create.call_state (addr(q330->state_call)) ;
                          end
                    end
              end
            else
              switch (pcmd->cmd) begin
                case C1_RQSRV :
                  if (q330->recvhdr.command == C1_SRVCH)
                    then
                      begin
                        loadsrvch (addr(p), addr(q330->srvch)) ;
                        memcpy (addr(q330->srvresp.serial), q330->par_create.q330id_serial, sizeof(t64)) ;
                        memcpy (addr(q330->srvresp.challenge), addr(q330->srvch.challenge), sizeof(t64)) ;
                        q330->srvresp.dpip = q330->srvch.dpip ;
                        q330->web_ip = q330->srvresp.dpip ;
                        q330->srvresp.dpport = q330->srvch.dpport ;
                        q330->srvresp.dpreg = q330->srvch.dpreg ;
#ifdef TEST_MD5
#ifdef ENDIAN_LITTLE
                        q330->srvresp.counter_chal[1] = 0x54321093 ;
                        q330->srvresp.counter_chal[0] = 0xFABCDE4B ;
#else
                        q330->srvresp.counter_chal[0] = 0x54321093 ;
                        q330->srvresp.counter_chal[1] = 0xFABCDE4B ;
#endif
#else
                        q330->srvresp.counter_chal[0] = ((longword)newrand(addr(q330->rsum)) shl 16) +
                                                        (longword)newrand(addr(q330->rsum)) ;
                        q330->srvresp.counter_chal[1] = ((longword)newrand(addr(q330->rsum)) shl 16) +
                                                        (longword)newrand(addr(q330->rsum)) ;
#endif
#ifdef ENDIAN_LITTLE
                        md5equiv[1] = q330->srvresp.dpip ;
                        md5equiv[0] = ((longword)q330->srvresp.dpport shl 16) or q330->srvresp.dpreg ;
#else
                        md5equiv[0] = q330->srvresp.dpip ;
                        md5equiv[1] = ((longword)q330->srvresp.dpport shl 16) or q330->srvresp.dpreg ;
#endif
                        strcpy(s1, dig2str(addr(q330->srvresp.challenge), addr(s3))) ;
                        strcat(s1, dig2str(addr(md5equiv), addr(s3))) ;
                        strcat(s1, dig2str(addr(q330->par_register.q330id_auth), addr(s3))) ;
                        strcat(s1, dig2str(addr(q330->par_create.q330id_serial), addr(s3))) ;
                        strcat(s1, dig2str(addr(q330->srvresp.counter_chal), addr(s3))) ;
                        calcmd5 (q330, addr(s1), addr(q330->srvresp.md5result)) ;
                        clear_cmsg (q330) ;
                        new_cmd (q330, C1_SRVRSP, 0) ;
                        q330->data_timer = 0 ; /* to read all structures */
                        q330->reg_timer = 0 ;
                      end
                  break ;
                case C1_SRVRSP :
                  if (q330->recvhdr.command == C1_CACK)
                    then
                      begin
                        clear_cmsg (q330) ;
                        strcpy(s, q330->station_ident) ;
                        sprintf(s2, " - Data%d (%s)", q330->par_create.q330id_dataport + 1,
                                showdot(q330->q330ip, addr(s1))) ;
                        strcat(s, s2) ;
                        libmsgadd (q330, LIBMSG_REGISTERED, addr(s)) ;
                        q330->registered = TRUE ;
                        q330->reg_tries = 0 ;
                        lib330_reboot (q330, FALSE) ;
                        q330->need_regmsg = (q330->cur_verbosity and VERB_REGMSG) ;
                      end
                  break ;
                case C1_DSRV :
                case C2_BOFF :
                  if (q330->recvhdr.command == C1_CACK)
                    then
                      begin
                        clear_cmsg (q330) ;
                        new_state (q330, q330->share.target_state) ;
                        q330->registered = FALSE ;
                        libmsgadd (q330, LIBMSG_DEREG, "") ;
                        close_sockets (q330) ;
                      end
                  break ;
                case C1_RQFGLS :
                  if (q330->recvhdr.command == C1_FGLS)
                    then
                      begin
                        psave = p ;
                        loadfgl (addr(p), addr(fgl)) ;
                        lock (q330) ;
                        memcpy (addr(q330->raw_fixed), p, RAW_FIXED_SIZE) ;
                        loadfix (addr(p), addr(q330->share.fixed)) ;
                        q330->q335 = ((q330->share.fixed.flags and FF_335) != 0) ;
                        p = psave ;
                        incn(p, fgl.gl_off) ;
                        memcpy (addr(q330->raw_global), p, RAW_GLOBAL_SIZE) ;
                        loadglob (addr(p), addr(q330->share.global)) ;
                        p = psave ;
                        incn(p, fgl.sc_off) ;
                        loadsensctrl (addr(p), addr(q330->share.sensctrl)) ;
                        if (fgl.lp_off == 0)
                          then
                            begin
                              unlock (q330) ;
                              libmsgadd (q330, LIBMSG_DATADIS, "") ;
                              start_deregistration (q330) ;
                              return ;
                            end
                        p = psave ;
                        incn(p, fgl.lp_off) ;
                        memcpy (addr(q330->raw_log), p, RAW_LOG_SIZE) ;
                        loadlog (addr(p), addr(q330->share.log)) ;
                        clear_cmsg (q330) ;
                        q330->share.log.flags = q330->share.log.flags and not LNKFLG_SAVE ;
                        if (q330->share.log.flags and LNKFLG_FILL)
                          then
                            strcpy(s2, "On") ;
                          else
                            strcpy(s2, "Off") ;
                        sprintf(s, "Win=%d Min. TO=%d Max. TO=%d Seq=%d Flood=%s", q330->share.log.window,
                                q330->share.log.rsnd_min, q330->share.log.rsnd_max,
                                q330->share.log.dataseq, s2) ;
                        q330->piggyok = (q330->share.log.flags and LNKFLG_PIGGY) == 0 ;
                        unlock (q330) ;
                        libmsgadd (q330, LIBMSG_COMBO, "") ;
                        libmsgadd (q330, LIBMSG_WINDOW, addr(s)) ;
                        if (lnot q330->link_recv)
                          then
                            reset_link (q330) ;
                        new_cfg (q330, make_bitmap(CRB_GLOB) or make_bitmap(CRB_FIX) or
                                       make_bitmap(CRB_LOG) or make_bitmap(CRB_SENSCTRL)) ;
#ifndef OMIT_SDUMP
                        if (q330->cur_verbosity and VERB_SDUMP)
                          then
                            begin
                              new_cmd (q330, C2_RQGPS, sizeof(tgps2)) ;
                              if (lnot q330->q335)
                                then
                                  begin
                                    new_cmd (q330, C1_RQMAN, sizeof(tman)) ;
                                    new_cmd (q330, C1_RQDCP, sizeof(tdcp)) ;
                                  end
                            end
#endif
                        if (q330->share.fixed.flags and FF_EP)
                          then
                            new_cmd (q330, C2_RQEPD, sizeof(tepdelay)) ;
                        if (q330->q335)
                          then
                            libmsgadd(q330, LIBMSG_Q335, "") ;
                      end
                  break ;
                case C1_SLOG :
                  if (q330->recvhdr.command == C1_CACK)
                    then
                      begin
                        lock (q330) ;
                        memcpy (addr(q330->share.log), addr(q330->share.newlog), sizeof(tlog)) ;
                        clear_cmsg (q330) ;
                        q330->share.log.flags = q330->share.log.flags and not LNKFLG_SAVE ;
                        if (q330->share.log.flags and LNKFLG_FILL)
                          then
                            strcpy(s2, "On") ;
                          else
                            strcpy(s2, "Off") ;
                        sprintf(s, "Win=%d Min. TO=%d Max. TO=%d Seq=%d Flood=%s", q330->share.log.window,
                                q330->share.log.rsnd_min, q330->share.log.rsnd_max,
                                q330->share.log.dataseq, s2) ;
                        q330->piggyok = (q330->share.log.flags and LNKFLG_PIGGY) == 0 ;
                        unlock (q330) ;
                        libmsgadd (q330, LIBMSG_WINDOW, addr(s)) ;
                        new_cfg (q330, make_bitmap(CRB_LOG)) ;
                      end
                  break ;
                case C1_RQGID :
                  if (q330->recvhdr.command == C1_GID)
                    then
                      begin
                        clear_cmsg (q330) ;
                        lock (q330) ;
                        loadgpsids (addr(p), addr(q330->share.gpsids)) ;
                        unlock (q330) ;
                        libmsgadd (q330, LIBMSG_GPSID, "") ;
                        new_cfg (q330, make_bitmap(CRB_GPSIDS)) ;
                      end
                  break ;
                case C1_RQSTAT :
                  if (q330->recvhdr.command == C1_STAT)
                    then
                      begin
                        req_mask = loadstatmap (addr(p)) ;
                        if (req_mask and make_bitmap(SRB_UMSG))
                          then
                            begin /* User message */
                              lock (q330) ;
                              loadumsg (addr(p), addr(q330->share.newuser)) ;
                              unlock (q330) ;
                              showdot (q330->share.newuser.sender, addr(s)) ;
                              strcat (s, ":") ;
                              strcat (s, q330->share.newuser.msg) ;
                              libmsgadd (q330, LIBMSG_USER, addr(s)) ;
                              lock (q330) ;
                              q330->share.newuser.msg[0] = 0 ; /* clear it */
                              unlock (q330) ;
                            end
                        if (req_mask and make_bitmap(SRB_LCHG))
                          then
                            begin /* change in logical port programming */
                              libmsgadd (q330, LIBMSG_LOGCHG, "") ;
                              lock (q330) ;
                              q330->share.have_config = q330->share.have_config and not make_bitmap(CRB_LOG) ; /* expired */
                              unlock (q330) ;
                              new_cmd (q330, C1_RQLOG, sizeof(tlog)) ;
                            end
                        if (req_mask and make_bitmap(SRB_TOKEN))
                          then
                            begin /* DP Token Change */
                              libmsgadd (q330, LIBMSG_TOKCHG, "") ;
                              lib_change_state (q330, LIBSTATE_RUNWAIT, LIBERR_TOKENS_CHANGE) ;
                            end
                        if (req_mask and make_bitmap(SRB_EPDLY))
                          then
                            begin /* EP Delay Change */
                              libmsgadd (q330, LIBMSG_EPDLYCHG, "") ;
                              new_cmd (q330, C2_RQEPD, sizeof(tepdelay)) ;
                            end
                        /* remove non-status structure flags */
                        req_mask = req_mask and not (make_bitmap(SRB_UMSG) or
                                    make_bitmap(SRB_LCHG) or make_bitmap(SRB_TOKEN)) ;
                        lock (q330) ;
                        for (bitnum = SRB_GLB ; bitnum <= SRB_FES ; bitnum++)
                          begin
                            mask = make_bitmap(bitnum) ;
                            if (mask and req_mask)
                              then
                                switch (bitnum) begin
                                  case SRB_GST :
                                    loadgpsstat (addr(p), addr(q330->share.stat_gps)) ;
                                    update_gps_stats (q330) ;
                                    break ;
                                  case SRB_GLB :
                                    pglob = addr(q330->share.stat_global) ;
                                    loadglobalstat (addr(p), pglob) ;
                                    if ((pglob->cal_stat and (CAL_SGON or CAL_ENON)) == 0)
                                      then
                                        clear_calstat (q330) ;
                                    q330->share.opstat.auxinp = pglob->misc_inp ;
                                    q330->share.opstat.clock_qual =
                                       translate_clock (addr(q330->qclock), pglob->clock_qual,
                                                        pglob->clock_loss) ;
                                    if (pglob->usec_offset < 500000)
                                      then
                                        l = pglob->usec_offset ;
                                      else
                                        l = pglob->usec_offset - 1000000 ;
                                    q330->share.opstat.clock_drift = l ;
                                    q330->share.opstat.pll_stat = (enum tpll_stat)(pglob->clock_qual shr 6) ;
                                    if (pglob->sec_offset)
                                      then
                                        begin /* Have a valid time */
                                          t = pglob->cur_sequence + pglob->sec_offset + pglob->usec_offset / 1.0E6 ;
                                          baler_callback (q330, BT_Q330TIME, lib_round(t)) ;
                                          if (q330->par_create.opt_zoneadjust)
                                            then
                                              begin /* check automatic timezone adjustment */
                                                l = lib_round((t - now ()) / 1800) ; /* get zone number ( in 30 minute increments ) */
                                                if ((q330->zone_adjust div 1800) != l)
                                                  then
                                                    begin /* change it */
                                                      sprintf(s, "%3.1f to %3.1f Hours", q330->zone_adjust / 3600.0, l * 0.5) ;
                                                      libmsgadd(q330, LIBMSG_ZONE, addr(s)) ;
                                                      q330->zone_adjust = l * 1800 ; /* to seconds */
                                                    end
                                              end
                                          l = lib_round(t) ;
                                          l = l - (q330->share.opstat.last_data_time - q330->qclock.zone) ; /* data difference */
                                          if (abs(l) < 10)
                                            then
                                              q330->dss_gate_on = TRUE ;
                                            else
                                              q330->dss_gate_on = FALSE ;
                                        end
                                      else
                                        q330->dss_gate_on = FALSE ;
                                    break ;
                                  case SRB_PWR :
                                    loadpwrstat (addr(p), addr(q330->share.stat_pwr)) ;
                                    break ;
                                  case SRB_BOOM :
                                    loadboomstat (addr(p), addr(q330->share.stat_boom), q330->q335) ;
                                    for (i = 0 ; i <= 5 ; i++)
                                      q330->share.opstat.mass_pos[i] = q330->share.stat_boom.booms[i] ;
                                    q330->share.opstat.sys_temp = q330->share.stat_boom.sys_temp ;
                                    q330->share.opstat.pwr_volt = q330->share.stat_boom.supply * 0.15 ;
                                    q330->share.opstat.pwr_cur = q330->share.stat_boom.main_cur * 0.001 ;
                                  break ;
                                  case SRB_PLL :
                                    loadpllstat (addr(p), addr(q330->share.stat_pll)) ;
                                    q330->share.opstat.pll_stat = (enum tpll_stat)(q330->share.stat_pll.state shr 6) ;
                                    break ;
                                  case SRB_GSAT :
                                    loadgpssats (addr(p), addr(q330->share.stat_sats)) ;
#ifndef OMIT_SEED
                                    if (q330->need_sats)
                                      then
                                        finish_log_clock (q330) ;
#endif
                                    break ;
                                  case SRB_ARP :
                                    loadarpstat (addr(p), addr(q330->share.stat_arp)) ;
                                    break ;
                                  case SRB_AUX :
                                    loadauxstat (addr(p), addr(q330->share.stat_auxad)) ;
                                    break ;
                                  case SRB_SS :
                                    loadssstat (addr(p), addr(q330->share.stat_sersens)) ;
                                    break ;
                                  case SRB_EP :
                                    loadepstat (addr(p), addr(q330->share.stat_ep)) ;
                                    break ;
                                  case SRB_FES :
                                    loadfestats (p, addr(q330->share.stat_fes)) ;
                                    break ;
                                  case SRB_LOG1 :
                                  case SRB_LOG2 :
                                  case SRB_LOG3 :
                                  case SRB_LOG4 :
                                    loadlogstat (addr(p), addr(q330->share.stat_log)) ;
                                    if (q330->need_regmsg)
                                      then
                                        begin
                                          q330->need_regmsg = FALSE ;
                                          q330->share.pingreq.pingtype = 0 ; /* simple ping */
                                          strcpy(q330->share.newuser.msg, q330->par_create.host_software) ;
                                          strcat(q330->share.newuser.msg, " registered.") ;
                                          unlock (q330) ;
                                          new_cmd (q330, C1_PING, 0) ;
                                          new_cmd (q330, C1_UMSG, 0) ;
                                          lock (q330) ;
                                        end
                                    plog = addr(q330->share.stat_log) ;
                                    l = q330->share.fixed.log_sz[plog->log_num] ;
                                    if ((q330->libstate == LIBSTATE_RUN) land (l))
                                      then
                                        begin
                                          perc = ((plog->pack_used * 100.0) / l + 0.3) ;
                                          q330->share.opstat.pkt_full = perc ;
                                          if ((((q330->par_register.opt_buflevel) land (perc < q330->par_register.opt_buflevel)) lor
                                                (q330->share.stat_log.flags and LPSF_PWROFF)) land (q330->share.freeze_timer <= 0))
                                            then
                                              if (lnot((q330->balesim) land ((q330->share.access_timer) lor
                                                                  (q330->share.stat_log.flags and LPSF_POWER))))
                                                then
                                                  begin
                                                    q330->reg_wait_timer = (integer)q330->par_register.opt_connwait * 60 ;
                                                    if (q330->reg_wait_timer == 0)
                                                      then
                                                        q330->reg_wait_timer = 600 ; /* if not set, use 10 minutes */
                                                    libmsgadd (q330, LIBMSG_BUFSHUT, "") ;
                                                    unlock (q330) ;
                                                    lib_change_state (q330, LIBSTATE_WAIT, LIBERR_BUFSHUT) ;
                                                    lock (q330) ;
                                                  end
                                        end
                                    if (q330->lastds_sent_count == 0)
                                      then
                                        begin
                                          q330->lastds_sent_count = plog->sent ;
                                          q330->lastds_resent_count = plog->resends ;
                                          q330->last_sent_count = plog->sent ;
                                          q330->last_resent_count = plog->resends ;
                                        end
                                    break ;
                                  case SRB_SER1 :
                                  case SRB_SER2 :
                                  case SRB_SER3 :
                                    loadserstat (addr(p), addr(q330->share.stat_serial)) ;
                                    break ;
                                  case SRB_ETH :
                                    loadethstat (addr(p), addr(q330->share.stat_ether)) ;
                                    break ;
                                  case SRB_BALER :
                                    loadbalestat (addr(p), addr(q330->share.stat_baler)) ;
                                    break ;
                                  case SRB_DYN :
                                    loaddynstat (addr(p), addr(q330->share.stat_dyn)) ;
                                    break ;
                                end
                            q330->stat_request = q330->stat_request and not req_mask ; /* satisfied */
                          end
                        unlock (q330) ;
                        new_status (q330, req_mask) ; /* let client know what was received */
                        clear_cmsg (q330) ;
                        if (q330->stat_request == 0)
                          then
                            begin
                              q330->status_timer = 0 ;
                              q330->last_status_received = now () ;
                              if (lnot q330->reboot_done)
                                then
                                  begin
                                    q330->reboot_done = TRUE ;
                                    cfg_start (q330) ;
                                  end
                            end
                          else
                            new_cmd (q330, C1_RQSTAT, stat_estimate(q330->stat_request)) ;
                      end
                  break ;
                case C1_UMSG :
                  if (q330->recvhdr.command == C1_CACK)
                    then
                      begin
                        clear_cmsg (q330) ;
                        lock (q330) ;
                        q330->share.newuser.msg[0] = 0 ; /* clearing the message is a flag not to re-send */
                        unlock (q330) ;
                      end
                  break ;
                case C1_PING :
                  if (q330->recvhdr.command == C1_PING)
                    then
                      begin
                        lock (q330) ;
                        if (q330->share.client_ping == CLP_SENT)
                          then
                            begin
                              q330->share.client_ping = CLP_IDLE ;
                              loadpinghdr (addr(p), addr(q330->pinghdr)) ;
                              if (q330->par_create.call_state)
                                then
                                  begin
                                    q330->state_call.context = q330 ;
                                    q330->state_call.state_type = ST_PING ;
                                    q330->state_call.subtype = q330->pinghdr.ping_type ;
                                    memcpy(addr(q330->state_call.station_name), addr(q330->station_ident), sizeof(string9)) ;
                                    t = now () ;
                                    q330->state_call.info = (t - q330->ping_send) * 1000.0 ; /* milliseconds */
                                    q330->pingbuffer.ping_type = q330->pinghdr.ping_type ;
                                    q330->pingbuffer.ping_id = q330->pinghdr.ping_opt ;
                                    q330->par_create.call_state (addr(q330->state_call)) ;
                                    if (q330->libstate == LIBSTATE_PING)
                                      then
                                        q330->share.target_state = LIBSTATE_IDLE ;
                                  end
                            end
                        unlock (q330) ;
                        clear_cmsg (q330) ;
                      end
                  break ;
                case C1_RQLOG :
                  if (q330->recvhdr.command == C1_LOG)
                    then
                      begin
                        lock (q330) ;
                        loadlog (addr(p), addr(q330->share.log)) ;
                        unlock (q330) ;
                        clear_cmsg (q330) ;
                        new_cfg (q330, make_bitmap(CRB_LOG)) ;
                      end
                  break ;
                case C2_RQEPD :
                  if (q330->recvhdr.command == C2_EPD)
                    then
                      begin
                        lock (q330) ;
                        loadepd (addr(p), addr(q330->share.epdelay)) ;
                        unlock (q330) ;
                        clear_cmsg (q330) ;
                        update_ep_delays (q330, FALSE, TRUE) ;
                      end
                  break ;
                case C2_RQEPCFG :
                  if (q330->recvhdr.command == C2_EPCFG)
                    then
                      begin
                        lock (q330) ;
                        loadepcfg (addr(p), addr(q330->share.epcfg)) ;
                        unlock (q330) ;
                        clear_cmsg (q330) ;
                        verify_epcfg (q330) ;
                      end
                  break ;
                case C2_SEPCFG :
                  if (q330->recvhdr.command == C1_CACK)
                    then
                      begin
                        lock (q330) ;
                        memcpy (addr(q330->share.epcfg), addr(q330->share.newepcfg), sizeof(tepcfg)) ;
                        unlock (q330) ;
                        clear_cmsg (q330) ;
                      end
                  break ;
                case C1_RQRT :
                  if (q330->recvhdr.command == C1_RT)
                    then
                      begin
                        lock (q330) ;
                        loadroutes(addr(p), q330->recvhdr.datalength, addr(q330->share.routelist)) ;
                        unlock (q330) ;
                        clear_cmsg (q330) ;
                        new_cfg (q330, make_bitmap(CRB_ROUTES)) ;
                      end
                  break ;
                case C1_RQDEV :
                  if (q330->recvhdr.command == C1_DEV)
                    then
                      begin
                        lock (q330) ;
                        loaddevs (addr(p), q330->recvhdr.datalength, addr(q330->share.devs)) ;
                        unlock (q330) ;
                        clear_cmsg (q330) ;
                        new_cfg (q330, make_bitmap(CRB_DEVS)) ;
                      end
                  break ;
                case C1_RQMEM :
                  if (q330->recvhdr.command == C1_MEM)
                    then
                      begin
                        clear_cmsg (q330) ;
                        loadmemhdr (addr(p), addr(q330->mem_hdr)) ;
                        if ((q330->libstate == LIBSTATE_READTOK) land
                           (q330->mem_hdr.memtype == (MT_CFG1 + q330->par_create.q330id_dataport)))
                          then
                            read_q330_cfg (q330, p) ;
                      end
                  break ;
                case C1_WEB :
                  if (q330->recvhdr.command == C1_CACK)
                    then
                      clear_cmsg (q330) ;
                  break ;
                case C2_BRDY :
                  if (q330->recvhdr.command == C2_BACK)
                    then
                      begin
                        loadback (addr(p), addr(q330->back)) ;
                        if (q330->par_create.q330id_dataport != q330->back.lport)
                          then
                            begin
                              libmsgadd (q330, LIBMSG_WRONGPORT, "") ;
                              clear_cmsg (q330) ;
                              return ;
                            end
                        q330->q330ip = q330->back.q330_ip ;
                        if ((q330->usesock == 0) land (q330->back.poc_ip))
                          then
                            q330->serial_ip = q330->back.poc_ip ;
                        q330->par_register.q330id_baseport = q330->back.bport ;
                        memcpy(addr(q330->par_create.q330id_serial), addr(q330->back.sernum), sizeof(t64)) ;
                        clear_cmsg (q330) ;
                        q330->q330cport = q330->par_register.q330id_baseport + 2 * (q330->par_create.q330id_dataport + 1) ;
                        q330->q330dport = q330->q330cport + 1 ;
                        q330->share.opstat.current_ip = q330->q330ip ;
                        q330->share.opstat.current_port = q330->q330cport ;
                        if (q330->back.flags and BA_PDOWN)
                          then
                            strcpy (s, " - Power Cycled") ;
                          else
                            strcpy (s, " - Continuous") ;
                        if (q330->back.flags and BA_MANUAL)
                          then
                            strcat (s, ", Manual On") ;
                        libmsgadd (q330, LIBMSG_BACK, addr(s)) ;
                        q330->share.access_timer = INITIAL_ACCESS_TIMOUT ; /* Force staying on a minimum amount of time */
                        if (q330->par_create.call_baler)
                          then
                            begin
                              q330->baler_call.context = q330 ;
                              q330->baler_call.baler_type = BT_BACK ;
                              memcpy(addr(q330->baler_call.station_name), addr(q330->station_ident), sizeof(string9)) ;
                              q330->baler_call.info2 = addr(q330->back) ;
                              q330->baler_call.info = q330->back.poc_ip ;
                              q330->baler_call.response = 0 ;
                              q330->par_create.call_baler (addr(q330->baler_call)) ;
                            end
#ifndef OMIT_NETWORK
                        if (q330->usesock)
                          then
                            open_sockets (q330, TRUE, TRUE) ; /* use real addresses */
#endif
                        new_state (q330, LIBSTATE_REG) ;
                        new_cmd (q330, C1_RQSRV, sizeof(t64)) ;
                        q330->reg_timer = 0 ;
                      end
                  break ;
                case C2_REGCHK :
                  if (q330->recvhdr.command == C2_REGRESP)
                    then
                      begin
                        check_resp = loadlongword (addr(p)) ;
                        clear_cmsg (q330) ;
                        baler_callback (q330, BT_REGRESP, check_resp) ;
                      end
                  break ;
#ifndef OMIT_SDUMP
                case C2_RQGPS :
                  if (q330->recvhdr.command == C2_GPS)
                    then
                      begin
                        loadgps2 (addr(p), addr(q330->gps2)) ;
                        clear_cmsg (q330) ;
                        /* no need to inform client, this info is only for status dump */
                      end
                  break ;
                case C1_RQMAN :
                  if (q330->recvhdr.command == C1_MAN)
                    then
                      begin
                        clear_cmsg (q330) ;
                        loadman (addr(p), addr(q330->man)) ;
                      end
                  break ;
                case C1_RQDCP :
                  if (q330->recvhdr.command == C1_DCP)
                    then
                      begin
                        clear_cmsg (q330) ;
                        loaddcp (addr(p), addr(q330->dcp)) ;
                      end
                  break ;
#endif
              end
        else
          add_status (q330, AC_SEQERR, 1) ;
end

void lib_timer (pq330 q330)
begin
  word piggy_threshold ;
  longword cfgnum ;
  longword bitmap ;
  tcommands *pc ;
  string95 s ;
#ifndef OMIT_SEED
  paqstruc paqs ;
#endif

  pc = addr(q330->commands) ;
#ifdef CMEX32
  if (q330->needtosayhello)
    then
      begin
        q330->needtosayhello = FALSE ;
        libmsgadd (q330, LIBMSG_CREATED, "") ;
      end
#endif
  if (q330->contmsg[0])
    then
      begin
        libmsgadd (q330, LIBMSG_CONTIN, addr(q330->contmsg)) ;
        q330->contmsg[0] = 0 ;
      end
  if (q330->libstate != LIBSTATE_RUN)
    then
      begin
        q330->conn_timer = 0 ;
        q330->share.opstat.gps_age = -1 ;
      end
    else
      q330->dynip_age = 0 ;
  baler_callback (q330, BT_TIMER, 0) ;
  if (q330->libstate != q330->share.target_state)
    then
      begin
        q330->share.opstat.runtime = 0 ;
        switch (q330->share.target_state) begin
          case LIBSTATE_WAIT :
          case LIBSTATE_IDLE :
            switch (q330->libstate) begin
              case LIBSTATE_IDLE :
                restore_thread_continuity (q330, FALSE, NIL) ; /* load up DP statistics LCQ's */
                purge_thread_continuity (q330) ; /* and mark as purged */
                new_state (q330, q330->share.target_state) ;
                break ;
              case LIBSTATE_CONN :
              case LIBSTATE_REG :
              case LIBSTATE_ANNC :
                close_sockets (q330) ;
                new_state (q330, q330->share.target_state) ;
                break ;
              case LIBSTATE_READCFG :
              case LIBSTATE_READTOK :
              case LIBSTATE_DECTOK :
                start_deregistration (q330) ;
                break ;
              case LIBSTATE_RUNWAIT :
              case LIBSTATE_RUN :
                start_deallocation(q330) ;
                break ;
              case LIBSTATE_PING :
                new_state (q330, q330->share.target_state) ;
                q330->registered = FALSE ;
                close_sockets (q330) ;
                break ;
              case LIBSTATE_WAIT :
                new_state (q330, q330->share.target_state) ;
                break ;
            end
            break ;
          case LIBSTATE_RUNWAIT :
            switch (q330->libstate) begin
              case LIBSTATE_DECTOK :
                decode_cfg (q330) ;
                strcpy(s, q330->station_ident) ;
                strcat(s, ", ") ;
                strcat(s, q330->par_create.host_software) ;
                libmsgadd(q330, LIBMSG_NETSTN, addr(s)) ;
                if (q330->cur_verbosity and VERB_SDUMP)
                  then
                    log_all_info (q330) ;
                break ;
              case LIBSTATE_RUN :
                start_deallocation(q330) ;
                break ;
              case LIBSTATE_IDLE :
                restore_thread_continuity (q330, FALSE, NIL) ; /* load up DP statistics LCQ's */
                purge_thread_continuity (q330) ; /* and mark as purged */
                lib_start_registration (q330) ;
                break ;
              case LIBSTATE_WAIT :
                lib_start_registration (q330) ;
                break ;
            end
            break ;
          case LIBSTATE_RUN :
            switch (q330->libstate) begin
              case LIBSTATE_RUNWAIT :
                send_dopen (q330) ;
                add_status (q330, AC_COMSUC, 1) ; /* complete cycle */
                new_state (q330, LIBSTATE_RUN) ;
#ifndef OMIT_SEED
                paqs = q330->aqstruc ;
                if (paqs->dss_def.port_number)
                  then
                    lib_dss_start (addr(paqs->dss_def), q330, q330->par_register.opt_dss_memory) ;
#endif
                break ;
            end
            break ;
          case LIBSTATE_PING :
            switch (q330->libstate) begin
              case LIBSTATE_IDLE :
                lib_start_ping (q330) ;
                break ;
            end
            break ;
          case LIBSTATE_TERM :
            if (lnot q330->terminate)
              then
                begin
                  if (q330->libstate != LIBSTATE_PING)
                    then
                      begin
#ifndef OMIT_SEED
                        flush_dplcqs (q330) ;
#endif
                        save_thread_continuity (q330) ;
                      end
                  q330->terminate = TRUE ; /* shutdown thread */
#ifdef CMEX32
                  new_state (q330, LIBSTATE_TERM) ;
#endif
                end
            break ;
        end
      end
  switch (q330->libstate) begin
    case LIBSTATE_IDLE :
      inc(q330->timercnt) ;
      if (q330->timercnt >= 10)
        then
          begin /* about 1 second */
            q330->timercnt = 0 ;
            q330->share.opstat.runtime = q330->share.opstat.runtime - 1 ;
            continuity_timer (q330) ;
          end
      return ;
    case LIBSTATE_TERM : return ;
    case LIBSTATE_WAIT :
      inc(q330->timercnt) ;
      if (q330->timercnt >= 10)
        then
          begin /* about 1 second */
            q330->timercnt = 0 ;
            dec(q330->reg_wait_timer) ;
            if (q330->reg_wait_timer <= 0)
              then
                begin
                  if ((q330->par_register.opt_dynamic_ip) land ((q330->par_register.q330id_address[0] == 0) lor
                     ((q330->par_register.opt_ipexpire) land ((q330->dynip_age div 60) >= q330->par_register.opt_ipexpire))))
                    then
                      q330->reg_wait_timer = 1 ;
                    else
                      lib_change_state (q330, LIBSTATE_RUNWAIT, LIBERR_NOERR) ;
                end
            q330->share.opstat.runtime = q330->share.opstat.runtime - 1 ;
            continuity_timer (q330) ;
          end
      return ;
  end
  lock (q330) ;
  if (q330->share.abort_requested)
    then
      begin
        q330->share.abort_requested = FALSE ;
        abort_cmsg (q330) ;
      end
  if (q330->share.usermessage_requested)
    then
      begin
        q330->share.usermessage_requested = FALSE ;
        memcpy (addr(q330->share.newuser), addr(q330->share.user_message), sizeof(tuser_message)) ;
        unlock (q330) ;
        new_cmd (q330, C1_UMSG, 0) ;
        lock (q330) ;
      end
  if (q330->share.webadv_requested)
    then
      begin
        q330->share.webadv_requested = FALSE ;
        unlock (q330) ;
        new_cmd (q330, C1_WEB, 0) ;
        lock (q330) ;
      end
  unlock (q330) ;
#ifndef OMIT_SEED
  dump_msgqueue (q330) ;
  if (q330->flush_all)
    then
      begin
        q330->flush_all = FALSE ;
        paqs = q330->aqstruc ;
        if (q330->libstate == LIBSTATE_RUN)
          then
            flush_lcqs (paqs) ;
        if (q330->libstate >= LIBSTATE_REG)
          then
            flush_dplcqs (q330) ;
      end
#endif
  if ((pc->cphase == CP_WAIT) land (pc->ctrlrecnt == 0))
    then
      begin
        if (pc->cmdq[pc->cmdout].cmd == C1_PING)
          then
            begin
              clear_cmsg (q330) ;
              if (q330->share.client_ping == CLP_SENT)
                then
                  begin
                    q330->share.client_ping = CLP_IDLE ;
                    if (q330->par_create.call_state)
                      then
                        begin
                          q330->state_call.context = q330 ;
                          q330->state_call.state_type = ST_PING ;
                          q330->state_call.subtype = 1 ;
                          memcpy(addr(q330->state_call.station_name), addr(q330->station_ident), sizeof(string9)) ;
                          q330->state_call.info = 0xFFFFFFFF ; /* no response */
                          q330->pingbuffer.ping_type = 1 ; /* fake a response */
                          q330->pingbuffer.ping_id = q330->pinghdr.ping_opt ;
                          q330->par_create.call_state (addr(q330->state_call)) ;
                          if (q330->libstate == LIBSTATE_PING)
                            then
                              q330->share.target_state = LIBSTATE_IDLE ;
                        end
                  end
            end
          else
            begin
              pc->cphase = CP_NEED ;
              if (pc->ctrl_retries < 10)
                then
                  inc(pc->ctrl_retries) ;
              add_status (q330, AC_CMDTO, 1) ;
              if (lnot q330->stalled_link)
                then
                  begin
                    q330->stalled_link = TRUE ;
                    state_callback (q330, ST_STALL, 1) ;
                  end
              if (q330->cur_verbosity and VERB_RETRY)
                then
                  libmsgadd (q330, LIBMSG_RETRY, command_name (pc->cmdq[pc->cmdout].cmd, addr(s))) ;
            end
      end
  if (q330->share.log_changed)
    then
      begin
        q330->share.log_changed = FALSE ;
        new_cmd (q330, C1_SLOG, 0) ;
      end
  if (q330->share.client_ping == CLP_REQ)
    then
      begin
        q330->share.client_ping = CLP_SENT ;
        new_cmd (q330, C1_PING, MAXDATA) ;
      end
  if (q330->share.tunnel_state == TS_REQ)
    then
      begin
        q330->share.tunnel_state = TS_SENT ;
        new_cmd (q330, q330->share.tunnel.reqcmd, TUNCMDFLAG) ;
      end
  for (cfgnum = CRB_LOG ; cfgnum <= CRB_DEVS ; cfgnum++)
    begin
      bitmap = make_bitmap(cfgnum) ;
      if (bitmap and q330->share.want_config)
        then
          begin
            q330->share.want_config = q330->share.want_config and not bitmap ;
            switch (cfgnum) begin
              case CRB_LOG :
                new_cmd(q330, C1_RQLOG, sizeof(tlog)) ;
                break ;
              case CRB_ROUTES :
                new_cmd(q330, C1_RQRT, MAXDATA) ;
                break ;
              case CRB_DEVS :
                new_cmd(q330, C1_RQDEV, MAXDATA) ;
                break ;
            end
          end
    end
  if (q330->ack_delay)
    then
      begin
        dec(q330->ack_delay) ;
        if ((q330->ack_delay == 0) land (q330->link_recv))
          then
            dack_out (q330) ;
      end
  inc(q330->timercnt) ;
  if (q330->timercnt >= 10)
    then
      begin /* about 1 second */
        q330->timercnt = 0 ;
        if (pc->ctrlrecnt)
          then
            dec(pc->ctrlrecnt) ;
        inc(q330->share.interval_counter) ;
        if (q330->share.freeze_timer > 0)
          then
            begin
              lock (q330) ;
              dec(q330->share.freeze_timer) ;
              unlock (q330) ;
            end
          else
            continuity_timer (q330) ;
        if (q330->libstate != LIBSTATE_RUN)
          then
            begin
              inc(q330->dynip_age) ;
              q330->share.opstat.runtime = q330->share.opstat.runtime - 1 ;
            end
          else
            begin
              q330->share.opstat.runtime = q330->share.opstat.runtime + 1 ;
              if (q330->update_ep_timer > 0)
                then
                  begin
                    dec(q330->update_ep_timer) ;
                    if (q330->update_ep_timer <= 0)
                      then
                        update_ep_delays (q330, FALSE, TRUE) ;
                  end
              add_status (q330, AC_DUTY, 1) ;
#ifndef OMIT_SEED
              paqs = q330->aqstruc ;
              if (paqs->cfg_timer > 0)
                then
                  begin
                    dec(paqs->cfg_timer) ;
                    if (paqs->cfg_timer <= 0)
                      then
                        flush_cfgblks (paqs, TRUE) ;
                  end
              if (paqs->log_timer > 0)
                then
                  begin
                    dec(paqs->log_timer) ;
                    if (paqs->log_timer <= 0)
                      then
                        flush_messages (paqs) ;
                  end
#endif
            end
        switch (q330->libstate) begin
          case LIBSTATE_DEREG :
            inc(q330->comtimer) ;
            if (q330->comtimer > 20)
              then
                begin
                  purge_cmdq (q330) ;
                  new_state (q330, q330->share.target_state) ;
                  q330->registered = FALSE ;
                  libmsgadd (q330, LIBMSG_DEREGTO, "") ;
                  add_status (q330, AC_COMATP, 1) ;
                  close_sockets (q330) ;
                end
            break ;
          case LIBSTATE_RUN :
            if (q330->share.freeze_timer <= 0)
              then
                inc(q330->data_timer) ;
            if (q330->data_timer > q330->data_timeout)
              then
                begin
                  purge_cmdq (q330) ;
                  lib_change_state (q330, LIBSTATE_WAIT, LIBERR_DATATO) ;
                  q330->reg_wait_timer = q330->data_timeout_retry ;
                  sprintf(s, "%d Minutes", q330->data_timeout_retry div 60) ;
                  libmsgadd (q330, LIBMSG_DATATO, addr(s)) ;
                  add_status (q330, AC_COMATP, 1) ;
                end
            if ((q330->share.log.flags and LNKFLG_FREEZE) == 0)
              then
                inc(q330->ack_timeout) ;
              else
                q330->ack_timeout = 0 ;
            if ((q330->ack_timeout > 100) lor ((q330->ack_timeout > 10) land (((paqstruc)(q330->aqstruc))->data_timetag < 1)))
              then
                begin
                  q330->ack_timeout = 0 ;
                  send_dopen (q330) ;
                end
            lock (q330) ;
            if (q330->share.access_timer)
              then
                dec(q330->share.access_timer) ;
            unlock (q330) ;
            if (lnot q330->piggyok)
              then
                begin
                  piggy_threshold = q330->share.status_interval shl 2 ;
                  if (piggy_threshold > 200)
                    then
                      piggy_threshold = 200 ;
                  if (q330->share.interval_counter >= piggy_threshold)
                    then
                      q330->piggyok = TRUE ;
                end
            if ((q330->share.have_config) land (q330->stat_request == 0) land (q330->piggyok) land
                 ((q330->share.interval_counter >= q330->share.status_interval)))
              then
                begin
                  q330->stat_request = make_bitmap(q330->par_create.q330id_dataport + SRB_LOG1) or
                                  make_bitmap(SRB_GLB) or make_bitmap (SRB_GST) or
                                  make_bitmap(SRB_BOOM) or make_bitmap (SRB_PLL) or q330->share.extra_status ;
                  if (q330->need_sats)
                    then
                      q330->stat_request = q330->stat_request or make_bitmap(SRB_GSAT) ;
                  new_cmd (q330, C1_RQSTAT, stat_estimate(q330->stat_request)) ;
                  q330->share.interval_counter = 0 ;
                  q330->piggyok = (q330->share.log.flags and LNKFLG_PIGGY) == 0 ;
                end
            if (q330->conn_timer < MAXLINT)
              then
                begin
                  inc(q330->conn_timer) ;
                  if ((q330->par_register.opt_conntime) land (q330->share.target_state != LIBSTATE_WAIT) land
                     ((q330->conn_timer div 60) >= q330->par_register.opt_conntime) land (q330->share.freeze_timer <= 0))
                    then
                      begin
                        if ((q330->balesim) land ((q330->share.access_timer) lor
                                            (q330->share.stat_log.flags and LPSF_POWER)))
                          then
                            break ; /* don't power down yet */
                        q330->reg_wait_timer = (integer)q330->par_register.opt_connwait * 60 ;
                        if (q330->reg_wait_timer == 0)
                          then
                            q330->reg_wait_timer = 600 ; /* if not set, use 10 minutes */
                        lib_change_state (q330, LIBSTATE_WAIT, LIBERR_CONNSHUT) ;
                        libmsgadd (q330, LIBMSG_CONNSHUT, "") ;
                      end
                end
            break ;
          case LIBSTATE_CONN :
          case LIBSTATE_REG :
          case LIBSTATE_ANNC :
            inc(q330->reg_timer) ;
            if (q330->reg_timer > 180) /* 3 minutes */
              then
                begin
                  purge_cmdq (q330) ;
                  lib_change_state (q330, LIBSTATE_WAIT, LIBERR_REGTO) ;
                  q330->registered = FALSE ;
                  add_status (q330, AC_COMATP, 1) ;
                  inc(q330->reg_tries) ;
                  if ((q330->par_register.opt_hibertime) land
                     (q330->par_register.opt_regattempts) land
                     (q330->reg_tries >= q330->par_register.opt_regattempts))
                    then
                      begin
                        q330->reg_tries = 0 ;
                        q330->reg_wait_timer = (integer)q330->par_register.opt_hibertime * 60 ;
                      end
                    else
                      q330->reg_wait_timer = 120 ;
                  sprintf(s, "%d Minutes", q330->reg_wait_timer div 60) ;
                  libmsgadd (q330, LIBMSG_SNR, addr(s)) ;
                end
            break ;
        end
        if ((q330->libstate >= LIBSTATE_READCFG) land (q330->libstate <= LIBSTATE_DEALLOC))
          then
            begin
              inc(q330->status_timer) ;
              if (q330->status_timer > q330->status_timeout) /* 5 minutes */
                then
                  begin
                    purge_cmdq (q330) ;
                    lib_change_state (q330, LIBSTATE_WAIT, LIBERR_STATTO) ;
                    q330->reg_wait_timer = q330->status_timeout_retry ;
                    sprintf(s, "%d Minutes", q330->status_timeout_retry div 60) ;
                    libmsgadd (q330, LIBMSG_STATTO, addr(s)) ;
                    add_status (q330, AC_COMATP, 1) ;
                  end
            end
      end
  lib_send_next_cmd (q330) ;
end
