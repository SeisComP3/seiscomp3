//- ****************************************************************************
//- 
//- Copyright 2009 Sandia Corporation. Under the terms of Contract
//- DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
//- retains certain rights in this software.
//- 
//- BSD Open Source License.
//- All rights reserved.
//- 
//- Redistribution and use in source and binary forms, with or without
//- modification, are permitted provided that the following conditions are met:
//- 
//-    * Redistributions of source code must retain the above copyright notice,
//-      this list of conditions and the following disclaimer.
//-    * Redistributions in binary form must reproduce the above copyright
//-      notice, this list of conditions and the following disclaimer in the
//-      documentation and/or other materials provided with the distribution.
//-    * Neither the name of Sandia National Laboratories nor the names of its
//-      contributors may be used to endorse or promote products derived from
//-      this software without specific prior written permission.
//- 
//- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//- ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//- LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//- CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//- SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//- INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//- CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//- ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//- POSSIBILITY OF SUCH DAMAGE.
//-
//- ****************************************************************************

#ifndef CPPGLOBALS_H_
#define CPPGLOBALS_H_

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <limits>
#include <cmath>
#include <iostream>
#include <vector>
#include <map>
#include <string>

// Definition of dllimport and dllexport here for Windows only
// Note that definitions are repeated for various tools/libraries.
//--------------------------
#if defined(_WIN32) || defined(WIN32)

  // exports when building GEOTESS dll, imports when linking to header files in
  // GEOTESS (Note that GEOTESS_EXPORTS should be defined when building a GEOTESS
  // DLL, and should  not be defined when linking with the GEOTESS DLL)
  #ifdef  GEOTESS_EXPORTS
  #define GEOTESS_EXP_IMP __declspec(dllexport)
  #else
  #define GEOTESS_EXP_IMP __declspec(dllimport)
  #endif

  // exports DLL for classes and functions that ONLY export
  // (Note that this is mainly used for templated classes that are not imported)
  #define GEOTESS_EXP     __declspec(dllexport)

  #define isnan(x) _isnan(x)

#else  // Sun does not need these

  #define GEOTESS_EXP_IMP
  #define GEOTESS_EXP

#endif

#ifndef ABSTRACT
//! Global constant used to make pure virtual functions readable.
#define ABSTRACT                            0
#endif

#ifndef int64
//! Sun defines long as int ... this defines long long as a true long (int64).
typedef long long int64;
#endif

#ifndef uByte
//! Unsigned-byte typedef
typedef unsigned char uByte;
#endif

#ifndef byte
//! signed-byte typedef
//typedef signed char byte;
#define byte signed char
#endif

// Definition of LONG
#if defined WIN32 || defined _WIN32
	#if defined _M_X64 || defined _M_AMD64
		#define LONG_INT long
		#define LONG_INT_F "%ld"
	#else
		#define LONG_INT long long
		#define LONG_INT_F "%lld"
	#endif
#else
	#if defined __amd64__ || defined __amd64 || defined __x86_64__ || defined __x86_64
		#define LONG_INT long
		#define LONG_INT_F "%ld"
	#else
		#define LONG_INT long long
		#define LONG_INT_F "%lld"
	#endif
#endif

//These values are also defined in Util/BaseGlobals.h in the STR.
//We won't define them again if it's already been included.
#ifndef BaseGlobals_H

//! \brief Definition of PI
//!
//! Definition of PI
static const double PI          = 3.1415926535897932384626;

//! \brief Definition of PI
//!
//! Definition of PI
static const double PI_OVER_TWO          = 0.5 * PI;

//! \brief Earth average radius in km.
//!
//! Earth average radius in km.
const double EARTH_RAD   = 6371.0;

/**
 * Equatorial radius of the Earth in km according to WGS84 ellipsoid.
 */
static const double EARTH_A = 6378.137;

/**
 * EARTH_E is the Earth's eccentricity squared, as defined by the WGS84 ellipsoid.
 * For the WGS84 ellipsoid:
 * flattening, f = 1/298.257223563
 * a = 6378.137 km
 * b = a(1-f) = 6356.752314245180 km
 * e = sqrt( (a^2 - b^2) / a^2 ) = 0.081819190843
 * e^2 = 0.006694379990141320
 */
static const double EARTH_E = 0.006694379990141316; // 0.006694379990141320;

//!
//! Conversion factor from radians to degrees.
static const double RAD_TO_DEG  = 180./3.1415926535897932384626;

//! \brief Conversion factor from degrees to radians.
//!
//! Conversion factor from degrees to radians.
static const double DEG_TO_RAD  = 3.1415926535897932384626/180.;

//! \brief Default constant for 'Not Available'.
//!
//! Default constant for 'Not Available'.
const double NA_VALUE    = -999999.0;

#endif

#define NaN_FLOAT std::numeric_limits<float>::quiet_NaN()
#define NaN_DOUBLE std::numeric_limits<double>::quiet_NaN()

#endif /* CPPGLOBALS_H_ */
