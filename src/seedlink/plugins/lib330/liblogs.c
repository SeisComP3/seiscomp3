/*   Lib330 Message Log Routines
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
    0 2006-10-12 rdr Created
    1 2008-01-03 rdr Add log_timer handling.
    2 2008-03-13 rdr Don't reset records_written at 999999.
    3 2010-08-08 rdr In spad protect against negative length difference.
*/
#ifndef liblogs_h
#include "liblogs.h"
#endif

#ifndef OMIT_SEED
#ifndef libmsgs_h
#include "libmsgs.h"
#endif
#ifndef libcvrt_h
#include "libcvrt.h"
#endif
#ifndef libdetect_h
#include "libdetect.h"
#endif
#ifndef libctrldet_h
#include "libctrldet.h"
#endif
#ifndef libsample_h
#include "libsample.h"
#endif
#ifndef libclient_h
#include "libclient.h"
#endif
#ifndef libsampcfg_h
#include "libsampcfg.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif
#ifndef libarchive_h
#include "libarchive.h"
#endif

/* copy string "s" to fixed width right padded field "b" */
void lib330_padright (string *s, pchar b, integer fld)
begin
  integer i, j ;

  j = strlen(s) ;
  if (j > fld)
    then
      j = fld ;
  for (i = 0 ; i < j ; i++)
    *b++ = (*s)[i] ;
  for (i = j ; i < fld ; i++)
    *b++ = ' ' ;
end

/* same as zpad but pads on left with spaces */
static char *spad (pchar s, integer lth)
begin
  integer len, diff ;

  len = strlen(s) ;
  diff = lth - len ;
  if (diff > 0)
    then
      begin
        memmove (addr(s[diff]), addr(s[0]), len + 1) ; /* shift existing string right */
        memset (addr(s[0]), ' ', diff) ; /* add spaces at front */
      end
  return s ;
end

/* return two digit string, padded on left with zero */
static char *two (byte b, string3 *s)
begin
  integer v ;

  v = b ;
  sprintf(s, "%d", v mod 100) ;
  zpad (s, 2) ;
  return s ;
end

static char *string_time (tsystemtime *nt, string31 *result)
begin
  string3 smth, sday, shour, smin, ssec ;

  sprintf(result, "%d-%s-%s %s:%s:%s", nt->wyear, two(nt->wmonth, addr(smth)),
          two(nt->wday, addr(sday)), two(nt->whour, addr(shour)),
          two(nt->wminute, addr(smin)), two(nt->wsecond, addr(ssec))) ;
  return result ;
end

static void fix_time (tseed_time *st)
begin
  tsystemtime greg ;
  longint usec ;

  convert_time (st->seed_fpt, addr(greg), addr(usec)) ;
  lib330_seed_time (st, addr(greg), usec) ;
end

void log_clock (pq330 q330, enum tclock_exception clock_exception, string95 *jump_amount)
begin
  string s ;
  tsystemtime newtime ;
  longint newusec ;
  integer i ;
  paqstruc paqs ;
  pbyte p ;
  seed_header *phdr ;
  tcom_packet *pcom ;
  plcq q ;
  timing *ptim ;

  paqs = q330->aqstruc ;
  q = paqs->tim_lcq ;
  if ((q == NIL) lor (q->com->ring == NIL))
    then
      return ;
  pcom = q->com ;
  phdr = addr(pcom->ring->hdr_buf) ;
  if (q330->need_sats)
    then
      finish_log_clock (q330) ; /* Already one pending, just use current satellite info */
  paqs->last_update = paqs->data_timetag ;
  memset(addr(pcom->ring->rec), 0, LIB_REC_SIZE) ;
  phdr->samples_in_record = 0 ;
  phdr->seed_record_type = 'D' ;
  phdr->continuation_record = ' ' ;
  phdr->sequence.seed_num = pcom->records_written + 1 ;
  inc(pcom->records_written) ;
  memcpy(addr(phdr->location_id), addr(q->location), sizeof(tlocation)) ;
  memcpy(addr(phdr->channel_id), addr(q->seedname), sizeof(tseed_name)) ;
  memcpy(addr(phdr->station_id_call_letters), addr(q330->station), sizeof(tseed_stn)) ;
  memcpy(addr(phdr->seednet), addr(q330->network), sizeof(tseed_net)) ;
  phdr->starting_time.seed_fpt = paqs->data_timetag ;
  phdr->sample_rate_factor = 0 ;
  phdr->sample_rate_multiplier = 1 ;
  phdr->number_of_following_blockettes = 2 ;
  phdr->tenth_msec_correction = 0 ; /* this is a delta, which we don't */
  phdr->first_data_byte = 0 ;
  phdr->first_blockette_byte = 48 ;
  phdr->dob.blockette_type = 1000 ;
  phdr->dob.word_order = 1 ;
  phdr->dob.rec_length = RECORD_EXP ;
  phdr->dob.dob_reserved = 0 ;
  phdr->dob.next_blockette = 56 ;
  phdr->dob.encoding_format = 0 ;
  ptim = addr(paqs->timing_buf) ;
  ptim->blockette_type = 500 ;
  ptim->next_blockette = 0 ;
  ptim->vco_correction = q330->share.stat_global.cur_vco / 40.96 ;
  convert_time (paqs->data_timetag, addr(newtime), addr(newusec)) ;
  lib330_seed_time (addr(ptim->time_of_exception), addr(newtime), newusec) ;
  ptim->usec99 = newusec mod 100 ;
  ptim->reception_quality = paqs->data_qual ;
  ptim->exception_count = paqs->except_count ;
  paqs->except_count = 0 ;
  switch (clock_exception) begin
    case CE_DAILY :
      lib330_padright ("Daily Timemark", addr(ptim->exception_type), 16) ;
      break ;
    case CE_VALID :
      lib330_padright ("Valid Timemark", addr(ptim->exception_type), 16) ;
      break ;
    case CE_JUMP :
      lib330_padright ("UnExp Timemark", addr(ptim->exception_type), 16) ;
      sprintf (s, "Jump of %s Seconds", (char *)jump_amount) ;
      lib330_padright (addr(s), addr(ptim->clock_status), 128) ;
      break ;
  end
  strcpy(s, addr(q330->share.gpsids[4])) ; /* 330 will place clock ID string here for all models */
/* first removing trailing spaces */
  for (i = strlen(s) - 1 ; i >= 0 ; i--)
    if (s[i] == ' ')
      then
        s[i] = 0 ;
      else
        break ;
/* next find beginning of string */
  i = strlen(s) - 1 ;
  while ((i > 0) land (s[i] != ' '))
    dec(i) ;
  lib330_padright (addr(s[i + 1]), addr(ptim->clock_model), 32) ;
  if (clock_exception == CE_JUMP)
    then
      begin
        p = addr(pcom->ring->rec) ;
        storeseedhdr (addr(p), phdr, FALSE) ;
        storetiming (addr(p), ptim) ;
        inc(q->records_generated_session) ;
        q->last_record_generated = secsince () ;
        send_to_client (paqs, q, pcom->ring, SCD_BOTH) ;
      end
    else
      q330->need_sats = TRUE ;
end

void finish_log_clock (pq330 q330)
begin
  string s ;
  string7 s1 ;
  word w ;
  paqstruc paqs ;
  pbyte p ;
  tcom_packet *pcom ;
  plcq q ;
  timing *ptim ;
  tstat_sat1 *pone ;

  paqs = q330->aqstruc ;
  q = paqs->tim_lcq ;
  if ((q == NIL) lor (q->com->ring == NIL))
    then
      return ;
  pcom = q->com ;
  q330->need_sats = FALSE ;
  ptim = addr(paqs->timing_buf) ;
  if (q330->share.stat_sats.sathdr.sat_count)
    then
      begin
        strcpy(s, "SNR=") ;
        for (w = 0 ; w <= q330->share.stat_sats.sathdr.sat_count - 1 ; w++)
          begin
            pone = addr(q330->share.stat_sats.sats[w]) ;
            if ((pone->num) land (pone->snr >= 20))
              then
                begin
                  if (strlen(s) > 4)
                    then
                      strcat(s, ",") ;
                  sprintf(s1, "%d", pone->snr) ;
                  strcat(s, s1) ;
                end
          end
      end
    else
      s[0] = 0 ;
  lib330_padright (addr(s), addr(ptim->clock_status), 128) ;
  p = addr(pcom->ring->rec) ;
  storeseedhdr (addr(p), addr(pcom->ring->hdr_buf), FALSE) ;
  storetiming (addr(p), ptim) ;
  inc(q->records_generated_session) ;
  q->last_record_generated = secsince () ;
  send_to_client (paqs, paqs->tim_lcq, pcom->ring, SCD_BOTH) ;
end

void flush_timing (paqstruc paqs)
begin
  plcq q ;

  q = paqs->tim_lcq ;
  if ((q == NIL) lor (q->com->ring == NIL))
    then
      return ;
  if ((q->arc.amini_filter) land (q->arc.total_frames > 0))
    then
      flush_archive (paqs, q) ;
end

void logevent (paqstruc paqs, pdet_packet det, tonset_mh *onset)
begin
  string on_, s ;
  string63 w ;
  string31 s1, s2 ;
  boolean mh ;
  tsystemtime newtime ;
  longint newusec ;
  murdock_detect eblk ;
  threshold_detect *pt ;
  double ts ;
  byte buffer[FRAME_SIZE] ; /* for creating "blockette image" */
  pbyte p ;
  pq330 q330 ;
  plcq ppar ;
  pdetector pdef ;

  q330 = paqs->owner ;
  ppar = det->parent ;
  pdef = det->detector_def ;
  mh = (pdef->dtype == MURDOCK_HUTT) ;
  if (mh)
    then
      sprintf(on_, "%c %c %c %c%c%c%c%c", onset->event_detection_flags + 0x63,
              onset->pick_algorithm + 0x41, onset->lookback_value + 0x30,
              onset->snr[0] + 0x30, onset->snr[1] + 0x30, onset->snr[2] + 0x30,
              onset->snr[3] + 0x30, onset->snr[4] + 0x30) ;
    else
      strcpy(on_, "           ") ;
  ts = onset->signal_onset_time.seed_fpt ;
  convert_time (onset->signal_onset_time.seed_fpt, addr(newtime), addr(newusec)) ;
  sprintf(s2, "%d", newusec) ;
  zpad(s2, 6) ;
  sprintf(w, " %s.%s ", string_time (addr(newtime), addr(s1)), s2) ;
  strcat(on_, w) ;
  if ((lnot mh) lor (onset->signal_amplitude >= 0))
    then
      begin
        sprintf(s1, "%d", lib_round(onset->signal_amplitude)) ;
        spad(s1, 10) ;
        strcat(on_, s1) ;
        strcat(on_, " ") ;
      end
    else
      strcat(on_, "?????????? ") ;
  sprintf(s2, "%d", lib_round(onset->background_estimate)) ;
  if (mh)
    then
      begin
        if ((onset->signal_period > 0) land (onset->signal_period < 1000))
          then
            begin
              sprintf(s1, "%6.2f ", onset->signal_period) ;
              strcat(on_, s1) ;
            end
          else
            strcat(on_, "???.?? ") ;
        if (onset->background_estimate >= 0)
          then
            begin
              spad(s2, 7) ;
              strcat(on_, s2) ;
            end
          else
            strcat(on_, "???????") ;
      end
    else
      begin
        zpad(s2, 14) ;
        strcat(on_, s2) ;
      end
  strcat(on_, " ") ;
  sprintf(w, "%s:%s", seed2string(ppar->location, ppar->seedname, addr(s2)), pdef->detname) ;
  while (strlen(w) < 18)
    strcat(w, " ") ;
  if (det->det_options and DO_MSG)
    then
      begin
        sprintf(s, "%s-%s", w, on_) ;
        libmsgadd(q330, LIBMSG_DETECT, addr(s)) ;
      end
  lib330_seed_time (addr(eblk.mh_onset.signal_onset_time), addr(newtime), newusec) ;
  eblk.mh_onset.signal_amplitude = onset->signal_amplitude ;
  eblk.mh_onset.signal_period = onset->signal_period ;
  eblk.mh_onset.background_estimate = onset->background_estimate ;
  eblk.mh_onset.event_detection_flags = onset->event_detection_flags ;
  eblk.mh_onset.reserved_byte = 0 ;
  pt = (pointer)addr(eblk) ;
  p = addr(buffer) ;
  if (mh)
    then
      begin
        eblk.blockette_type = 201 ;
        eblk.next_blockette = 0 ;
        memcpy(addr(eblk.mh_onset.snr), addr(onset->snr), 6) ;
        eblk.mh_onset.lookback_value = onset->lookback_value ;
        eblk.mh_onset.pick_algorithm = onset->pick_algorithm ;
        lib330_padright (addr(pdef->detname), addr(eblk.s_detname), 24) ;
        storemurdock (addr(p), addr(eblk)) ;
      end
    else
      begin
        pt->blockette_type = 200 ;
        pt->next_blockette = 0 ;
        lib330_padright (addr(pdef->detname), addr(pt->s_detname), 24) ;
        storethreshold (addr(p), pt) ;
      end
  if (ppar->lcq_opt and LO_DETP)
    then
      begin
        if (q330->par_create.mini_embed)
          then
            add_blockette (paqs, ppar, addr(buffer), ts) ;
        if (q330->par_create.mini_separate)
          then
            build_separate_record (paqs, ppar, addr(buffer), ts, PKC_EVENT) ;
      end
end

static void set_cal2 (pcal2 pc2, tdp_cals *cals, paqstruc paqs)
begin
  word w ;
  integer idx, sub ;
  plcq pq ;

  pc2->calibration_amplitude = (cals->amplitude + 1) * -6 ;
  pc2->cal2_res = 0 ;
  pc2->ref_amp = 0 ;
  memcpy(addr(pc2->coupling), addr(cals->coupling), 12) ;
  lib330_padright ("3DB@10Hz", addr(pc2->rolloff), 12) ;
  memset(addr(pc2->calibration_input_channel), ' ', 3) ;
  w = cals->monitor_map ; /* find first LCQ that is in the monitor map */
  if (w == 0)
    then
      return ;
  for (idx = 0 ; idx <= CHANNELS - 1 ; idx++)
    if (w and (1 shl idx))
      then
        for (sub = FREQUENCIES - 1 ; sub >= 0 ; sub--)
          begin
            pq = paqs->mdispatch[idx][sub] ;
            if (pq)
              then
                begin
                  memcpy(addr(pc2->calibration_input_channel), pq->seedname, 3) ;
                  w = 0 ; /* found one, don't need another */
                  break ;
                end
          end
end

void log_cal (pq330 q330, pbyte pb)
begin
  tdp_cals cals ;
  plcq q ;
  byte chan ;
  integer idx, sub ;
  word map ;
  step_calibration *pstep ;
  sine_calibration *psine ;
  random_calibration *prand ;
  abort_calibration *pabort ;
  random_calibration cblk ;
  paqstruc paqs ;
  byte buffer[FRAME_SIZE] ; /* for creating "blockette image" */
  pbyte p ;

  paqs = q330->aqstruc ;
  chan = loadbyte (addr(pb)) ;
  loadbyte (addr(pb)) ;
  memset(addr(cblk), 0, sizeof(random_calibration)) ;
  p = addr(buffer) ;
  switch (chan and not DCM) begin
    case SP_CALSTART :
      cals.waveform = loadword (addr(pb)) ;
      cals.amplitude = loadint16 (addr(pb)) ;
      cals.duration = loadword (addr(pb)) ;
      cals.calbit_map = loadword (addr(pb)) ;
      map = cals.calbit_map ;
      cals.monitor_map = loadword (addr(pb)) ;
      cals.freqdiv = loadword (addr(pb)) ;
      cals.spare = loadword (addr(pb)) ;
      loadblock (addr(pb), 12, addr(cals.coupling)) ;
      psine = addr(cblk) ; /* common fields */
      psine->next_blockette = 0 ;
      psine->calibration_time.seed_fpt = paqs->data_timetag ;
      fix_time (addr(psine->calibration_time)) ;
      if (cals.waveform and 0x80)
        then
          psine->calibration_flags = 4 ; /* automatic */
        else
          psine->calibration_flags = 0 ;
      psine->calibration_duration = (longint)cals.duration * 10000 ;
      switch (cals.waveform and 7) begin
        case 0 : /* sine */
          psine->blockette_type = 310 ;
          psine->calibration_flags = psine->calibration_flags or 0x10 ;
          psine->sine_period = cals.freqdiv ;
          set_cal2 (addr(psine->sine2), addr(cals), paqs) ;
          storesine (addr(p), psine) ;
          break ;
        case 1 :
        case 2 :
        case 4 : /* random */
          prand = addr(cblk) ;
          prand->blockette_type = 320 ;
      /* white uses random amplitudes, red and telegraph don't */
          if ((cals.waveform and 7) == 2)
            then
              prand->calibration_flags = prand->calibration_flags or 0x10 ;
          set_cal2 (addr(prand->random2), addr(cals), paqs) ;
          switch (cals.waveform and 7) begin
            case 1 :
              lib330_padright("Red", addr(prand->noise_type), 8) ;
              break ;
            case 2 :
              lib330_padright("White", addr(prand->noise_type), 8) ;
              break ;
            case 4 :
              lib330_padright("Telegraf", addr(prand->noise_type), 8) ;
              break ;
          end
          storerandom (addr(p), prand) ;
          break ;
        case 3 : /* step */
          pstep = addr(cblk) ;
          pstep->blockette_type = 300 ;
          pstep->number_of_steps = 1 ;
          if ((cals.waveform and 0x40) == 0)
            then
              pstep->calibration_flags = pstep->calibration_flags or 1 ; /* positive pulse */
          pstep->interval_duration = 0 ;
          set_cal2 (addr(pstep->step2), addr(cals), paqs) ;
          storestep (addr(p), pstep) ;
          break ;
      end
      break ;
    case SP_CALABORT :
      cals.calbit_map = loadword (addr(pb)) ;
      map = cals.calbit_map ;
      cals.monitor_map = loadword (addr(pb)) ;
      pabort = addr(cblk) ;
      pabort->blockette_type = 395 ;
      pabort->next_blockette = 0 ;
      pabort->calibration_time.seed_fpt = paqs->data_timetag ;
      fix_time (addr(pabort->calibration_time)) ;
      storeabort (addr(p), pabort) ;
      break ;
    default :
      return ;
  end
  /* need a record for each channel in the map */
  for (idx = 0 ; idx <= CHANNELS - 1 ; idx++)
    if (map and (1 shl idx))
      then
        for (sub = 0 ; sub <= FREQUENCIES - 1 ; sub++)
          begin
            q = paqs->mdispatch[idx][sub] ;
            while (q)
              begin
                inc(q->calibrations_session) ;
                if (q->lcq_opt and LO_CALP)
                  then
                    begin
                      if (q330->par_create.mini_embed)
                        then
                          add_blockette (paqs, q, addr(buffer), paqs->data_timetag) ;
                      if (q330->par_create.mini_separate)
                        then
                          build_separate_record (paqs, q, addr(buffer), paqs->data_timetag, PKC_CALIBRATE) ;
                    end
                q = q->dispatch_link ;
              end
          end
end

void log_message (pq330 q330, string *msg)
begin
  integer i ;
  double ts ;
  paqstruc paqs ;
  pbyte p ;
  pchar pc ;
  plcq q ;
  tcom_packet *pcom ;
  seed_header *phdr ;

  paqs = q330->aqstruc ;
  q = paqs->msg_lcq ;
  if ((q == NIL) lor (q->com->ring == NIL))
    then
      return ; /* not initialized */
  ts = now() ;
  pcom = q->com ;
  phdr = addr(pcom->ring->hdr_buf) ;
  i = strlen(msg) + 2 ;
  if ((pcom->frame >= 2) land
     (((pcom->frame + i) > NONDATA_AREA) lor (ts > (phdr->starting_time.seed_fpt + 60))))
    then
      begin /* won't fit in current frame or was too long ago */
        phdr->samples_in_record = pcom->frame ;
        q330->nested_log = TRUE ;
        p = addr(pcom->ring->rec) ;
        storeseedhdr (addr(p), phdr, FALSE) ;
        inc(q->records_generated_session) ;
        q->last_record_generated = secsince () ;
        send_to_client (paqs, q, pcom->ring, SCD_BOTH) ;
        q330->nested_log = FALSE ;
        pcom->frame = 0 ;
      end
  if (pcom->frame < 2)
    then
      begin /* at least a blank line */
        memset(addr(pcom->ring->rec), 0, LIB_REC_SIZE) ;
        phdr->samples_in_record = 0 ;
        pcom->frame = 0 ;
        phdr->starting_time.seed_fpt = ts ;
        phdr->seed_record_type = 'D' ;
        phdr->continuation_record = ' ' ;
        phdr->sequence.seed_num = pcom->records_written + 1 ;
        inc(pcom->records_written) ;
        memcpy(addr(phdr->location_id), addr(q->location), sizeof(tlocation)) ;
        memcpy(addr(phdr->channel_id), addr(q->seedname), sizeof(tseed_name)) ;
        memcpy(addr(phdr->station_id_call_letters), addr(q330->station), sizeof(tseed_stn)) ;
        memcpy(addr(phdr->seednet), addr(q330->network), sizeof(tseed_net)) ;
        phdr->sample_rate_factor = 0 ;
        phdr->sample_rate_multiplier = 1 ;
        phdr->number_of_following_blockettes = 1 ;
        phdr->tenth_msec_correction = 0 ; /* this is a delta, which we don't */
        phdr->first_data_byte = 56 ;
        phdr->first_blockette_byte = 48 ;
        phdr->dob.blockette_type = 1000 ;
        phdr->dob.word_order = 1 ;
        phdr->dob.rec_length = RECORD_EXP ;
        phdr->dob.dob_reserved = 0 ;
        phdr->dob.next_blockette = 0 ;
        phdr->dob.encoding_format = 0 ;
      end
  strcat(msg, "\x0D\x0A") ;
  pc = (pointer)((pntrint)addr(pcom->ring->rec) + 56 + pcom->frame) ; /* add text to data area */
  memcpy(pc, msg, strlen(msg)) ;
  incn(pcom->frame, strlen(msg)) ;
  paqs->log_timer = LOG_TIMEOUT ;
end

void flush_messages (paqstruc paqs)
begin
  pbyte p ;
  pq330 q330 ;
  plcq q ;
  tcom_packet *pcom ;
  seed_header *phdr ;

  q330 = paqs->owner ;
  q = paqs->msg_lcq ;
  if ((q == NIL) lor (q->com->ring == NIL))
    then
      return ; /* not initialized */
  pcom = q->com ;
  phdr = addr(pcom->ring->hdr_buf) ;
  if (pcom->frame >= 2)
    then
      begin
        phdr->samples_in_record = pcom->frame ;
        q330->nested_log = TRUE ;
        p = addr(pcom->ring->rec) ;
        storeseedhdr (addr(p), phdr, FALSE) ;
        inc(q->records_generated_session) ;
        q->last_record_generated = secsince () ;
        send_to_client (paqs, q, pcom->ring, SCD_BOTH) ;
        q330->nested_log = FALSE ;
        pcom->frame = 0 ;
      end
  if ((q->arc.amini_filter) land (q->arc.hdr_buf.samples_in_record))
    then
      flush_archive (paqs, q) ;
  paqs->log_timer = 0 ;
end
#endif

