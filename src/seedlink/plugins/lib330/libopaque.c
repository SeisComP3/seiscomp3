/*   Lib330 Opaque Blockette Routines
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
    1 2006-12-28 rdr Set/clear cfg_timer. Since the cfg_timer was added there is
                     no need to flush configuration manually at startup.
    2 2008-01-16 rdr Fix record length for CNP blockette data.
    3 2008-02-25 rdr Don't clear cfg_timer unless final flush. cfg_lastwritten set
                     to data time, not host time.
    4 2008-03-13 rdr Don't reset records_written at 999999.
*/
#ifndef OMIT_SEED
#ifndef libopaque_h
#include "libopaque.h"
#endif

#ifndef libmsgs_h
#include "libmsgs.h"
#endif
#ifndef libseed_h
#include "libseed.h"
#endif
#ifndef libcvrt_h
#include "libcvrt.h"
#endif
#ifndef libsample_h
#include "libsample.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif
#ifndef libsampcfg_h
#include "libsampcfg.h"
#endif
#ifndef libarchive_h
#include "libarchive.h"
#endif

void flush_cfgblks (paqstruc paqs, boolean final)
begin
  pbyte p ;
  plcq q ;
  pcom_packet pcom ;

  if (paqs->cfg_lcq == NIL)
    then
      return ;
  q = paqs->cfg_lcq ;
  pcom = q->com ;
  pcom->ring->hdr_buf.number_of_following_blockettes = pcom->blockette_count + 1 ;
  if (pcom->blockette_index > 56)
    then
      begin
        p = addr(pcom->ring->rec) ;
        storeseedhdr (addr(p), addr(pcom->ring->hdr_buf), FALSE) ;
        inc(q->records_generated_session) ;
        q->last_record_generated = secsince () ;
        send_to_client (paqs, q, pcom->ring, SCD_BOTH) ;
      end
  memset (addr(pcom->ring->rec), 0, LIB_REC_SIZE) ;
  memset (addr(pcom->ring->hdr_buf), 0, sizeof(seed_header)) ;
  pcom->blockette_index = 56 ;
  pcom->last_blockette = 48 ;
  pcom->blockette_count = 0 ;
  if (final)
    then
      begin
        paqs->cfg_timer = 0 ;
        if ((q->arc.amini_filter) land (q->arc.total_frames > 1))
          then
            flush_archive (paqs, q) ;
      end
end

void add_cfg (paqstruc paqs, string7 *name, pointer buf, integer size,
              integer num, byte flags)
begin
  integer required ;
  topaque_hdr hdr ;
  pbyte p ;
  pq330 q330 ;
  plcq q ;
  pcom_packet pcom ;
  seed_header *phdr ;

  if (paqs->cfg_lcq == NIL)
    then
      return ;
  q = paqs->cfg_lcq ;
  paqs->cfg_timer = CFG_TIMEOUT ;
  pcom = q->com ;
  phdr = addr(pcom->ring->hdr_buf) ;
  q330 = paqs->owner ;
  q->data_written = FALSE ;
  required = (OPAQUE_HDR_SIZE + (size + 3)) and 0xFFFC ; /* long-word align blockettes */
  if ((pcom->blockette_index + required) > LIB_REC_SIZE)
    then
      flush_cfgblks (paqs, FALSE) ; /* not enough room */
  if (pcom->blockette_index == 56)
    then
      begin /* build new header */
        phdr->seed_record_type = 'D' ;
        phdr->continuation_record = ' ' ;
        phdr->sequence.seed_num = pcom->records_written + 1 ;
        inc(pcom->records_written) ;
        memcpy(addr(phdr->location_id), addr(q->location), sizeof(tlocation)) ;
        memcpy(addr(phdr->channel_id), addr(q->seedname), sizeof(tseed_name)) ;
        memcpy(addr(phdr->station_id_call_letters), addr(q330->station), sizeof(tseed_stn)) ;
        memcpy(addr(phdr->seednet), addr(q330->network), sizeof(tseed_net)) ;
        phdr->sample_rate_multiplier = 1 ;
        phdr->number_of_following_blockettes = pcom->blockette_count + 1 ;
        phdr->first_blockette_byte = 48 ;
        phdr->starting_time.seed_fpt = paqs->data_timetag ;
        phdr->dob.blockette_type = 1000 ;
        phdr->dob.word_order = 1 ;
        phdr->dob.rec_length = RECORD_EXP ;
        phdr->dob.next_blockette = 56 ;
      end
    else
      begin /* extend link */
        p = (pointer)((integer)addr(pcom->ring->rec) + pcom->last_blockette + 2) ;
        storeword (addr(p), pcom->blockette_index) ;
      end ;
  pcom->last_blockette = pcom->blockette_index ;
  p = (pointer)((integer)addr(pcom->ring->rec) + pcom->blockette_index) ;
  hdr.blockette_type = 2000 ;
  hdr.next_blockette = 0 ;
  hdr.blk_lth = OPAQUE_HDR_SIZE + size - 2 ; /* only 2 character record type */
  hdr.data_off = OPAQUE_HDR_SIZE - 2 ;
  hdr.recnum = num ;
  hdr.word_order = 1 ; /* big endian */
  hdr.opaque_flags = flags ;
  hdr.hdr_fields = 1 ;
  hdr.rec_type[0] = (*name)[0] ;
  hdr.rec_type[1] = (*name)[1] ;
  hdr.rec_type[2] = '~' ;
  storeopaque (addr(p), addr(hdr), 3, buf, size) ;
  pcom->blockette_index = pcom->blockette_index + required ;
  inc(pcom->blockette_count) ;
end

void start_cfgblks (paqstruc paqs)
begin
  integer sizeleft, sz ;
  integer rnum ;
  pbyte pb ;
  byte flag ;
  integer maxsz ;
  pq330 q330 ;
  string3 s ;

  q330 = paqs->owner ;
  maxsz = NONDATA_AREA - OPAQUE_HDR_SIZE ;
  paqs->cfg_lastwritten = paqs->data_timetag ;
  add_cfg (paqs, "GL", addr(q330->raw_global), RAW_GLOBAL_SIZE, 0, 0) ;
  add_cfg (paqs, "FX", addr(q330->raw_fixed), RAW_FIXED_SIZE, 0, 0) ;
  sprintf(s, "L%d", q330->par_create.q330id_dataport + 1) ;
  add_cfg (paqs, addr(s), addr(q330->raw_log), RAW_LOG_SIZE, 0, 0) ;
  sizeleft = q330->cfgsize ;
  pb = (pointer)q330->cfgbuf ;
  rnum = 0 ;
  while (sizeleft > 0)
    begin
      flag = 1 ; /* a stream */
      if (sizeleft > maxsz)
        then
          sz = maxsz ;
        else
          sz = sizeleft ;
      if (q330->cfgsize > maxsz)
        then
          begin /* have a series */
            if (rnum == 0)
              then
                flag = flag or 4 ; /* first of a series */
            else if (sizeleft == sz)
              then
                flag = flag or 8 ; /* last of a series */
              else
                flag = flag or 0xC ; /* middle of a series */
          end
      sprintf(s, "T%d", q330->par_create.q330id_dataport + 1) ;
      add_cfg (paqs, addr(s), pb, sz, rnum, flag) ;
      inc(rnum) ;
      incn(pb, sz) ;
      sizeleft = sizeleft - sz ;
    end
/*  flush_cfgblks (paqs, TRUE) ; */
end

void flush_cnp (paqstruc paqs, plcq q, pcom_packet pcom)
begin
  pbyte p ;

  pcom->ring->hdr_buf.number_of_following_blockettes = pcom->blockette_count + 1 ;
  if (pcom->blockette_index > 56)
    then
      begin
        p = addr(pcom->ring->rec) ;
        storeseedhdr (addr(p), addr(pcom->ring->hdr_buf), FALSE) ;
        inc(q->records_generated_session) ;
        q->last_record_generated = secsince () ;
        send_to_client (paqs, q, pcom->ring, SCD_BOTH) ;
      end
  memset (addr(pcom->ring->rec), 0, LIB_REC_SIZE) ;
  memset (addr(pcom->ring->hdr_buf), 0, sizeof(seed_header)) ;
  pcom->blockette_index = 56 ;
  pcom->last_blockette = 48 ;
  pcom->blockette_count = 0 ;
  if ((q->arc.amini_filter) land (q->arc.total_frames > 1))
    then
      flush_archive (paqs, q) ;
end

void process_cnp (pq330 q330, pbyte pb)
begin
  integer required ;
  topaque_hdr hdr ;
  pbyte p ;
  paqstruc paqs ;
  byte flags ;
  word size ;
  plcq q ;
  pcom_packet pcom ;
  seed_header *phdr ;

  paqs = q330->aqstruc ;
  if (paqs->data_timetag < 1)
    then
      return ; /* don't know what time it is yet */
  loadbyte (addr(pb)) ; /* skip channel */
  flags = loadbyte (addr(pb)) ;
  size = loadword (addr(pb)) ;
  q = paqs->proc_lcq ;
  pcom = q->com ;
  phdr = addr(pcom->ring->hdr_buf) ;
  q->data_written = FALSE ;
  required = (OPAQUE_HDR_SIZE + (size - 1)) and 0xFFFC ; /* long-word align blockettes */
  if (q->lcq_opt and LO_CNPP)
    then
      flags = flags or 2 ; /* preserve timetag */
  if (((pcom->blockette_index + required) > LIB_REC_SIZE) lor (flags and 2))
    then
      flush_cnp (paqs, q, pcom) ; /* make sure we are starting fresh */
  if (pcom->blockette_index == 56)
    then
      begin /* build new header */
        phdr->seed_record_type = 'D' ;
        phdr->continuation_record = ' ' ;
        phdr->sequence.seed_num = pcom->records_written + 1 ;
        inc(pcom->records_written) ;
        memcpy(addr(phdr->location_id), addr(q->location), sizeof(tlocation)) ;
        memcpy(addr(phdr->channel_id), addr(q->seedname), sizeof(tseed_name)) ;
        memcpy(addr(phdr->station_id_call_letters), addr(q330->station), sizeof(tseed_stn)) ;
        memcpy(addr(phdr->seednet), addr(q330->network), sizeof(tseed_net)) ;
        phdr->sample_rate_multiplier = 1 ;
        phdr->number_of_following_blockettes = pcom->blockette_count + 1 ;
        phdr->first_blockette_byte = 48 ;
        phdr->starting_time.seed_fpt = paqs->data_timetag ;
        phdr->dob.blockette_type = 1000 ;
        phdr->dob.word_order = 1 ;
        phdr->dob.rec_length = RECORD_EXP ;
        phdr->dob.next_blockette = 56 ;
      end
    else
      begin /* extend link */
        p = (pointer)((integer)addr(pcom->ring->rec) + pcom->last_blockette + 2) ;
        storeword (addr(p), pcom->blockette_index) ;
      end
  pcom->last_blockette = pcom->blockette_index ;
  p = (pointer)((integer)addr(pcom->ring->rec) + pcom->blockette_index) ;
  hdr.blockette_type = 2000 ;
  hdr.next_blockette = 0 ;
  hdr.blk_lth = OPAQUE_HDR_SIZE + size - 4 ; /* use blockette size minus 4 */
  hdr.data_off = OPAQUE_HDR_SIZE ;
  if (((flags and 0xD) == 0x5) lor ((flags and 1) == 0))
    then
      pcom->last_sample = 0 ; /* start of stream or not a stream */
  if (flags and 1)
    then
      hdr.recnum = pcom->last_sample ; /* stream oriented */
    else
      hdr.recnum = 0 ;
  hdr.word_order = 1 ; /* big endian */
  hdr.opaque_flags = flags ;
  hdr.hdr_fields = 1 ;
  string2fixed(addr(hdr.rec_type), "CNPB~") ;
  if ((flags and 0xC) == 0x8)
    then
      pcom->last_sample = 0 ; /* End of stream */
    else
      inc(pcom->last_sample) ; /* to next in stream */
  storeopaque (addr(p), addr(hdr), 5, pb, size - 4) ;
  pcom->blockette_index = (pcom->blockette_index + required + 3) and 0xFFFC ; /* longword align */
  inc(pcom->blockette_count) ;
end

#endif
