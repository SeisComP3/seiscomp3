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
//- Program:       QueryProfile
//- Module:        $RCSfile: QueryProfile.h,v $
//- Revision:      $Revision: 1.13 $
//- Last Modified: $Date: 2013/02/22 15:55:24 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef QueryProfile_H
#define QueryProfile_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <vector>

using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "SLBMGlobals.h"
#include "InterpolatedProfile.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

class Grid;
class GridProfile;

class SLBM_EXP_IMP QueryProfile : public InterpolatedProfile
//! \brief An InterpolatedProfile object that also has information about the
//! the P and S wave velocity as a function of depth for all layers, 
//! and the velocity gradient in the mantle.
//!
//! The QueryProfile class represents a Profile based on depth, velocity and
//! gradient values interpolated from values of nearby GridProfile objects.  
//! The simplest way to obtain a QueryProfile object is to call 
//! Grid::getQueryProfile().
{

public:

	//! \brief Parameterized constructor.
	//!
	//! Parameterized constructor that builds a QueryProfile object based on values
	//! interpolated from nearby GridProfile objects.
	//! @param grid a reference to the Grid object.  Grid::findProfile() will
	//! be called to retrieve the neighbors and interpolation coefficients.
	//! @param location the Location of the query position.
	QueryProfile(Grid& grid, Location& location);

	//! \brief Copy constructor.
	//!
	//! Copy constructor.
	QueryProfile(const QueryProfile& QueryProfile);

	~QueryProfile();

	//! \brief Equal operator.
	//!
	//! Equal operator.
	QueryProfile& operator=(const QueryProfile& other);

	//! \brief Equality operator.
	//!
	//! Equality operator.
	bool operator==(const QueryProfile& other);

	//! \brief Inequality operator.
	//!
	//! Inequality operator.
	bool operator!=(const QueryProfile& other) {return ! (*this == other);};

	//! \brief Retrieve the number of intervals associated with this Profile.
	//! 
	//! Retrieve the number of intervals associated with this Profile.
	int  nIntervals() { return NLAYERS; };

	//! \brief Retrieve all the interval depth and velocity information contained in
	//! this QueryProfile object.
	//! 
	//! Retrieve all the interval depth and velocity information contained in
	//! this QueryProfile object.
	void getData(
			int* nodeIds,
			double* coefficients,
			int& nNeighbors,
			double* depths, 
			double* pvelocities,
			double* svelocities,
			double& pgradient,
			double& sgradient
			);

	vector<int>& getNodeIds() { return nodeIds; };

	//! \brief Retrieve the depth of the top of the k'th interval, in km.
	//!
	//! Retrieve the depth of the top of the k'th interval, in km.
	double*  getDepth() { return depth; }; 

	//! \brief Retrieve the P or S wave velocity of the k'th interval, in km/sec.
	//! 
	//! Retrieve the P or S wave velocity of the k'th interval, in km/sec.
	double* getVelocity(const int& waveType) 
	{ return (waveType==PWAVE ? pvelocity : svelocity); };

	//! \brief Retrieve the P or S wave velocity gradient, in 1/sec.
	//! 
	//! Retrieve the P or S wave velocity gradient, in 1/sec.
	double* getMantleGradient() { return gradient; };

	//! \brief Returns a formatted string containing detailed information about this Profile.
	//!
	//! Returns a formatted string containing detailed information about this Profile.
	string toString();

	static int getClassCount();

private:

	static int queryProfileClassCount;

	Location location;

	vector<int> nodeIds;

	//! \brief Depth of the top of each interval, in km.
	//!
	//! Depth of the top of each interval, in km.  
	double depth[NLAYERS];

	//! \brief The P velocity of each interval, in km/sec.
	//! 
	//! The P velocity of each interval, in km/sec.
	double pvelocity[NLAYERS];

	//! \brief The S velocity of each interval, in km/sec.
	//! 
	//! The S velocity of each interval, in km/sec.
	double svelocity[NLAYERS];

	//! \brief The P and S velocity gradients in the mantle, in 1/sec.
	//! 
	//! The P and S velocity gradients in the mantle, in 1/sec.
	double gradient[2];

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  INLINE FUNCTIONS 
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

inline void QueryProfile::getData(
			int* nodeids,
			double* coeff,
			int& n,
			double* d, 
			double* pv,
			double* sv,
			double& pg,
			double& sg
			)
{
	getWeights(nodeids, coeff, n);

	for (int i=0; i<NLAYERS; i++)
	{
		d[i] = depth[i];
		pv[i] = pvelocity[i];
		sv[i] = svelocity[i];
	}

	pg = gradient[PWAVE];
	sg = gradient[SWAVE];
}

} // end slbm namespace

#endif // QueryProfile.h
