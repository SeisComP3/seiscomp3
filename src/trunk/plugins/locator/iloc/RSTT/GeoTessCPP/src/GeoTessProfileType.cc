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

#include "GeoTessProfileType.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

/**
 * Instantiate the enums for this type
 */
const GeoTessProfileType GeoTessProfileType::EMPTY    ("EMPTY");
const GeoTessProfileType GeoTessProfileType::THIN     ("THIN");
const GeoTessProfileType GeoTessProfileType::CONSTANT ("CONSTANT");
const GeoTessProfileType GeoTessProfileType::NPOINT   ("NPOINT");
const GeoTessProfileType GeoTessProfileType::SURFACE  ("SURFACE");
const GeoTessProfileType GeoTessProfileType::SURFACE_EMPTY  ("SURFACE_EMPTY");

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

/**
 * Set the enum count and add all enums for this type to the array.
 */
const int						GeoTessProfileType::aSize         = 6;
GeoTessProfileType const*	GeoTessProfileType::aArray[aSize] = {
		&GeoTessProfileType::EMPTY,
		&GeoTessProfileType::THIN,
		&GeoTessProfileType::CONSTANT,
		&GeoTessProfileType::NPOINT,
		&GeoTessProfileType::SURFACE,
		&GeoTessProfileType::SURFACE_EMPTY };

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

} // end namespace geotess
