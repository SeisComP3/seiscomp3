/*****************************************************************************
 * mseed.cc
 *
 * Mini-SEED format implementation
 *
 * (c) 2000 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <iostream>
#include <cstdio>
#include <cstdlib>

#include <netinet/in.h>

#include "qtime.h"

#include "libslink.h"

#include "buffer.h"
#include "mseed.h"
#include "diag.h"

namespace SProc_private {

using namespace std;

Buffer *MSEEDFormat::get_buffer(const INT_TIME &it, int usec_correction,
  int timing_quality, void *&dataptr, int &datalen)
  {
    int n;
    Buffer *buf = bufs->get_buffer();
    sl_fsdh_s* fsdh = (sl_fsdh_s *)buf->data();

    internal_check(buf->size == (1 << rec_length));
    memset(fsdh, 0, buf->size);

    const int start_frames = (sizeof(sl_fsdh_s) + sizeof(sl_blkt_1000_s) +
      sizeof(sl_blkt_1001_s) + 63) & 0xffffffc0;    /* align to 64 bytes */

    dataptr = (void *)((char *) fsdh + start_frames);
    datalen = buf->size - start_frames;

    fsdh->dhq_indicator = 'D';
    fsdh->reserved = ' ';
    
    strncpy(fsdh->station, station_id.c_str(), 5);
    if((n = station_id.length()) < 5) memset(fsdh->station + n, 32, 5 - n);
    strncpy(fsdh->location, location_id.c_str(), 2);
    if((n = location_id.length()) < 2) memset(fsdh->location + n, 32, 2 - n);
    strncpy(fsdh->channel, stream_name.c_str(), 3);
    if((n = stream_name.length()) < 3) memset(fsdh->channel + n, 32, 3 - n);
    strncpy(fsdh->network, network_id.c_str(), 2);
    if((n = network_id.length()) < 2) memset(fsdh->network + n, 32, 2 - n);
    
#ifdef MSEED_ROUND_TENTH_MILLISEC
    EXT_TIME et = int_to_ext(add_dtime(it, 50));
#else
    EXT_TIME et = int_to_ext(it);
#endif

    div_t d_tms = div(et.usec, 100);

    fsdh->start_time.year = htons(et.year);
    fsdh->start_time.day = htons(et.doy);
    fsdh->start_time.hour = et.hour;
    fsdh->start_time.min = et.minute;
    fsdh->start_time.sec = et.second;
    fsdh->start_time.fract = htons(d_tms.quot);
    fsdh->samprate_fact = (int16_t)htons(sample_rate_factor);
    fsdh->samprate_mult = (int16_t)htons(sample_rate_multiplier);
    fsdh->num_blockettes = 1;
    
    div_t d_corr = div(usec_correction + ((usec_correction < 0) ? -50: 50), 100);
    fsdh->time_correct = (int32_t)htonl(d_corr.quot);
    
    fsdh->begin_data = htons(start_frames);
    fsdh->begin_blockette = htons(sizeof(sl_fsdh_s));

    sl_blkt_1000_s* blkt_1000 = (sl_blkt_1000_s *)((char *) fsdh + sizeof(sl_fsdh_s));
    blkt_1000->blkt_type = htons(1000);          /* Data Only SEED Blockette */

    if(packtype == Steim1Packet) blkt_1000->encoding = 10;
    else if(packtype == Steim2Packet) blkt_1000->encoding = 11;
    else blkt_1000->encoding = 0;
    
    blkt_1000->word_swap = 1;                    /* big endian */
    blkt_1000->rec_len = rec_length;             /* 9 = 512 bytes */
    
    if(timing_quality >= 0)
      {
        blkt_1000->next_blkt = htons(sizeof(sl_fsdh_s) + sizeof(sl_blkt_1000_s));
        ++fsdh->num_blockettes;
        
        sl_blkt_1001_s* blkt_1001 = (sl_blkt_1001_s *)((char *) fsdh +
          sizeof(sl_fsdh_s) + sizeof(sl_blkt_1000_s));

        blkt_1001->blkt_type = htons(1001);      /* Data Extension Blockette */
        blkt_1001->timing_qual = timing_quality;
        
#ifdef MSEED_ROUND_TENTH_MILLISEC
        blkt_1001->usec = d_tms.rem - 50;
#else
        blkt_1001->usec = d_tms.rem;
#endif
      }
    
    return buf;
  }

void MSEEDFormat::queue_buffer(Buffer *buf, int samples, int frames)
  {
    sl_fsdh_s* fsdh = (sl_fsdh_s *)buf->data();
    char temp[7];

    sprintf(temp, "%06d", sequence);
    memcpy(fsdh->sequence_number,temp,6);
    sequence = (sequence + 1) % 1000000;
    fsdh->dhq_indicator = 'D';
    fsdh->num_samples = htons(samples);

    sl_blkt_1000_s* blkt_1000 = (sl_blkt_1000_s *)((char *) fsdh + sizeof(sl_fsdh_s));
    
    if(ntohs(blkt_1000->next_blkt) != 0)
      {
        sl_blkt_1001_s* blkt_1001 = (sl_blkt_1001_s *)((char *) fsdh +
          sizeof(sl_fsdh_s) + sizeof(sl_blkt_1000_s));

        blkt_1001->frame_cnt = frames;
      }

    bufs->queue_buffer(buf);
  }

} // namespace SProc_private

