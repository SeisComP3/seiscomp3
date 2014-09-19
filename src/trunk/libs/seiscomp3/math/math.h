/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_MATH_MATH_H__
#define __SEISCOMP_MATH_MATH_H__


#include <seiscomp3/core.h>
#include <cmath>
#include <complex>


#ifndef deg2rad
#define deg2rad(d) (M_PI*(d)/180.0)
#endif
#ifndef rad2deg
#define rad2deg(d) (180.0*(d)/M_PI)
#endif

#ifndef HALF_PI
#define HALF_PI (M_PI/2)
#endif


#if defined(_MSC_VER)
	#include <float.h>
#endif


namespace Seiscomp {
namespace Math {


typedef std::complex<double> Complex;


#if defined(WIN32)
template <typename T>
	inline bool isNaN(T v) { return _isnan(v)!=0; }
#elif defined(__SUNPRO_CC) || defined(__sun)
	template <typename T>
	inline bool isNaN(T v) { return isnan(v)!=0; }
#else
	template <typename T>
	inline bool isNaN(T v) { return std::isnan(v)!=0; }
#endif


/** Rounds the given double value*/
SC_SYSTEM_CORE_API double round(double val);



} // namespace Math
} // namespace Seiscomp

#endif
