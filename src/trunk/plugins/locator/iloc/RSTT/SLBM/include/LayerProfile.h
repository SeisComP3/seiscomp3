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
//- Program:       LayerProfile
//- Module:        $RCSfile: LayerProfile.h,v $
//- Revision:      $Revision: 1.13 $
//- Last Modified: $Date: 2012/12/03 23:59:05 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef LayerProfile_H
#define LayerProfile_H

// **** _SYSTEM INCLUDES_ ******************************************************

using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "SLBMGlobals.h"
#include "InterpolatedProfile.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

class GreatCircle;

//! \brief A Profile object based on values interpolated from nearby GridProfile objects.
//!
//! A LayerProfile object only stores the depth, velocity and gradient for a single 
//! interval of the Earth model.  For a LayerProfile object instantiated to support the Pn
//! or Sn phase, the interval represented is the mantle.  For Pg and Lg phases,
//! the represented interval is the middle crust.  For Pn and Pg, the velocity stored
//! is the P wave velocity.  For Sn and Lg, S wave velocity is stored.  For Pn and Sn, 
//! P wave and S wave mantle velocity gradient information is stored, respectively.
//! For Pg and Lg, the mantle gradient attribute is set to SLBMGlobals::NA_VALUE.
//!
//! LayerProfile objects are designed to be as small as possible.  From parent
//! class InterpolatedProfile, they inherit the node IDs and interpolation
//! coefficients that relate them to neighboring GridProfile objects.  On top 
//! of this they store a single radius and velocity value.  If the supported 
//! phase is Pn or Sn, they also store a gradient value.
//!
//! LayerProfile objects do not store a Location and hence do not know their
//! position in space.  The GreatCircle object that owns the LayerProfile 
//! object can determine its position however.
class SLBM_EXP_IMP LayerProfile : public InterpolatedProfile
{

public:

	//! \brief Parameterized constructor.
	//!
	//! Parameterized constructor that builds a LayerProfile object based on values
	//! interpolated from nearby GridProfile objects.
	//! @param greatCircle the GreatCircle object of which this LayerProfile will be a member.
	//! @param location the Location where the profile is be constructed.
	LayerProfile(GreatCircle* greatCircle, Location& location);

	//! \brief Copy constructor.
	//!
	//! Copy constructor.
	LayerProfile(const LayerProfile& LayerProfile);

	//! \brief Destructor.
	//!
	//! Destructor.
	~LayerProfile();

	//! \brief Equal operator.
	//!
	//! Equal operator.
	LayerProfile& operator=(const LayerProfile& other);

	//! \brief Equality operator.
	//!
	//! Equality operator.
	bool operator==(const LayerProfile& other);

	//! \brief Inequality operator.
	//!
	//! Inequality operator.
	bool operator!=(const LayerProfile& other) {return ! (*this == other);};

	//! \brief Retrieve the number of intervals associated with this Profile
	//! (always returns 1).
	//! 
	//! Retrieve the number of intervals associated with this Profile
	//! (always returns 1).
	int  nIntervals() { return 1; };

	//! \brief Retrieve the radius of the top of the interval represented
	//! by this LayerProfile, in km.
	//!
	//! Retrieve the radius of the top of the interval represented
	//! by this LayerProfile, in km.
	double getRadius() { return radius; };

	//! \brief Retrieve the velocity of the interval represented
	//! by this LayerProfile, in km/s.
	//!
	//! Retrieve the velocity of the interval represented
	//! by this LayerProfile, in km/s.
	double getVelocity() { return velocity; };

	//! \brief Retrieve the velocity gradient in the interval represented
	//! by this LayerProfile, in 1/s.
	//!
	//! Retrieve the velocity gradient in the interval represented
	//! by this LayerProfile, in 1/s.  For Pg and Lg, returns 
	//! SLBMGlobals::NA_VALUE.
	virtual double getGradient() { return NA_VALUE; };

	virtual size_t memSize();

	static int getClassCount();

protected:

	static int layerProfileClassCount;

	//! \brief The radius of the interface represented by this
	//! LayerProfile object, in km.
	//! 
	//! The radius of the interface represented by this
	//! LayerProfile object, in km.
	double radius;

	//! \brief The P or S velocity of the interval, in km/sec.
	//! 
	//! The P or S velocity of the interval, in km/sec.
	double velocity;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  INLINE FUNCTIONS 
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

inline size_t LayerProfile::memSize()
{
	return  InterpolatedProfile::memSize() 
		+ sizeof(radius)
		+ sizeof(velocity)
		;
}

} // end slbm namespace

#endif // LayerProfile.h
