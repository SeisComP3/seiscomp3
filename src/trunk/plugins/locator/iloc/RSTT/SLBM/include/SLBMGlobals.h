//- ****************************************************************************
//- 
//- Copyright 2009 Sandia Corporation. Under the terms of Contract 
//- DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains 
//- certain rights in this software.
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

#ifndef SLBMGlobals_H
#define SLBMGlobals_H

#include "CPPGlobals.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

//! \brief Software Version Number
//! 
//! Software Version Number
static const char* SlbmVersion = "3.0.5";

// version 3.0.3 4/22/2014  sballar
// fixed bug in GridGeoTess::getActiveNodeNeighbors that caused the
// code to return twice as many node neighbors as it was supposed to.
// Also improved Uncertainty::constructors so that the paths to the
// uncertainty files get assembled with appropriate path separators.
// Fixed bug where modelPath was not set properly in GridGeoTess.cc.
// Improved performance of GeoTessCPP::natural neighbor interpolation.
// in GeoTessCPP version 2.2.0.
//
// version 3.0.2 10/10/2013  sballar
// Added "include <cstring>" to GeoTess IFStream.h so that
// it would compile with RedHat Linux v6.4
// Also changed getline() to getline_slbm() in slbmtestc.c to
// circumvent conflict in new version of gcc compiler on Linxux

//! \brief Util Library Version Number
//! 
//! Util Library  Version Number
//static const char* UtilVersion = "1.0.4";

//! \brief TauPLoc Library Version Number
//! 
//! TauPLoc Library  Version Number
//static const char* TauPLocVersion = "1.1.2";

//! \brief Convenience constant for P wave index.
//! 
//! Convenience constant for P wave index.
static const int PWAVE = 0;

//! \brief Convenience constant for S wave index
//! 
//! Convenience constant for S wave index
static const int SWAVE = 1;

//! \brief Convenience constant for Pn phase
//!
//! Convenience constant for Pn phase
static const int Pn = 0;

//! \brief Convenience constant for Sn phase
//!
//! Convenience constant for Sn phase
static const int Sn = 1;

//! \brief Convenience constant for Pg phase
//!
//! Convenience constant for Pg phase
static const int Pg = 2;

//! \brief Convenience constant for attribute TT
//!
//! Convenience constant for attribute TT
static const int TT = 0;

//! \brief Convenience constant for attribute SH
//!
//! Convenience constant for attribute SH
static const int SH = 1;

//! \brief Convenience constant for attribute AZ
//!
//! Convenience constant for attribute AZ
static const int AZ = 2;

//! \brief Convenience constant for Lg phase
//!
//! Convenience constant for Lg phase
static const int Lg = 3;

static const int WATER          = 0;
static const int SEDIMENT1      = 1;
static const int SEDIMENT2      = 2;
static const int SEDIMENT3      = 3;
static const int UPPER_CRUST    = 4;
static const int MIDDLE_CRUST_N = 5;
static const int MIDDLE_CRUST_G = 6;
static const int LOWER_CRUST    = 7;
static const int MANTLE         = 8;
static const int NLAYERS        = 9;		
// MANTLE has to be last layer and must == NLAYERS-1

// Parameter TOP_LAYER controls behavior when sources/receivers 
// are in layer WATER.  
// If TOP_LAYER = 0, and phase = Pn/Pg then rays that travel 
// through the WATER layer travel at the velocity of water.  
// If TOP_LAYER = 0, and phase = Sn/Lg then rays that travel 
// through the WATER layer cause an exception to be thrown.
// If TOP_LAYER = 1 and the depth of a source or receiver is 
// less than the depth of the top of layer SEDIMENT1, then
// the code will search downward through the layers, starting 
// at layer SEDIMENT1, until if finds a layer with non-zero
// thickness.  It will then extend the top of that layer up
// to the depth of the source/receiver.  This essentially 
// assumes that all sources/receivers in the ocean are actually
// located on islands that are too small to have been explicitly
// defined in the model.
//
// Bottom line is: to include water set TOP_LAYER = 0.
// To ignore water, set TOP_LAYER = 1;
static const int TOP_LAYER = 1;

// sb commented this out 12/2/2012
//static const int nCoefficients = 3;

} // end slbm namespace

// Definition of dllimport and dllexport here for Windows only
// Note that definitions are repeated for various tools/libraries.
//--------------------------
#if defined(_WIN32) || defined(WIN32)

  // exports when building SLBM dll, imports when linking to header files in
  // SLBM (Note that SLBM_EXPORTS should be defined when building a SLBM
  // DLL, and should  not be defined when linking with the SLBM DLL)
  #ifdef  SLBM_EXPORTS
  #define SLBM_EXP_IMP __declspec(dllexport)
  #else
  #define SLBM_EXP_IMP __declspec(dllimport)
  #endif

  // exports DLL for classes and functions that ONLY export
  // (Note that this is mainly used for templated classes that are not imported)
  #define SLBM_EXP     __declspec(dllexport)

#else  // Sun does not need these

  #define SLBM_EXP_IMP
  #define SLBM_EXP 

#endif
//--------------------------

#endif // SLBMGlobals_H
