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
//- Program:       GridProfile
//- Module:        $RCSfile: GridProfileSLBM.h,v $
//- Revision:      $Revision: 1.5 $
//- Last Modified: $Date: 2012/12/05 14:17:06 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef GRIDPROFILESLBM_H_
#define GRIDPROFILESLBM_H_

// **** _SYSTEM INCLUDES_ ******************************************************

#include <vector>
#include <set>

using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "SLBMGlobals.h"
#include "Location.h"
#include "GeoStack.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

class InterpolatedProfile;
class Grid;

class SLBM_EXP_IMP GridProfileSLBM : public GridProfile
//! \brief Manages all information related to a single node in a Grid object.
//!
//! Manages all information related to a single node in a Grid object
//! including: the Location of the node, the depths of all model interfaces,
//! the P and S velocities of each model interval, and the P and S velocity
//! gradients in the mantle.
//!
//! GridProfileSLBM objects are created in Grid::loadVelocityModel() and deleted
//! in Grid::~Grid.
{

// let Grid objects access protected members of GridProfileSLBM.
friend class Grid;

public:

	//! \brief Parameterized constructor.
	//!
	//! Parameterized constructor.  Note that the Location that this
	//! inherits from is having depth set to zero so that the radius
	//! of this object is equivalent to calling getEarthRadius().
	GridProfileSLBM(Grid& g,
			 const int& i, const double& lat,
			 const double& lon, const double& elev,
			 const double& zwater, GeoStack* gstack);

	//! \brief Copy constructor.
	//!
	//! Copy constructor.
	GridProfileSLBM(const GridProfileSLBM &other);

	//! \brief Destructor.
	//!
	//! Destructor.
	~GridProfileSLBM();

	static string class_name() { return "GridProfileSLBM"; };

	//! \brief Retrieve the index of the GeoStack that holds the
	//! information about model interfaces, velocities and
	//! gradients.
	//!
	//! Retrieve the index of the GeoStack that holds the
	//! information about model interfaces, velocities and
	//! gradients.
	int getGeoStackId() { return geoStack->getIndex(); };

	//! \brief Retrieve all the data associated with this Profile.
	//!
	//! Retrieve all the data associated with this Profile.
	//! @param depths the depths of the top of
	//! each interval associated with this Profile.
	//! @param pvelocity the P velocities of  each interval, in km/sec.
	//! @param svelocity the S velocities of  each interval, in km/sec.
	//! @param gradient a 2-element array specifying the P and S velocity
	//! gradient in the mantle, in 1/sec.
	void getData(double* depths, double* pvelocity, double* svelocity, double* gradient);

	//! \brief Set the P and S velocities and gradients associated with this
	//! GridProfileSLBM object to specified values.
	//!
	//! Set the P and S velocities and gradients associated with this
	//! GridProfileSLBM object to specified values.
	//! @param depths the depths of the interfaces, in km below sea level.
	//! @param pvelocities the P velocities stored in this GridProfileSLBM, in km/sec.
	//! @param svelocities the S velocities stored in this GridProfileSLBM, in km/sec.
	//! @param gradients a 2-element vector specifying the P and S velocity
	//! gradients in the mantle, in 1/sec.
	void setData(double* depths, double* pvelocities,
		double* svelocities,	double* gradients);

	//! \brief Set the layer depths to specified values.
	//!
	//! Set the layer depths to specified values.
	//! @param depths the layer depths, in km below sea level.
	void setDepths(const vector<double>& depths);

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

	//! \brief Retrieve the radius of the k'th interval, in km.
	//!
	//! Retrieve the radius of the k'th interval, in km.
	double getInterfaceRadius(const int& k);

	//! \brief Retrieve the depth of the k'th interval, in km
	//! relative sea level.
	//!
	//! brief Retrieve the depth of the k'th interval, in km
	//! relative sea level.
	double getInterfaceDepth(const int& k);

	//! \brief Retrieve the P or S velocity of the k'th interval, in km/sec.
	//!
	//! Retrieve the P or S velocity of the k'th interval, in km/sec.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @param k the index of the desired interval.  The shallowest interval
	//! is index 0.
	double getVelocity(const int& waveType, const int& k)
	{ return geoStack->getVelocity(waveType, k); };

	//! \brief Retrieve the P or S velocity gradient in the mantle, in 1/sec.
	//!
	//! Retrieve the P or S velocity gradient in the mantle, in 1/sec.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	double getMantleGradient(const int& waveType)
	{ return geoStack->getMantleGradient(waveType); };

	//! \brief Retrieve the depths of all intervals, in km.
	//!
	//! Retrieve the depths of all intervals, in km.
	void getInterfaceDepths(vector<double>& depths);

	//! \brief Retrieve the P or S velocities of all intervals, in km/sec.
	//!
	//! Retrieve the P or S velocities of all intervals, in km/sec.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @param velocity (output) the P or S velocity of all the intervals, in km/sec
	void getVelocity(const int& waveType, double* velocity)
	{ geoStack->getVelocity(waveType, velocity); };

	//! \brief Retrieve the P and S velocity gradients in the mantle, in 1/sec.
	//!
	//! Retrieve the P and S velocity gradients in the mantle, in 1/sec.
	void getMantleGradient(double* g) { geoStack->getMantleGradient(g); };

	double getWaterThick() { return waterThick; };

protected:

	Grid& grid;

	//! \brief The GeoStack object that holds all the interface depths,
	//! velocities and gradients.
	//!
	//! The GeoStack object that holds all the interface depths,
	//! velocities and gradients.
	GeoStack* geoStack;

	double waterThick;

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  INLINE FUNCTIONS
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

inline double GridProfileSLBM::getInterfaceRadius(const int& k)
{
	// radius is the radius of the surface of the solid earth.
	// geoStack->getDepth(k) returns the depth of top of layer k
	// relative to surface of solid earth.
	// The difference is the radius of the top of layer k.
	// Layer 0 is the water layer and waterThick is the nominal
	// thickness of the water layer.  Since waterThick is only
	// a nominal value, it is not guaranteed that top of water
	// is at sea level.
	if (k > 0)
		return radius - geoStack->getDepth(k);
	return radius + waterThick;
}

inline void GridProfileSLBM::getData(double* d,
			double* pv, double* sv, double* g)
{
	double r = getEarthRadius();
	for (int k=0; k<NLAYERS; k++)
	{
		d[k]  = r - getInterfaceRadius(k);
		pv[k] = geoStack->getVelocity(PWAVE, k);
		sv[k] = geoStack->getVelocity(SWAVE, k);
	}
	g[PWAVE] = geoStack->getMantleGradient(PWAVE);
	g[SWAVE] = geoStack->getMantleGradient(SWAVE);
}

inline double GridProfileSLBM::getInterfaceDepth(const int& k)
{
	// getEarthRadius is the radius of sea level.
	return getEarthRadius() - getInterfaceRadius(k);
}

inline void GridProfileSLBM::getInterfaceDepths(vector<double>& depths)
{
	depths.resize(NLAYERS);
	double r = getEarthRadius();
	for (int k=0; k<NLAYERS; k++)
		depths[k] = r - getInterfaceRadius(k);
}

} // end slbm namespace

#endif // GRIDPROFILESLBM_H_.h
