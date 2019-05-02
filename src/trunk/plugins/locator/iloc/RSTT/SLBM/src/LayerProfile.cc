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
//- Module:        $RCSfile: LayerProfile.cc,v $
//- Revision:      $Revision: 1.13 $
//- Last Modified: $Date: 2012/12/03 23:59:05 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#include "Grid.h"
#include "LayerProfile.h"
#include "GridProfile.h"
#include "GreatCircle.h"
#include <iostream>

//using namespace std;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

// **** _STATIC INITIALIZATIONS_************************************************

int LayerProfile::layerProfileClassCount = 0;

LayerProfile::LayerProfile(GreatCircle* greatCircle, 
						   Location& loc)
						   : InterpolatedProfile(greatCircle->getGrid(), loc)
{
	++layerProfileClassCount;

	// find the index of the head wave interface from the great circle and
	// interpolate the radius at that interface.
	interpRadius(greatCircle->getHeadWaveInterface(), radius);

	// find out if this LayerProfile supports pwave or swave from the greatcircle.
	// Also find the index of the head wave interface from the greatcircle.
	// Then interpolate the p or s velocity at that interface.
	interpVelocity(greatCircle->getPhase()%2, 
		greatCircle->getHeadWaveInterface(), 
		velocity);
}

LayerProfile::LayerProfile(const LayerProfile &other) 
	: InterpolatedProfile(other),
	radius(other.radius), 
	velocity(other.velocity)
{
	++layerProfileClassCount;
}

LayerProfile::~LayerProfile()
{
	--layerProfileClassCount;
}

LayerProfile& LayerProfile::operator=(const LayerProfile& other)
{
	InterpolatedProfile::operator = (other);
	radius = other.radius;
	velocity=other.velocity;
    return *this;
}

bool LayerProfile::operator==(const LayerProfile& other)
{
	return 
		InterpolatedProfile::operator == (other)
		&& radius == other.radius
		&& velocity == other.velocity;
}

int LayerProfile::getClassCount()
{
  return layerProfileClassCount;
}

} // end slbm namespace
