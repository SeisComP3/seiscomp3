/***************************************************************************** 
 * steim2.h
 *
 * Steim2 encoder
 *
 * (c) 2000 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef STEIM2_H
#define STEIM2_H

#include <string>

#include "utils.h"
#include "spclock.h"
#include "format.h"
#include "encoder.h"

#include "big-endian.h"

namespace SProc_private {

using namespace std;
using namespace Utilities;

//*****************************************************************************
// Steim2Frame
//*****************************************************************************

struct Steim2Frame
  {
    u_int32_t nibble_word;
    u_int32_t sample_word[15];
  } PACKED;

//*****************************************************************************
// Steim2Encoder
//*****************************************************************************

class Steim2Encoder: public Encoder
  {
  private:
    const rc_ptr<Format> format;
    SPClock clk;
    int sample_count;
    int frame_count;
    int bp;
    int fp;
    int spw;
    int32_t last_sample;
    int32_t buf[8];
    u_int32_t nibble_word;
    Packet<Steim2Frame> current_packet;

    void update_spw(int bp);
    void store(int32_t value);
    void init_packet();
    void finish_packet();
    void update_packet();
    
    int number_of_frames(const Packet<Steim2Frame> &pckt)
      {
        return (pckt.datalen >> 6);
      }

    Packet<Steim2Frame> get_packet()
      {
        return format->get_packet<Steim2Frame>(clk.get_time(bp),
          clk.correction(), clk.quality());
      }

    void queue_packet(Packet<Steim2Frame> &pckt)
      {
        format->queue_packet(pckt, sample_count, frame_count + (fp > 0));
      }

  public:
    Steim2Encoder(rc_ptr<Format> format_init, int freqn, int freqd):
      format(format_init), clk(freqn, freqd), sample_count(0), frame_count(0),
      bp(0), fp(0), spw(4), last_sample(0), nibble_word(0) {}

    void send_data(int32_t sample_val);
    void flush();
    
    void sync_time(const SPClock &otherclk)
      {
        clk.sync_time(otherclk, 0, 0);
      }
  };

} // namespace SProc_private

namespace SProc {

using SProc_private::Steim2Encoder;

} // namespace SProc

#endif // STEIM2_H

