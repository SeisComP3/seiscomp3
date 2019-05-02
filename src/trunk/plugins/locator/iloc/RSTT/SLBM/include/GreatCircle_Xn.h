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
//- Program:       GreatCircle_Xn
//- Module:        $RCSfile: GreatCircle_Xn.h,v $
//- Revision:      $Revision: 1.19 $
//- Last Modified: $Date: 2011/11/30 22:41:26 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef GreatCircle_Xn_H
#define GreatCircle_Xn_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <vector>
#include <string>
#include <iostream>

using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "SLBMGlobals.h"
#include "GreatCircle.h"
#include "Grid.h"
#include "Location.h"
#include "CrustalProfile.h"
#include "LayerProfile.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

// **** _CLASS DEFINITION_ *****************************************************
//
//! \brief The GreatCircle_Xn class manages information related to a great circle path
//! between two Locations on the Earth.  It uses the
//! Zhao method to compute travel times for Pn and Sn phases.
//!
//! The GreatCircle_Xn class manages information related to a great circle path
//! between two Locations on the Earth.  It uses the
//! Zhao method to compute travel times for Pn and Sn phases.
//!
//! GreatCircle_Xn inherits much of its 
//! functionality from the GreatCircle class from which it is derived but
//! overrides the computeTravelTime() and toString() methods.
//!
//! Applications should use a GreatCircleFactory object to obtain pointers to  
//! GreatCircle_Xn objects.
//!
class SLBM_EXP_IMP GreatCircle_Xn : public GreatCircle
{

public:

	//! \brief Parameterized constructor.  
	//! 
	//! Parameterized constructor.  
	//! @param _phase the phase that this GreatCircle_Xn object is to support.
	//! Must be one of SLBMGlobals::Pn or SLBMGlobals::Sn.
	//! @param _grid The Grid from which LayerProfile objects
	//! will be extracted.
	//! @param latSource the geographic latitude of the source, in radians
	//! @param lonSource the geographic longitude of the source, in radians
	//! @param depthSource the depth of the source below sea level, in km
	//! @param latReceiver the geographic latitude of the receiver, in radians
	//! @param lonReceiver the geographic longitude of the receiver, in radians
	//! @param depthReceiver the depth of the receiver below sea level, in km
	//! @param chMax c is the zhao c parameter and h is the turning depth of the
	//! ray below the moho.  Zhao method only valid for c*h << 1.
	//! When c*h > ch_max, then slbm will throw an exception.
	GreatCircle_Xn(
		const int& _phase,
		Grid& _grid, 
		const double& latSource, 
		const double& lonSource,
		const double& depthSource,
		const double& latReceiver,
		const double& lonReceiver,
		const double& depthReceiver,
		const double& chMax);

	//! \brief Destructor.  Deletes the source and receiver CrustalProfile objects
	//! and all LayerProfile objects created by this GreatCircle_Xn object.  
	//!
	//! Destructor.  Deletes the source and receiver CrustalProfile objects
	//! and all LayerProfile objects created by this GreatCircle_Xn object.  
	~GreatCircle_Xn();

	//! \brief Copy constructor.
	//! 
	//! Copy constructor.
	GreatCircle_Xn(const GreatCircle_Xn &other);

	//! \brief Equal operator.
	//! 
	//! Equal operator.
	GreatCircle_Xn& operator=(const GreatCircle_Xn& other);

	void getZhaoParameters(double& Vm, double& Gm, double& H, double& C, double& Cm, int& udSign);

	//! \brief Retrieve a formatted string providing a detailed description
	//! of the information managed by this GreatCircle object.  
	//!
	//! Retrieve a formatted string providing a detailed description
	//! of the information managed by this GreatCircle object.  
	//! @param verbosity specifies the amount of information
	//! that is to be included in the return string.  Each 
	//! verbosity level includes all information in preceeding
	//! verbosity levels.
	//! - 0 : nothing.  An empty string is returned.
	//! - 1 : total distance and travel time summary
	//! - 2 : gradient correction information for Pn/Sn.
	//!       Nothing for Pg/Lg
	//! - 3 : Source and receiver profiles
	//! - 4 : Grid node weights.
	//! - 5 : Head wave interface profiles
	//! - 6 : Interpolation coefficients for great circle nodes on
	//!       the head wave interface.
	string toString(const int& verbosity);

private:

	//! c is the zhao c parameter and h is the turning depth of the 
	//! ray below the moho.  Zhao method only valid for c*h << 1.
	//! When c*h > ch_max, then slbm will throw an exception.
	double ch_max;

	//! \brief The path average velocity in the mantle along
	//! the great circle, in km/sec.
	//!
	//! The path average velocity in the mantle along
	//! the great circle, in km/sec.
	double Vm;

	//! \brief The path average velocity gradient in the mantle along
	//! the great circle, in 1/sec.
	//!
	//! The path average velocity gradient in the mantle along
	//! the great circle, in 1/sec.
	double Gm;

	//! \brief The depth of the turning point of the ray below the 
	//! Moho, in km.
	//!
	//! The depth of the turning point of the ray below the 
	//! Moho, in km.
	double H;

	//! \brief The average of Moho radius at the source and receiver, in km.
	//!
	//! The average of Moho radius at the source and receiver, in km.
	double rMoho;
	double rZm;

	//! \brief Flag indicating whether the ray leaves the source in 
	//! the upgoing (-1) or downgoing (+1) direction.
	//!
	//! Flag indicating whether the ray leaves the source in 
	//! the upgoing (-1) or downgoing (+1) direction.
	int udSign;

	double V0;

	//! brief Computes the geometry of the ray, all the components of 
	//! the traveltime, and the total travel time.
	//!
	//! Computes the geometry of the ray, all the components of 
	//! the traveltime, and the total travel time.
	void computeTravelTime();

	bool inCrust;

	void computeTravelTimeCrust();

	void computeTravelTimeMantle();

	double c, cm, zm, zhao_r, cz, cmz;

	const double cmin;

	double func(const double& h);

	void mnbrak(double &ax, double &bx, double &cx, 
				double &fa, double &fb, double &fc);

	double brent(const double ax, const double bx, const double cx, 
		const double tol, double &xmin);

	void SWAP(double &a, double &b)	{double dum=a; a=b; b=dum;}

	double SIGN(const double &a, const double &b)
		{return b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a);};

	void shft3(double &a, double &b, double &cc, const double d)
		{ a=b; b=cc; cc=d;};

};

inline void GreatCircle_Xn::getZhaoParameters(double& Vm_, double& Gm_, double& H_, double& C_, double& Cm_, int& udSign_)
{
	Vm_ = Vm;
	Gm_ = Gm;
	H_ = H;
	C_ = c;
	Cm_ = cm;
	udSign_ = udSign;
}

} // end slbm namespace

#endif // GreatCircle_Xn.h
