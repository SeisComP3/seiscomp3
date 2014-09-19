/*****************************************************************************
 * encoder.h
 *
 * Abstract Encoder interface
 *
 * (c) 2000 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef ENCODER_H
#define ENCODER_H

#include <sys/types.h>

#include "spclock.h"

namespace SProc_private {

class Encoder
  {
  public:
    virtual void send_data(int32_t sample_val) =0;
    virtual void sync_time(const SPClock &clk) =0;
    virtual void flush() =0;
    virtual ~Encoder() {}
  };

} // namespace SProc_private

#endif // ENCODER_H

