/***************************************************************************** 
 * steim1.cc
 *
 * Steim1 encoder
 *
 * (c) 2000 Andres Heinloo, GFZ Potsdam
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
#include "steim1.h"
#include "diag.h"

namespace SProc_private {

using namespace std;
using namespace CPPStreams;

void Steim1Encoder::update_spw(int bp)
  {
    int spw1 = 4;

    internal_check(bp < 4);
    if(buf[bp] < -32768 || buf[bp] > 32767) spw1 = 1;
    else if(buf[bp] < -128 || buf[bp] > 127) spw1 = 2;
    if(spw1 < spw) spw = spw1;
  }

void Steim1Encoder::store(int32_t value)
  {
#ifdef DEBUG_STEIM1
    DEBUG_MSG("store(value=" << value << ", diff=" << value - last_sample << ")" << endl);
#endif

    internal_check(bp < 4);
    buf[bp] = value - last_sample;
    last_sample = value;
    update_spw(bp);
    ++bp;
  }  
    
void Steim1Encoder::init_packet()
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

void Steim1Encoder::finish_packet()
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

void Steim1Encoder::update_packet()
  {
    unsigned int nibble = 0;
    u_int32_t sample_word = 0;
    
#ifdef DEBUG_STEIM1    
    DEBUG_MSG("update_packet(bp=" << bp << ", spw=" << spw << ")" << endl);
#endif
    
    internal_check(bp < 5);
    
    int used = bp;

    while(used > spw)
      {
        --used;
        spw = 4;
        for(int i = 0; i < used; ++i) update_spw(i);
      }
    
    while(used < spw) spw >>= 1;

    used = spw;
 
    switch(spw)
      {
      case 4: 
        nibble = 1;
        sample_word = ((buf[0] & 0xff) << 24) | ((buf[1] & 0xff) << 16) |
            ((buf[2] & 0xff) <<  8) | (buf[3] & 0xff);
        break;
      case 2:
        nibble = 2;
        sample_word = ((buf[0] & 0xffff) << 16) | (buf[1] & 0xffff);
        break;
      case 1: 
        nibble = 3;
        sample_word = buf[0]; 
        break;
      default:
        internal_check(0);
      }
      
    nibble_word |= (nibble << (30 - ((fp + 1) << 1)));
    
    spw = 4;
    for(int i = 0; i < bp - used; ++i)
      {
        buf[i] = buf[i + used];
        update_spw(i);
      }

    bp -= used;
    sample_count += used;

#ifdef DEBUG_STEIM1
    DEBUG_MSG("used=" << used << ", bp=" << bp << endl);
#endif
    current_packet.data[frame_count].nibble_word = htonl(nibble_word);
    current_packet.data[frame_count].sample_word[fp] = htonl(sample_word);
    if(++fp < 15) return;

#ifdef DEBUG_STEIM1
    DEBUG_MSG("nibble_word=0x" << setfill('0') << setw(8) << hex << nibble_word << endl);
#endif
    nibble_word = 0;
    fp = 0;
    ++frame_count;
    return;
  }

void Steim1Encoder::send_data(int32_t sample_val)
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

void Steim1Encoder::flush()
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

