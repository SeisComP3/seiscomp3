/*   Lib330 Archival Miniseed Routines
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
    0 2006-10-13 rdr Created
    1 2006-11-28 rdr Don't "or" in SQF_QUESTIONABLE_TIMETAG when appending data. Unless
                     it's set by the first 512 record in the larger record, it doesn't
                     matter. Clear backup_tag and backup_qual if not extending a data record.
                     Don't extend data records if the continiuity is not good.
    2 2007-03-02 rdr Since "appended" wasn't check before make a slight change in usage to
                     indicate that new data has been added since last written to client.
                     If no new data hasn't been written to an existing record don't bother
                     client with useless update.
    3 2008-02-11 rdr Adjust first_data_byte in seed header when data gets moved to make
                     room for more blockettes.
    4 2008-03-13 rdr Don't reset records_written at 999999.  Don't set records_written from
                     last data record. If not set by continuity it's OK to start over since
                     SEED sequence numbers are informational only.
    5 2009-06-25 rdr Increment blockette count when appending timing blockettes.
*/
#ifndef OMIT_SEED
#ifndef libarchive_h
#include "libarchive.h"
#endif
#ifndef libmsgs_h
#include "libmsgs.h"
#endif
#ifndef libseed_h
#include "libseed.h"
#endif
#ifndef libdetect_h
#include "libdetect.h"
#endif
#ifndef libcvrt_h
#include "libcvrt.h"
#endif
#ifndef libctrldet_h
#include "libctrldet.h"
#endif
#ifndef libcompress_h
#include "libcompress.h"
#endif
#ifndef libsampcfg_h
#include "libsampcfg.h"
#endif
#ifndef libfilters_h
#include "libfilters.h"
#endif
#ifndef libopaque_h
#include "libopaque.h"
#endif
#ifndef liblogs_h
#include "liblogs.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif
#ifndef libsample_h
#include "libsample.h"
#endif

static void clear_archive (tarc *parc, integer size)
begin

  parc->appended = FALSE ;
  parc->existing_record = FALSE ;
  parc->total_frames = 0 ;
  memset(parc->pcfr, 0, size) ;
  memset(addr(parc->hdr_buf), 0, sizeof(seed_header)) ;
end

void flush_archive (paqstruc paqs, plcq q)
begin
#define JAN_1_2006 189388800 /* first possible valid data */
#define MAX_DATE 0x7FFF0000 /* above this just has to be nonsense */
  pq330 q330 ;
  pbyte p ;
  tarc *parc ;

  q330 = paqs->owner ;
  parc = addr(q->arc) ;
  p = (pointer)parc->pcfr ; /* start of record */
  q330->miniseed_call.timestamp = parc->hdr_buf.starting_time.seed_fpt ;
  if ((q330->miniseed_call.timestamp < JAN_1_2006) lor (q330->miniseed_call.timestamp > MAX_DATE))
    then
      begin
        clear_archive (parc, paqs->arc_size) ;
        return ; /* impossible time */
      end
  if (((q->pack_class == PKC_MESSAGE) land (parc->hdr_buf.samples_in_record == 0)) lor
      ((q->pack_class != PKC_MESSAGE) land (parc->total_frames == 0)))
    then
      begin
        clear_archive (parc, paqs->arc_size) ;
        return ; /* nothing to write */
      end
  storeseedhdr (addr(p), addr(parc->hdr_buf), q->pack_class == PKC_DATA) ; /* make sure is current */
  q330->miniseed_call.context = q330 ;
  memcpy(addr(q330->miniseed_call.station_name), addr(q330->station_ident), sizeof(string9)) ;
  strcpy(addr(q330->miniseed_call.location), addr(q->slocation)) ;
  strcpy(addr(q330->miniseed_call.channel), addr(q->sseedname)) ;
  q330->miniseed_call.chan_number = q->lcq_num ;
  q330->miniseed_call.rate = q->rate ;
  q330->miniseed_call.cl_session = 0 ;
  q330->miniseed_call.cl_offset = 0 ;
  q330->miniseed_call.filter_bits = parc->amini_filter ;
  q330->miniseed_call.packet_class = q->pack_class ;
  if (parc->existing_record)
    then
      if (parc->appended)
        then
          begin
            inc(parc->records_overwritten_session) ;
            if (parc->leave_in_buffer)
              then
                q330->miniseed_call.miniseed_action = MSA_INC ; /* middle of increments */
              else
                q330->miniseed_call.miniseed_action = MSA_FINAL ; /* last increment */
          end
        else
          begin /* client is up to date, leave alone */
            parc->leave_in_buffer = FALSE ;
            clear_archive (parc, paqs->arc_size) ;
            return ;
          end
    else
      begin
        inc(parc->records_written_session) ;
        if (parc->leave_in_buffer)
          then
            q330->miniseed_call.miniseed_action = MSA_FIRST ; /* incremental new record */
          else
            q330->miniseed_call.miniseed_action = MSA_ARC ; /* non-incremental new record */
      end
  parc->last_updated = secsince () ;
  q330->miniseed_call.data_size = paqs->arc_size ;
  q330->miniseed_call.data_address = parc->pcfr ;
  if (q330->par_create.call_aminidata)
    then
      q330->par_create.call_aminidata (addr(q330->miniseed_call)) ;
  if (parc->leave_in_buffer)
    then
      parc->existing_record = TRUE ; /* has been sent to client once */
    else
      clear_archive (parc, paqs->arc_size) ; /* starting over */
  parc->leave_in_buffer = FALSE ;
  parc->appended = FALSE ; /* client is up to date */
end

void archive_512_record (paqstruc paqs, plcq q, pcompressed_buffer_ring pbuf)
begin
  pq330 q330 ;
  double drate, tdiff ;
  integer fcnt, bcnt, dbcnt, i ;
  pbyte psrc, pdest, plink, plast ;
  integer size, src, dest, next, offset ;
  tarc *parc ;

  q330 = paqs->owner ;
  parc = addr(q->arc) ;
  switch (q->pack_class) begin
    case PKC_DATA :
      if (q->rate > 0)
        then
          drate = q->rate ;
      else if (q->rate < 0)
        then
          drate = 1.0 / abs(q->rate) ;
        else
          return ; /* zero is not a valid data rate */
      bcnt = pbuf->hdr_buf.number_of_following_blockettes - 2 ;
      fcnt = pbuf->hdr_buf.deb.frame_count ;
      dbcnt = parc->hdr_buf.number_of_following_blockettes - 2 ; /* already there */
      if (parc->total_frames > 1)
        then
          begin /* check for gaps */
            tdiff = pbuf->hdr_buf.starting_time.seed_fpt -
                    (parc->hdr_buf.starting_time.seed_fpt + parc->hdr_buf.samples_in_record / drate) ;
            if (((bcnt + fcnt + parc->total_frames) > paqs->arc_frames) lor (fabs(tdiff) > q->gap_secs))
              then /* won't fit or time gap */
                flush_archive (paqs, q) ;
          end
      psrc = (pointer)((pntrint)addr(pbuf->rec) + FRAME_SIZE) ;
      if (parc->total_frames > 1)
        then
          begin /* append to existing record */
            if (bcnt > 0)
              then
                begin /* need to insert one or more blockettes before data */
                  if (parc->total_frames > (dbcnt + 1))
                    then
                      begin /* need to move some data to further in record */
                        psrc = (pointer)((pntrint)parc->pcfr + FRAME_SIZE * (dbcnt + 1)) ;
                        pdest = (pointer)((pntrint)parc->pcfr + FRAME_SIZE * (dbcnt + bcnt + 1)) ;
                        memmove (pdest, psrc, FRAME_SIZE * (parc->total_frames - dbcnt - 1)) ;
                        incn(parc->hdr_buf.first_data_byte, bcnt * FRAME_SIZE) ;
                      end
                  /* copy new blockettes in archive record */
                  psrc = (pointer)((pntrint)addr(pbuf->rec) + FRAME_SIZE) ;
                  pdest = (pointer)((pntrint)parc->pcfr + FRAME_SIZE * (dbcnt + 1)) ;
                  memcpy(pdest, psrc, bcnt * FRAME_SIZE) ;
                  incn(psrc, bcnt * FRAME_SIZE) ;
                  incn(dbcnt, bcnt) ;
                  /* update blockette links */
                  for (i = 0 ; i <= dbcnt - 1 ; i++)
                    begin
                      if (i)
                        then
                          begin /* extend link */
                            plink = (pointer)((pntrint)parc->pcfr + FRAME_SIZE * i + 2) ;
                            storeword (addr(plink), FRAME_SIZE * (i + 1)) ;
                          end
                        else
                          parc->hdr_buf.deb.next_blockette = 64 ; /* make sure goes to first blockette */
                    end
                  incn(parc->total_frames, bcnt) ;
                end
            if (fcnt > 0)
              then
                begin
                  pdest = (pointer)((pntrint)parc->pcfr + parc->total_frames * FRAME_SIZE) ; /* add to end */
                  memcpy(pdest, psrc, fcnt * FRAME_SIZE) ;
                  incn(parc->total_frames, fcnt) ;
                end
            parc->appended = TRUE ;
            parc->hdr_buf.activity_flags = parc->hdr_buf.activity_flags or pbuf->hdr_buf.activity_flags ;
            parc->hdr_buf.data_quality_flags = parc->hdr_buf.data_quality_flags or
                    (pbuf->hdr_buf.data_quality_flags and not SQF_QUESTIONABLE_TIMETAG) ;
            parc->hdr_buf.io_flags = parc->hdr_buf.io_flags or pbuf->hdr_buf.io_flags ;
            if ((pbuf->hdr_buf.data_quality_flags and SQF_QUESTIONABLE_TIMETAG) == 0)
              then /* turn off error condition in archive */
                parc->hdr_buf.data_quality_flags = parc->hdr_buf.data_quality_flags and not
                                                    SQF_QUESTIONABLE_TIMETAG ;
            if (pbuf->hdr_buf.deb.qual > parc->hdr_buf.deb.qual)
              then
                begin /* new record has better timetag */
                  parc->hdr_buf.starting_time.seed_fpt = pbuf->hdr_buf.starting_time.seed_fpt -
                               (parc->hdr_buf.samples_in_record / drate) ; /* new timestamp */
                  parc->hdr_buf.deb.qual = pbuf->hdr_buf.deb.qual ; /* use higher quality */
                end
            incn(parc->hdr_buf.samples_in_record, pbuf->hdr_buf.samples_in_record) ;
            incn(parc->hdr_buf.number_of_following_blockettes, bcnt) ;
            incn(parc->hdr_buf.deb.frame_count, fcnt) ;
            psrc = (pointer)((pntrint)addr(pbuf->rec) + (bcnt + 1) * FRAME_SIZE + 8) ;
            pdest = (pointer)((pntrint)parc->pcfr + (dbcnt + 1) * FRAME_SIZE + 8) ;
            memcpy (pdest, psrc, 4) ; /* update last sample value */
            if (parc->total_frames >= paqs->arc_frames)
              then
                flush_archive (paqs, q) ; /* totally full dude */
            else if (parc->incremental)
              then
                begin
                  parc->leave_in_buffer = TRUE ;
                  flush_archive (paqs, q) ; /* write update to record, don't clear */
                end
          end
        else
          begin /* new record */
            memcpy(addr(parc->hdr_buf), addr(pbuf->hdr_buf), sizeof(seed_header)) ; /* copy header */
            parc->hdr_buf.dob.rec_length = q330->par_create.amini_exponent ;
            parc->hdr_buf.sequence.seed_num = parc->records_written + 1 ;
            inc(parc->records_written) ;
            psrc = (pointer)((pntrint)addr(pbuf->rec) + FRAME_SIZE) ;
            pdest = (pointer)((pntrint)parc->pcfr + FRAME_SIZE) ;
            if ((bcnt + fcnt) > 0)
              then
                memcpy(pdest, psrc, (bcnt + fcnt) * FRAME_SIZE) ;
            parc->total_frames = 1 + bcnt + fcnt ;
            parc->appended = TRUE ;
            parc->existing_record = FALSE ;
            if (parc->incremental)
              then
                begin
                  parc->leave_in_buffer = TRUE ;
                  flush_archive (paqs, q) ; /* write new record, but don't clear */
                end
          end
      break ;
    case PKC_MESSAGE :
      if (pbuf->hdr_buf.samples_in_record == 0)
        then
          return ;
      if (((pbuf->hdr_buf.samples_in_record + parc->hdr_buf.samples_in_record) > (paqs->arc_size - NONDATA_OVERHEAD)) lor
           (pbuf->hdr_buf.starting_time.seed_fpt > (parc->hdr_buf.starting_time.seed_fpt + 60)))
        then /* won't fit or not the same time */
          flush_archive (paqs, q) ;
      psrc = (pointer)((pntrint)addr(pbuf->rec) + NONDATA_OVERHEAD) ;
      pdest = (pointer)((pntrint)parc->pcfr + NONDATA_OVERHEAD + parc->hdr_buf.samples_in_record) ;
      if (parc->hdr_buf.samples_in_record == 0)
        then
          begin /* new record */
            memcpy(addr(parc->hdr_buf), addr(pbuf->hdr_buf), sizeof(seed_header)) ; /* copy header */
            parc->hdr_buf.dob.rec_length = q330->par_create.amini_exponent ;
            parc->hdr_buf.samples_in_record = 0 ; /* don't count first record twice! */
            parc->hdr_buf.sequence.seed_num = parc->records_written + 1 ;
            inc(parc->records_written) ;
            parc->appended = FALSE ;
            parc->existing_record = FALSE ;
          end
      memcpy(pdest, psrc, pbuf->hdr_buf.samples_in_record) ;
      incn(parc->hdr_buf.samples_in_record, pbuf->hdr_buf.samples_in_record) ;
      parc->appended = TRUE ;
      break ;
    case PKC_TIMING : /* Note: incoming will only have one blockette */
      if ((TIMING_BLOCKETTE_SIZE + parc->total_frames) > paqs->arc_size)
        then
          flush_archive (paqs, q) ; /* new one won't fit */
      if (parc->total_frames > 0)
        then
          begin
            if ((lib_round(pbuf->hdr_buf.starting_time.seed_fpt) div 3600) !=
                (lib_round(parc->hdr_buf.starting_time.seed_fpt) div 3600))
              then
                flush_archive (paqs, q) ; /* different hour, start new record */
          end
      psrc = (pointer)((pntrint)addr(pbuf->rec) + NONDATA_OVERHEAD) ;
      if (parc->total_frames > 0)
        then
          begin /* append to existing record, put data starting at total_frames */
            pdest = (pointer)((pntrint)parc->pcfr + parc->total_frames) ;
            memcpy (pdest, psrc, TIMING_BLOCKETTE_SIZE) ;
            psrc = (pointer)((pntrint)parc->pcfr + parc->total_frames - TIMING_BLOCKETTE_SIZE + 2) ; /* previous blockette */
            storeword (addr(psrc), parc->total_frames) ; /* extend link */
            incn(parc->total_frames, TIMING_BLOCKETTE_SIZE) ;
            inc(parc->hdr_buf.number_of_following_blockettes) ; /* a blockette was added */
            parc->appended = TRUE ;
          end
        else
          begin /* new record */
            memcpy(addr(parc->hdr_buf), addr(pbuf->hdr_buf), sizeof(seed_header)) ; /* copy header */
            parc->hdr_buf.dob.rec_length = q330->par_create.amini_exponent ;
            parc->hdr_buf.sequence.seed_num = parc->records_written + 1 ;
            inc(parc->records_written) ;
            pdest = (pointer)((pntrint)parc->pcfr + NONDATA_OVERHEAD) ;
            memcpy (pdest, psrc, TIMING_BLOCKETTE_SIZE) ;
            parc->total_frames = NONDATA_OVERHEAD + TIMING_BLOCKETTE_SIZE ;
            parc->appended = TRUE ;
            parc->existing_record = FALSE ;
          end
      break ;
    case PKC_OPAQUE : /* note : blockette_index is the next free blockette location */
      bcnt = pbuf->hdr_buf.number_of_following_blockettes - 1 ; /* to be added */
      if (bcnt == 0)
        then
          return ; /* nothing to do */
      size = q->com->blockette_index - NONDATA_OVERHEAD ; /* always a multiple of 4 bytes */
      if (((size + parc->total_frames) > paqs->arc_size) lor (q->lcq_opt and LO_CNPP))
        then
          flush_archive (paqs, q) ; /* new one won't fit or must preserve time */
      if (parc->total_frames > 0)
        then
          begin
            if ((lib_round(pbuf->hdr_buf.starting_time.seed_fpt) div 3600) !=
                (lib_round(parc->hdr_buf.starting_time.seed_fpt) div 3600))
              then
                flush_archive (paqs, q) ; /* different hour, start new record */
          end
      psrc = (pointer)((pntrint)addr(pbuf->rec) + NONDATA_OVERHEAD) ;
      if (parc->total_frames == 0)
        then
          begin /* new record */
            memcpy(addr(parc->hdr_buf), addr(pbuf->hdr_buf), sizeof(seed_header)) ; /* copy header */
            parc->hdr_buf.dob.rec_length = q330->par_create.amini_exponent ;
            parc->hdr_buf.sequence.seed_num = parc->records_written + 1 ;
            inc(parc->records_written) ;
            pdest = (pointer)((pntrint)parc->pcfr + NONDATA_OVERHEAD) ;
            memcpy (pdest, psrc, size) ; /* copy blockettes in as they are */
            parc->total_frames = q->com->blockette_index ;
            parc->appended = TRUE ;
            parc->existing_record = FALSE ;
          end
        else
          begin /* need to extend an existing record */
            plink = NIL ;
            plast = plink ;
            dest = parc->hdr_buf.dob.next_blockette ;
            while (dest) /* find the last blockette, plast will have it's link address */
              begin
                plink = (pointer)((pntrint)parc->pcfr + dest + 2) ;
                plast = plink ;
                dest = loadword (addr(plink)) ;
              end
            pdest = (pointer)((pntrint)parc->pcfr + parc->total_frames) ;
            memcpy (pdest, psrc, size) ; /* move in blockettes */
            storeword (addr(plast), parc->total_frames) ; /* adding to the list, need to rebuild links */
            src = NONDATA_OVERHEAD ;
            dest = parc->total_frames ; /* where we currently are */
            for (i = 1 ; i <= bcnt - 1 ; i++)
              begin
                plink = (pointer)((pntrint)addr(pbuf->rec) + src + 2) ;
                next = loadword (addr(plink)) ; /* get old starting offset of next frame */
                offset = next - src ; /* amount to jump to get to next frame */
                pdest = (pointer)((pntrint)parc->pcfr + dest + 2) ;
                storeword (addr(pdest), dest + offset) ; /* new starting offset of next blockette */
                src = next ;
                dest = dest + offset ;
              end
            incn(parc->total_frames, size) ;
            incn(parc->hdr_buf.number_of_following_blockettes, bcnt) ;
            parc->appended = TRUE ;
          end
      break ;
  end
end

/* ask the client for the last record. If onelcq is NIL then read all normal or dp lcqs
  based on the from330 flag, else read that one lcq */
void preload_archive (pq330 q330, boolean from330, plcq onelcq)
begin
  paqstruc paqs ;
  plcq q ;
  pbyte p ;
  integer fcnt ;
  tarc *parc ;

  paqs = q330->aqstruc ;
  if (onelcq)
    then
      q = onelcq ;
  else if (from330)
    then
      q = paqs->lcqs ;
    else
      q = paqs->dplcqs ;
  while (q)
    begin
      if (q->arc.amini_filter)
        then
          begin
            parc = addr(q->arc) ;
            q330->miniseed_call.context = q330 ;
            memcpy(addr(q330->miniseed_call.station_name), addr(q330->station_ident), sizeof(string9)) ;
            strcpy(addr(q330->miniseed_call.location), addr(q->slocation)) ;
            strcpy(addr(q330->miniseed_call.channel), addr(q->sseedname)) ;
            q330->miniseed_call.chan_number = q->lcq_num ;
            q330->miniseed_call.rate = q->rate ;
            q330->miniseed_call.cl_session = 0 ;
            q330->miniseed_call.cl_offset = 0 ;
            q330->miniseed_call.filter_bits = parc->amini_filter ;
            q330->miniseed_call.packet_class = q->pack_class ;
            q330->miniseed_call.miniseed_action = MSA_GETARC ;
            q330->miniseed_call.data_size = paqs->arc_size ;
            q330->miniseed_call.data_address = parc->pcfr ;
            if (q330->par_create.call_aminidata)
              then
                begin
                  q330->par_create.call_aminidata (addr(q330->miniseed_call)) ;
                  if (q330->miniseed_call.miniseed_action == MSA_RETARC)
                    then
                      begin /* extract seed header and set flags */
                        p = (pointer)parc->pcfr ;
                        loadseedhdr (addr(p), addr(parc->hdr_buf), (q330->miniseed_call.packet_class == PKC_DATA)) ;
                        fcnt = parc->hdr_buf.deb.frame_count + parc->hdr_buf.number_of_following_blockettes - 1 ;
                        if ((q->pack_class == PKC_DATA) land (fcnt < paqs->arc_frames) land (paqs->contingood))
                          then
                            begin /* try to extend */
                              parc->hdr_buf.sequence.seed_num = parc->records_written ;
                              parc->hdr_buf.starting_time.seed_fpt =
                                extract_time(addr(parc->hdr_buf.starting_time), parc->hdr_buf.deb.usec99) ;
                              parc->existing_record = TRUE ;
                              parc->total_frames = fcnt ;
                            end
                          else
                            begin /* not data record or can't extend existing record */
                              memset(addr(parc->hdr_buf), 0, sizeof(seed_header)) ; /* throw away the header */
                              memset(parc->pcfr, 0, paqs->arc_size) ; /* and the data */
                              q->backup_tag = 0 ;
                              q->backup_qual = 0 ; /* if setup by continuity */
                            end
                      end
                end
          end
      if (onelcq)
        then
          break ;
        else
          q = q->link ;
    end
end

#endif
