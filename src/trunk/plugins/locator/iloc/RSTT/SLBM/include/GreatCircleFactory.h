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


#ifndef GreatCircleFactory_H
#define GreatCircleFactory_H

#include "GreatCircle.h"
#include "GreatCircle_Xn.h"
#include "GreatCircle_Xg.h"

using namespace std;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

class Grid;

//! \brief A factory class that will return the correct type of GreatCircle object for the 
//! specified phase.
//!
//! A factory class that will return the correct type of GreatCircle object for the 
//! specified phase.
class SLBM_EXP_IMP GreatCircleFactory
{
  public:

	//! \brief Default constructor
	//!
	//! Default constructor. Doesn't do anything special.
	GreatCircleFactory();

	//! \brief Destructor
	//!
	//! Destructor. Doesn't do anything special.
	~GreatCircleFactory(); 

	//! \brief Create the appropriate type of GreatCircle object for the specified phase.
	//! 
	//! Create the appropriate type of GreatCircle object for the specified phase.
	//! If the phase is 'Pn' or 'Sn', a GreatCircle_Xn object will be returned.
	//! If the phase is 'Pg' or 'Lg', a GreatCircle_Xg object will be returned.
	//! @param phase must be one of 'Pn', 'Sn', 'Pg', 'Lg'.
	//! @param grid the Grid object from which CrustalProfile and LayerProfile obects
	//! will be interpolated.
	//! @param latSource the geographic latitude of the source, in radians
	//! @param lonSource the geographic longitude of the source, in radians
	//! @param depthSource the depth of the source below sea level, in km
	//! @param latReceiver the geographic latitude of the receiver, in radians
	//! @param lonReceiver the geographic longitude of the receiver, in radians
	//! @param depthReceiver the depth of the receiver below sea level, in km
	//! @param chMax c is the zhao c parameter and h is the turning depth of the
	//! ray below the moho.  Zhao method only valid for c*h << 1.
	//! When c*h > ch_max, then slbm will throw an exception.
	static GreatCircle*  create(
		const int& phase,
		Grid* grid, 
		const double& latSource, 
		const double& lonSource,
		const double& depthSource,
		const double& latReceiver,
		const double& lonReceiver,
		const double& depthReceiver,
		const double& chMax
		);

private:

};

} // end slbm namespace

#endif // GreatCircleFactory_H
