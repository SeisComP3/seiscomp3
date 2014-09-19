/*   Lib330 time series configuration routines
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
    1 2006-10-26 rdr Fix mangled filter cutoff descriptions in verbose mode.
    2 2008-03-02 rdr Add setting scd_evt and scd_cont based on lcq options.
    3 2008-03-18 rdr Add set_gaps to setup both the gap_secs and new gap_offset. Gap_offset
                     needs to be overridden for decimated channels based on the rate of
                     the source channel.
    4 2008-04-03 rdr If opt_compat is set then force netserv to event if archive is event.
    5 2008-04-26 rdr Setup scd_cont for DP LCQs.
    6 2008-06-19 rdr Initialize log name in allocate_aqstruc.
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
#endif

longword secsince (void)
begin

  return lib_round(now()) ;
end

void clear_sg (paqstruc paqs)
begin

  memset (addr(paqs->first_sg), 0, (longint)addr(paqs->last_sg) - (longint)addr(paqs->first_sg)) ;
  string2fixed (addr(paqs->log_tim.log_location), "  ") ;
  string2fixed (addr(paqs->log_tim.log_seedname), "LOG") ;
  string2fixed (addr(paqs->log_tim.tim_location), "  ") ;
  string2fixed (addr(paqs->log_tim.tim_seedname), "ACE") ;
#ifndef OMIT_SEED
  paqs->cnp_lcqs = NIL ;
#endif
  memset (addr(paqs->dispatch), 0, sizeof(tdispatch)) ;
  memset (addr(paqs->mdispatch), 0, sizeof(tmdispatch)) ;
end

pointer allocate_aqstruc (tcontext ownedby)
begin
  paqstruc paqs ;
  pq330 q330 ;
#ifndef OMIT_SEED
  integer n ;
  tmhqp mhqp ;
#endif

  q330 = ownedby ;
  getbuf (q330, addr(paqs), sizeof(taqstruc)) ;
  paqs->owner = ownedby ;
  clear_sg (paqs) ;
#ifndef OMIT_SEED
  getbuf (q330, addr(mhqp), sizeof(mholdqtype)) ;
  paqs->mholdq = mhqp ;
  paqs->mhqnxi = mhqp ;
  paqs->mhqnxo = mhqp ;
  for (n = 2 ; n <= MHOLDQSZ ; n++)
    begin
      getbuf (q330, addr(mhqp->link), sizeof(mholdqtype)) ;
      mhqp = mhqp->link ;
    end
  mhqp->link = paqs->mholdq ;
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

void deallocate_dplcqs (pq330 q330)
begin
  plcq pl, p ;
  paqstruc paqs ;

  paqs = q330->aqstruc ;
  pl = paqs->dplcqs ;
  while (pl)
    begin
      p = pl->link ;
#ifndef OMIT_SEED
      if (pl->com)
        then
          begin
            free (pl->com->ring) ;
            free (pl->com) ;
          end
      if (pl->arc.pcfr)
        then
          free (pl->arc.pcfr) ;
#endif
      free (pl) ;
      pl = p ;
    end
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
      getbuf (q330, addr(pl->databuf), pl->datasize) ;
      if (pl->rate > 1)
        then
          getbuf (q330, addr(pl->idxbuf), pl->rate + 1) ;
      switch (pl->rate) begin
        case 100 :
          pl->segsize = SS_100 ;
          break ;
        case 200 :
          pl->segsize = SS_200 ;
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
            getbuf (q330, addr(pl->mergedbuf), pl->segsize) ;
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
          end
      else if (pl->raw_data_source == READ_PREV_STREAM)
        then
          begin
#ifndef OMIT_SEED
            if (pl->prev_link->rate > 0)
              then
                pl->input_sample_rate = pl->prev_link->rate ;
              else
                pl->input_sample_rate = 1.0 / abs(pl->prev_link->rate) ;
            if (pl->input_sample_rate >= 0.999)
              then
                pl->gap_offset = 1.0 ;
              else
                pl->gap_offset = 1.0 / pl->input_sample_rate ; /* set new gap offset based on input rate */
            if (pl->source_fir)
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
            /* see if root source is 1hz */
            p = pl->prev_link ;
            while (p->prev_link)
              p = p->prev_link ;
            if ((p) land (p->rate == 1))
              then
                begin
                  /* yes, need to synchronize based on rate */
                  pl->slipping = TRUE ;
                  pl->slip_modulus = abs(pl->rate) ; /* .1hz has modulus of 10 */
                end
#endif
          end
      else if ((pl->raw_data_source >= MESSAGE_STREAM) land (pl->raw_data_source <= CFG_STREAM))
        then
          begin
#ifndef OMIT_SEED
            pl->com->blockette_index = 56 ; /* header plus blockette 1000 */
#endif
            switch (pl->raw_data_source) begin
              case MESSAGE_STREAM :
                pl->pack_class = PKC_MESSAGE ;
                break ;
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
          getbuf (q330, addr(pl->arc.pcfr), paqs->arc_size) ;
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
end

void init_dplcq (paqstruc paqs, plcq pl, boolean newone)
begin
#ifndef OMIT_SEED
  pcompressed_buffer_ring pr ;
#endif
  pq330 q330 ;

  q330 = paqs->owner ;
  pl->dtsequence = 0 ;
  if ((enum tacctype)pl->raw_data_field <= AC_LAST)
    then
      q330->share.accmstats[(enum tacctype)pl->raw_data_field].ds_lcq = pl ;
  else if (pl->raw_data_field == AC_DATA_LATENCY)
    then
      paqs->data_latency_lcq = pl ;
  else if (pl->raw_data_field == AC_STATUS_LATENCY)
    then
      paqs->status_latency_lcq = pl ;
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
  pl->pack_class = PKC_DATA ;
#ifndef OMIT_SEED
  pl->scd_cont = SCD_BOTH ; /* both are continuous */
  if (pl->com->maxframes >= FRAMES_PER_RECORD)
    then
      pl->com->maxframes = FRAMES_PER_RECORD - 1 ;
  if (lnot newone)
    then
      return ; /* don't need to allocate new buffers */
  pr = malloc (sizeof(tcompressed_buffer_ring)) ;
  memset (pr, 0, sizeof(tcompressed_buffer_ring)) ;
  pr->full = FALSE ;
  pl->com->ring = pr ;
  pr->link = pl->com->ring ; /* just keeps going back to itself */
  if (pl->arc.amini_filter)
    then
      begin
        pl->arc.pcfr = malloc (paqs->arc_size) ;
        memset (pl->arc.pcfr, 0, paqs->arc_size) ;
      end
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
/*  totrec : longword ; */
  pq330 q330 ;

  q330 = paqs->owner ;
#ifndef OMIT_SEED
  if (q330->need_sats)
    then
      finish_log_clock (q330) ;
  flush_lcqs (paqs) ;
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
        new_cmd (q330, C1_SLOG, 0) ;
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
