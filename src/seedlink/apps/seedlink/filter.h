/*****************************************************************************
 * filter.h
 *
 * Abstract FIR Filter interface
 *
 * (c) 2000 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef FILTER_H
#define FILTER_H

#include <string>

#include "cbuf.h"

namespace SProc_private {

using namespace std;

class Filter
  {
  protected:
    const int dec;
    int len;
    
    Filter(const string &name_init, int dec_init): dec(dec_init), name(name_init) {}

  public:
    const string name;

    int length()
      {
        return len;
      }

    int decimation()
      {
        return dec;
      }

    virtual double apply(CircularBuffer<double>::iterator p) =0;
    virtual double shift() =0;
    virtual ~Filter() {}
  };

} // namespace SProc_private

namespace SProc {

using SProc_private::Filter;

} // namespace SProc

#endif // FILTER_H

