/***************************************************************************** 
 * steim2.cc
 *
 * Steim2 encoder
 *
 * (c) 2004 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>

#include <netinet/in.h>

#include "cppstreams.h"
#include "steim2.h"
#include "diag.h"

using namespace std;
using namespace CPPStreams;

namespace SProc_private {

void Steim2Encoder::update_spw(int bp)
  {
    internal_check(bp < 7);
    
    if(buf[bp] < -536870912)
      {
        logs(LOG_WARNING) << "value " << buf[bp] << " is too large for "
          "Steim2 encoding" << endl;
        buf[bp] = -536870912;
        spw = 1;
        return;
      }

    if(buf[bp] > 536870911)
      {
        logs(LOG_WARNING) << "value " << buf[bp] << " is too large for "
          "Steim2 encoding" << endl;
        buf[bp] = 536870911;
        spw = 1;
        return;
      }

    int spw1 = 7;
    if(buf[bp] < -16384 || buf[bp] > 16383) spw1 = 1;
    else if(buf[bp] < -512 || buf[bp] > 511) spw1 = 2;
    else if(buf[bp] < -128 || buf[bp] > 127) spw1 = 3;
    else if(buf[bp] < -32 || buf[bp] > 31) spw1 = 4;
    else if(buf[bp] < -16 || buf[bp] > 15) spw1 = 5;
    else if(buf[bp] < -8  || buf[bp] > 7) spw1 = 6;
    if(spw1 < spw) spw = spw1;
  }

void Steim2Encoder::store(int32_t value)
  {
#ifdef DEBUG_STEIM2
    DEBUG_MSG("store(value=" << value << ", diff=" << value - last_sample << ")" << endl);
#endif

    internal_check(bp < 7);
    buf[bp] = value - last_sample;
    last_sample = value;
    update_spw(bp);
    ++bp;
  }  
    
void Steim2Encoder::init_packet()
  {
    int i;
    int32_t begin_sample = last_sample;

    for(i = 1; i < bp; ++i)
      {
        begin_sample -= buf[i];
      }
    
#ifdef DEBUG_ENC    
    DEBUG_MSG("init_packet(bp=" << bp << ", begin_sample=" << begin_sample << ")" << endl);
#endif

    current_packet.data[0].sample_word[0] = htonl(begin_sample);
    sample_count = 0;
    frame_count = 0;
    nibble_word = 0;
    fp = 2;
  }

void Steim2Encoder::finish_packet()
  {
    int i;
    int32_t end_sample = last_sample;
    
    for(i = 0; i < bp; ++i)
      {
        end_sample -= buf[i];
      }

#ifdef DEBUG_ENC
    DEBUG_MSG("finish_packet(bp=" << bp << ", end_sample=" << end_sample
      << ", sample_count=" << sample_count << ")" << endl);
#endif
    
    current_packet.data[0].sample_word[1] = htonl(end_sample);
  }

void Steim2Encoder::update_packet()
  {
    unsigned int nibble = 0;
    u_int32_t sample_word = 0;
    
#ifdef DEBUG_STEIM2    
    DEBUG_MSG("update_packet(bp=" << bp << ", spw=" << spw << ")" << endl);
#endif
    
    internal_check(bp < 8);
    
    int used = bp;

    while(used > spw)
      {
        --used;
        spw = 7;
        for(int i = 0; i < used; ++i) update_spw(i);
      }
    
    spw = used;
    
    switch(spw)
      {
      case 7:
        nibble = 3;
        sample_word = (2U << 30) | ((buf[0] & 0xf) << 24) |
            ((buf[1] & 0xf) << 20) | ((buf[2] & 0xf) << 16) |
            ((buf[3] & 0xf) << 12) | ((buf[4] & 0xf) << 8) |
            ((buf[5] & 0xf) << 4) | (buf[6] & 0xf);
        break;
      case 6:
        nibble = 3;
        sample_word = (1U << 30) | ((buf[0] & 0x1f) << 25) |
            ((buf[1] & 0x1f) << 20) | ((buf[2] & 0x1f) << 15) |
            ((buf[3] & 0x1f) << 10) | ((buf[4] & 0x1f) << 5) |
            (buf[5] & 0x1f);
        break;
      case 5:
        nibble = 3;
        sample_word = ((buf[0] & 0x3f) << 24) | ((buf[1] & 0x3f) << 18) |
            ((buf[2] & 0x3f) << 12) | ((buf[3] & 0x3f) << 6) |
            (buf[4] & 0x3f);
        break;
      case 4: 
        nibble = 1;
        sample_word = ((buf[0] & 0xff) << 24) | ((buf[1] & 0xff) << 16) |
            ((buf[2] & 0xff) <<  8) | (buf[3] & 0xff);
        break;
      case 3:
        nibble = 2;
        sample_word = (3U << 30) | ((buf[0] & 0x3ff) << 20) |
            ((buf[1] & 0x3ff) << 10) | (buf[2] & 0x3ff);
        break;
      case 2:
        nibble = 2;
        sample_word = (2U << 30) | ((buf[0] & 0x7fff) << 15) |
            (buf[1] & 0x7fff);
        break;
      case 1:
        nibble = 2;
        sample_word = (1U << 30) | (buf[0] & 0x3fffffff);
        break;
      default:
        internal_check(0);
      }
      
    nibble_word |= (nibble << (30 - ((fp + 1) << 1)));
    
    spw = 7;
    for(int i = 0; i < bp - used; ++i)
      {
        buf[i] = buf[i + used];
        update_spw(i);
      }

    bp -= used;
    sample_count += used;

#ifdef DEBUG_STEIM2
    DEBUG_MSG("used=" << used << ", bp=" << bp << endl);
#endif
    current_packet.data[frame_count].nibble_word = htonl(nibble_word);
    current_packet.data[frame_count].sample_word[fp] = htonl(sample_word);
    if(++fp < 15) return;

#ifdef DEBUG_STEIM2
    DEBUG_MSG("nibble_word=0x" << setfill('0') << setw(8) << hex << nibble_word << endl);
#endif
    nibble_word = 0;
    fp = 0;
    ++frame_count;
    return;
  }

void Steim2Encoder::send_data(int32_t sample_val)
  {
    store(sample_val);
    clk.tick();

    while(bp >= spw)
      {
        if(!current_packet.valid())
          {
            current_packet = get_packet();
            init_packet();
          }
        update_packet();
        if(frame_count == number_of_frames(current_packet))
          {
            finish_packet();
            queue_packet(current_packet);
          }
      }
  }

void Steim2Encoder::flush()
  {
    while(bp)
      {
        if(!current_packet.valid())
          {
            current_packet = get_packet();
            init_packet();
          }
        update_packet();
        if(frame_count == number_of_frames(current_packet))
          {
            finish_packet();
            queue_packet(current_packet);
          }
      }
    
    if(current_packet.valid())
      {
        finish_packet();
        queue_packet(current_packet);
      }
  }

} // namespace SProc_private

