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
//- Program:       InterpolatedProfile
//- Module:        $RCSfile: InterpolatedProfile.h,v $
//- Revision:      $Revision: 1.22 $
//- Last Modified: $Date: 2013/02/22 15:31:24 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef InterpolatedProfile_H
#define InterpolatedProfile_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <vector>

using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "SLBMGlobals.h"
#include "GridProfile.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

class SLBM_EXP_IMP InterpolatedProfile
//! \brief A Profile object based on values interpolated from nearby GridProfile objects.
//!
//! The InterpolatedProfile class  represents a Profile based on depth, velocity and 
//! gradient values interpolated from values from nearby GridProfile objects. 
//! 
//! An InterpolatedProfile object is not particularly useful since it does not own any
//! containers to store depth, velocity and gradient information.  CrustalProfile, 
//! LayerProfile and QueryProfile objects are derived off of InterpolatedProfile 
//! class and provide that functionality.
//! 
{
public:

	//! \brief Default constructor.
	//!
	//! Default constructor.
	InterpolatedProfile();

	//! \brief Parameterized constructor.
	//!
	//! Parameterized constructor that sets up a Profile object based on values 
	//! interpolated from nearby GridProfile objects.
	//! @param grid a reference to the Grid object.  Grid::findProfile() will
	//! be called to retrieve the neighbors and interpolation coefficients.
	//! @param location the Location where the profile is to be constructed
	InterpolatedProfile(Grid& grid, Location& location);

	//! \brief Copy constructor.
	//!
	//! Copy constructor.
	InterpolatedProfile(const InterpolatedProfile& other);

	//! \brief Destructor.
	//!
	//! Destructor.
	virtual ~InterpolatedProfile();
	
	//! \brief Equal operator.
	//!
	//! Equal operator.
	InterpolatedProfile& operator=(const InterpolatedProfile& other);

	//! \brief Equality operator.
	//!
	//! Equality operator.
	bool operator==(const InterpolatedProfile& other);

	//! \brief Inequality operator.
	//!
	//! Inequality operator.
	bool operator!=(const InterpolatedProfile& other) {return ! (*this == other);};

	int getNCoefficients() { return nodes.size(); }

	//! \brief Retrieve a list of pointers to the GridProfile objects upon which
	//! this InterpolatedProfile is dependent.
	//!
	//! Retrieve a list of pointers to the GridProfile objects upon which
	//! this InterpolatedProfile is dependent.
	vector<GridProfile*>& getNodes() { return nodes; }

	GridProfile* getNode(const int& i) { return nodes[i]; }

	vector<int>& getNodeIds() { return nodeIds; }

	int getNodeId(const int& i) { return nodeIds[i]; }

	//! \brief Retrieve the interpolation coefficients that define the dependency
	//! of this InterpolatedProfile on its neighbors.
	//!
	//! Retrieve the interpolation coefficients that define the dependency
	//! of this InterpolatedProfile on its neighbors.  The calling
	//! application is given a const reference to the double array of
	//! values owned by this InterpolatedProfile object.  There will be
	//! SLBMGlobals::nCoefficients elements.
	vector<double>& getCoefficients() { return coefficients; };

	double getCoefficient(const int& i) { return coefficients[i]; }

	//! \brief Retrieve the ID numbers of the GridProfiles that contributed
	//! to the interpolated values at this InterpolatedProfile object.
	//!
	//! Retrieve the ID numbers of the GridProfiles that contributed
	//! to the interpolated values at this InterpolatedProfile object.
	//! It is the responsibility of the caller to supply an int array
	//! large enough to hold the requested values.
	//! There will be SLBMGlobals::nCoefficients elements.
	 void getNodeIds(int* nodeIds, int& size);

	//! \brief Retrieve the interpolation coefficients that were
	//! applied to the neighboring GridProfiles that contributed
	//! to the interpolated values at this InterpolatedProfile object.
	//!
	//! Retrieve the interpolation coefficients that were
	//! applied to the neighboring GridProfiles that contributed
	//! to the interpolated values at this InterpolatedProfile object.
	//! It is the responsibility of the caller to supply a double array
	//! large enough to hold the requested values.
	//! There will be SLBMGlobals::nCoefficients elements.
	void getCoefficients(double* coeff, int& size);

	void getWeights(int* nodeIds, double* coeff, int&size);

	//! \brief Calculate a single radius value based on the neighboring
	//! GridProfile objects and the interpolation coefficients.
	//!
	//! Calculate a single radius value based on the neighboring
	//! GridProfile objects and the interpolation coefficients.
	//! @param k the interval id of the desired radius.
	//! @param radius the interpolated radius value, in km.
	void interpRadius(const int& k, double& radius);

	//! \brief Calculate a single depth value based on the neighboring
	//! GridProfile objects and the interpolation coefficients.
	//!
	//! Calculate a single depth value based on the neighboring
	//! GridProfile objects and the interpolation coefficients.
	//! @param k the interval id of the desired radius.
	//! @param depth the interpolated depth value, in km.
	void interpDepth(const int& k, double& depth);

	//! \brief Calculate a single velocity value based on the neighboring
	//! GridProfile objects and the interpolation coefficients.
	//!
	//! Calculate a single velocity value based on the neighboring
	//! GridProfile objects and the interpolation coefficients.
	//! @param type  either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @param k the interval id of the desired velocity.
	//! @param velocity the interpolated velocity value, in km/sec.
	void interpVelocity(const int& type, const int& k, double& velocity);

	//! \brief Calculate a single gradient value based on the neighboring
	//! GridProfile objects and the interpolation coefficients.
	//!
	//! Calculate a single gradient value based on the neighboring
	//! GridProfile objects and the interpolation coefficients.
	//! @param type  either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @param gradient the interpolated gradient value, in 1/sec.
	void interpGradient(const int& type, double& gradient);

		//! \brief Returns true if all of the neighboring GridProfile objects 
	//! are active nodes.
	//!
	//! Returns true if all of the neighboring GridProfile objects 
	//! are active nodes.
	bool isActiveProfile();

	virtual size_t memSize();

	static int getClassCount(); 

protected:

	static int interpolatedProfileClassCount;

	//! \brief An array of pointers to the GridProfile objects upon which 
	//! this InterpolatedProfile is dependent.
	//!
	//! An array of pointers to the GridProfile objects upon which 
	//! this InterpolatedProfile is dependent.
	vector<GridProfile*> nodes;

	//! \brief An array of pointers to the GridProfile objects upon which
	//! this InterpolatedProfile is dependent.
	//!
	//! An array of pointers to the GridProfile objects upon which
	//! this InterpolatedProfile is dependent.
	vector<int> nodeIds;

	//! \brief The interpolation coefficients which should be applied to this InterpolatedProfile's
	//! neighbors in order to compute interpolated quantities. 
	//!
	//! The interpolation coefficients which should be applied to this InterpolatedProfile's
	//! neighbors in order to compute interpolated quantities. 
	vector<double> coefficients;

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  INLINE FUNCTIONS 
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

inline void InterpolatedProfile::getWeights(int* nodeids, double* coeff, int& size)
{
	size = nodes.size();
	for (int i=0; i<(int)nodes.size(); i++)
	{
		nodeids[i] = nodes[i]->getNodeId();
		coeff[i] = coefficients[i];
	}
}

inline void InterpolatedProfile::getNodeIds(int* nodeids, int& size)
{
	size = nodes.size();
	for (int i=0; i<(int)nodes.size(); i++)
		nodeids[i] = nodes[i]->getNodeId();
}

inline void InterpolatedProfile::getCoefficients(double* coeff, int& size)
{
	size = nodes.size();
	for (int i=0; i<(int)nodes.size(); i++)
		coeff[i] = coefficients[i];
}

inline void InterpolatedProfile::interpRadius(const int& k, double& radius)
{
	radius = 0.;
	for (int i=0; i<(int)nodes.size(); i++) if (nodes[i])
		radius += nodes[i]->getInterfaceRadius(k) * coefficients[i];
}

inline void InterpolatedProfile::interpDepth(const int& k, double& depth)
{
	depth = 0.;
	for (int i=0; i<(int)nodes.size(); i++) if (nodes[i])
		depth += nodes[i]->getInterfaceDepth(k) * coefficients[i];
}

inline void InterpolatedProfile::interpVelocity(const int& type, const int& k, double& velocity)
{
	velocity = 0.;
	for (int i=0; i<(int)nodes.size(); i++) if (nodes[i])
		velocity += nodes[i]->getVelocity(type,k) * coefficients[i];
}

inline void InterpolatedProfile::interpGradient(const int& type, double& gradient)
{
	gradient = 0.;
	for (int i=0; i<(int)nodes.size(); i++) if (nodes[i])
		gradient += nodes[i]->getMantleGradient(type) * coefficients[i];
}

inline bool InterpolatedProfile::isActiveProfile()
{
	for (int i=0; i<(int)nodes.size(); i++)
		if (nodes[i]->getActiveNodeId() < 0)
			return false;
	return true;
}

} // end slbm namespace

#endif // InterpolatedProfile.h
