/*****************************************************************************
 * filterimpl.h
 *
 * FIR Filter implementation
 *
 * (c) 2000 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef FILTERIMPL_H
#define FILTERIMPL_H

#include <string>

#include "filter.h"

namespace SProc_private {

using namespace std;

class FilterImpl: public Filter
  {
  public:
    enum FilterType { ZeroPhase, MinimumPhase };
    
  private:
    const FilterType type;
    const double gain;
    const double *const points;
  
  public:
    FilterImpl(const string &name, FilterType type_init, int len_init, int dec,
      double gain_init, const double *points_init):
      Filter(name, dec), type(type_init), gain(gain_init), points(points_init)
      {
        len = len_init;
      }

    ~FilterImpl()
      {
        delete[] points;
      }
    
    double apply(CircularBuffer<double>::iterator p)
      {
        double acc = 0;
        
        if(type == ZeroPhase)
          {
            for(int i = 0; i < len / 2; ++i)
                acc += *(p++) * points[i] * gain;

            if (len % 2) 
                acc += *(p++) * points[len/2] * gain;

            for(int i = len / 2 - 1; i >= 0; --i)
                acc += *(p++) * points[i] * gain;
          }
        else
          {
            for(int i = 0; i < len; ++i)
                acc += *(p++) * points[i] * gain;
          }

        return acc;
      }

    double shift()
      {
        return ((type == ZeroPhase) ? (double(len) / 2.0 - 0.5) : 0);
      }
  };

} // namespace SProc_private

namespace SProc {

using SProc_private::FilterImpl;

} // namespace SProc

#endif // FILTERIMPL_H

