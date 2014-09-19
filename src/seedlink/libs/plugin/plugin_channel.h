/***************************************************************************** 
 * plugin_channel.h
 *
 * C++ wrapper for send_raw()
 *
 * (c) 2002 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef PLUGIN_CHANNEL_H
#define PLUGIN_CHANNEL_H

#include <string>

#include <sys/types.h>

#include "qtime.h"

#include "utils.h"
#include "plugin.h"

namespace SeedlinkPlugin_private {

using namespace std;
using namespace Utilities;

// make it small to prevent send_raw3() from blocking
const int CHANNEL_BUFSIZE = 200;

//*****************************************************************************
// OutputChannel
//*****************************************************************************

class OutputChannel
  {
  private:
    int32_t data_buffer[CHANNEL_BUFSIZE];
    bool timemark;
    INT_TIME it;
    int corr;
    int qual;
    int n_samples;
    int zero_count;
    const int max_zeros;
    const double scale;

    void flush_buffer();
    void flush_zeros();
    void send_gap();
    void do_put_sample(int32_t sample_val);
  
  public:
    const string channel_name;
    const string station_name;

    OutputChannel(const string &channel_name_init,
      const string &station_name_init, int max_zeros_init, double scale_init):
      timemark(false), corr(0), qual(-1), n_samples(0), zero_count(0),
      max_zeros(max_zeros_init), scale(scale_init),
      channel_name(channel_name_init), station_name(station_name_init)
      {
        memset(&it, 0, sizeof(INT_TIME));
      }

    void flush();
    void flush_streams();
    void set_timemark(const INT_TIME &it_mark, int usec_correction,
      int timing_quality);
    void put_sample(int sample_val);
    void put_sample(long sample_val);
    void put_sample(double sample_val);
  };

} // namespace SeedlinkPlugin_private

namespace SeedlinkPlugin {

using SeedlinkPlugin_private::OutputChannel;

} // namespace SeedlinkPlugin

#endif // PLUGIN_CHANNEL_H

