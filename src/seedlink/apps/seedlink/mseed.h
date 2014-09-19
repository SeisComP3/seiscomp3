/*****************************************************************************
 * mseed.h
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

#ifndef MSEED_H
#define MSEED_H

#include <string>

#include "qtime.h"

#include "buffer.h"
#include "utils.h"
#include "format.h"

namespace SProc_private {

using namespace std;
using namespace Utilities;

class MSEEDFormat: public Format
  {
  private:
    const rc_ptr<BufferStore> bufs;
    const int rec_length;
    const PacketType packtype;
    const string station_id;
    const string network_id;
    const string location_id;
    const string stream_name;
    int sample_rate_factor;
    int sample_rate_multiplier;
    int sequence;

    Buffer *get_buffer(const INT_TIME &it, int usec_correction,
      int timing_quality, void *&dataptr, int &datalen);
    void queue_buffer(Buffer *buf, int samples, int frames);
    
  public:
    MSEEDFormat(rc_ptr<BufferStore> bufs_init, int rec_length_init,
      PacketType packtype_init, const string &station_id_init,
      const string &network_id_init, const string &location_id_init,
      const string &stream_name_init, int freqn, int freqd):
      bufs(bufs_init), rec_length(rec_length_init), packtype(packtype_init),
      station_id(station_id_init), network_id(network_id_init),
      location_id(location_id_init), stream_name(stream_name_init), sequence(0)
      {
        if(freqn == 0 || freqd == 0)
          {
            sample_rate_factor = 0;
            sample_rate_multiplier = 0;
          }
        else if(!(freqn % freqd))
          {
            sample_rate_factor = freqn / freqd;
            sample_rate_multiplier = 1;
          }
        else if(!(freqd % freqn))
          {
            sample_rate_factor = -freqd / freqn;
            sample_rate_multiplier = 1;
          }
        else
          {
            sample_rate_factor = -freqd;
            sample_rate_multiplier = freqn;
          }
      }
  };

} // namespace SProc_private

namespace SProc {

using SProc_private::MSEEDFormat;

} // namespace SProc

#endif // MSEED_H

