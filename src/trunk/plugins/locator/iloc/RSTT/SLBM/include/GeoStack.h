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
//- Program:       GeoStack
//- Module:        $RCSfile: GeoStack.h,v $
//- Revision:      $Revision: 1.13 $
//- Last Modified: $Date: 2013/05/04 19:41:51 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef GeoStack_H
#define GeoStack_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <vector>
#include <set>

using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "SLBMGlobals.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

class SLBM_EXP_IMP GeoStack
//! \brief Manages all information related to a single node in a Grid object.
//!
//! \brief Manages all information related to a single node in a Grid object
//! including: the depths of all model interfaces, the P and S velocities of each 
//! model interval, and the P and S velocity gradients in the mantle.
//!
//! GeoStack objects are created in Grid::loadVelocityModel() and deleted 
//! in Grid::~Grid.
{

public:

	//! \brief Default constructor.
	//!
	//! Default constructor.
	GeoStack();

    GeoStack(const int& index, double* depths, double* pvelocities, 
		double* svelocities, double* gradients);

	//! \brief Destructor.
	//!
	//! Destructor.
	~GeoStack();

	//! \brief Copy constructor
	//! 
	//! Copy constructor.
	GeoStack(const GeoStack& GeoStack);

	//! \brief Equal operator.
	//! 
	//! Equal operator.
	GeoStack& operator=(const GeoStack& other);

	//! \brief Equality operator
	//! 
	//! Equality operator.
	bool operator==(const GeoStack& other);

	//! \brief Inequality operator
	//! 
	//! Inequality operator.
	bool operator!=(const GeoStack& other) {return ! (*this == other);};

	int getRefCount() { return refCount; };

	void incRefCount() { ++refCount; };

	void decRefCount() { --refCount; };

	void setRefCount(const int& count) { refCount = count; };

	int getIndex() { return index; };

	void setIndex(const int& idx) { index = idx; };

	//! \brief Retrieve all the data associated with this Profile.
	//!
	//! Retrieve all the data associated with this Profile.
	//! @param depths the depths of the top of 
	//! each interval associated with this Profile.
	//! @param pvelocity the P velocities of  each interval, in km/sec.
	//! @param svelocity the S velocities of  each interval, in km/sec.
	//! @param gradient a 2-element array specifying the P and S velocity
	//! gradient in the mantle, in 1/sec.
	void getData(double* depths, 
		double* pvelocity, double* svelocity, double* gradient);

	//! \brief Set the P and S velocities and gradients associated with this
	//! GeoStack object to specified values.
	//!
	//! Set the P and S velocities and gradients associated with this
	//! GeoStack object to specified values.
	//! @param depths the depths of the layers below sea level, in km.
	//! @param pvelocities the P velocities stored in this GeoStack, in km/sec.
	//! @param svelocities the S velocities stored in this GeoStack, in km/sec.
	//! @param gradients a 2-element vector specifying the P and S velocity
	//! gradients in the mantle, in 1/sec.
	void setData(double* depths, double* pvelocities, double* svelocities,	double* gradients);

	//! \brief Set the layer depths to specified values.
	//!
	//! Set the layer depths to specified values.
	//! @param z the depths of the layers below surface of solid earth
	void setDepth(const vector<double>& z);

	//! \brief Set the P or S velocity to specified values.
	//!
	//! Set the P or S velocity to specified values.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @param velocity the P or S velocity, in km/sec.
	void setVelocity(const int& waveType, const vector<double>& velocity);

	//! \brief Set the P and S velocity gradients to specified values.
	//!
	//! Set the P and S velocity gradients to specified values.
	//! @param gradient the P and S velocity gradients, in km/sec.
	void setGradient(const vector<double>& gradient);

	//! \brief Retrieve the depth of the k'th interval, in km.
	//! 
	//! Retrieve the depth of the k'th interval, in km.
	double getDepth(const int& k) { return depth[k]; };

	double* getDepth() { return depth; };

	//! \brief Retrieve the P or S velocity of the k'th interval, in km/sec.
	//! 
	//! Retrieve the P or S velocity of the k'th interval, in km/sec.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @param k the index of the desired interval.  The shallowest interval
	//! is index 0.
	double getVelocity(const int& waveType, const int& k);

	//! \brief Retrieve the P or S velocity gradient in the mantle, in 1/sec.
	//! 
	//! Retrieve the P or S velocity gradient in the mantle, in 1/sec.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	double getMantleGradient(const int& waveType);

	//! \brief Retrieve the P or S velocities of all intervals, in km/sec.
	//! 
	//! Retrieve the P or S velocities of all intervals, in km/sec.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @param velocity (output) this argument will be populated with the P or S velocity in km/sec
	void getVelocity(const int& waveType, double* velocity);

	//! \brief Retrieve the P and S velocity gradients in the mantle, in 1/sec.
	//! 
	//! Retrieve the P and S velocity gradients in the mantle, in 1/sec.
	void getMantleGradient(double* g) { g[0] = gradient[0]; g[1] = gradient[1]; };

	//! \brief Return true if (1) any finite thickness layer above the middle crust
	//! has a velocity that exceeds the Pg/Lg velocity of the middle crust
	//! or (2) any finite thickness crustal layer has a velocity that exceeds
	//! the mantle velocity.
	//!
	//! Return true if (1) any finite thickness layer above the middle crust
	//! has a velocity that exceeds the Pg/Lg velocity of the middle crust
	//! or (2) any finite thickness crustal layer has a velocity that exceeds
	//! the mantle velocity.
	bool hasLowVelocityZone();

	//! \brief Return the information content of this GeoStack formatted in 
	//! small text table.
	//!
	//! Return the information content of this GeoStack formatted in 
	//! small text table.
	string toString();

	static int getClassCount();

	bool thicknessTest();

protected:

	static int geoStackClassCount;

	int index;

	int refCount;

	//! \brief depth of the top of each interval, in km.
	//!
	//! depth of the top of each interval, in km.  There will be n elements,
	//! one for each crustal layer, and one more for the mantle.  The
	//! last element will be the depth of the Moho.
	double depth[NLAYERS];

	//! \brief The P and S velocity of each layer, in km/sec.
	//! 
	//! The P and S velocity of each layer, in km/sec.
	//! This will be an 2 x n vector, where n is the number of intervals.  The
	//! last elements are the velocity in the mantle.
	double pvelocity[NLAYERS];
	double svelocity[NLAYERS];
	
	//! \brief P and S velocity gradients in the mantle, in 1/sec.  
	//! 
	//! 2-element vector containing the P and S velocity gradients in the mantle, in 1/sec.  
	double gradient[2];

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  INLINE FUNCTIONS 
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

inline void GeoStack::getData(double* d, double* pv, double* sv, double* g)
{
	for (int i=0; i<NLAYERS; i++)
	{
		d[i] = depth[i];
		pv[i] = pvelocity[i];
		sv[i] = svelocity[i];
	}
	g[PWAVE] = gradient[PWAVE];
	g[SWAVE] = gradient[SWAVE];
}

inline void GeoStack::setData(double* depths, double* pvelocities, 
							  double* svelocities,	double* gradients)
{
	for (int i=0; i<NLAYERS; i++)
	{
		depth[i] = depths[i];
		pvelocity[i] = pvelocities[i];
		svelocity[i] = svelocities[i];
	}
	gradient[PWAVE] = gradients[PWAVE];
	gradient[SWAVE] = gradients[SWAVE];

	thicknessTest();
}

inline double GeoStack::getVelocity(const int& waveType, const int& k)
{
	if (waveType == PWAVE)
		return pvelocity[k];
	return svelocity[k];
}

inline double GeoStack::getMantleGradient(const int& waveType)
{
	return gradient[waveType];
}

inline void GeoStack::getVelocity(const int& waveType, double* v)
{
	if (waveType == PWAVE)
		for (int k=0; k<NLAYERS; ++k)
			v[k] = pvelocity[k];
	else
		for (int k=0; k<NLAYERS; ++k)
			v[k] = svelocity[k];
}

inline void GeoStack::setDepth(const vector<double>& z)
{
	for (int i=0; i<NLAYERS; i++)
		depth[i] = z[i];

	thicknessTest();
}

inline void GeoStack::setVelocity(const int& waveType, const vector<double>& v)
{
	if (waveType == PWAVE)
		for (int i=0; i<NLAYERS; i++)
			pvelocity[i] = v[i];
	else if (waveType == SWAVE)
		for (int i=0; i<NLAYERS; i++)
			svelocity[i] = v[i];
}

inline void GeoStack::setGradient(const vector<double>& g)
{
	gradient[PWAVE] = g[PWAVE];
	gradient[SWAVE] = g[SWAVE];
}

} // end slbm namespace

#endif // GeoStack.h
