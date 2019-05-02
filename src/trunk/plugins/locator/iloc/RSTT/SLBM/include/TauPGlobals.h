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

//- ****************************************************************************
//-
//- Program:       SNL TauP Location Library (TauPLoc)
//- Module:        $RCSfile: TauPGlobals.h,v $
//- Creator:       Jim Hipp
//- Creation Date: October 1, 2007
//- Revision:      $Revision: 1.4 $
//- Last Modified: $Date: 2012/12/20 15:26:24 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************

#ifndef TAUPGLOBALS_H
#define TAUPGLOBALS_H

#include "CPPUtils.h"

// **** _BEGIN TAUP NAMESPACE_ *************************************************

namespace taup {

// Definition of dllimport and dllexport here for Windows only
// Note that definitions are repeated for various tools/libraries.
//--------------------------
#if defined(_WIN32) || defined(WIN32)

  // exports when building TAUP dll, imports when linking to header files in
  // TAUP (Note that SLBM_EXPORTS should be defined when building a TAUP
  // DLL, and should  not be defined when linking with the TAUP DLL)
  #ifdef  TAUP_EXPORTS
  #define TAUP_EXP_IMP __declspec(dllexport)
  #else
  #define TAUP_EXP_IMP __declspec(dllimport)
  #endif

  // exports DLL for classes and functions that ONLY export
  // (Note that this is mainly used for templated classes that are not imported)
  #define TAUP_EXP     __declspec(dllexport)

#else  // Sun does not need these

  #define TAUP_EXP_IMP
  #define TAUP_EXP 

#endif
//--------------------------

} // end namespace taup

#endif // TAUPGLOBALS_H
