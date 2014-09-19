/*   Lib330 Sliding Window routines
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
    0 2006-09-30 rdr Created
    1 2006-10-26 rdr Add setting of last_data_time.
    2 2006-11-19 rdr In DC_COMP processing reset "p" each time in case multiple LCQ's
                     for this data source
    3 2006-12-18 rdr Use dataport instead of host_dataport.
    4 2007-03-12 rdr Purge continuity when first start of second blockette is received.
    5 2007-06-26 rdr Change data record debugging to show DSN instead of ACK. If get a
                     sequence gap then set data_timetag to -1 to avoid processing blockettes
                     before a timing blockette is received.
    6 2007-07-06 rdr Data_timetag set to 0 instead of -1, anything less than +1 is considered
                     not yet set. Clear lasttime at the same time.
    7 2008-07-29 rdr Add handling of new serial sensor blockettes. Change serial sensor
                     configuration recording to use the enclosing blockette size minus
                     the overhead since the size will be different for different sensors.
    8 2008-08-19 rdr Add TCP support. Put in missing AC_WRITE update in dack_out.
*/
#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif
#ifndef libmsgs_h
#include "libmsgs.h"
#endif
#ifndef libsampglob_h
#include "libsampglob.h"
#endif
#ifndef libsample_h
#include "libsample.h"
#endif
#ifndef libslider_h
#include "libslider.h"
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
#ifndef libstats_h
#include "libstats.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif
#ifndef libcont_h
#include "libcont.h"
#endif

#ifndef OMIT_SEED
#ifndef liblogs_h
#include "liblogs.h"
#endif
#ifndef libsample_h
#include "libsample.h"
#endif
#ifndef libopaque_h
#include "libopaque.h"
#endif
#ifndef libcompress_h
#include "libcompress.h"
#endif
#endif

void allocate_packetbuffers (pq330 q330)
begin
  integer i ;

  for (i = 0 ; i <= 255 ; i++)
    getbuf (q330, addr(q330->pkt_bufs[i]), sizeof(tpkt_buf)) ;
end

static void reset_window (pq330 q330)
begin
  word w ;

  lock (q330) ;
  w = q330->share.log.window ;
  unlock (q330) ;
  if (w)
    then
      begin
        q330->window_size = w ;
        q330->autowin = FALSE ;
      end
  else if (lnot q330->autowin)
    then
      begin
        q330->window_size = 2 ;
        q330->autowin = TRUE ;
      end
end

void reset_link (pq330 q330)
begin
  integer i ;
  paqstruc paqs ;
  string31 s ;
  ppkt_buf pbuf ;

  paqs = q330->aqstruc ;
  lock (q330) ;
  q330->last_packet = q330->share.log.dataseq ;
  unlock (q330) ;
  for (i = 0 ; i <= 255 ; i++)
    begin
      pbuf = q330->pkt_bufs[i] ;
      pbuf->valid = FALSE ;
    end
  q330->link_recv = TRUE ;
  q330->lasttime = 0 ;
  paqs->data_timetag = 0.0 ;
  q330->lastseq = 0 ;
  q330->lastfill = 0 ;
  q330->ack_counter = 0 ;
  q330->autowin = FALSE ;
  reset_window (q330) ;
  q330->ack_delay = 0 ;
  paqs->data_qual = 0 ;
  sprintf(s, "%d", q330->last_packet) ;
  libmsgadd(q330, LIBMSG_LINKRST, addr(s)) ;
end

void send_dopen (pq330 q330)
begin
  pbyte p, pref, psave ;
  integer lth, msglth, err ;
  string95 s, s1 ;

  q330->ack_timeout = 0 ;
  q330->ack_counter = 0 ;
  if (lnot q330->registered)
    then
      return ;
  p = addr(q330->dataout.qdp) ;
  psave = p ;
  storeqdphdr (addr(p), DT_OPEN, 0, 0, 0) ;
  pref = p ; /* pointer after header for length calculation */
  libmsgadd (q330, LIBMSG_DTOPEN, "") ;
  lth = (integer)p - (integer)pref ; /* length of data */
  p = psave ;
  incn(p, 6) ; /* point at length */
  storeword (addr(p), lth) ;
  msglth = QDP_HDR_LTH + lth ;
  p = psave ;
  storelongint (addr(p), gcrccalc (addr(q330->crc_table), (pointer)((integer)p + 4), msglth - 4)) ;
  if (q330->cur_verbosity and VERB_PACKET)
    then
      begin /* log the message sent */
        p = addr(q330->dataout.qdp) ;
        loadqdphdr (addr(p), addr(q330->recvhdr)) ; /* for display purposes */
        packet_time (now(), addr(s)) ;
        command_name (q330->recvhdr.command, addr(s1)) ;
        strcat (s, s1) ;
        sprintf(s1, ", Lth=%d Seq=%d Ack=%d", q330->recvhdr.datalength, q330->recvhdr.sequence,
                q330->recvhdr.acknowledge) ;
        strcat(s, s1) ;
        libmsgadd(q330, LIBMSG_PKTOUT, addr(s)) ;
      end
  if (q330->usesock)
    then
      begin
        if (q330->tcp)
          then
            begin
              if (q330->cpath == INVALID_SOCKET)
                then
                  return ;
              p = (pointer)((integer)addr(q330->dataout.qdp) - 4) ;
              pref = p ; /* save start of tcp packet */
              storeword (addr(p), 1) ; /* data port */
              storeword (addr(p), msglth) ; /* qdp length */
              err = send(q330->cpath, (pchar)pref, msglth + 4, 0) ;
            end
          else
            begin
              if (q330->dpath == INVALID_SOCKET)
                then
                  return ;
              err = sendto(q330->dpath, addr(q330->dataout.qdp), msglth, 0, addr(q330->dsockout), sizeof(struct sockaddr)) ;
            end
        if (err == SOCKET_ERROR)
          then
            begin
              err =
#ifdef X86_WIN32
                     WSAGetLastError() ;
#else
                     errno ;
#endif
              if ((q330->tcp) land (err != EWOULDBLOCK))
                then
                  begin
                    sprintf(s1, "%d, Waiting 10 minutes", err) ;
                    tcp_error (q330, addr(s1)) ;
                  end
                else
                  begin
                    sprintf(s1, "%d", err) ;
                    libmsgadd(q330, LIBMSG_CANTSEND, addr(s1)) ;
                    add_status (q330, AC_IOERR, 1) ; /* add one I/O error */
                  end
            end
          else
            add_status (q330, AC_WRITE, msglth + IP_HDR_LTH + UDP_HDR_LTH) ;
      end
#ifndef OMIT_SERIAL
    else
      begin
        memcpy(addr(q330->commands.cmsgout.qdp), addr(q330->dataout.qdp), msglth) ;
        send_packet (q330, msglth, q330->q330dport, q330->dataport) ;
      end
#endif
end

typedef char string25[26] ;
typedef string25 tdigph[4] ;
typedef string25 tdigphr[4] ;
typedef string25 tgpsph[7] ;
typedef string25 tpwrph[5] ;
typedef string25 tgpscold[4] ;
const tdigph digph = {"Start", "Timemark Wait", "Internal Marks", "External Marks"} ;
const tdigphr digphr = {"Phase out of Range", "GPS Time Received", "Command Received"} ;
const tgpsph gpsph = {"Power Off - GPS Lock", "Power Off - PLL Lock", "Power Off - Max. Time",
                  "Power Off - Command", "Coldstart - ", "Power On - Automatic",
                  "Power On - Command"} ;
const tpwrph pwrph = {"Not Charging", "Bulk", "Absorption", "Float", "Equalization"} ;
const tgpscold gpscold = {"Command Received", "Reception Timeout",
                      "GPS & RTC out of phase ", "Large time jump"} ;

static void proc_insequence (pq330 q330, integer pktidx)
begin
  paqstruc paqs ;
  boolean seqgap_occurred ;
  boolean update_ok ;
  byte chan, idx, sub, subchan, portnum ;
  word lth, skip ;
  word wordval, wordval2, wordval3 ;
  integer loops, hr, v1, v2, v3 ;
  tqdp qhdr ;
  pbyte p, psave, pstart, pend, p2 ;
  longword dsn ;
  longint lval1, lval2 ;
  longword tmp_sec, tmp_usec ;
  integer diff ;
  double r, t ;
  string s ;

  paqs = q330->aqstruc ;
  seqgap_occurred = FALSE ;
  loops = 0 ;
  p = addr(q330->pkt_bufs[pktidx]->buf.qdp) ;
  loadqdphdr (addr(p), addr(qhdr)) ;
  psave = p ;
  lth = qhdr.datalength ;
  if (lth > MAXDATA)
    then
      begin
        sprintf(s, "%d", lth) ;
        libmsgadd (q330, LIBMSG_INVLTH, addr(s)) ;
        return ;
      end
  switch (qhdr.command) begin
    case DT_FILL :
      add_status (q330, AC_FILL, 1) ;
      dsn = loadlongword (addr(p)) ;
      if (dsn != (q330->lastfill + 1))
        then
          begin
            v1 = q330->lastfill ;
            v2 = dsn ;
            sprintf(s, "%d to %d", v1, v2) ;
            libmsgadd(q330, LIBMSG_FILLJMP, addr(s)) ;
          end
      q330->lastfill = dsn ;
      add_status (q330, AC_FILL, 1) ;
      break ;
    case DT_DATA :
      q330->data_timer = 0 ;
      dsn = loadlongword (addr(p)) ;
      v1 = paqs->dt_data_sequence ;
      v2 = dsn ;
      if ((dsn != paqs->dt_data_sequence) land (dsn != (paqs->dt_data_sequence + 1)))
        then
          begin
            if (paqs->dt_data_sequence == 0)
              then
                begin
                  sprintf(s, "%d", v2) ;
                  libdatamsg(q330, LIBMSG_SEQBEG, addr(s)) ; /* no continutity */
                end
              else
                begin
                  if ((q330->lastseq == 0) land (seqspread(dsn, paqs->dt_data_sequence) < 0) land
                                 (abs(seqspread(dsn, paqs->dt_data_sequence)) < MAXSPREAD))
                    then
                      begin
                        sprintf(s, "%d to %d", v1, v2) ;
                        libdatamsg (q330, LIBMSG_SEQOVER, addr(s)) ;
                      end
                    else
                      begin
                        v3 = seqspread(dsn, paqs->dt_data_sequence) ;
                        sprintf(s, "%d, %d to %d", v3, v1, v2) ;
                        libdatamsg (q330, LIBMSG_SEQGAP, addr(s)) ;
                        add_status (q330, AC_GAPS, 1) ;
                        inc(q330->share.opstat.totalgaps) ;
                        seqgap_occurred = TRUE ;
                      end
                end
            q330->lasttime = 0 ;
            paqs->data_timetag = 0 ;
          end
      else if (q330->lastseq == 0)
        then
          begin
            sprintf(s, "%d to %d", v1, v2) ;
            libdatamsg (q330, LIBMSG_SEQRESUME, addr(s)) ;
          end
      paqs->dt_data_sequence = dsn ; /* set global data record sequence number */
      q330->lastseq = paqs->dt_data_sequence ;
      pend = psave ;
      incn(pend, lth) ; /* one past end of blockettes */
      while ((loops < 100) land ((integer)p < (integer)pend))
        begin
          inc(loops) ;
          pstart = p ;
          chan = loadbyte (addr(p)) ; /* get channel */
          idx = chan and 127 ;
          sub = chan and not DCM ;
          subchan = loadbyte (addr(p)) ; /* subchannel or first 8 bit value */
          wordval = loadword (addr(p)) ; /* two 8 bit values, one 16 bit value, or blockette size */
          if ((chan and 0x80) == 0)
            then
              switch (chan and DCM_ST) begin
                case DC_ST38 : /* already have the entire blockette */
                  switch (chan and not DCM_ST) begin
                    case ST38_GPSPH :
                      strcpy(s, gpsph[subchan]) ;
                      if (subchan == GPS_COLD)
                        then
                          strcat(s, gpscold[wordval shr 8]) ;
                      libdatamsg(q330, LIBMSG_GPSSTATUS, addr(s)) ;
                      process_gps_phase_change (q330, subchan, wordval shr 8) ;
                      break ;
                    case ST38_DIGPH :
                      strcpy(s, digph[subchan]) ;
                      if (wordval shr 8)
                        then
                          begin
                            strcat(s, ", Reason=") ;
                            strcat(s, digphr[wordval shr 8]) ;
                          end
                      libdatamsg(q330, LIBMSG_DIGPHASE, addr(s)) ;
                      break ;
                    case ST38_BACKUP :
                      libdatamsg(q330, LIBMSG_SAVEBACKUP, "") ;
                      break ;
                    case ST38_WINSTAT :
                      if (subchan == RWR_START)
                        then
                          libdatamsg(q330, LIBMSG_SCHEDSTART, "") ;
                        else
                          begin
                            libdatamsg(q330, LIBMSG_SCHEDEND, "") ;
#ifndef OMIT_SEED
                            flush_lcqs (paqs) ;
                            no_previous (paqs) ;
#endif
                          end
                      break ;
                    case ST38_LEAP :
                      libdatamsg(q330, LIBMSG_LEAPDET, "") ;
                      break ;
                  end
                  break ;
                case DC_ST816 :
                  switch (chan and not DCM_ST) begin
                    case ST816_PSPH :
                      libdatamsg (q330, LIBMSG_SMUPHASE, pwrph[subchan]) ;
                      break ;
                    case ST816_AFAULT :
                      if (subchan != 0)
                        then
                          libdatamsg (q330, LIBMSG_APWRON, "") ;
                        else
                          libdatamsg (q330, LIBMSG_APWROFF, "") ;
                      break ;
                    case ST816_CALERR :
                      paqs->calerr_bitmap = subchan ;
                      break ;
                  end
                  break ;
                case DC_ST32 :
                  lval1 = loadlongint (addr(p)) ;
                  switch (chan and not DCM_ST) begin
                    case ST32_DRIFT :
                      v1 = wordval ;
                      r = v1 + lval1 * 1.0E-6 ;
                      sprintf(s, "8.6f Seconds", r) ;
                      libdatamsg (q330, LIBMSG_PHASERANGE, addr(s)) ;
                      break ;
                  end
                  break ;
                case DC_ST232 :
                  loadlongint (addr(p)) ;
                  loadlongint (addr(p)) ;
                  break ;
              end
            else
              switch (chan and DCM) begin
                case DC_MN816 :
                  paqs->proc_lcq = paqs->dispatch[idx] ;
                  while (paqs->proc_lcq)
                    begin
                      process_one (q330, wordval) ;
                      paqs->proc_lcq = paqs->proc_lcq->dispatch_link ;
                    end
                  break ;
                case DC_MN38 :
                  paqs->proc_lcq = paqs->dispatch[idx] ;
                  while (paqs->proc_lcq)
                    begin
                      switch (sub) begin
                        case 0 :
                          switch (paqs->proc_lcq->raw_data_field) begin
                            case 0 :
                              process_one (q330, subchan) ;
                              break ;
                            case 1 :
                              process_one (q330, bsex(wordval shr 8)) ;
                              break ;
                          end
                          break ;
                        case 1 :
                          switch (paqs->proc_lcq->raw_data_field) begin
                            case 0 :
                              process_one (q330, subchan) ;
                              break ;
                            case 1 :
                              process_one (q330, wordval shr 8) ;
                              break ;
                            case 2 :
                              process_one (q330, wordval and 255) ;
                              break ;
                          end
                          break ;
                        case 2 :
                          switch (paqs->proc_lcq->raw_data_field) begin
                            case 0 :
                              process_one (q330, subchan) ;
                              break ;
                            default :
                              process_one (q330, (subchan shr (paqs->proc_lcq->raw_data_field - 1)) and 1) ;
                              break ;
                          end
                          break ;
                        case 3 :
                          switch (paqs->proc_lcq->raw_data_field) begin
                            case 0 :
                              process_one (q330, subchan and 1) ;
                              break ;
                            case 1 :
                              process_one (q330, (subchan shr 1) and 1) ;
                              break ;
                            case 2 :
                              process_one (q330, (subchan shr 2) and 1) ;
                              break ;
                            case 3 :
                              process_one (q330, (subchan shr 3) and 1) ;
                              break ;
                            case 4 :
                              process_one (q330, (subchan shr 4) and 1) ;
                              break ;
                            case 6 :
                              process_one (q330, (subchan shr 6) and 1) ;
                              break ;
                            case 7 :
                              process_one (q330, (subchan shr 7) and 1) ;
                              break ;
                          end
                          break ;
                      end
                      paqs->proc_lcq = paqs->proc_lcq->dispatch_link ;
                    end
                  break ;
                case DC_MN32 :
                  lval1 = loadlongint (addr(p)) ; /* Serial Sensor */
                  paqs->proc_lcq = paqs->dispatch[idx] ;
                  while (paqs->proc_lcq)
                    begin
                      process_variable (q330, subchan, wordval, lval1) ;
                      paqs->proc_lcq = paqs->proc_lcq->dispatch_link ;
                    end
                  break ;
                case DC_MN232 :
                  lval1 = loadlongint (addr(p)) ;
                  lval2 = loadlongint (addr(p)) ;
                  switch (chan and not DCM) begin
                    case 0 : /* Timing update */
                      if (paqs->first_data)
                        then
                          begin
                            paqs->first_data = FALSE ;
                            purge_continuity (q330) ;
                          end
                      update_ok = TRUE ;
                      tmp_sec = lval1 ;
                      tmp_usec = lval2 ;
                      paqs->data_qual = translate_clock (addr(q330->qclock), subchan, wordval) ;
                      diff = dsn + tmp_sec + q330->qclock.zone ;
                      hr = (diff mod 86400) div 3600 ; /* hours */
                      if (q330->par_create.call_state)
                        then
                          begin
                            q330->state_call.context = q330 ;
                            q330->state_call.state_type = ST_TICK ;
                            q330->state_call.subtype = 0 ;
                            memcpy(addr(q330->state_call.station_name), addr(q330->station_ident), sizeof(string9)) ;
                            q330->state_call.info = diff ;
                            q330->state_call.subtype = tmp_usec ;
                            q330->par_create.call_state (addr(q330->state_call)) ;
                          end
                      r = diff ; /* make double precision */
                      t = r + tmp_usec / 1.0E6 ;
                      if (paqs->data_timetag < 1)
                        then
                          begin
                            paqs->data_timetag = t ;
                            q330->saved_data_timetag = t ;
#ifndef OMIT_SEED
                            if (paqs->log_cfg.flags and LC_START)
                              then
                                start_cfgblks (paqs) ;
#endif
                          end
                        else
                          begin
                            if ((t - paqs->data_timetag - 1) > 0.5)
                              then
                                begin
                                  v1 = lib_round(t - paqs->data_timetag - 1.0) ;
                                  add_status (q330, AC_MISSING, v1) ;
                                end
                            paqs->data_timetag = t ; /* we are running */
                            q330->saved_data_timetag = t ;
#ifndef OMIT_SEED
                            v1 = t - paqs->cfg_lastwritten + 0.5 ;
                            if ((paqs->log_cfg.flags and LC_INT) land
                                ((v1 div 60) >= paqs->log_cfg.interval))
                              then
                                start_cfgblks (paqs) ;
#endif
                          end
                      q330->share.opstat.last_data_time = lib_round(paqs->data_timetag) ;
                      if (q330->lasttime)
                        then
                          begin
                            if (abs(t - q330->lasttime - 1.0) > 0.5)
                              then
                                begin
                                  sprintf(s, "%9.6f seconds", t - q330->lasttime - 1.0) ; /* conv if needed */
                                  if (lnot seqgap_occurred)
                                    then
                                      begin
                                        libdatamsg(q330, LIBMSG_TIMEJMP, addr(s)) ;
#ifndef OMIT_SEED
                                        log_clock (q330, CE_JUMP, addr(s)) ; /* any jump that big is logged! */
#endif
                                      end
                                  update_ok = FALSE ;
                                end
                              else
                                begin
                                  diff = (t - q330->lasttime - 1.0) * 1E6 + 0.5 ;
                                  if (abs(diff) >= 10)
                                    then
                                      begin
                                        sprintf(s, "%d usec", diff) ;
                                        libdatamsg(q330, LIBMSG_TIMEJMP, addr(s)) ;
                                      end
                                  lock (q330) ;
                                  if (abs(diff) >= q330->share.global.jump_thresh)
                                    then
                                      begin
                                        sprintf(s, "%9.6f", t - q330->lasttime - 1.0) ; /* conv if needed */
                                        unlock (q330) ;
#ifndef OMIT_SEED
                                        log_clock (q330, CE_JUMP, addr(s)) ;
#endif
                                        update_ok = FALSE ;
                                      end
                                    else
                                      unlock (q330) ;
                                end
                          end
                      q330->lasttime = t ;
                      add_status (q330, AC_THROUGH, 1) ;
#ifndef OMIT_SEED
                      inc(paqs->except_count) ;
                      if ((lnot paqs->daily_done) land (hr == 0))
                        then
                          begin
                            paqs->daily_done = TRUE ;
                            update_ok = FALSE ; /* don't it again */
                            log_clock (q330, CE_DAILY, "") ;
                          end
                      else if (hr > 0)
                        then
                          paqs->daily_done = FALSE ;
                      if (update_ok)
                        then
                          if ((paqs->last_update + q330->qclock.clock_filt) < paqs->data_timetag)
                            then
                              begin
                                if ((paqs->data_qual != paqs->last_data_qual) land
                                   ((paqs->data_qual == q330->qclock.q_locked) lor (paqs->data_qual == q330->qclock.q_track) lor
                                    (paqs->data_qual == q330->qclock.q_hold) lor (paqs->data_qual == q330->qclock.q_off) lor
                                    (paqs->data_qual == q330->qclock.q_high) lor (paqs->data_qual == q330->qclock.q_low) lor
                                    (paqs->data_qual == q330->qclock.q_never) lor
                                    (paqs->last_data_qual == NO_LAST_DATA_QUAL)))
                                  then
                                    log_clock (q330, CE_VALID, "") ;
                                paqs->last_data_qual = paqs->data_qual ;
                              end
#endif
                      break ;
                  end
                  paqs->proc_lcq = paqs->dispatch[idx] ;
                  while (paqs->proc_lcq)
                    begin
                      switch (paqs->proc_lcq->raw_data_field) begin
                        case 1 :
                          diff = lval2 ;
                          if (diff >= 500000)
                            then
                              diff = diff - 1000000 ;
                          process_one (q330, diff) ;
                          break ;
                        case 2 :
                          process_one (q330, paqs->data_qual) ;
                          break ;
                        case 3 :
                          process_one (q330, wordval) ;
                          break ;
                      end
                      paqs->proc_lcq = paqs->proc_lcq->dispatch_link ;
                    end
                  break ;
                case DC_AG816 :
                  paqs->proc_lcq = paqs->dispatch[idx] ;
                  if (sub == 0)
                    then
                      begin
                        for (idx = 0 ; idx <= CHANNELS - 1 ; idx++)
                          for (sub = 0 ; sub <= FREQUENCIES - 1 ; sub++)
                            begin
                              paqs->proc_lcq = paqs->mdispatch[idx][sub] ;
                              while (paqs->proc_lcq)
                                begin
                                  paqs->proc_lcq->calstat = (wordval and (3 shl idx)) ;
                                  paqs->proc_lcq = paqs->proc_lcq->dispatch_link ;
                                end
                            end
                      end
                    else
                      while (paqs->proc_lcq)
                        begin
                          process_one (q330, wordval) ;
                          paqs->proc_lcq = paqs->proc_lcq->dispatch_link ;
                        end
                  break ;
                case DC_AG38 :
                  paqs->proc_lcq = paqs->dispatch[idx] ;
                  while (paqs->proc_lcq)
                    begin
                      switch (sub) begin
                        case 0 :
                        case 1 :
                          switch (paqs->proc_lcq->raw_data_field) begin
                            case 0 :
                              process_one (q330, bsex(subchan)) ;
                              break ;
                            case 1 :
                              process_one (q330, bsex(wordval shr 8)) ;
                              break ;
                            case 2 :
                              process_one (q330, bsex(wordval and 255)) ;
                              break ;
                          end
                          break ;
                        case 2 :
                          switch (paqs->proc_lcq->raw_data_field) begin
                            case 0 :
                              process_one (q330, bsex(subchan)) ;
                              break ;
                            case 1 :
                              process_one (q330, bsex(wordval shr 8)) ;
                              break ;
                          end
                          break ;
                      end
                      paqs->proc_lcq = paqs->proc_lcq->dispatch_link ;
                    end
                  break ;
                case DC_AG32 :
                  loadlongint (addr(p)) ;
                  break ;
                case DC_AG232 :
                  loadlongint (addr(p)) ;
                  loadlongint (addr(p)) ;
                  break ;
                case DC_CNP38 :
                  paqs->proc_lcq = paqs->dispatch[idx] ;
                  while (paqs->proc_lcq)
                    begin
                      switch (paqs->proc_lcq->raw_data_field) begin
                        case 0 :
                          process_one (q330, bsex(subchan)) ;
                          break ;
                        case 1 :
                          process_one (q330, wordval shr 8) ;
                          break ;
                        case 2 :
                          process_one (q330, wordval and 255) ;
                          break ;
                      end
                      paqs->proc_lcq = paqs->proc_lcq->dispatch_link ;
                    end
                  break ;
                case DC_CNP816 : break ;
                case DC_CNP316 :
                  wordval2 = loadword (addr(p)) ;
                  wordval3 = loadword (addr(p)) ;
                  paqs->proc_lcq = paqs->dispatch[idx] ;
                  while (paqs->proc_lcq)
                    begin
                      switch (sub) begin
                        case 0 :
                          switch (paqs->proc_lcq->raw_data_field) begin
                            case 0 :
                              process_one (q330, wordval) ;
                              break ;
                            case 1 :
                              process_one (q330, wordval2) ;
                              break ;
                            case 2 :
                              process_one (q330, sex(wordval3)) ;
                              break ;
                            case 3 :
                              process_one (q330, subchan) ;
                              break ;
                          end
                          break ;
                        case 1 :
                        case 2 :
                        case 3 :
                          switch (paqs->proc_lcq->raw_data_field) begin
                            case 0 :
                              switch (subchan and 3) begin
                                case 1 :
                                  process_one (q330, wordval) ;
                                  break ;
                                case 2 :
                                  process_one (q330, sex(wordval)) ;
                                  break ;
                              end
                              break ;
                            case 1 :
                              switch ((subchan shr 2) and 3) begin
                                case 1 :
                                  process_one (q330, wordval2) ;
                                  break ;
                                case 2 :
                                  process_one (q330, sex(wordval2)) ;
                                  break ;
                              end
                              break ;
                            case 2 :
                              switch ((subchan shr 4) and 3) begin
                                case 1 :
                                  process_one (q330, wordval3) ;
                                  break ;
                                case 2 :
                                  process_one (q330, sex(wordval3)) ;
                                  break ;
                              end
                              break ;
                          end
                          break ;
                      end
                      paqs->proc_lcq = paqs->proc_lcq->dispatch_link ;
                    end
                  break ;
                case DC_CNP232 :
                  loadlongint (addr(p)) ;
                  loadlongint (addr(p)) ;
                  break ;
                case DC_D32 : /* 1hz data */
                  lval1 = loadlongint (addr(p)) ;
                  paqs->proc_lcq = paqs->mdispatch[idx and 7][0] ;
                  while (paqs->proc_lcq)
                    begin
#ifndef OMIT_SEED
                      if (q330->lastseq == 0)
                        then
                          paqs->proc_lcq->com->charging = TRUE ;
#endif
                      process_one (q330, lval1) ;
                      paqs->proc_lcq = paqs->proc_lcq->dispatch_link ;
                    end
                  break ;
                case DC_COMP :
                  psave = p ; /* start of blockette + 4 */
                  skip = (wordval - 1) and 0xFFFC ; /* + 3 not including first 4 */
                  if ((wordval > MAXDATA) lor
                      ((wordval + (integer)pstart) > (integer)pend))
                    then
                      begin
                        v1 = wordval ;
                        sprintf(s, "%d", v1) ;
                        libdatamsg (q330, LIBMSG_INVBLKLTH, addr(s)) ;
                        return ;
                      end
                  paqs->proc_lcq = paqs->mdispatch[idx and 7][subchan and 7] ;
                  while (paqs->proc_lcq)
                    begin
#ifndef OMIT_SEED
                      if (q330->lastseq == 0)
                        then
                          paqs->proc_lcq->com->charging = TRUE ;
#endif
                      p = psave ; /* in case multiple users */
                      process_comp (q330, p, wordval) ;
                      paqs->proc_lcq = paqs->proc_lcq->dispatch_link ;
                    end
                  p = psave ;
                  incn(p, skip) ;
                  break ;
                case DC_MULT :
                  psave = p ; /* start of blockette + 4 */
                  skip = ((wordval and DMSZ) - 1) and 0xFFFC ; /* + 3 not including first 4 */
                  if (((wordval and DMSZ) > MAXDATA) lor
                     (((wordval and DMSZ) + (integer)pstart) > (integer)pend))
                    then
                      begin
                        v1 = wordval and DMSZ ;
                        sprintf(s, "%d", v1) ;
                        libdatamsg (q330, LIBMSG_INVBLKLTH, addr(s)) ;
                        return ;
                      end
                  paqs->proc_lcq = paqs->mdispatch[idx and 7][subchan and 7] ;
                  while (paqs->proc_lcq)
                    begin
#ifndef OMIT_SEED
                      if (q330->lastseq == 0)
                        then
                          paqs->proc_lcq->com->charging = TRUE ;
#endif
                      if (paqs->proc_lcq->dholdq)
                        then
                          if (paqs->proc_lcq->dholdq->ppkt)
                            then
                              begin
                                if (paqs->proc_lcq->dholdq->ppkt->seg_freq == subchan)
                                  then /* detect duplicate queued pkt */
                                    paqs->proc_lcq->dholdq->ppkt = NIL ;
                                  else
                                    begin
                                      p2 = (pointer)paqs->proc_lcq->dholdq->ppkt ; /* skip chan/subchan */
                                      process_mult (q330, p2, q330->lastseq) ;
                                    end
                              end
                      p = psave ;
                      decn(p, 4) ; /* backup to start */
                      process_mult (q330, p, q330->lastseq) ;
                      paqs->proc_lcq = paqs->proc_lcq->dispatch_link ;
                    end
                  p = psave ;
                  incn(p, skip) ;
                  break ;
                case DC_SPEC :
                  psave = p ; /* start of blockette + 4 */
                  decn(p, 4) ; /* backup to start of blockette */
                  v1 = wordval ;
                  switch (chan and not DCM) begin
                    case SP_CALSTART :
                      skip = 24 ;
#ifndef OMIT_SEED
                      log_cal (q330, p) ;
#endif
                      break ;
                    case SP_CALABORT :
                      skip = 4 ;
#ifndef OMIT_SEED
                      log_cal (q330, p) ;
#endif
                      break ;
                    case SP_CNP :
                      skip = (wordval - 1) and 0xFFFC ; /* + 3 not including first 4 */
                      if (wordval > MAXDATA)
                        then
                          begin
                            sprintf(s, "%d", v1) ;
                            libdatamsg (q330, LIBMSG_INVBLKLTH, addr(s)) ;
                            return ;
                          end
                      p2 = psave ;
                      portnum = loadbyte (addr(p2)) ; /* get CNP port number */
#ifndef OMIT_SEED
                      paqs->proc_lcq = paqs->cnp_lcqs ;
                      while (paqs->proc_lcq)
                        begin
                          if (paqs->proc_lcq->raw_data_field == portnum)
                            then
                              process_cnp (q330, p) ;
                          paqs->proc_lcq = paqs->proc_lcq->dispatch_link ;
                        end
#endif
                      break ;
                    case SP_CFGBLK :
                      skip = (wordval - 1) and 0xFFFC ; /* + 3 not including first 4 */
                      if (wordval > MAXDATA)
                        then
                          begin
                            sprintf(s, "%d", v1) ;
                            libdatamsg (q330, LIBMSG_INVBLKLTH, addr(s)) ;
                            return ;
                          end
                      p2 = psave ;
#ifndef OMIT_SEED
                      switch (subchan) begin
                        case CFG_GLOBAL :
                          add_cfg (paqs, "GL", p2, RAW_GLOBAL_SIZE, 0, 0) ;
                          break ;
                        case CFG_FIXED :
                          add_cfg (paqs, "FX", p2, RAW_FIXED_SIZE, 0, 0) ;
                          break ;
                        case CFG_AUXAD :
                          add_cfg (paqs, "XA", p2, CFGSZ_AUXAD, 0, 0) ;
                          break ;
                        case CFG_SS1 :
                          add_cfg (paqs, "S1", p2, wordval - 4, 0, 0) ;
                          break ;
                        case CFG_SS2 :
                          add_cfg (paqs, "S2", p2, wordval - 4, 0, 0) ;
                          break ;
                      end
#endif
                      break ;
                    default :
                      v1 = chan ;
                      sprintf(s, "%X", v1) ;
                      zpad(s, 2) ;
                      libmsgadd(q330, LIBMSG_INVSPEC, addr(s)) ;
                      break ;
                  end
                  if (skip)
                    then
                      begin
                        p = psave ;
                        incn(p, skip) ;
                      end
                  break ;
                default :
                  v1 = chan ;
                  sprintf(s, "%X", v1) ;
                  zpad(s, 2) ;
                  libmsgadd(q330, LIBMSG_INVBLK, addr(s)) ;
                  break ;
            end
        end
      break ;
  end
end

void dack_out (pq330 q330)
begin
  pbyte p, pref ;
  integer msglth, err, j ;
  string95 s, s1, s2 ;
  longword acks[4] ;
  boolean added ;

  q330->ack_delay = 0 ;
  q330->piggyok = TRUE ;
  p = addr(q330->dataout.qdp) ;
  incn(p, 6) ; /* point at length */
  msglth = loadword (addr(p)) + QDP_HDR_LTH ;
  if (q330->cur_verbosity and VERB_PACKET)
    then
      begin /* log the message sent */
        p = addr(q330->dataout.qdp) ;
        loadqdphdr (addr(p), addr(q330->recvhdr)) ; /* for display purposes */
        p = addr(q330->dataout.qdp) ;
        incn(p, 16) ; /* point to ack map */
        for (j = 0 ; j <= 3 ; j++)
          acks[j] = loadlongword(addr(p)) ;
        packet_time (now(), addr(s)) ;
        command_name (q330->recvhdr.command, addr(s1)) ;
        strcat (s, s1) ;
        sprintf(s1, ", Lth=%d Seq=%d Ack=%d", q330->recvhdr.datalength, q330->recvhdr.sequence,
                q330->recvhdr.acknowledge) ;
        strcat(s, s1) ;
        strcat(s, ", Acking ") ;
        added = FALSE ;
        j = 0 ;
        while (j <= 127)
          begin
            if (acks[(j shr 5) and 3] and (1 shl (longword)(j and 31)))
              then
                begin
                  added = TRUE ;
                  sprintf(s1, "%d,", q330->recvhdr.acknowledge + j) ;
                  strcat(s, s1) ;
                end
            if (strlen(s) >= 88)
              then
                begin
                  libmsgadd(q330, LIBMSG_PKTOUT, addr(s)) ;
                  added = FALSE ;
                  sprintf(s, "%s%s, Acking ", packet_time (now(), addr(s2)), command_name (q330->recvhdr.command, addr(s1))) ;
                end
            inc(j) ;
          end
        if (added)
          then
            libmsgadd(q330, LIBMSG_PKTOUT, addr(s)) ;
      end
  if (q330->usesock)
    then
      begin
        if (q330->tcp)
          then
            begin
              if (q330->cpath == INVALID_SOCKET)
                then
                  return ;
              p = (pointer)((integer)addr(q330->dataout.qdp) - 4) ;
              pref = p ; /* save start of tcp packet */
              storeword (addr(p), 1) ; /* data port */
              storeword (addr(p), msglth) ; /* qdp length */
              err = send(q330->cpath, (pchar)pref, msglth + 4, 0) ;
            end
          else
            begin
              if (q330->dpath == INVALID_SOCKET)
                then
                  return ;
              err = sendto(q330->dpath, addr(q330->dataout.qdp), msglth, 0, addr(q330->dsockout), sizeof(struct sockaddr)) ;
            end
        if (err == SOCKET_ERROR)
          then
            begin
              err =
#ifdef X86_WIN32
                     WSAGetLastError() ;
#else
                     errno ;
#endif
              if ((q330->tcp) land (err != EWOULDBLOCK))
                then
                  begin
                    sprintf(s1, "%d, Waiting 10 minutes", err) ;
                    tcp_error (q330, addr(s1)) ;
                  end
                else
                  begin
                    sprintf(s1, "%d", err) ;
                    libmsgadd(q330, LIBMSG_CANTSEND, addr(s1)) ;
                    add_status (q330, AC_IOERR, 1) ; /* add one I/O error */
                  end
            end
          else
            add_status (q330, AC_WRITE, msglth + IP_HDR_LTH + UDP_HDR_LTH) ;
      end
#ifndef OMIT_SERIAL
    else
      begin
        memcpy(addr(q330->commands.cmsgout.qdp), addr(q330->dataout.qdp), msglth) ;
        send_packet (q330, msglth, q330->q330dport, q330->dataport) ;
      end
#endif
end

static void send_dack (pq330 q330)
begin
  integer i, j, k ;
  word lowseq, idx ;
  tdp_ack pack ;
  pbyte p, pref, psave ;
  integer lth, msglth ;
  ppkt_buf pbuf ;

  q330->ack_timeout = 0 ;
  repeat
    pbuf = q330->pkt_bufs[q330->last_packet and 255] ;
    if (pbuf->valid)
      then
        begin
          proc_insequence (q330, q330->last_packet and 255) ;
          pbuf->valid = FALSE ;
          inc(q330->last_packet) ;
        end
      else
        break ;
  until (q330->libstate != LIBSTATE_RUN)) ;
  lowseq = q330->last_packet - 1 ;
  memset (addr(pack), 0, sizeof(tdp_ack)) ;
  pack.acks[0] = 1 ; /* last_packet - 1 */
  idx = q330->last_packet ;
  for (i = 1 ; i <= WINBUFS - 1 ; i++)
    begin
      j = (i shr 5) and 3 ;
      k = i and 31 ;
      pbuf = q330->pkt_bufs[idx and 255] ;
      if (pbuf->valid)
        then
          pack.acks[j] = pack.acks[j] or (1 shl (longint)k) ; /*add those in queue*/
      inc(idx) ;
    end
  p = addr(q330->dataout.qdp) ;
  psave = p ;
  storeqdphdr (addr(p), DT_DACK, 0, 0, lowseq) ;
  pref = p ; /* pointer after header for length calculation */
  storedack (addr(p), addr(pack)) ;
  lth = (integer)p - (integer)pref ; /* length of data */
  p = psave ;
  incn(p, 6) ; /* point at length */
  storeword (addr(p), lth) ;
  msglth = QDP_HDR_LTH + lth ;
  p = psave ;
  storelongint (addr(p), gcrccalc (addr(q330->crc_table), (pointer)((integer)p + 4), msglth - 4)) ;
  inc(q330->ack_counter) ;
  lock (q330) ;
  if (q330->ack_counter < q330->share.log.ack_cnt)
    then
      begin
        if (q330->ack_delay == 0)
          then
            q330->ack_delay = q330->share.log.ack_to ;
        unlock (q330) ;
        return ;
      end
  unlock (q330) ;
  q330->ack_counter = 0 ;
  dack_out (q330) ;
end

void process_data (pq330 q330)
begin
  word hw ;
  boolean good ;
  integer i, j, k ;
  string95 s, s1 ;
  ppkt_buf pbuf ;
  pbyte p ;
  longword dsn ;

  if (lnot q330->link_recv)
    then
      return ;
  if (q330->libstate != LIBSTATE_RUN)
    then
      return ;
  add_status (q330, AC_PACKETS, 1) ;
  hw = q330->last_packet + WINWRAP ;
  good = q330->recvhdr.sequence >= q330->last_packet ;
  if (hw > q330->last_packet)
    then
      good = (good) land (q330->recvhdr.sequence <= hw) ;
    else
      good = (good) lor (q330->recvhdr.sequence <= hw) ;
  if (q330->cur_verbosity and VERB_PACKET)
    then
      begin /* log the message received */
        packet_time (now(), addr(s)) ;
        command_name (q330->recvhdr.command, addr(s1)) ;
        strcat (s, s1) ;
        p = (pointer) ((integer)(addr(q330->datain.qdp)) + QDP_HDR_LTH) ;
        dsn = loadlongword(addr(p)) ;
        sprintf(s1, ", Lth=%d Seq=%d DSN=%d", q330->recvhdr.datalength, q330->recvhdr.sequence,
                dsn) ;
        strcat(s, s1) ;
        if (good)
          then
            strcat(s, " Inside the Window") ;
          else
            strcat(s, " Outside the Window") ;
        libmsgadd(q330, LIBMSG_PKTIN, addr(s)) ;
      end
  if (good)
    then
      begin
        pbuf = q330->pkt_bufs[q330->recvhdr.sequence and 255] ;
        pbuf->valid = TRUE ;
        memcpy (addr(pbuf->buf.qdp), addr(q330->datain.qdp), q330->recvhdr.datalength + QDP_HDR_LTH) ;
        lock (q330) ;
        memset (addr(q330->share.slidestat), 0, sizeof(tslidestat)) ;
        q330->share.slidestat.low_seq = q330->last_packet - 1 ;
        q330->share.slidestat.latest = q330->recvhdr.sequence ;
        for (i = 0 ; i <= 255 ; i++)
          begin
            j = (i shr 5) and 7 ;
            k = i and 31 ;
            pbuf = q330->pkt_bufs[i] ;
            if (pbuf->valid)
              then
                q330->share.slidestat.validmap[j] = q330->share.slidestat.validmap[j] or (1 shl (longword)k) ;
          end
        unlock (q330) ;
      end
    else
      add_status (q330, AC_SEQERR, 1) ;
  send_dack (q330) ;
end
