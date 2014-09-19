/***************************************************************************** 
 * plugin_channel.cc
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

#include "plugin_channel.h"
#include "plugin_exceptions.h"

namespace SeedlinkPlugin_private {

using namespace std;
using namespace Utilities;

//*****************************************************************************
// OutputChannel
//*****************************************************************************

void OutputChannel::flush_buffer()
  {
    if(timemark)
      {
        EXT_TIME et = int_to_ext(it);
        struct ptime pt;
        pt.year = et.year;
        pt.yday = et.doy;
        pt.hour = et.hour;
        pt.minute = et.minute;
        pt.second = et.second;
        pt.usec = et.usec;

        int r = send_raw3(station_name.c_str(), channel_name.c_str(), &pt, corr,
          qual, data_buffer, n_samples);

        if(r < 0) throw PluginBrokenLink(strerror(errno));
        else if(r == 0 && n_samples != 0) throw PluginBrokenLink();
      }
    else if(n_samples != 0)
      {
        int r = send_raw3(station_name.c_str(), channel_name.c_str(), NULL, 0,
          0, data_buffer, n_samples);
    
        if(r < 0) throw PluginBrokenLink(strerror(errno));
        else if(r == 0) throw PluginBrokenLink();
      }

    n_samples = 0;
    timemark = false;
  }

void OutputChannel::flush_zeros()
  {
    for(; zero_count > 0; --zero_count)
      {
        data_buffer[n_samples++] = 0;
        if(n_samples == CHANNEL_BUFSIZE) flush_buffer();
      }
  }

void OutputChannel::send_gap()
  {
    flush_buffer();
    if(send_raw3(station_name.c_str(), channel_name.c_str(), NULL,
      corr, qual, NULL, zero_count) < 0)
        throw PluginBrokenLink(strerror(errno));

    zero_count = 0;
  }

void OutputChannel::flush()
  {
    if(zero_count <= max_zeros)
        flush_zeros();
    
    if(n_samples != 0)
        flush_buffer();
  }
    
void OutputChannel::flush_streams()
  {
    flush();
    
    if(send_flush3(station_name.c_str(), channel_name.c_str()) < 0)
        throw PluginBrokenLink(strerror(errno));
  }

void OutputChannel::set_timemark(const INT_TIME &it_mark, int usec_correction,
  int timing_quality)
  {
    flush();
    
    zero_count = 0;
    it = it_mark;
    corr = usec_correction;
    qual = timing_quality;
    timemark = true;
  }

void OutputChannel::do_put_sample(int32_t sample_val)
  {
    if(max_zeros != -1)
      {
        if(sample_val == 0)
          {
            ++zero_count;
            return;
          }

        if(zero_count > max_zeros) send_gap();
        else flush_zeros();
      }

    data_buffer[n_samples++] = sample_val;
    if(n_samples == CHANNEL_BUFSIZE) flush_buffer();
  }

void OutputChannel::put_sample(int sample_val)
  {
    if(scale < 0)
        do_put_sample(static_cast<int32_t>(sample_val));
    else
        do_put_sample(static_cast<int32_t>(scale * sample_val));
  }

void OutputChannel::put_sample(long sample_val)
  {
    if(scale < 0)
        do_put_sample(static_cast<int32_t>(sample_val));
    else
        do_put_sample(static_cast<int32_t>(scale * sample_val));
  }

void OutputChannel::put_sample(double sample_val)
  {
    if(scale < 0)
        do_put_sample(static_cast<int32_t>(sample_val));
    else
        do_put_sample(static_cast<int32_t>(scale * sample_val));
  }

} // SeedlinkPlugin_private

