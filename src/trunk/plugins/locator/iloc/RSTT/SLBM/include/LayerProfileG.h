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
//- Program:       LayerProfileG
//- Module:        $RCSfile: LayerProfileG.h,v $
//- Revision:      $Revision: 1.11 $
//- Last Modified: $Date: 2012/12/03 23:59:05 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef LayerProfileG_H
#define LayerProfileG_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <vector>

using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "LayerProfile.h"
#include "InterpolatedProfile.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

class SLBM_EXP_IMP LayerProfileG : public LayerProfile
//! \brief A LayerProfile with mantle velocity gradient information.
//!
//! A LayerProfile with mantle velocity gradient information.
//! The base class, LayerProfile, only stores the depth and velocity at a point
//! on the head wave interface in the earth model.  LayerProfileG class adds
//! mantle velocity gradient information as well.
//!
{

public:

	//! \brief Parameterized constructor.
	//!
	//! Parameterized constructor that builds a LayerProfileG object based on values
	//! interpolated from nearby GridProfile objects.
	//! @param greatCircle the GreatCircle object of which this LayerProfileG will be a member.
	//! @param location the Location where the profile is to be constructed.
	LayerProfileG(GreatCircle* greatCircle, Location& location);

	//! \brief Copy constructor.
	//!
	//! Copy constructor.
	LayerProfileG(const LayerProfileG& LayerProfileG);

	//! \brief Destructor.
	//!
	//! Destructor.
	~LayerProfileG();

	//! \brief Equal operator.
	//!
	//! Equal operator.
	LayerProfileG& operator=(const LayerProfileG& other);

	//! \brief Equality operator.
	//!
	//! Equality operator.
	bool operator==(const LayerProfileG& other);

	//! \brief Inequality operator.
	//!
	//! Inequality operator.
	bool operator!=(const LayerProfileG& other) {return ! (*this == other);};

	//! \brief Retrieve the velocity gradient in the interval represented
	//! by this LayerProfileG, in 1/s.
	//!
	//! Retrieve the velocity gradient in the interval represented
	//! by this LayerProfileG, in 1/s.
	double getGradient() { return gradient; };

	size_t memSize();

private:

	//! \brief P or S velocity gradients in the mantle, in 1/sec.  
	//! 
	//! P or S velocity gradients in the mantle, in 1/sec.  
	double gradient;

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  INLINE FUNCTIONS 
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

inline size_t LayerProfileG::memSize()
{
	return  LayerProfile::memSize() + sizeof(gradient);
}

} // end slbm namespace

#endif // LayerProfileG.h
