/*   Lib330 time series configuration routines
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
    0 2006-09-30 rdr Created
    1 2006-10-26 rdr Fix mangled filter cutoff descriptions in verbose mode.
    2 2007-09-07 rdr Add call to print_generated_rectotals.
    3 2008-01-10 rdr LOG LCQ moved to DP. Make number of client message buffers host
                     programmable. DPLCQ buffers come out of thrbuf instead of using
                     getmem. Remove deallocate_dplcqs.
    4 2008-03-18 rdr Add setting scd_evt and scd_cont based on lcq options.
                     Add set_gaps to setup both the gap_secs and new gap_offset. Gap_offset
                     needs to be overridden for decimated channels based on the rate of
                     the source channel.
    5 2008-04-03 rdr If opt_compat is set then force netserv to event if archive is event.
    6 2008-04-26 rdr Setup scd_cont for DP LCQs.
    7 2008-06-19 rdr Initialize log name in allocate_aqstruc.
    8 2009-02-09 rdr Add EP support.
    9 2009-03-11 rdr Protect against divide by zero errors when using a variable rate
                     channel as the source for a decimated channel, these values will be
                     calculated once the source rate is known.
                     Change EP delays to 20us increments from 5us. Add filter delay dump
                     for EP channels.
   10 2009-04-18 rdr Fix filter delay string for EP channels.
   11 2009-05-16 rdr Don't worry about delays for EP channels that are disabled. If tokens
                     changed while running don't automatically check EP delays unless there
                     is no update within 2 minutes. Normally Willard updates EP configuration
                     after sending tokens.
   12 2009-07-28 rdr Add DSS support.
   13 2009-09-05 rdr Change update_ep_delays to show delay values when it is different from
                     previous value (which is normally zero) or the show flag is on. Don't
                     get an update unless the udpate flag is set.
   14 2009-09-07 rdr Fix recursive mutex locking in verify_mapping.
   15 2010-03-27 rdr Q335 support added.
   16 2011-03-17 rdr Setup new gain_bits in LCQ init for deb_flags usage.
*/
#ifndef libsampcfg_h
#include "libsampcfg.h"
#endif
#ifndef libmsgs_h
#include "libmsgs.h"
#endif
#ifndef libslider_h
#include "libslider.h"
#endif
#ifndef libseed_h
#include "libseed.h"
#endif
#ifndef libcmds_h
#include "libcmds.h"
#endif
#ifndef q330types_h
#include "q330types.h"
#endif
#ifndef libcont_h
#include "libcont.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif

#ifndef OMIT_SEED
#ifndef libfilters_h
#include "libfilters.h"
#endif
#ifndef libarchive_h
#include "libarchive.h"
#endif
#ifndef liblogs_h
#include "liblogs.h"
#endif
#ifndef libsample_h
#include "libsample.h"
#endif
#ifndef libverbose_h
#include "libverbose.h"
#endif
#ifndef libdss_h
#include "libdss.h"
#endif
#endif

#define EP_UPDATE_TIME 120 /* 2 minutes */

longword secsince (void)
begin

  return lib_round(now()) ;
end

void clear_sg (paqstruc paqs)
begin

  memset (addr(paqs->first_sg), 0, (pntrint)addr(paqs->last_sg) - (pntrint)addr(paqs->first_sg)) ;
  string2fixed (addr(paqs->log_tim.tim_location), "  ") ;
  string2fixed (addr(paqs->log_tim.tim_seedname), "ACE") ;
#ifndef OMIT_SEED
  paqs->cnp_lcqs = NIL ;
#endif
  memset (addr(paqs->dispatch), 0, sizeof(tdispatch)) ;
  memset (addr(paqs->mdispatch), 0, sizeof(tmdispatch)) ;
  memset (addr(paqs->epdispatch), 0, sizeof(tepdispatch)) ;
end

pointer allocate_aqstruc (tcontext ownedby)
begin
  paqstruc paqs ;
  pq330 q330 ;
#ifndef OMIT_SEED
  integer n, msgcnt ;
  pmsgqueue msgq ;
#endif

  q330 = ownedby ;
  getthrbuf (q330, addr(paqs), sizeof(taqstruc)) ;
  paqs->owner = ownedby ;
  clear_sg (paqs) ;
#ifndef OMIT_SEED
  getthrbuf (q330, addr(msgq), sizeof(tmsgqueue)) ;
  paqs->msgqueue = msgq ;
  paqs->msgq_in = msgq ;
  paqs->msgq_out = msgq ;
  msgcnt = q330->par_create.opt_client_msgs ;
  if (msgcnt < MIN_MSG_QUEUE_SIZE)
    then
      msgcnt = MIN_MSG_QUEUE_SIZE ;
  for (n = 2 ; n <= msgcnt ; n++)
    begin
      getthrbuf (q330, addr(msgq->link), sizeof(tmsgqueue)) ;
      msgq = msgq->link ;
    end
  msgq->link = paqs->msgqueue ;
  n = q330->par_create.amini_exponent ;
  if (n >= 9)
    then
      begin
        paqs->arc_size = 512 ;
        while (n > 9)
          begin
            paqs->arc_size = paqs->arc_size shl 1 ;
            dec(n) ;
          end
      end
    else
      paqs->arc_size = 0 ; /* defeat */
  paqs->arc_frames = paqs->arc_size div FRAME_SIZE ;
  string2fixed (addr(paqs->log_tim.log_location), "  ") ;
  string2fixed (addr(paqs->log_tim.log_seedname), "LOG") ;
#endif
  return paqs ;
end

char *realtostr (double r, integer digits, string31 *result)
begin
  string15 fmt ;

  sprintf(fmt, "%%%d.%df", digits + 2, digits) ;
  sprintf(result, fmt, r) ;
  return result ;
end

static char *scvrate (integer rate, string *result)
begin

  if (rate >= 0)
    then
      sprintf(result, "%d", rate) ;
    else
      realtostr(-1.0 / rate, 4, result) ;
  return result ;
end

static char *sfcorner (pq330 q330, integer chan, string *result)
begin
  word filt ;

  lock (q330) ;
  filt = (q330->share.global.filter_map shr ((chan div 3) * 2)) and 0x3 ;
  unlock (q330) ;
  switch (filt) begin
    case 0 :
      strcpy(result, "Linear all") ;
      break ;
    case 1 :
      strcpy(result, "Linear below 100sps") ;
      break ;
    case 2 :
      strcpy(result, "Linear below 40sps") ;
      break ;
    case 3 :
      strcpy(result, "Linear below 20sps") ;
      break ;
  end
  return result ;
end

void set_gaps (plcq q)
begin

  if (q->rate > 0)
    then
      begin
        q->gap_secs = q->gap_threshold / q->rate ;
        q->gap_offset = 1.0 ; /* default is one set of data points per second */
      end
    else
      begin
        q->gap_secs = q->gap_threshold * fabs(q->rate) ;
        q->gap_offset = fabs(q->rate) ; /* default is one data point per "rate" seconds */
      end
end

void update_ep_delays (pq330 q330, boolean show, boolean update)
begin
  plcq pchan ;
  paqstruc paqs ;
  integer i ;
  boolean have_all, found ;
  string63 s ;
  string15 s1, s2, s4 ;
  tfloat last_delay ;

  paqs = q330->aqstruc ;
  if (paqs == NIL)
    then
      return ;
  q330->update_ep_timer = 0 ;
  have_all = TRUE ;
  pchan = paqs->lcqs ;
  while (pchan)
    begin
      if ((pchan->raw_data_source == (DC_SPEC + 4)) land ((pchan->lcq_opt and LO_DOFF) == 0))
        then
          begin
            found = FALSE ;
            for (i = 0 ; i < q330->share.epdelay.chancnt ; i++)
              if (pchan->raw_data_field == (q330->share.epdelay.chandlys[i] shr 24))
                then
                  begin
                    last_delay = pchan->delay ;
                    pchan->delay = (q330->share.epdelay.chandlys[i] and 0xFFFFFF) * 20.0E-6 ;
                    found = TRUE ;
                    if (pchan->delay != last_delay)
                      then
                        begin
                          if (last_delay != 0.0)
                            then
                              flush_lcq (paqs, pchan, pchan->com) ;
                          if ((show) land (q330->cur_verbosity and VERB_LOGEXTRA))
                            then
                              begin
                                sprintf(s, "%s@%s=%s", seed2string(addr(pchan->location), addr(pchan->seedname), addr(s1)),
                                        scvrate(pchan->rate, addr(s2)), realtostr(pchan->delay, 6, addr(s4))) ;
                                libmsgadd (q330, LIBMSG_FILTDLY, addr(s)) ;
                              end
                        end
                    break ;
                  end
            if (lnot found)
              then
                have_all = FALSE ;
          end
      pchan = pchan->link ;
    end
  if ((update) land (lnot have_all))
    then
      new_cmd (q330, C2_RQEPCFG, sizeof(tepcfg)) ;
end

void verify_epcfg (pq330 q330)
begin
  plcq pchan ;
  paqstruc paqs ;
  integer i ;
  boolean changes, found ;
  byte mask ;

  paqs = q330->aqstruc ;
  if (paqs == NIL)
    then
      return ;
  changes = FALSE ;
  memcpy (addr(q330->share.newepcfg), addr(q330->share.epcfg), sizeof(tepcfg)) ;
  mask = 1 shl q330->par_create.q330id_dataport ;
  pchan = paqs->lcqs ;
  while (pchan)
    begin
      if ((pchan->raw_data_source == (DC_SPEC + 4)) land ((pchan->lcq_opt and LO_DOFF) == 0))
        then
          begin
            found = FALSE ;
            for (i = 0 ; i < q330->share.newepcfg.chancnt ; i++)
              if (pchan->raw_data_field == q330->share.newepcfg.chanmasks[i].chan)
                then
                  begin
                    found = TRUE ;
                    if ((q330->share.newepcfg.chanmasks[i].mask and mask) == 0)
                      then
                        begin
                          changes = TRUE ;
                          q330->share.newepcfg.chanmasks[i].mask = q330->share.newepcfg.chanmasks[i].mask or mask ;
                        end
                    break ;
                  end
            if (lnot found)
              then
                if (q330->share.newepcfg.chancnt < EP_MAXCHAN)
                  then
                    begin
                      changes = TRUE ;
                      q330->share.newepcfg.chanmasks[q330->share.newepcfg.chancnt].chan = pchan->raw_data_field ;
                      q330->share.newepcfg.chanmasks[q330->share.newepcfg.chancnt].mask = mask ;
                      inc(q330->share.newepcfg.chancnt) ;
                    end
          end
      pchan = pchan->link ;
    end
  if (changes)
    then
      new_cmd (q330, C2_SEPCFG, sizeof(tepcfg)) ;
end

void set_gain_bits (pq330 q330, plcq q, byte *gb)
begin
  word w, chan ;

  chan = q->raw_data_source and not DCM ; /* get channel */
  if (chan >= 3)
    then
      w = (q330->share.global.input_map shr (2 + (chan shl 1))) and 3 ;
    else
      w = (q330->share.global.input_map shr (chan shl 1)) and 3 ;
  if (((q330->share.global.gain_map shr (chan shl 1)) and 3) == GAIN_PON)
    then
      w = w or DEB_LOWV ;
  *gb = (byte)w ;
end


void init_lcq (paqstruc paqs)
begin
  plcq p, pl ;
  integer i, j ;
  pq330 q330 ;
  string s ;
  string15 s1, s2, s4 ;
  string31 s3 ;
#ifndef OMIT_SEED
  pcompressed_buffer_ring pr, lastpr ;
  integer buffers ;
#endif

  pl = paqs->lcqs ;
  q330 = paqs->owner ;
  while (pl)
    begin
      pl->dtsequence = 0 ;
      if (pl->rate > 0)
        then
          pl->datasize = pl->rate * sizeof(longint) ;
        else
          pl->datasize = sizeof(longint) ;
      getbuf (q330, (pointer *)addr(pl->databuf), pl->datasize) ;
      if (pl->rate > 1)
        then
          getbuf (q330, (pointer *)addr(pl->idxbuf), (pl->rate + 1) * sizeof(word)) ;
      switch (pl->rate) begin
        case 100 :
          pl->segsize = SS_100 ;
          break ;
        case 200 :
          pl->segsize = SS_200 ;
          break ;
        case 250 :
          pl->segsize = SS_250 ;
          break ;
        case 500 :
          pl->segsize = SS_500 ;
          break ;
        case 1000 :
          pl->segsize = SS_1000 ;
          break ;
        default :
          pl->segsize = 0 ;
          break ;
      end
      if (q330->par_create.call_secdata)
        then
          begin
            pl->onesec_filter = q330->par_create.opt_secfilter and OSF_ALL ;
            if ((q330->par_create.opt_secfilter and OSF_DATASERV) land (pl->lcq_opt and LO_DATAS))
              then
                pl->onesec_filter = pl->onesec_filter or OSF_DATASERV ;
            if ((q330->par_create.opt_secfilter and OSF_1HZ) land (pl->rate == 1) land
               (pl->raw_data_source >= DC_D32) land (pl->raw_data_source <= DC_D32 + 5))
              then
                pl->onesec_filter = pl->onesec_filter or OSF_1HZ ;
            if ((q330->par_create.opt_secfilter and OSF_EP) land (pl->rate == 1) land
               (pl->raw_data_source == (DC_SPEC or 4)))
              then
                pl->onesec_filter = pl->onesec_filter or OSF_EP ;
          end
#ifndef OMIT_SEED
      if (q330->par_create.call_minidata)
        then
          begin
            pl->mini_filter = q330->par_create.opt_minifilter and OMF_ALL ;
            if ((q330->par_create.opt_minifilter and OMF_NETSERV) land (pl->lcq_opt and LO_NETS))
              then
                pl->mini_filter = pl->mini_filter or OMF_NETSERV ;
          end
      if ((paqs->arc_size > 0) land (q330->par_create.call_aminidata))
        then
          begin
            pl->arc.amini_filter = q330->par_create.opt_aminifilter and OMF_ALL ;
            pl->arc.incremental = (pl->rate <= q330->par_create.amini_512highest) ;
          end
#endif
      pl->dholdq = NIL ;
      if (pl->segsize)
        then
          begin
            getbuf (q330, addr(pl->segbuf), pl->segsize) ;
            pl->seg_next = pl->segbuf ;
            pl->seg_seq = 0xFFFFFFFF ;
            getbuf (q330, addr(pl->dholdq), sizeof(dholdqtype)) ;
            pl->dholdq->ppkt = NIL ;
            getbuf (q330, (pointer *)addr(pl->mergedbuf), pl->segsize) ;
          end
      pl->pack_class = PKC_DATA ; /* assume data */
      if (pl->raw_data_source and 0x80)
        then
          begin
            if (pl->raw_data_source < DC_D32)
              then
                begin
                  i = pl->raw_data_source and 127 ;
                  if (paqs->dispatch[i] == NIL)
                    then
                      paqs->dispatch[i] = pl ;
                    else
                      begin
                        p = paqs->dispatch[i] ;
                        while (p->dispatch_link)
                          p = p->dispatch_link ;
                        p->dispatch_link = pl ;
                      end
                end
            else if ((pl->raw_data_source and DCM) == DC_D32)
              then
                begin
                  i = pl->raw_data_source and not DCM ; /* get channel */
                  j = pl->raw_data_field ; /* frequency bit */
                  if (paqs->mdispatch[i][j] == NIL)
                    then
                      paqs->mdispatch[i][j] = pl ;
                    else
                      begin
                        p = paqs->mdispatch[i][j] ;
                        while (p->dispatch_link)
                          p = p->dispatch_link ;
                        p->dispatch_link = pl ;
                      end
                  set_gain_bits (q330, pl, addr(pl->gain_bits)) ;
                  lock (q330) ;
                  if (i <= 2)
                    then
                      pl->delay = q330->share.fixed.ch13_delay[7 - j] * 1.0E-6 ;
                    else
                      pl->delay = q330->share.fixed.ch46_delay[7 - j] * 1.0E-6 ;
                  unlock (q330) ;
                  if (q330->cur_verbosity and VERB_LOGEXTRA)
                    then
                      begin
                         ;
                        sprintf(s, "%s:%d@%s,%s=%s", seed2string(addr(pl->location), addr(pl->seedname), addr(s1)),
                                i + 1, scvrate(pl->rate, addr(s2)),
                                sfcorner(q330, i, addr(s3)), realtostr(pl->delay, 6, addr(s4))) ;
                        libmsgadd (q330, LIBMSG_FILTDLY, addr(s)) ;
                      end
                end
            else if (pl->raw_data_source == (DC_SPEC or 2))
              then
                begin
                  pl->pack_class = PKC_OPAQUE ;
#ifndef OMIT_SEED
                  pl->com->blockette_index = 56 ; /* header plus blockette 1000 */
                  if (paqs->cnp_lcqs == NIL)
                    then
                      paqs->cnp_lcqs = pl ;
                    else
                      begin
                        p = paqs->cnp_lcqs ;
                        while (p->dispatch_link)
                          p = p->dispatch_link ;
                        p->dispatch_link = pl ;
                      end
#endif
                end
            else if (pl->raw_data_source == (DC_SPEC or 4))
              then
                begin
                  i = pl->raw_data_field ;
                  if (paqs->epdispatch[i] == NIL)
                    then
                      paqs->epdispatch[i] = pl ;
                    else
                      begin
                        p = paqs->epdispatch[i] ;
                        while (p->dispatch_link)
                          p = p->dispatch_link ;
                        p->dispatch_link = pl ;
                      end
                end
          end
      else if (pl->raw_data_source == READ_PREV_STREAM)
        then
          begin
#ifndef OMIT_SEED
            if (pl->prev_link->rate > 0)
              then
                pl->input_sample_rate = pl->prev_link->rate ;
            else if (pl->prev_link->rate < 0)
              then
                pl->input_sample_rate = 1.0 / abs(pl->prev_link->rate) ;
              else
                pl->input_sample_rate = 0.0 ;
            if (pl->input_sample_rate >= 0.999)
              then
                pl->gap_offset = 1.0 ;
            else if (pl->input_sample_rate != 0)
              then
                pl->gap_offset = 1.0 / pl->input_sample_rate ; /* set new gap offset based on input rate */
              else
                pl->gap_offset = 1.0 ;
            if ((pl->source_fir) land (pl->input_sample_rate != 0))
              then
                pl->delay = pl->prev_link->delay + (pl->source_fir->dly / pl->input_sample_rate) ;
              else
                pl->delay = pl->prev_link->delay ;
            if (q330->cur_verbosity and VERB_LOGEXTRA)
              then
                begin
                  sprintf(s, "%s:%s@%s=%s", seed2string(addr(pl->location), addr(pl->seedname), addr(s1)),
                          seed2string(addr(pl->prev_link->location), addr(pl->prev_link->seedname), addr(s2)),
                          scvrate(pl->rate, addr(s3)), realtostr(pl->delay, 6, addr(s4))) ;
                  libmsgadd (q330, LIBMSG_FILTDLY, addr(s)) ;
                end
            pl->com->charging = TRUE ;
            p = pl->prev_link ;
            while (p->prev_link)
              p = p->prev_link ;
            /* see if root source is 1hz */
            if ((p) land (p->rate == 1))
              then
                begin
                  /* yes, need to synchronize based on rate */
                  pl->slipping = TRUE ;
                  pl->slip_modulus = abs(pl->rate) ; /* .1hz has modulus of 10 */
                end
            /* see if root source is main digitizer */
            if ((p) land ((p->raw_data_source and DCM) == DC_D32))
              then
                set_gain_bits (q330, p, addr(pl->gain_bits)) ;
#endif
          end
      else if ((pl->raw_data_source >= MESSAGE_STREAM) land (pl->raw_data_source <= CFG_STREAM))
        then
          begin
#ifndef OMIT_SEED
            pl->com->blockette_index = 56 ; /* header plus blockette 1000 */
#endif
            switch (pl->raw_data_source) begin
              case TIMING_STREAM :
                pl->pack_class = PKC_TIMING ;
                break ;
              case CFG_STREAM :
                pl->pack_class = PKC_OPAQUE ;
                break ;
            end
          end
#ifndef OMIT_SEED
      if (pl->com->maxframes >= FRAMES_PER_RECORD)
        then
          pl->com->maxframes = FRAMES_PER_RECORD - 1 ;
      buffers = pl->pre_event_buffers + 1 ; /* need one for construction */
      pr = NIL ;
      lastpr = NIL ;
      while (buffers > 0)
        begin
          getbuf (q330, addr(pr), sizeof(tcompressed_buffer_ring)) ;
          pr->link = NIL ;
          pr->full = FALSE ;
          if (pl->com->ring == NIL)
            then
              pl->com->ring = pr ;
          else if (lastpr)
            then
              lastpr->link = pr ;
          lastpr = pr ;
          dec(buffers) ;
        end
      if (pr)
        then
          pr->link = pl->com->ring ;
      if (pl->arc.amini_filter)
        then
          getbuf (q330, (pointer *)addr(pl->arc.pcfr), paqs->arc_size) ;
      if (pl->lcq_opt and LO_EVENT)
        then
          begin
            if ((q330->par_create.opt_compat) lor (pl->lcq_opt and LO_NSEVT))
              then
                pl->scd_evt = SCD_BOTH ; /* both outputs are event */
              else
                begin /* archive is event but 512 is continuous */
                  pl->scd_evt = SCD_ARCH ;
                  pl->scd_cont = SCD_512 ;
                end
          end
        else
          begin
            if (pl->lcq_opt and LO_NSEVT)
              then
                begin /* acrhive is continuous but 512 is event */
                  pl->scd_evt = SCD_512 ;
                  pl->scd_cont = SCD_ARCH ;
                end
              else
                pl->scd_cont = SCD_BOTH ;/* both are continuous */
          end
      allocate_lcq_filters (paqs, pl) ;
#endif
      pl = pl->link ;
    end
  if (q330->share.fixed.flags and FF_EP)
    then
      begin
        if (q330->share.liberr == LIBERR_TOKENS_CHANGE)
          then
            begin
              q330->update_ep_timer = EP_UPDATE_TIME ;
              update_ep_delays (q330, TRUE, FALSE) ;
            end
          else
            update_ep_delays (q330, TRUE, TRUE) ;
      end
end

void init_dplcq (paqstruc paqs, plcq pl, boolean newone)
begin
#ifndef OMIT_SEED
  pcompressed_buffer_ring pr ;
#endif
  pq330 q330 ;

  q330 = paqs->owner ;
  pl->dtsequence = 0 ;
  if (pl->raw_data_source != MESSAGE_STREAM)
    then
      begin
        if ((enum tacctype)pl->raw_data_field <= AC_LAST)
          then
            q330->share.accmstats[(enum tacctype)pl->raw_data_field].ds_lcq = pl ;
        else if (pl->raw_data_field == AC_DATA_LATENCY)
          then
            paqs->data_latency_lcq = pl ;
        else if (pl->raw_data_field == AC_STATUS_LATENCY)
          then
            paqs->status_latency_lcq = pl ;
      end
  if (q330->par_create.call_secdata)
    then
      begin
        pl->onesec_filter = q330->par_create.opt_secfilter and OSF_ALL ;
        if ((q330->par_create.opt_secfilter and OSF_DATASERV) land (pl->lcq_opt and LO_DATAS))
          then
            pl->onesec_filter = pl->onesec_filter or OSF_DATASERV ;
      end
#ifndef OMIT_SEED
  if (q330->par_create.call_minidata)
    then
      begin
        pl->mini_filter = q330->par_create.opt_minifilter and OMF_ALL ;
        if ((q330->par_create.opt_minifilter and OMF_NETSERV) land (pl->lcq_opt and LO_NETS))
          then
            pl->mini_filter = pl->mini_filter or OMF_NETSERV ;
      end
  if ((paqs->arc_size > 0) land (q330->par_create.call_aminidata))
    then
      begin
        pl->arc.amini_filter = q330->par_create.opt_aminifilter and OMF_ALL ;
        pl->arc.incremental = (pl->rate <= q330->par_create.amini_512highest) ;
      end
#endif
  pl->dholdq = NIL ;
  if (pl->raw_data_source == MESSAGE_STREAM)
    then
      pl->pack_class = PKC_MESSAGE ;
    else
      pl->pack_class = PKC_DATA ;
#ifndef OMIT_SEED
  pl->scd_cont = SCD_BOTH ; /* both are continuous */
  if (pl->com->maxframes >= FRAMES_PER_RECORD)
    then
      pl->com->maxframes = FRAMES_PER_RECORD - 1 ;
  if (lnot newone)
    then
      return ; /* don't need to allocate new buffers */
  getthrbuf (q330, addr(pr), sizeof(tcompressed_buffer_ring)) ;
  pr->full = FALSE ;
  pl->com->ring = pr ;
  pr->link = pl->com->ring ; /* just keeps going back to itself */
  if (pl->arc.amini_filter)
    then
      getthrbuf (q330, (pointer *)addr(pl->arc.pcfr), paqs->arc_size) ;
#endif
end

void init_dplcqs (paqstruc paqs)
begin
  plcq pl ;
  pq330 q330 ;

  q330 = paqs->owner ;
  pl = paqs->dplcqs ;
  while (pl)
    begin
      init_dplcq (paqs, pl, TRUE) ;
      pl = pl->link ;
    end
#ifndef OMIT_SEED
  if (paqs->arc_size > 0)
    then
      preload_archive (q330, FALSE, NIL) ;
#endif
end

void deallocate_sg (paqstruc paqs)
begin
  pmem_manager pm ;
  integer mem ;
  longword totrec ;
  pq330 q330 ;
  string s ;

  q330 = paqs->owner ;
#ifndef OMIT_SEED
  if (q330->need_sats)
    then
      finish_log_clock (q330) ;
  if (q330->cur_verbosity and VERB_LOGEXTRA)
    then
      begin
        flush_lcqs (paqs) ;
        flush_dplcqs (q330) ;
        totrec = print_generated_rectotals (q330) ;
        sprintf(s, "Done: %d recs. seq end: %d", totrec, paqs->dt_data_sequence) ;
        libdatamsg (q330, LIBMSG_TOTAL, addr(s)) ;
      end
  flush_lcqs (paqs) ;
  if (q330->dssstruc)
    then
      lib_dss_stop (q330->dssstruc) ;
#endif
  mem = 0 ;
  pm = q330->memory_head ;
  while (pm)
    begin
      mem = mem + pm->sofar ;
      pm = pm->next ;
    end
  mem = (mem + 0xFFFF) and 0xFFFF0000 ; /* lib_round up to nearest 64KB */
  q330->cur_memory_required = mem ;
  save_continuity (q330) ;
/* totrec = print_actual_rectotals ;
         newuser.msg = inttostr(totrec) + ' recs. seq end: ' +
              inttostr(dt_data_sequence) ; */
  clear_sg (paqs) ;
  mem_release (q330) ; /* release all that memory used by LCQ's */
end

void verify_mapping (pq330 q330)
begin
  tfreqs newfreq ;
  plcq pchan ;
  boolean diff ;
  integer i ;
  paqstruc paqs ;

  paqs = q330->aqstruc ;
  memset (addr(newfreq), 0, sizeof(tfreqs)) ;
  pchan = paqs->lcqs ;
  while (pchan)
    begin
      if (((pchan->raw_data_source and DCM) == DC_D32) land ((pchan->lcq_opt and LO_DOFF) == 0))
        then
          newfreq[pchan->raw_data_source and 7] = newfreq[pchan->raw_data_source and 7] or (1 shl pchan->raw_data_field) ;
      pchan = pchan->link ;
    end
  diff = FALSE ;
  lock (q330) ;
  for (i = 0 ; i <= CHANNELS - 1 ; i++)
    begin
      if (q330->share.log.freqs[i] != newfreq[i])
        then
          diff = TRUE ;
    end
  if (diff)
    then
      begin
        memcpy(addr(q330->share.newlog), addr(q330->share.log), sizeof(tlog)) ;
        memcpy(addr(q330->share.newlog.freqs), addr(newfreq), sizeof(tfreqs)) ;
        q330->share.newlog.flags = q330->share.newlog.flags or LNKFLG_SAVE ;
        unlock (q330) ;
        new_cmd (q330, C1_SLOG, 0) ;
        return ;
      end
  unlock (q330) ;
end

#ifndef OMIT_SEED
enum tliberr lib_lcqstat (pq330 q330, tlcqstat *lcqstat)
begin
  paqstruc paqs ;
  plcq q ;
  longword cur ;
  integer pass ;
  tonelcqstat *pone ;

  paqs = q330->aqstruc ;
  lcqstat->count = 0 ;
  cur = secsince() ;
  for (pass = 1 ; pass <= 2 ; pass++)
    begin
      if (pass == 1)
        then
          q = paqs->lcqs ;
        else
          q = paqs->dplcqs ;
      while ((lcqstat->count < MAX_LCQ) land (q))
        begin
          pone = addr(lcqstat->entries[lcqstat->count]) ;
          strcpy(addr(pone->location), addr(q->slocation)) ;
          strcpy(addr(pone->channel), addr(q->sseedname)) ;
          pone->chan_number = q->lcq_num ;
          pone->rec_cnt = q->records_generated_session ;
          pone->rec_seq = q->com->records_written ;
          if (q->last_record_generated == 0)
            then
              pone->rec_age = -1 ; /* not written */
            else
              pone->rec_age = cur - q->last_record_generated ;
          pone->det_count = q->detections_session ;
          pone->cal_count = q->calibrations_session ;
          pone->arec_cnt = q->arc.records_written_session ;
          pone->arec_over = q->arc.records_overwritten_session ;
          if (q->arc.last_updated == 0)
            then
              pone->arec_age = -1 ;
            else
              pone->arec_age = cur - q->arc.last_updated ;
          pone->arec_seq = q->arc.records_written ;
          inc(lcqstat->count) ;
          q = q->link ;
        end
    end
  return LIBERR_NOERR ;
end
#endif

enum tliberr lib_commevents (pq330 q330, tcommevents *commevents)
begin
  paqstruc paqs ;

  paqs = q330->aqstruc ;
  if (q330->libstate != LIBSTATE_RUN)
    then
      return LIBERR_NOSTAT ;
  memcpy(commevents, addr(paqs->commevents), sizeof(tcommevents)) ;
  return LIBERR_NOERR ;
end

void clear_calstat (pq330 q330)
begin
  paqstruc paqs ;
  plcq pl ;

  paqs = q330->aqstruc ;
  if (q330->libstate != LIBSTATE_RUN)
    then
      return ;
  pl = paqs->lcqs ;
  while (pl)
    begin
      pl->calstat = FALSE ; /* can't be on */
      pl = pl->link ;
    end
end

#ifndef OMIT_SEED
void lib_setcommevent (pq330 q330, integer number, boolean seton)
begin
  paqstruc paqs ;

  paqs = q330->aqstruc ;
  if ((q330->libstate == LIBSTATE_RUN) land (number >= 0) and (number < CE_MAX))
    then
      paqs->commevents[number].ison = seton ;
end
#endif

enum tliberr lib_getdpcfg (pq330 q330, tdpcfg *dpcfg)
begin
  paqstruc paqs ;
  plcq q ;

  paqs = q330->aqstruc ;
  if ((q330->libstate != LIBSTATE_RUN) land (q330->libstate != LIBSTATE_RUNWAIT))
    then
      return LIBERR_CFGWAIT ;
  memset (dpcfg, 0, sizeof(tdpcfg)) ;
  memcpy(addr(dpcfg->station_name), addr(q330->station_ident), sizeof(string9)) ;
  dpcfg->web_port = paqs->webport ;
  dpcfg->webip = q330->web_ip ;
  dpcfg->net_port = paqs->netport ;
  dpcfg->datas_port = paqs->dservport ;
  dpcfg->dss = paqs->dss_def ;
  memcpy(addr(dpcfg->clock), addr(q330->qclock), sizeof(tclock)) ;
  q = paqs->lcqs ;
  while (q)
    begin
#ifndef OMIT_SEED
      dpcfg->buffer_counts[q->lcq_num] = q->pre_event_buffers + 1 ;
#else
      dpcfg->buffer_counts[q->lcq_num] = 1 ;
#endif
      q = q->link ;
    end
  return LIBERR_NOERR ;
end
