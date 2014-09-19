/*   Lib330 Seed Routines
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
    1 2008-03-13 rdr Use modulus to restrict SEED record number to between 1 and 999999.
*/
#ifndef libseed_h
#include "libseed.h"
#endif

#ifndef OMIT_SEED
#ifndef libsupport_h
#include "libsupport.h"
#endif
#ifndef libcvrt_h
#include "libcvrt.h"
#endif
#endif
/* convert seedname and location into string */
char *seed2string(tlocation *loc, tseed_name *sn, pchar result)
begin
  string15 s ;
  integer lth ;

  lth = 0 ;
  if ((*loc)[0] != ' ')
    then
      begin
        s[lth++] = (*loc)[0] ;
        if ((*loc)[1] != ' ')
          then
            s[lth++] = (*loc)[1] ;
        s[lth++] = '-' ;
      end
  s[lth++] = (*sn)[0] ;
  s[lth++] = (*sn)[1] ;
  s[lth++] = (*sn)[2] ;
  s[lth] = 0 ;
  strcpy(result, s) ;
  return result ;
end

/* convert C string to fixed width field */
void string2fixed(pointer p, pchar s)
begin

  memcpy(p, s, strlen(s)) ;
end

#ifndef OMIT_SEED
void lib330_seed_time (tseed_time *st, tsystemtime *greg, longint usec)
begin

  st->seed_yr = greg->wyear ;
  st->seed_hr = greg->whour ;
  st->seed_minute = greg->wminute ;
  st->seed_seconds = greg->wsecond ;
  st->seed_unused = 0 ;
  st->seed_tenth_millisec = usec div 100 ;
  st->seed_jday = day_julian (greg->wyear, greg->wmonth, greg->wday) ;
end

void fix_seed_header (seed_header *hdr, tsystemtime *greg,
                           longint usec, boolean setdeb)
begin
  string7 s ;
  longword recnum ;

  recnum = ((hdr->sequence.seed_num - 1) mod 999999) + 1 ; /* restrict to 1 .. 999999 */
  sprintf(s, "%d", recnum) ;
  zpad(addr(s), 6) ;
  memcpy(addr(hdr->sequence.seed_num), addr(s), 6) ;
  lib330_seed_time (addr(hdr->starting_time), greg, usec) ;
  if (setdeb)
    then
      hdr->deb.usec99 = usec mod 100 ;
end

void convert_time (double fp, tsystemtime *greg, longint *usec)
begin
  longint jul ;
  double fjul, fusec ;

  jul = lib_round (fp) ;
  fjul = jul ;
  fusec = (fp - fjul) * 1.0E6 ;
  *usec = lib_round(fusec) ;
  if (*usec < 0)
    then
      begin
        jul = jul - 1 ;
        *usec = *usec + 1000000 ;
      end
  if (*usec >= 1000000)
    then
      begin
        jul = jul + 1 ;
        *usec = *usec - 1000000 ;
      end
  lib330_gregorian (jul, greg) ;
end

double extract_time (tseed_time *st, byte usec)
begin
  tsystemtime greg ;
  word j, dim ;
  double time ;

  j = st->seed_jday ;
  greg.wyear = st->seed_yr ;
  greg.whour = st->seed_hr ;
  greg.wminute = st->seed_minute ;
  greg.wsecond = st->seed_seconds ;
  greg.wmonth = 1 ;
  repeat
    dim = days_mth[greg.wmonth] ;
    if (((greg.wyear mod 4) == 0) land (greg.wmonth == 2))
      then
        dim = 29 ;
    if (j <= dim)
      then
        break ;
    j = j - dim ;
    inc(greg.wmonth) ;
  until (j > 400)) ; /* just in case */
  greg.wday = j ;
  time = lib330_julian(addr(greg)) ; /* seconds since 2000 */
  return time + st->seed_tenth_millisec / 10000.0 + usec * 0.000001 ;
end

void storetime (pbyte *p, tseed_time *seedtime)
begin

  storeword (p, seedtime->seed_yr) ;
  storeword (p, seedtime->seed_jday) ;
  storebyte (p, seedtime->seed_hr) ;
  storebyte (p, seedtime->seed_minute) ;
  storebyte (p, seedtime->seed_seconds) ;
  storebyte (p, seedtime->seed_unused) ;
  storeword (p, seedtime->seed_tenth_millisec) ;
end

void storeseedhdr (pbyte *pdest, seed_header *hdr, boolean hasdeb)
begin
  tsystemtime newtime ;
  longint newusec ;
  double time_save ;
  longword seq_save ;

  time_save = hdr->starting_time.seed_fpt ;
  seq_save = hdr->sequence.seed_num ;
  convert_time (hdr->starting_time.seed_fpt, addr(newtime), addr(newusec)) ;
  fix_seed_header (hdr, addr(newtime), newusec, hasdeb) ;
  storeblock (pdest, 6, addr(hdr->sequence.seed_ch)) ;
  storebyte (pdest, (byte)hdr->seed_record_type) ;
  storebyte (pdest, (byte)hdr->continuation_record) ;
  storeblock (pdest, 5, addr(hdr->station_id_call_letters)) ;
  storeblock (pdest, 2, addr(hdr->location_id)) ;
  storeblock (pdest, 3, addr(hdr->channel_id)) ;
  storeblock (pdest, 2, addr(hdr->seednet)) ;
  storetime (pdest, addr(hdr->starting_time)) ;
  storeword (pdest, hdr->samples_in_record) ;
  storeint16 (pdest, hdr->sample_rate_factor) ;
  storeint16 (pdest, hdr->sample_rate_multiplier) ;
  storebyte (pdest, hdr->activity_flags) ;
  storebyte (pdest, hdr->io_flags) ;
  storebyte (pdest, hdr->data_quality_flags) ;
  storebyte (pdest, hdr->number_of_following_blockettes) ;
  storelongint (pdest, hdr->tenth_msec_correction) ;
  storeword (pdest, hdr->first_data_byte) ;
  storeword (pdest, hdr->first_blockette_byte) ;
  /* all seed records have this one */
  storeword (pdest, hdr->dob.blockette_type) ;
  storeword (pdest, hdr->dob.next_blockette) ;
  storebyte (pdest, hdr->dob.encoding_format) ;
  storebyte (pdest, hdr->dob.word_order) ;
  storebyte (pdest, hdr->dob.rec_length) ;
  storebyte (pdest, hdr->dob.dob_reserved) ;
  /* only data records have this one */
  if (hasdeb)
    then
      begin
        storeword (pdest, hdr->deb.blockette_type) ;
        storeword (pdest, hdr->deb.next_blockette) ;
        storebyte (pdest, hdr->deb.qual) ;
        storebyte (pdest, hdr->deb.usec99) ;
        storebyte (pdest, hdr->deb.deb_flags) ;
        storebyte (pdest, hdr->deb.frame_count) ;
      end
  hdr->starting_time.seed_fpt = time_save ;
  hdr->sequence.seed_num = seq_save ;
end

void storemurdock (pbyte *pdest, murdock_detect *mdet)
begin

  storeword (pdest, mdet->blockette_type) ;
  storeword (pdest, mdet->next_blockette) ;
  storesingle (pdest, mdet->mh_onset.signal_amplitude) ;
  storesingle (pdest, mdet->mh_onset.signal_period) ;
  storesingle (pdest, mdet->mh_onset.background_estimate) ;
  storebyte (pdest, mdet->mh_onset.event_detection_flags) ;
  storebyte (pdest, mdet->mh_onset.reserved_byte) ;
  storetime (pdest, addr(mdet->mh_onset.signal_onset_time)) ;
  storeblock (pdest, 6, addr(mdet->mh_onset.snr)) ;
  storebyte (pdest, mdet->mh_onset.lookback_value) ;
  storebyte (pdest, mdet->mh_onset.pick_algorithm) ;
  storeblock (pdest, 24, addr(mdet->s_detname)) ;
end

void storethreshold (pbyte *pdest, threshold_detect *tdet)
begin

  storeword (pdest, tdet->blockette_type) ;
  storeword (pdest, tdet->next_blockette) ;
  storesingle (pdest, tdet->thr_onset.signal_amplitude) ;
  storesingle (pdest, tdet->thr_onset.signal_period) ;
  storesingle (pdest, tdet->thr_onset.background_estimate) ;
  storebyte (pdest, tdet->thr_onset.event_detection_flags) ;
  storebyte (pdest, tdet->thr_onset.reserved_byte) ;
  storetime (pdest, addr(tdet->thr_onset.signal_onset_time)) ;
  storeblock (pdest, 24, addr(tdet->s_detname)) ;
end

void storetiming (pbyte *pdest, timing *tim)
begin

  storeword (pdest, tim->blockette_type) ;
  storeword (pdest, tim->next_blockette) ;
  storesingle (pdest, tim->vco_correction) ;
  storetime (pdest, addr(tim->time_of_exception)) ;
  storebyte (pdest, tim->usec99) ;
  storebyte (pdest, tim->reception_quality) ;
  storelongint (pdest, tim->exception_count) ;
  storeblock (pdest, 16, addr(tim->exception_type)) ;
  storeblock (pdest, 32, addr(tim->clock_model)) ;
  storeblock (pdest, 128, addr(tim->clock_status)) ;
end

void storecal2 (pbyte *pdest, cal2 *c2)
begin

  storesingle (pdest, c2->calibration_amplitude) ;
  storeblock (pdest, 3, addr(c2->calibration_input_channel)) ;
  storebyte (pdest, c2->cal2_res) ;
  storesingle (pdest, c2->ref_amp) ;
  storeblock (pdest, 12, addr(c2->coupling)) ;
  storeblock (pdest, 12, addr(c2->rolloff)) ;
end

void storestep (pbyte *pdest, step_calibration *stepcal)
begin

  storeword (pdest, stepcal->blockette_type) ;
  storeword (pdest, stepcal->next_blockette) ;
  storetime (pdest, addr(stepcal->calibration_time)) ;
  storebyte (pdest, stepcal->number_of_steps) ;
  storebyte (pdest, stepcal->calibration_flags) ;
  storelongword (pdest, stepcal->calibration_duration) ;
  storelongword (pdest, stepcal->interval_duration) ;
  storecal2 (pdest, addr(stepcal->step2)) ;
end

void storesine (pbyte *pdest, sine_calibration *sinecal)
begin

  storeword (pdest, sinecal->blockette_type) ;
  storeword (pdest, sinecal->next_blockette) ;
  storetime (pdest, addr(sinecal->calibration_time)) ;
  storebyte (pdest, sinecal->res) ;
  storebyte (pdest, sinecal->calibration_flags) ;
  storelongword (pdest, sinecal->calibration_duration) ;
  storesingle (pdest, sinecal->sine_period) ;
  storecal2 (pdest, addr(sinecal->sine2)) ;
end

void storerandom (pbyte *pdest, random_calibration *randcal)
begin

  storeword (pdest, randcal->blockette_type) ;
  storeword (pdest, randcal->next_blockette) ;
  storetime (pdest, addr(randcal->calibration_time)) ;
  storebyte (pdest, randcal->res) ;
  storebyte (pdest, randcal->calibration_flags) ;
  storelongword (pdest, randcal->calibration_duration) ;
  storecal2 (pdest, addr(randcal->random2)) ;
  storeblock (pdest, 8, addr(randcal->noise_type)) ;
end

void storeabort (pbyte *pdest, abort_calibration *abortcal)
begin

  storeword (pdest, abortcal->blockette_type) ;
  storeword (pdest, abortcal->next_blockette) ;
  storetime (pdest, addr(abortcal->calibration_time)) ;
  storeword (pdest, abortcal->res) ;
end

void storeopaque (pbyte *pdest, topaque_hdr *ophdr, integer rectypelth,
                       pointer pbuf, integer psize)
begin

  storeword (pdest, ophdr->blockette_type) ;
  storeword (pdest, ophdr->next_blockette) ;
  storeword (pdest, ophdr->blk_lth) ;
  storeword (pdest, ophdr->data_off) ;
  storelongword (pdest, ophdr->recnum) ;
  storebyte (pdest, ophdr->word_order) ;
  storebyte (pdest, ophdr->opaque_flags) ;
  storebyte (pdest, ophdr->hdr_fields) ;
  storeblock (pdest, rectypelth, addr(ophdr->rec_type)) ;
  storeblock (pdest, psize, pbuf) ;
end

/* we this inline instead of calling storelongword to save the procedure call */
void storeframe (pbyte *pdest, compressed_frame *cf)
begin
  integer i ;
  longword lw ;

  for (i = 0 ; i <= WORDS_PER_FRAME - 1 ; i++)
    begin
      lw = (*cf)[i] ;
#ifdef ENDIAN_LITTLE
      lw = htonl(lw) ;
#endif
      memcpy(*pdest, addr(lw), 4) ;
      incn(*pdest, 4) ;
    end
end

void loadblkhdr (pbyte *p, blk_min *blk)
begin

  blk->blockette_type = loadword (p) ;
  blk->next_blockette = loadword (p) ;
end

void loadtime (pbyte *p, tseed_time *seedtime)
begin

  seedtime->seed_yr = loadword (p) ;
  seedtime->seed_jday = loadword (p) ;
  seedtime->seed_hr = loadbyte (p) ;
  seedtime->seed_minute = loadbyte (p) ;
  seedtime->seed_seconds = loadbyte (p) ;
  seedtime->seed_unused = loadbyte (p) ;
  seedtime->seed_tenth_millisec = loadword (p) ;
end

void loadseedhdr (pbyte *psrc, seed_header *hdr, boolean hasdeb)
begin

  loadblock (psrc, 6, addr(hdr->sequence.seed_ch)) ;
  hdr->seed_record_type = (char)loadbyte (psrc) ;
  hdr->continuation_record = (char)loadbyte (psrc) ;
  loadblock (psrc, 5, addr(hdr->station_id_call_letters)) ;
  loadblock (psrc, 2, addr(hdr->location_id)) ;
  loadblock (psrc, 3, addr(hdr->channel_id)) ;
  loadblock (psrc, 2, addr(hdr->seednet)) ;
  loadtime (psrc, addr(hdr->starting_time)) ;
  hdr->samples_in_record = loadword (psrc) ;
  hdr->sample_rate_factor = loadint16 (psrc) ;
  hdr->sample_rate_multiplier = loadint16 (psrc) ;
  hdr->activity_flags = loadbyte (psrc) ;
  hdr->io_flags = loadbyte (psrc) ;
  hdr->data_quality_flags = loadbyte (psrc) ;
  hdr->number_of_following_blockettes = loadbyte (psrc) ;
  hdr->tenth_msec_correction = loadlongint (psrc) ;
  hdr->first_data_byte = loadword (psrc) ;
  hdr->first_blockette_byte = loadword (psrc) ;
  hdr->dob.blockette_type = loadword (psrc) ;
  hdr->dob.next_blockette = loadword (psrc) ;
  hdr->dob.encoding_format = loadbyte (psrc) ;
  hdr->dob.word_order = loadbyte (psrc) ;
  hdr->dob.rec_length = loadbyte (psrc) ;
  hdr->dob.dob_reserved = loadbyte (psrc) ;
  if (hasdeb)
    then
      begin
        hdr->deb.blockette_type = loadword (psrc) ;
        hdr->deb.next_blockette = loadword (psrc) ;
        hdr->deb.qual = loadbyte (psrc) ;
        hdr->deb.usec99 = loadbyte (psrc) ;
        hdr->deb.deb_flags = loadbyte (psrc) ;
        hdr->deb.frame_count = loadbyte (psrc) ;
      end
end

void loadtiming (pbyte *psrc, timing *tim)
begin

  tim->blockette_type = loadword (psrc) ;
  tim->next_blockette = loadword (psrc) ;
  tim->vco_correction = loadsingle (psrc) ;
  loadtime (psrc, addr(tim->time_of_exception)) ;
  tim->usec99 = loadbyte (psrc) ;
  tim->reception_quality = loadbyte (psrc) ;
  tim->exception_count = loadlongint (psrc) ;
  loadblock (psrc, 16, addr(tim->exception_type)) ;
  loadblock (psrc, 32, addr(tim->clock_model)) ;
  loadblock (psrc, 128, addr(tim->clock_status)) ;
end

void loadmurdock (pbyte *psrc, murdock_detect *mdet)
begin

  mdet->blockette_type = loadword (psrc) ;
  mdet->next_blockette = loadword (psrc) ;
  mdet->mh_onset.signal_amplitude = loadsingle (psrc) ;
  mdet->mh_onset.signal_period = loadsingle (psrc) ;
  mdet->mh_onset.background_estimate = loadsingle (psrc) ;
  mdet->mh_onset.event_detection_flags = loadbyte (psrc) ;
  mdet->mh_onset.reserved_byte = loadbyte (psrc) ;
  loadtime (psrc, addr(mdet->mh_onset.signal_onset_time)) ;
  loadblock (psrc, 6, addr(mdet->mh_onset.snr)) ;
  mdet->mh_onset.lookback_value = loadbyte (psrc) ;
  mdet->mh_onset.pick_algorithm = loadbyte (psrc) ;
  loadblock (psrc, 24, addr(mdet->s_detname)) ;
end

void loadthreshold (pbyte *psrc, threshold_detect *tdet)
begin

  tdet->blockette_type = loadword (psrc) ;
  tdet->next_blockette = loadword (psrc) ;
  tdet->thr_onset.signal_amplitude = loadsingle (psrc) ;
  tdet->thr_onset.signal_period = loadsingle (psrc) ;
  tdet->thr_onset.background_estimate = loadsingle (psrc) ;
  tdet->thr_onset.event_detection_flags = loadbyte (psrc) ;
  tdet->thr_onset.reserved_byte = loadbyte (psrc) ;
  loadtime (psrc, addr(tdet->thr_onset.signal_onset_time)) ;
  loadblock (psrc, 24, addr(tdet->s_detname)) ;
end

void loadcal2 (pbyte *psrc, cal2 *c2)
begin

  c2->calibration_amplitude = loadsingle (psrc) ;
  loadblock (psrc, 3, addr(c2->calibration_input_channel)) ;
  c2->cal2_res = loadbyte (psrc) ;
  c2->ref_amp = loadsingle (psrc) ;
  loadblock (psrc, 12, addr(c2->coupling)) ;
  loadblock (psrc, 12, addr(c2->rolloff)) ;
end

void loadstep (pbyte *psrc, step_calibration *stepcal)
begin

  stepcal->blockette_type = loadword (psrc) ;
  stepcal->next_blockette = loadword (psrc) ;
  loadtime (psrc, addr(stepcal->calibration_time)) ;
  stepcal->number_of_steps = loadbyte (psrc) ;
  stepcal->calibration_flags = loadbyte (psrc) ;
  stepcal->calibration_duration = loadlongword (psrc) ;
  stepcal->interval_duration = loadlongword (psrc) ;
  loadcal2 (psrc, addr(stepcal->step2)) ;
end

void loadsine (pbyte *psrc, sine_calibration *sinecal)
begin

  sinecal->blockette_type = loadword (psrc) ;
  sinecal->next_blockette = loadword (psrc) ;
  loadtime (psrc, addr(sinecal->calibration_time)) ;
  sinecal->res = loadbyte (psrc) ;
  sinecal->calibration_flags = loadbyte (psrc) ;
  sinecal->calibration_duration = loadlongword (psrc) ;
  sinecal->sine_period = loadsingle (psrc) ;
  loadcal2 (psrc, addr(sinecal->sine2)) ;
end

void loadrandom (pbyte *psrc, random_calibration *randcal)
begin

  randcal->blockette_type = loadword (psrc) ;
  randcal->next_blockette = loadword (psrc) ;
  loadtime (psrc, addr(randcal->calibration_time)) ;
  randcal->res = loadbyte (psrc) ;
  randcal->calibration_flags = loadbyte (psrc) ;
  randcal->calibration_duration = loadlongword (psrc) ;
  loadcal2 (psrc, addr(randcal->random2)) ;
  loadblock (psrc, 8, addr(randcal->noise_type)) ;
end

void loadabort (pbyte *psrc, abort_calibration *abortcal)
begin

  abortcal->blockette_type = loadword (psrc) ;
  abortcal->next_blockette = loadword (psrc) ;
  loadtime (psrc, addr(abortcal->calibration_time)) ;
  abortcal->res = loadword (psrc) ;
end

void loadopaquehdr (pbyte *psrc, topaque_hdr *ophdr)
begin

  ophdr->blockette_type = loadword (psrc) ;
  ophdr->next_blockette = loadword (psrc) ;
  ophdr->blk_lth = loadword (psrc) ;
  ophdr->data_off = loadword (psrc) ;
  ophdr->recnum = loadlongword (psrc) ;
  ophdr->word_order = loadbyte (psrc) ;
  ophdr->opaque_flags = loadbyte (psrc) ;
  ophdr->hdr_fields = loadbyte (psrc) ;
  loadblock (psrc, 5, addr(ophdr->rec_type)) ;
end

#endif
