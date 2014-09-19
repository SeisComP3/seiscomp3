/*****************************************************************************
 * spclock.h
 *
 * Stream Processor Clock
 *
 * (c) 2000 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef SPCLOCK_H
#define SPCLOCK_H

#include "qtime.h"

namespace SProc_private {

class SPClock
  {
  private:
    INT_TIME itime;
    int seconds;
    int ticks;
    int corr;
    int qual;
    double gap;
  
  public:
    const int freqn;
    const int freqd;
    
    SPClock(int freqn_init, int freqd_init): seconds(0), ticks(0), corr(0), gap(0),
      freqn(freqn_init), freqd(freqd_init)
      {
        memset(&itime, 0, sizeof(INT_TIME));
      }

    void sync_time(const SPClock &ref, int tick_diff, double shift)
      {
        itime = add_dtime(ref.itime, 1000000 * (double(ref.seconds) +
          (double(ref.ticks - tick_diff) + shift) *
          double(freqd) / double(freqn)));
        ticks = 0;
        seconds = 0;
        corr = ref.corr;
        qual = ref.qual;
        gap = ref.gap;
      }

    void set_time(const INT_TIME &it, int usec_correction, int timing_quality)
      {
        itime = add_dtime(itime, 1000000 * (double(seconds) +
          double(ticks) * double(freqd) / double(freqn)));
        ticks = 0;
        seconds = 0;
        corr = usec_correction;
        qual = timing_quality;
        
        if(itime.year == 0 && itime.second == 0 && itime.usec == 0) gap = 0.0;
        else gap = tdiff(it, itime);
        itime = it;
      }

    void add_ticks(int n, int usec_correction, int timing_quality)
      {
        itime = add_dtime(itime, 1000000 * (double(seconds) +
          double(ticks + n) * double(freqd) / double(freqn)));
        ticks = 0;
        seconds = 0;
        corr = usec_correction;
        qual = timing_quality;
        gap = 1000000 * double(n) * double(freqd) / double(freqn);
      }
    
    void tick()
      {
        if(++ticks == freqn)
          {
            seconds += freqd;
            ticks = 0;
          }
      }

    INT_TIME get_time(int tick_diff) const
      {
        INT_TIME it = add_dtime(itime, 1000000 * (double(seconds) +
          double(ticks - tick_diff) * double(freqd) / double(freqn)));
        
        return it;
      }

    int correction() const
      {
        return corr;
      }

    int quality() const
      {
        return qual;
      }

    double time_gap() const
      {
        return gap;
      }
  };

} // namespace SProc_private

#endif // SPCLOCK_H

