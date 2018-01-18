/*   Lib330 Time Series data routines
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
    0 2006-10-01 rdr Created
    1 2006-11-28 rdr Get rid of spurious q_samples variable in process_lcq.
                     Some restructering to remove complexity required by MShear, but
                     not 330.
    2 2006-12-30 rdr Fix compilation with OMIT_SEED defined.
    3 2007-03-09 rdr Fix botched (and possibly debauched) Pascal to C translation in proc_mult.
    4 2007-08-04 rdr Some foolishness to get around gcc-avr32 optimizer bugs when
                     calling "average".
    5 2008-01-10 rdr Move flushing of message LCQ into DP area.
    6 2008-03-13 rdr Add support for seperate event handling for archival and 512 byte.
                     Don't reset records_written at 999999.
    7 2009-04-06 rdr Fix Gap offset for channels decimated from Paros input.
    8 2009-09-05 rdr Ignore data for EP channels that don't yet have a valid delay.
    9 2010-03-27 rdr Fix building segmented data structure.
   10 2011-03-17 rdr For Q335 new usage of deb_flags.
   11 2011-09-22 rdr In process_mult make sure have first segment, if not then don't
                     call process_lcq.
*/
#ifndef libsample_h
#include "libsample.h"
#endif
#ifndef libsampglob_h
#include "libsampglob.h"
#endif
#ifndef libsampcfg_h
#include "libsampcfg.h"
#endif
#ifndef libcompress_h
#include "libcompress.h"
#endif
#ifndef libmsgs_h
#include "libmsgs.h"
#endif
#ifndef libcvrt_h
#include "libcvrt.h"
#endif

#ifndef OMIT_SEED
#ifndef libsupport_h
#include "libsupport.h"
#endif
#ifndef libfilters_h
#include "libfilters.h"
#endif
#ifndef libdetect_h
#include "libdetect.h"
#endif
#ifndef liblogs_h
#include "liblogs.h"
#endif
#ifndef libarchive_h
#include "libarchive.h"
#endif
#ifndef libctrldet_h
#include "libctrldet.h"
#endif
#ifndef libopaque_h
#include "libopaque.h"
#endif
#endif

longint sex (longint l)
begin

  if (l and 0x8000)
    then
      l = l or (longint)0xFFFF0000 ;
  return l ;
end

longint bsex (longint l)
begin

  if (l and 0x80)
    then
      l = l or (longint)0xFFFFFF00 ;
  return l ;
end

longint seqspread (longword new_, longword last)
begin
  longint d ;

  d = new_ - last ; /* returns a signed value, which takes care of 32-bit wrapping */
  return d ;
end

word translate_clock (tclock *qclock, word qual, word loss)
begin
  word val ;
  integer i ;

  val = 0 ;
  if ((qual >= PLL_TRACK) lor (qual and (CQ_3D or CQ_2D or CQ_1D)))
    then
      switch (qual and PLL_LOCK) begin
        case PLL_LOCK :
          val = qclock->q_locked ;
          break ;
        case PLL_TRACK :
          val = qclock->q_track ;
          break ;
        case PLL_HOLD :
          val = qclock->q_hold ;
          break ;
        case PLL_OFF :
          val = qclock->q_off ;
          break ;
      end
  else if (qual and CQ_LOCK)
    then
      begin
        if (qclock->degrade_time)
          then
            i = qclock->q_high - loss div qclock->degrade_time ;
          else
            i = qclock->q_high ;
        if (i < qclock->q_low)
          then
            i = qclock->q_low ;
        val = i ;
      end
    else
      val = qclock->q_never ;
  return val ;
end

#ifndef OMIT_SEED
static boolean detect_record (paqstruc paqs, pdet_packet det, plcq q)
begin
  boolean have_detection, result ;
  tonset_mh onset_save ;
  con_common *pcc ;

  pcc = det->cont ;
  have_detection = FALSE ;
  result = FALSE ;
  if (pcc->detector_enabled)
    then
      begin
        repeat
          if (((det->detector_def->dtype == MURDOCK_HUTT) land (E_detect (det))) lor
              ((det->detector_def->dtype == THRESHOLD) land (Te_detect (det))))
            then
              begin
                result = TRUE ;
                if ((pcc->new_onset) land (lnot have_detection))
                  then
                    begin
                      have_detection = TRUE ;
                      memcpy(addr(onset_save), addr(det->onset), sizeof(tonset_mh)) ;
                    end
              end
        until (lnot (det->remaining))) ;
        if (have_detection)
          then
            begin
              result = TRUE ;
              inc(pcc->total_detections) ;
              onset_save.signal_onset_time.seed_fpt = onset_save.signal_onset_time.seed_fpt - q->delay ;
              inc(q->detections_session) ;
              if (det->det_options and DO_LOG)
                then
                  logevent (paqs, det, addr(onset_save)) ;
            end
      end
  return result ;
end

void run_detector_chain (paqstruc paqs, plcq q, double rtime)
begin
  pdet_packet det ;
  con_common *pcc ;

  det = q->det ;
  while (det)
    begin
      pcc = det->cont ;
      pcc->startt = rtime ;
      if (pcc->detector_on)
        then
          begin
            pcc->detector_on = detect_record (paqs, det, q) ;
            if (lnot (pcc->detector_on))
              then
                pcc->first_detection = FALSE ;
          end
        else
          pcc->detector_on = detect_record (paqs, det, q) ;
      pcc->detection_declared = ((pcc->detector_on) land (lnot (pcc->first_detection))) ;
      det = det->link ;
    end
end

void set_timetag (plcq q, double *tt)
begin
  double correction, timetag_ ;
  double dprate, dpmark ;

  if (q->timetag < 1)
    then
      timetag_ = q->backup_tag ;
    else
      timetag_ = q->timetag ;
  dprate = q->rate ;
  dpmark = q->com->time_mark_sample ;
  if (q->rate > 0)
    then
      correction = - q->delay - (dpmark - 1) / dprate ;
    else
      correction = - q->delay - (dpmark - 1) * fabs(dprate) ;
  *tt = timetag_ + correction ;
end

void send_to_client (paqstruc paqs, plcq q, pcompressed_buffer_ring pbuf, byte dest)
begin
#define JAN_1_2006 189388800 /* first possible valid data */
#define MAX_DATE 0x7FFF0000 /* above this just has to be nonsense */
  pq330 q330 ;

  q330 = paqs->owner ;
  q330->miniseed_call.context = q330 ;
  memcpy(addr(q330->miniseed_call.station_name), addr(q330->station_ident), sizeof(string9)) ;
  strcpy(addr(q330->miniseed_call.location), addr(q->slocation)) ;
  strcpy(addr(q330->miniseed_call.channel), addr(q->sseedname)) ;
  q330->miniseed_call.chan_number = q->lcq_num ;
  q330->miniseed_call.rate = q->rate ;
  q330->miniseed_call.cl_session = 0 ;
  q330->miniseed_call.cl_offset = 0 ;
  q330->miniseed_call.timestamp = pbuf->hdr_buf.starting_time.seed_fpt ;
  if ((q330->miniseed_call.timestamp < JAN_1_2006) lor (q330->miniseed_call.timestamp > MAX_DATE))
    then
      return ; /* not possible */
  q330->miniseed_call.filter_bits = q->mini_filter ;
  q330->miniseed_call.packet_class = q->pack_class ;
  q330->miniseed_call.miniseed_action = MSA_512 ;
  q330->miniseed_call.data_size = LIB_REC_SIZE ;
  q330->miniseed_call.data_address = addr(pbuf->rec) ;
  if ((dest and SCD_512) land (q->mini_filter) land (q330->par_create.call_minidata))
    then
      q330->par_create.call_minidata (addr(q330->miniseed_call)) ;
  if ((dest and SCD_ARCH) land (q->arc.amini_filter) land (q->pack_class != PKC_EVENT) land
      (q->pack_class != PKC_CALIBRATE) land (q330->par_create.call_aminidata))
    then
      archive_512_record (paqs, q, pbuf) ;
end

void install_header (paqstruc paqs, plcq q, pcom_packet pcom)
begin
  pbyte p ;
  double dprate, dpsamps ;
  tclock *pclock ;
  pq330 q330 ;
  seed_header *phdr ;

  q330 = paqs->owner ;
  pclock = addr(q330->qclock) ;
  phdr = addr(pcom->ring->hdr_buf) ;
  p = (pointer)((pntrint)addr(pcom->ring->rec) + (pcom->blockette_count + 1) * FRAME_SIZE + 8) ; /* ending sample address */
  storelongint (addr(p), pcom->last_sample) ;
  phdr->samples_in_record = pcom->next_compressed_sample - 1 ;
  phdr->activity_flags = 0 ; /* Activity Flags set in finish/ringman */
  if (q->timequal >= pclock->q_off)
   then
     phdr->io_flags = SIF_LOCKED ;
   else
     phdr->io_flags = 0 ;
  if (q->timequal < pclock->q_low)
    then
      phdr->data_quality_flags = SQF_QUESTIONABLE_TIMETAG ;
    else
      phdr->data_quality_flags = 0 ;
  if (((q->raw_data_source and DCM) == DC_D32) land
     (paqs->calerr_bitmap and (1 shl (q->raw_data_source and 7))))
    then
      phdr->data_quality_flags = phdr->data_quality_flags or SQF_AMPERR ;
  if (pcom->charging)
    then
      begin
        phdr->data_quality_flags = phdr->data_quality_flags or SQF_CHARGING ;
        pcom->charging = FALSE ;
      end
  phdr->seed_record_type = 'D' ;
  phdr->continuation_record = ' ' ;
  phdr->sequence.seed_num = pcom->records_written + 1 ;
  memcpy(addr(phdr->location_id), addr(q->location), sizeof(tlocation)) ;
  memcpy(addr(phdr->channel_id), addr(q->seedname), sizeof(tseed_name)) ;
  memcpy(addr(phdr->station_id_call_letters), addr(q330->station), sizeof(tseed_stn)) ;
  memcpy(addr(phdr->seednet), addr(q330->network), sizeof(tseed_net)) ;
  phdr->sample_rate_factor = q->rate ;
  phdr->sample_rate_multiplier = 1 ;
  phdr->number_of_following_blockettes = pcom->blockette_count + 2 ;
  phdr->tenth_msec_correction = 0 ; /* this is a delta, which we don't */
  phdr->first_data_byte = (pcom->blockette_count + 1) * FRAME_SIZE ;
  phdr->first_blockette_byte = 48 ;
  if (q->timetag < 1)
    then
      begin
        q->timetag = q->backup_tag ;
        q->timequal = q->backup_qual ;
      end
  set_timetag (q, addr(phdr->starting_time.seed_fpt)) ;
  phdr->dob.blockette_type = 1000 ;
  phdr->dob.word_order = 1 ;
  phdr->dob.rec_length = RECORD_EXP ;
  phdr->dob.dob_reserved = 0 ;
  phdr->dob.next_blockette = 56 ;
  phdr->dob.encoding_format = 11 ;
  phdr->deb.blockette_type = 1001 ;
  if (pcom->blockette_count)
    then
      phdr->deb.next_blockette = 64 ;
    else
      phdr->deb.next_blockette = 0 ;
  if (q330->q335)
    then
      phdr->deb.deb_flags = DEB_NEWBITS or q->gain_bits ;
    else
      phdr->deb.deb_flags = DEB_Q330 or DEB_PB14_FIX ;
  if (q->lcq_opt and LO_EVENT)
    then
      phdr->deb.deb_flags = phdr->deb.deb_flags or DEB_EVENT_ONLY ;
  phdr->deb.qual = q->timequal ;
  phdr->deb.usec99 = 0 ; /* for now */
  phdr->deb.frame_count = pcom->frame - 1 - pcom->blockette_count ;
  dprate = q->rate ;
  dpsamps = pcom->next_compressed_sample - 1 ;
  if (q->rate >= 0.999)
    then
      q->backup_tag = q->timetag + dpsamps / dprate ;
    else
      q->backup_tag = q->timetag + dpsamps * fabs(dprate) ;
  q->backup_qual = q->timequal ;
  pcom->blockette_count = 0 ; /* no detection/cal blockettes */
  q->timequal = 0 ;
  q->timetag = 0 ; /* not set yet */
end

void ringman (paqstruc paqs, plcq q, boolean *now_on, boolean *was_on, longword *next)
begin
  pcompressed_buffer_ring lring ;
  boolean contig ;
  byte sohsave ;
  pbyte p ;

  if ((*now_on) lor (*was_on))
    then
      begin
        lring = q->com->ring ;
        contig = *was_on ;
        repeat
          if (lnot (contig))
            then
              lring = lring->link ; /* the first will be the oldest */
          if ((lring->full) land (lring->hdr_buf.sequence.seed_num >= *next))
            then
              begin
                sohsave = lring->hdr_buf.activity_flags ;
                if (lnot (*was_on)) /* if not time-contiguous */
                  then
                    lring->hdr_buf.activity_flags = lring->hdr_buf.activity_flags or (SAF_EVENT_IN_PROGRESS + SAF_BEGIN_EVENT) ;
                  else
                    lring->hdr_buf.activity_flags = lring->hdr_buf.activity_flags or SAF_EVENT_IN_PROGRESS ;
                *was_on = TRUE ; /* next are contiguous until event done */
                *next = lring->hdr_buf.sequence.seed_num + 1 ;
/* write the event blocks to the output files */
                p = addr(lring->rec) ;
                storeseedhdr (addr(p), addr(lring->hdr_buf), TRUE) ;
                if (q->scd_evt and SCD_ARCH)
                  then
                    begin /* archival output only */
                      inc(q->records_generated_session) ;
                      q->last_record_generated = secsince () ;
                    end
                send_to_client (paqs, q, lring, q->scd_evt) ;
                lring->hdr_buf.activity_flags = sohsave ;
              end
        until ((contig) lor (lring == q->com->ring))) ;
      end
  if (lnot (*now_on))
    then
      begin
        if ((q->scd_evt and SCD_ARCH) land (*was_on) land (q->arc.amini_filter))
          then
            flush_archive (paqs, q) ;
        *was_on = FALSE ; /* contiguity broken */
      end
end

void finish_record (paqstruc paqs, plcq q, pcom_packet pcom)
begin
  byte sohsave ;
  pbyte p ;
  pq330 q330 ;
  seed_header *phdr ;

  q330 = paqs->owner ;
  install_header (paqs, q, pcom) ;
  inc(pcom->records_written) ;
  if ((lnot (q->data_written)) land (q->lcq_num != 0xFF))
    then
      evaluate_detector_stack (q330, q) ;
  pcom->frame = 1 ;
  pcom->next_compressed_sample = 1 ; /* start of record */
/* turn on EVENT_IN_PROGRESS in current record if any detectors are on */
  phdr = addr(pcom->ring->hdr_buf) ;
  if (q->gen_on)
    then
      phdr->activity_flags = phdr->activity_flags or SAF_EVENT_IN_PROGRESS ;
  if ((lnot q->gen_on) land (q->gen_last_on))
    then
      phdr->activity_flags = phdr->activity_flags or SAF_END_EVENT ;
  if (q->cal_on)
    then
      phdr->activity_flags = phdr->activity_flags or SAF_CAL_IN_PROGRESS ;
/* write continuous data this comm links */
  if (q->scd_cont)
    then
      begin
        sohsave = phdr->activity_flags ;
        if ((q->gen_on) land (lnot q->gen_last_on))
          then
            phdr->activity_flags = phdr->activity_flags or (SAF_EVENT_IN_PROGRESS or SAF_BEGIN_EVENT) ;
        p = addr(pcom->ring->rec) ;
        storeseedhdr (addr(p), phdr, TRUE) ;
        if (q->scd_cont and SCD_ARCH)
          then
            begin /* archival output only */
              inc(q->records_generated_session) ;
              q->last_record_generated = secsince () ;
            end
        send_to_client (paqs, q, pcom->ring, q->scd_cont) ;
        phdr->activity_flags = sohsave ;
      end
  q->data_written = TRUE ;
/* flag the current record in the pre-event buffer ring as being full */
  pcom->ring->full = TRUE ;
  if (q->scd_evt)
    then
      ringman (paqs, q, addr(q->gen_on), addr(q->gen_last_on), addr(q->gen_next)) ;
  pcom->ring = pcom->ring->link ;
  memset(addr(pcom->ring->rec), 0, LIB_REC_SIZE) ;
  memset(addr(pcom->ring->hdr_buf), 0, sizeof(seed_header)) ;
  pcom->ring->full = FALSE ;
  q->gen_on = FALSE ; /* done with record, unlatch */
end

void flush_lcq (paqstruc paqs, plcq q, pcom_packet pcom)
begin
  integer used, loops ;
  pbyte p ;
  pq330 q330 ;
  string15 s ;

  if (pcom->peek_total > 0)
    then
      begin
        loops = 0 ;
        repeat
          used = compress_block (paqs, q, pcom) ;
          pcom->next_compressed_sample = pcom->next_compressed_sample + used ;
          if (pcom->frame >= (pcom->maxframes + 1))
            then
              finish_record (paqs, q, pcom) ;
          inc(loops) ;
        until ((pcom->peek_total <= 0) lor (loops > 20))) ;
        if (loops > 20)
          then
            begin
              q330 = paqs->owner ;
              seed2string(q->location, q->seedname, addr(s)) ;
              libmsgadd (q330, LIBMSG_LBFAIL, addr(s)) ;
            end
      end
  if (pcom->next_compressed_sample > 1)
    then
      begin
        if ((pcom->block > 1) land (pcom->block < WORDS_PER_FRAME))
          then
            begin /* complete partial frame */
              pcom->flag_word = pcom->flag_word shl ((WORDS_PER_FRAME - pcom->block) shl 1) ;
              pcom->frame_buffer[0] = pcom->flag_word ;
              p = (pointer)((pntrint)addr(pcom->ring->rec) + pcom->frame * FRAME_SIZE) ;
              storeframe (addr(p), addr(pcom->frame_buffer)) ;
              memset(addr(pcom->frame_buffer), 0, sizeof(compressed_frame)) ;
              inc(pcom->frame) ;
              pcom->block = 1 ;
              pcom->flag_word = 0 ;
            end
        finish_record (paqs, q, pcom) ;
      end
  if ((q->arc.amini_filter) land (q->arc.total_frames > 1))
    then
      flush_archive (paqs, q) ;
end

void add_blockette (paqstruc paqs, plcq q, pword pw, double time)
begin
  pbyte psrc, pdest, plink ;
  tcom_packet *pcom ;

  pcom = q->com ;
  if (pcom->frame >= pcom->maxframes) /* working on last one */
    then
      flush_lcq (paqs, q, pcom) ; /* need to start a new record */
  if (pcom->frame == 1)
    then
      pcom->ring->hdr_buf.starting_time.seed_fpt = time ; /* make sure there is a time */
  if (pcom->frame > (pcom->blockette_count + 1))
    then
      begin
        psrc = (pointer)((pntrint)addr(pcom->ring->rec) + FRAME_SIZE * (pcom->blockette_count + 1)) ;
        pdest = (pointer)((pntrint)addr(pcom->ring->rec) + FRAME_SIZE * (pcom->blockette_count + 2)) ;
        memmove (pdest, psrc, FRAME_SIZE * (pcom->frame - pcom->blockette_count - 1)) ;
      end
  pdest = (pointer)((pntrint)addr(pcom->ring->rec) + FRAME_SIZE * (pcom->blockette_count + 1)) ;
  memcpy (pdest, pw, FRAME_SIZE) ;
  if (pcom->blockette_count)
    then
      begin /* extended link from previous blockette */
        plink = (pointer)((pntrint)addr(pcom->ring->rec) + FRAME_SIZE * pcom->blockette_count + 2) ;
        storeword (addr(plink), FRAME_SIZE * (pcom->blockette_count + 1)) ;
      end
  inc(pcom->blockette_count) ;
  inc(pcom->frame) ;
  if (pcom->frame >= (pcom->maxframes + 1))
    then
      finish_record (paqs, q, pcom) ;
end

void build_separate_record (paqstruc paqs, plcq q, pword pw,
                            double time, enum tpacket_class pclass)
begin
  pbyte pdest, p ;
  pq330 q330 ;
  seed_header *phdr ;

  q330 = paqs->owner ;
  memset (addr(paqs->detcal_buf), 0, sizeof(tcompressed_buffer_ring)) ;
  phdr = addr(paqs->detcal_buf.hdr_buf) ;
  phdr->starting_time.seed_fpt = time ;
  pdest = (pointer)((pntrint)addr(paqs->detcal_buf.rec) + FRAME_SIZE) ;
  memcpy (pdest, pw, FRAME_SIZE) ;
  phdr->activity_flags = SAF_BEGIN_EVENT ;
  phdr->seed_record_type = 'D' ;
  phdr->continuation_record = ' ' ;
  phdr->sequence.seed_num = 1 ;
  memcpy(addr(phdr->location_id), addr(q->location), sizeof(tlocation)) ;
  memcpy(addr(phdr->channel_id), addr(q->seedname), sizeof(tseed_name)) ;
  memcpy(addr(phdr->station_id_call_letters), addr(q330->station), sizeof(tseed_stn)) ;
  memcpy(addr(phdr->seednet), addr(q330->network), sizeof(tseed_net)) ;
  phdr->sample_rate_factor = q->rate ;
  phdr->sample_rate_multiplier = 1 ;
  phdr->number_of_following_blockettes = 2 ;
  phdr->tenth_msec_correction = 0 ; /* this is a delta, which we don't */
  phdr->first_blockette_byte = 48 ;
  phdr->dob.blockette_type = 1000 ;
  phdr->dob.word_order = 1 ;
  phdr->dob.rec_length = RECORD_EXP ;
  phdr->dob.dob_reserved = 0 ;
  phdr->dob.next_blockette = 64 ;
  phdr->dob.encoding_format = 11 ;
  p = addr(paqs->detcal_buf.rec) ;
  storeseedhdr (addr(p), phdr, FALSE) ;
  q->pack_class = pclass ; /* override normal data class */
  send_to_client (paqs, q, addr(paqs->detcal_buf), SCD_BOTH) ;
  q->pack_class = PKC_DATA ; /* restore to normal */
end

void set_slip (paqstruc paqs, plcq q)
begin
  pdownstream_packet down ;
  plcq p ;

  down = q->downstream_link ;
  while (down)
    begin
      p = down->derived_q ;
      if (p->slip_modulus)
        then
          begin
            flush_lcq (paqs, p, p->com) ;
            p->slipping = TRUE ;
            /* must restore fir filter to waiting to start state */
            if (p->fir)
              then
                begin
                  p->fir->fcount = p->fir->flen - 1 ;
                  p->fir->f = (pointer)((pntrint)p->fir->fbuf + (p->fir->flen - 1) * sizeof(tfloat)) ;
                end
          end
      set_slip (paqs, p) ; /* recursive */
      down = down->link ;
    end
end
#endif

void process_lcq (paqstruc paqs, plcq q, integer src_samp, tfloat dv)
begin
  string95 s ;
  string31 s1, s2 ;
  volatile integer samples ; /* gcc kept cloberring this without volatile */
  integer i ;
  longint dsamp ;        /* temp integer sample */
  tfloat sf ;
  plong p1 ;
#ifndef OMIT_SEED
  integer used ;
  longint int_time ; /* integer equivalent of sample time */
  piirfilter pi ;
  pdownstream_packet down ;
  pfloat p2 ;
  tfir_packet *pfir ;
#endif
  pq330 q330 ;

  q330 = paqs->owner ;
  if ((q->raw_data_source == (DC_SPEC + 4)) land (q->delay == 0.0))
    then
      return ; /* don't have a valid delay value yet */
#ifndef OMIT_SEED
  q->data_written = FALSE ;
  if (q->slipping)
    then
      begin /* for derived, see if reached modulus */
        int_time = lib_round((src_samp - 1) * q->input_sample_rate + paqs->data_timetag) ;
        if ((int_time mod q->slip_modulus) == (q->slip_modulus - 1))
          then
            q->slipping = FALSE ;
          else
            return ;
      end
  if (paqs->data_timetag < 1)
    then
      q->com->charging = TRUE ;
#endif
  if (abs(src_samp) <= 1)
    then
      begin
        q->timemark_occurred = TRUE ; /* fist sample */
        if ((q->last_timetag > 1) land (fabs(paqs->data_timetag - q->last_timetag - q->gap_offset) > q->gap_secs))
          then
            begin
              if (q330->cur_verbosity and VERB_LOGEXTRA)
                then
                  begin
                    sprintf(s, "%s %s", seed2string(q->location, q->seedname, addr(s1)),
                            realtostr(paqs->data_timetag - q->last_timetag - q->gap_offset, 6, addr(s2))) ;
                    libdatamsg (q330, LIBMSG_TIMEDISC, addr(s)) ;
                  end
#ifndef OMIT_SEED
              flush_lcq (paqs, q, q->com) ; /* gap in the data */
              set_slip (paqs, q) ;
#endif
            end
        q->last_timetag = paqs->data_timetag ;
        q->dtsequence = paqs->dt_data_sequence ;
      end
  dsamp = 0 ;
  sf = 0.0 ;
#ifndef OMIT_SEED
  pi = q->stream_iir ;
#endif
  if (src_samp > 0) /* only done due to decimation from a higher rate */
    then
      begin
        samples = 1 ;
#ifndef OMIT_SEED
        pfir = q->fir ;
        if (pfir)
          then
            begin /*this only processes one sample at a time, it's probably <=1hz anyway*/
              *(pfir->f) = dv ;
              inc(pfir->f) ;
              inc(pfir->fcount) ;
              if (pfir->fcount < pfir->flen)
                then
                  return ;
/*--------------------------------------------------------------------------------------
 This convolution may appear backwards, but for non-symetrical filters, the coefficients
 are defined in the reverse order, to match the reverse order of the input values
---------------------------------------------------------------------------------------*/
              sf = mac_and_shift (pfir) ;
              decn(pfir->f, pfir->fdec) ;
              decn(pfir->fcount, pfir->fdec) ; ;
              sf = sf * q->firfixing_gain ;
              q->processed_stream = sf ;
              dsamp = lib_round(sf) ;
              while (pi)
                begin
                  p2 = addr(pi->out) ;
                  *p2 = multi_section_filter (pi, sf) ;
                  pi = pi->link ;
                end
            end
          else
            return ; /* nothing to */
#endif
      end
  else if (src_samp == 0)
    then
      begin /* 1hz or lower */
        samples = 1 ;
#ifndef OMIT_SEED
        while (pi)
          begin
            p1 = (pointer)q->databuf ;
            p2 = addr(pi->out) ;
            sf = *p1 ; /* convert to floating point */
            *p2 = multi_section_filter (pi, sf) ;
            pi = pi->link ;
          end
#endif
      end
    else
      begin /* pre-compressed data */
        samples = decompress_blockette (paqs, q) ;
#ifndef OMIT_SEED
        while (pi)
          begin
            p1 = (pointer)q->databuf ;
            p2 = addr(pi->out) ;
            for (i = 1 ; i <= samples ; i++)
              begin
                sf = *p1 ; /* convert to floating point */
                *p2 = multi_section_filter (pi, sf) ;
                inc(p2) ;
                inc(p1) ;
              end
            pi = pi->link ;
          end
#endif
      end
#ifndef OMIT_SEED
  run_detector_chain (paqs, q, paqs->data_timetag) ;
#endif
  if (q->calstat)
    then
      begin
        q->cal_on = q->calstat ;
        q->calinc = 0 ;
      end
  else if ((q->cal_on) land (lnot q->calstat))
    then
      begin
        inc(q->calinc) ;
        if (q->calinc > q->caldly)
          then
            q->cal_on = q->calstat ;
      end
#ifndef OMIT_SEED
/* Process misc. flags that must be processed a sample at a time */
  if ((q->downstream_link) lor (q->avg_filt))
    then
      begin
        p1 = (pointer)q->databuf ;
        p2 = NIL ;
        if (q->avg_filt)
          then
            p2 = addr(q->avg_filt->out) ;
        for (i = 1 ; i <= samples ; i++)
          begin
            if (src_samp <= 0)
              then
                begin
                  dsamp = *p1 ;
                  sf = dsamp ;
                end
            down = q->downstream_link ;
            while (down)
              begin
                process_lcq (paqs, down->derived_q, i, sf) ;
                down = down->link ;
              end
            if (q->avg_filt)
              then
                begin
                  average (paqs, q->avg, sf, *p2, q) ;
                  inc(p2) ;
                end
            inc(p1) ;
          end
      end
#endif
  if (q->lcq_opt and LO_NOUT)
    then
      return ; /* not generating any data */
  if (q->timemark_occurred)
    then
      begin
        q->timemark_occurred = FALSE ;
        if ((paqs->data_qual > q->timequal) lor (q->timetag == 0))
          then
            begin
              q->timetag = paqs->data_timetag ;
              q->timequal = paqs->data_qual ;
#ifndef OMIT_SEED
              q->com->time_mark_sample = q->com->peek_total + q->com->next_compressed_sample ;
#endif
            end
      end
  p1 = (pointer)q->databuf ;
  if (q->onesec_filter)
    then
      begin
        q330->onesec_call.total_size = sizeof(tonesec_call) - ((MAX_RATE - samples) * sizeof(longint)) ;
        q330->onesec_call.context = q330 ;
        memcpy(addr(q330->onesec_call.station_name), addr(q330->station_ident), sizeof(string9)) ;
        strcpy(addr(q330->onesec_call.location), addr(q->slocation)) ;
        strcpy(addr(q330->onesec_call.channel), addr(q->sseedname)) ;
        q330->onesec_call.chan_number = q->lcq_num ;
        q330->onesec_call.cl_session = 0 ;
        q330->onesec_call.cl_offset = 0 ;
        q330->onesec_call.timestamp = paqs->data_timetag - q->delay ;
        q330->onesec_call.qual_perc = paqs->data_qual ;
        q330->onesec_call.filter_bits = q->onesec_filter ;
        q330->onesec_call.rate = q->rate ;
        q330->onesec_call.activity_flags = 0 ;
#ifndef OMIT_SEED
        if (q->gen_on)
          then
            q330->onesec_call.activity_flags = q330->onesec_call.activity_flags or SAF_EVENT_IN_PROGRESS ;
#endif
        if (q->cal_on)
          then
            q330->onesec_call.activity_flags = q330->onesec_call.activity_flags or SAF_CAL_IN_PROGRESS ;
        if (paqs->data_qual >= q330->qclock.q_off)
         then
           q330->onesec_call.io_flags = SIF_LOCKED ;
         else
           q330->onesec_call.io_flags = 0 ;
        if (paqs->data_qual < q330->qclock.q_low)
          then
            q330->onesec_call.data_quality_flags = SQF_QUESTIONABLE_TIMETAG ;
          else
            q330->onesec_call.data_quality_flags = 0 ;
        q330->onesec_call.src_channel = q->raw_data_source ;
        q330->onesec_call.src_subchan = q->raw_data_field ;
        if ((samples > 1) land (src_samp < 0))
          then
            memcpy (addr(q330->onesec_call.samples), q->databuf, samples * sizeof(longint)) ;
      end
  if (src_samp >= 0)
    then
      begin /* data hasn't been pre-compressed */
#ifndef OMIT_SEED
        if (src_samp)
          then
            q->com->peeks[q->com->next_in] = dsamp ;
          else
            q->com->peeks[q->com->next_in] = *p1 ;
        q330->onesec_call.samples[0] = q->com->peeks[q->com->next_in] ;
        q->com->next_in = (q->com->next_in + 1) and (PEEKELEMS - 1) ;
        inc(q->com->peek_total) ;
#else
        if (src_samp)
          then
            q330->onesec_call.samples[0] = dsamp ;
          else
            q330->onesec_call.samples[0] = *p1 ;
#endif
        if (q->onesec_filter)
          then
            q330->par_create.call_secdata (addr(q330->onesec_call)) ;
#ifndef OMIT_SEED
        if (q->com->peek_total < MAXSAMPPERWORD)
          then
            return ;
        used = compress_block (paqs, q, q->com) ;
        q->com->next_compressed_sample = q->com->next_compressed_sample + used ;
        if (q->com->frame >= (q->com->maxframes + 1))
          then
            finish_record (paqs, q, q->com) ;
#endif
      end
    else
      begin
#ifndef OMIT_SEED
        while (samples > 0)
          begin
            used = build_blocks (paqs, q, q->com) ;
            if (used == 0)
              then
                begin
                  sprintf(s, "%s, %d samples left", seed2string(q->location, q->seedname, addr(s1)), samples) ;
                  libdatamsg (q330, LIBMSG_RECOMP, addr(s)) ;
                  samples = 0 ;
                end
              else
                begin
                  q->com->next_compressed_sample = q->com->next_compressed_sample + used ;
                  if (q->com->frame >= (q->com->maxframes + 1))
                    then
                      finish_record (paqs, q, q->com) ;
                end
            samples = samples - used ;
          end
#else
        samples = 0 ;
#endif
        if (q->onesec_filter)
          then
            q330->par_create.call_secdata (addr(q330->onesec_call)) ;
      end
end

#ifndef OMIT_SEED
void flush_lcqs (paqstruc paqs)
begin
  plcq q ;

  q = paqs->lcqs ;
  while (q)
    begin
      switch (q->pack_class) begin
        case PKC_OPAQUE :
          if (q->raw_data_source == (DC_SPEC + 2))
            then
              flush_cnp (paqs, q, q->com) ;
            else
              flush_cfgblks (paqs, TRUE) ;
          break ;
        case PKC_TIMING :
          flush_timing (paqs) ;
          break ;
        case PKC_DATA :
          if ((q->lcq_opt and LO_NOUT) == 0)
            then
              flush_lcq (paqs, q, q->com) ;
          break ;
      end
      q = q->link ;
    end
end

void flush_dplcqs (pq330 q330)
begin
  plcq q ;
  paqstruc paqs ;

  paqs = q330->aqstruc ;
  q = paqs->dplcqs ;
  while (q)
    begin
      switch (q->pack_class) begin
        case PKC_DATA :
          flush_lcq (paqs, q, q->com) ;
          break ;
        case PKC_MESSAGE :
          flush_messages (paqs) ;
          break ;
      end
      q = q->link ;
    end
end
#endif

void process_one (pq330 q330, longint data)
begin
  paqstruc paqs ;
  plcq q ;
  string15 s ;

  paqs = q330->aqstruc ;
  q = paqs->proc_lcq ;
  if (paqs->data_timetag < 1)
    then
      begin
        libdatamsg (q330, LIBMSG_DISCARD, seed2string(q->location, q->seedname, addr(s))) ;
        return ; /* don't know what time it is yet */
      end
  if (seqspread (paqs->dt_data_sequence, q->dtsequence) <= 0) /* has this second of data already been processed? */
    then
      begin
        if (abs(seqspread(paqs->dt_data_sequence, q->dtsequence)) > MAXSPREAD)
          then
            q->dtsequence = paqs->dt_data_sequence ; /* if we're not within continuity distance, forget it */
        return ;
      end
  (*(q->databuf))[0] = data ; /* just one data point */
  process_lcq (paqs, q, 0, 0.0) ;
end

void process_comp (pq330 q330, pbyte p, integer size)
begin
  paqstruc paqs ;
  pbyte psave ;
  word offset ;
  plcq q ;
  tprecomp *pcmp ;
  string15 s ;

  psave = p ;
  decn(psave, 4) ; /* we have already read the first four bytes */
  paqs = q330->aqstruc ;
  q = paqs->proc_lcq ;
  pcmp = addr(q->precomp) ;
  if (paqs->data_timetag < 1)
    then
      begin
        libdatamsg (q330, LIBMSG_DISCARD, seed2string(q->location, q->seedname, addr(s))) ;
        return ; /* don't know what time it is yet */
      end
  if (seqspread (paqs->dt_data_sequence, q->dtsequence) <= 0) /* has this second of data already been processed? */
    then
      begin
        if (abs(seqspread(paqs->dt_data_sequence, q->dtsequence)) > MAXSPREAD)
          then
            q->dtsequence = paqs->dt_data_sequence ; /* if we're not within continuity distance, forget it */
        return ;
      end
  pcmp->prev_sample = loadlongint (addr(p)) ;
  offset = loadword (addr(p)) ;
  pcmp->pmap = p ; /* flags start here */
  incn(psave, offset) ;
  pcmp->pdata = psave ; /* data starts here */
  pcmp->blocks = (size - offset) shr 2 ; /* number of 32 bit blocks */
  pcmp->mapidx = 0 ;
  process_lcq (paqs, q, -1, 0.0) ;
end

void process_mult (pq330 q330, pbyte psave, longword seq)
begin
  paqstruc paqs ;
  word offset ;
  psegment_ring ps, lps ;
  byte seg_freq ;
  boolean have_first ;
  word size ;
  pbyte p ;
  plcq q ;
  tprecomp *pcmp ;
  string15 s ;

  p = psave ;
  paqs = q330->aqstruc ;
  loadbyte (addr(p)) ;
  seg_freq = loadbyte (addr(p)) ;
  size = loadword (addr(p)) ;
  q = paqs->proc_lcq ;
  pcmp = addr(q->precomp) ;
  seed2string(q->location, q->seedname, addr(s)) ;
  if (paqs->data_timetag < 1)
    then
      begin
        libdatamsg (q330, LIBMSG_DISCARD, addr(s)) ;
        return ; /* don't know what time it is yet */
      end
  if (seqspread (paqs->dt_data_sequence, q->dtsequence) <= 0) /* has this second of data already been processed? */
    then
      begin
        if (abs(seqspread(paqs->dt_data_sequence, q->dtsequence)) > MAXSPREAD)
          then
            q->dtsequence = paqs->dt_data_sequence ; /* if we're not within continuity distance, forget it */
        return ;
      end
  if (seq != q->seg_seq)
    then
      begin /* purge old data */
        q->seg_next = q->segbuf ;
        q->pseg = NIL ;
        memset(q->segbuf, 0, q->segsize) ;
        q->seg_count = 0 ;
        q->seg_high = 0 ;
      end
  q->seg_seq = seq ;
  if ((seg_freq and 0xF8) == 0)
    then
      begin /* first segment is actually type pdp_comp */
        memcpy (addr(q->seg_next->seg), psave, size) ;
        if (q->pseg)
          then
            q->seg_next->link = q->pseg ; /* was already something there */
        q->pseg = q->seg_next ; /* first in list */
        q->seg_next = (pointer)((pntrint)q->seg_next + size + sizeof(pointer)) ;
        if (((pntrint)q->seg_next - (pntrint)q->segbuf) > q->segsize)
          then
            libdatamsg (q330, LIBMSG_SEGOVER, addr(s)) ;
        inc(q->seg_count) ;
      end
    else
      begin /* following segments */
        if (size and DMLS)
          then
            begin
              q->seg_high = (seg_freq shr 3) + 1 ; /* this is number of last segment */
              if ((q->seg_count == 0) land (q->dholdq) land (q->dholdq->ppkt == NIL))
                                                     /* tests for no segments yet received wotks with 2 */
                then /* if last segment received out-of-order */
                  begin /* enqueue this pkt, set ppkt non-NIL, and return */
                    q->dholdq->ppkt = addr(q->dholdq->pkt) ;
                    memcpy (addr(q->dholdq->pkt), psave, size and DMSZ) ;
                  end
            end
        size = size and DMSZ ;
        memcpy (addr(q->seg_next->seg), psave, size) ;
        ps = q->pseg ;
        lps = NIL ;
        while (ps)
          begin
            if (ps->seg.seg_freq > q->seg_next->seg.seg_freq)
              then
                begin
                  q->seg_next->link = ps ;
                  if (lps == NIL)
                    then
                      q->pseg = q->seg_next ;
                    else
                      lps->link = q->seg_next ;
                  break ; /* done */
                end
            lps = ps ;
            ps = ps->link ;
          end
        if (ps == NIL)
          then /* this is new highest */
            if (lps)
              then
                lps->link = q->seg_next ; /* add this to end of current list */
              else
                q->pseg = q->seg_next ; /* first one in list */
        q->seg_next = (pointer)((pntrint)q->seg_next + size + sizeof(pointer)) ;
        if (((pntrint)q->seg_next - (pntrint)q->segbuf) > q->segsize)
          then
            libdatamsg (q330, LIBMSG_SEGOVER, addr(s)) ;
        inc(q->seg_count) ;
      end
  ps = q->pseg ;
  have_first = FALSE ;
  if ((q->seg_high) land (q->seg_high == q->seg_count))
    then
      begin
        pcmp->blocks = 0 ;
        while (ps)
          begin
            p = (pointer)addr(ps->seg) ;
            loadbyte (addr(p)) ;
            seg_freq = loadbyte (addr(p)) ;
            size = loadword (addr(p)) and DMSZ ;
            if (seg_freq <= 7) /* first one is COMP type, not MULT. */
              then
                begin
                  pcmp->prev_sample = loadlongint (addr(p)) ;
                  offset = loadword (addr(p)) ;
                  pcmp->pmap = p ; /* flags start here */
                  pcmp->pdata = (pointer)q->mergedbuf ;
                  pcmp->mapidx = 0 ;
                  p = (pointer)((pntrint)addr(ps->seg) + offset) ; /* data starts here */
                  memcpy (addr((*(q->mergedbuf))[0]), p, size - offset) ;
                  pcmp->blocks = (size - offset) shr 2 ; /* number of 32 bit blocks so far */
                  have_first = TRUE ;
                end
              else
                begin
                  memcpy (addr((*(q->mergedbuf))[pcmp->blocks]), p, size - 4) ;
                  pcmp->blocks = pcmp->blocks + ((size - 4) shr 2) ; /* add in more 32 bit blocks */
                end
            ps = ps->link ;
          end
        q->seg_count = 0 ;
        q->seg_seq = 0xFFFFFFFF ; /* flag as all used up */
        if (have_first)
          then
            process_lcq (paqs, q, -1, 0.0) ;
      end
  if (q->dholdq)
    then
      q->dholdq->ppkt = NIL ;
end

void process_variable (pq330 q330, integer sps, integer dly5ms, longint data)
begin
#ifndef OMIT_SEED
  pdownstream_packet down ;
  plcq p ;
  single r ;
#endif
  paqstruc paqs ;
  plcq q ;
  string15 s ;
  integer adjrate ;

  paqs = q330->aqstruc ;
  q = paqs->proc_lcq ;
  if (paqs->data_timetag < 1)
    then
      begin
        libdatamsg (q330, LIBMSG_DISCARD, seed2string(q->location, q->seedname, addr(s))) ;
        return ; /* don't know what time it is yet */
      end
  if (seqspread (paqs->dt_data_sequence, q->dtsequence) <= 0) /* has this second of data already been processed? */
    then
      begin
        if (abs(seqspread(paqs->dt_data_sequence, q->dtsequence)) > MAXSPREAD)
          then
            q->dtsequence = paqs->dt_data_sequence ; /* if we're not within continuity distance, forget it */
        return ;
      end
  if (sps == 1)
    then
      adjrate = 1 ;
    else
      adjrate = -sps ; /* for sub-hertz represent as negative */
  if ((q->variable_rate_set == 0) lor (q->rate != adjrate))
    then
      begin /* not setup yet or has changed from previous data */
        q->variable_rate_set = TRUE ;
        q->rate = adjrate ;
        q->delay = dly5ms * 0.005 ; /* in seconds */
        set_gaps (q) ;
#ifndef OMIT_SEED
        down = q->downstream_link ;
        while (down)
          begin
            p = down->derived_q ;
            if (p->prev_link->rate > 0)
              then
                p->input_sample_rate = p->prev_link->rate ;
              else
                p->input_sample_rate = 1.0 / abs(p->prev_link->rate) ;
            if (p->source_fir)
              then
                r = p->input_sample_rate / p->source_fir->dec ; /* output rate */
              else
                r = p->input_sample_rate ; /* no decimation, so what's the point? */
            if (r >= 0.999)
              then
                p->rate = lib_round(r) ; /* rounded hertz and about */
              else
                p->rate = -lib_round(1.0 / r) ; /* sub-hertz */
            set_gaps (p) ;
            if (p->input_sample_rate >= 0.999)
              then
                p->gap_offset = 1.0 ;
              else
                p->gap_offset = 1.0 / p->input_sample_rate ; /* set new gap offset based on input rate */
            if (p->source_fir)
              then
                p->delay = p->prev_link->delay + (p->source_fir->dly / p->input_sample_rate) ;
              else
                p->delay = p->prev_link->delay ;
            down = down->link ;
          end
#endif
      end
  (*(q->databuf))[0] = data ;
  process_lcq (paqs, q, 0, 0.0) ;
end
