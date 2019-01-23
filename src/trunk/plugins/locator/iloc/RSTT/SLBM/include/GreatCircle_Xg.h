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
//- Program:       GreatCircle_Xg
//- Module:        $RCSfile: GreatCircle_Xg.h,v $
//- Revision:      $Revision: 1.28 $
//- Last Modified: $Date: 2011/11/30 22:41:26 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef GreatCircle_Xg_H
#define GreatCircle_Xg_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <vector>
#include <string>
#include <iostream>

// **** _LOCAL INCLUDES_ *******************************************************

#include "SLBMGlobals.h"
#include "GreatCircle.h"
#include "Grid.h"
#include "Location.h"
#include "CrustalProfile.h"
#include "LayerProfile.h"
#include "TauPSite.h"

using namespace std;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

// **** _CLASS DEFINITION_ *****************************************************
//! \brief The GreatCircle_Xg class manages information related to a great circle path
//! between two Locations on the Earth.  Its computeTravelTime() methods compute 
//! travel times for Pg or Lg phases.
//!
//! The GreatCircle_Xg class manages information related to a great circle path
//! between two Locations on the Earth.  It inherits much of its 
//! functionality from the GreatCircle class from which it is derived but
//! overrides the computeTravelTime() method to compute travel times for 
//! Pg and Lg phases.  It also overrides the toString() method.
//!
//! Applications should use a GreatCircleFactory object to obtain pointers to  
//! GreatCircle_Xg objects.
//!
class SLBM_EXP GreatCircle_Xg : public GreatCircle
{

public:

	//! \brief Parameterized constructor.  
	//! 
	//! Parameterized constructor.  
	//! @param _phase the phase that this GreatCircle_Xg object is to support.
	//! Must be one of SLBMGlobals::Pg or SLBMGlobals::Lg.
	//! @param _grid The Grid from which LayerProfile objects will be extracted.
	//! @param latSource the geographic latitude of the source, in radians
	//! @param lonSource the geographic longitude of the source, in radians
	//! @param depthSource the depth of the source below sea level, in km
	//! @param latReceiver the geographic latitude of the receiver, in radians
	//! @param lonReceiver the geographic longitude of the receiver, in radians
	//! @param depthReceiver the depth of the receiver below sea level, in km
	GreatCircle_Xg(
		const int& _phase,
		Grid& _grid, 
		const double& latSource, 
		const double& lonSource,
		const double& depthSource,
		const double& latReceiver,
		const double& lonReceiver,
		const double& depthReceiver);

	//! \brief Destructor.  Calls GreatCircle destructor, which deletes
	//! the source and receiver CrustalProfile objects
	//! and all LayerProfile objects created by this GreatCircle_Xg object.  
	//!
	//! Destructor.  Calls destructor, which deletes
	//! the source and receiver CrustalProfile objects
	//! and all LayerProfile objects created by this GreatCircle_Xg object.  
	~GreatCircle_Xg();

	//! \brief Copy constructor.
	//! 
	//! Copy constructor.
	GreatCircle_Xg(const GreatCircle_Xg &other);

	//! \brief Equal operator.
	//! 
	//! Equal operator.
	GreatCircle_Xg& operator=(const GreatCircle_Xg& other);

	//! brief Computes the geometry of the ray, all the components of 
	//! the traveltime, and the total travel time.
	//!
	//! Computes the geometry of the ray, all the components of 
	//! the traveltime, and the total travel time.
	void computeTravelTime();

	void getZhaoParameters(double& Vm, double& Gm, double& H, double& C, double& Cm, int& udSign);

	void getPgLgComponents(double& tTotal, 
								double& tTaup, double& tHeadwave, 
								double& pTaup, double& pHeadwave, 
								double& trTaup, double& trHeadwave);

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
	
	//! \brief The ray parameter for the crustal stack at the source, 
	//! in sec/km.
	//!
	//! The ray parameter for the crustal stack at the source, in sec/km.
	double pSource;

	//! Total travel time using headwave solution method.
	double tHeadwave;

	//! Total travel time using TauP solution method.
	//double tTaup;

	//! Spherical ray parameter computed using the headwave method
	double pHeadwave;

	//! Spherical ray parameter computed using the headwave method
	double pTaup;

	//! turning radius computed using the headwave method
	double trHeadwave;

	//! turning radius computed using the taup method
	//double trTaup;

	double taupModelRadius;

	taup::TravelTimeResult* taupResult;

	void computeTravelTimeTaup();

	void computeTravelTimeHeadwave();

	//void computeTravelTimeHeadwave2();

	void toStringTaup(ostringstream&, const int& verbosity);

	void toStringHeadwave(ostringstream&, const int& verbosity);

};

inline void GreatCircle_Xg::getZhaoParameters(double& Vm_, double& Gm_, double& H_, double& C_, double& Cm_, int& udSign_)
{
	Vm_ = NA_VALUE;
	Gm_ = NA_VALUE;
	H_ = NA_VALUE;
	C_ = NA_VALUE;
	Cm_ = NA_VALUE;
	udSign_ = -999;
}

inline void GreatCircle_Xg::getPgLgComponents(double& tT, double& tP, double& tH, 
      double& pT, double& pH, double& trT, double& trH)
{ 
	tT = tTotal; 
	tP = (taupResult ? taupResult->ttrT : NA_VALUE);
	tH = tHeadwave; 
	pT = (taupResult ? taupResult->ttrP / taupResult->ttrR : NA_VALUE);
	pH = pHeadwave;
	trT = (taupResult ? taupResult->ttrR : NA_VALUE);
	trH = trHeadwave;
}

} // end slbm namespace

#endif // GreatCircle_Xg.h
