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
//- Module:        $RCSfile: GridProfileGeoTess.h,v $
//- Revision:      $Revision: 1.13 $
//- Last Modified: $Date: 2013/07/25 23:12:46 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef GRIDPROFILEGEOTESS_H_
#define GRIDPROFILEGEOTESS_H_

// **** _SYSTEM INCLUDES_ ******************************************************

#include <vector>
#include <set>
#include <map>

using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPGlobals.h"
#include "GeoTessProfile.h"

using namespace geotess;

#include "SLBMGlobals.h"
#include "Location.h"
#include "GeoStack.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

class InterpolatedProfile;
class Grid;

class SLBM_EXP_IMP GridProfileGeoTess : public GridProfile
//! \brief Manages all information related to a single node in a Grid object.
//!
//! Manages all information related to a single node in a Grid object
//! including: the Location of the node, the depths of all model interfaces,
//! the P and S velocities of each model interval, and the P and S velocity
//! gradients in the mantle.
//!
//! GridProfileGeoTess objects are created in Grid::loadVelocityModel() and deleted
//! in Grid::~Grid.
{

// let Grid objects access protected members of GridProfileGeoTess.
friend class Grid;

public:

	//! \brief Parameterized constructor.
	//!
	//! Parameterized constructor.  Note that the Location that this
	//! inherits from is having depth set to zero so that the radius
	//! of this object is equivalent to calling getEarthRadius().
	GridProfileGeoTess(Grid& g, const int& nodeId, Location& location);

	//! \brief Copy constructor.
	//!
	//! Copy constructor.
	GridProfileGeoTess(const GridProfileGeoTess &other);

	//! \brief Destructor.
	//!
	//! Destructor.
	~GridProfileGeoTess();

	static string class_name() { return "GridProfileGeoTess"; };

	//! \brief Retrieve the index of the GeoStack that holds the
	//! information about model interfaces, velocities and
	//! gradients.
	//!
	//! Retrieve the index of the GeoStack that holds the
	//! information about model interfaces, velocities and
	//! gradients.
	int getGeoStackId() { return -1; };

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
	//! GridProfileGeoTess object to specified values.
	//!
	//! Set the P and S velocities and gradients associated with this
	//! GridProfileGeoTess object to specified values.
	//! @param depths the depths of the interfaces, in km below sea level.
	//! @param pvelocities the P velocities stored in this GridProfileGeoTess, in km/sec.
	//! @param svelocities the S velocities stored in this GridProfileGeoTess, in km/sec.
	//! @param gradients a 2-element vector specifying the P and S velocity
	//! gradients in the mantle, in 1/sec.
	void setData(double depths[NLAYERS], double pvelocities[NLAYERS],
			double svelocities[NLAYERS], double gradients[2]);

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
	double getInterfaceRadius(const int& k)
	{ return gtProfiles[NLAYERS-k]->getRadiusTop(); }

	//! \brief Retrieve the depth of the k'th interval, in km
	//! relative sea level.
	//!
	//! brief Retrieve the depth of the k'th interval, in km
	//! relative sea level.
	double getInterfaceDepth(const int& k)
	{ return earthRadius - gtProfiles[NLAYERS-k]->getRadiusTop(); }

	//! \brief Retrieve the P or S velocity of the k'th interval, in km/sec.
	//!
	//! Retrieve the P or S velocity of the k'th interval, in km/sec.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @param k the index of the desired interval.  The shallowest interval
	//! is index 0.
	double getVelocity(const int& waveType, const int& k)
	{ return gtProfiles[NLAYERS-k]->getValueTop(waveType); };

	//! \brief Retrieve the P or S velocity gradient in the mantle, in 1/sec.
	//!
	//! Retrieve the P or S velocity gradient in the mantle, in 1/sec.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	double getMantleGradient(const int& waveType)
	{ return gtProfiles[0]->getValueTop(waveType); };

	//! \brief Retrieve the depths of all intervals, in km.
	//!
	//! Retrieve the depths of all intervals, in km.
	void getInterfaceDepths(vector<double>& depths);

	//! \brief Retrieve the P or S velocities of all intervals, in km/sec.
	//!
	//! Retrieve the P or S velocities of all intervals, in km/sec.
	//!
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @param velocities a double array with at least NLAYERS elements that will be
	//! populated with the velocity information
	void getVelocity(const int& waveType, double* velocities)
	{ for (int k=0; k<NLAYERS; k++) velocities[k] = gtProfiles[NLAYERS-k]->getValueTop(waveType); }

	//! \brief Retrieve the P and S velocity gradients in the mantle, in 1/sec.
	//!
	//! Retrieve the P and S velocity gradients in the mantle, in 1/sec.
	//!
	//! @param g a 2-element array that will be populated with the gradients.
	void getMantleGradient(double* g)
	{ g[PWAVE] = gtProfiles[0]->getValueTop(PWAVE); g[SWAVE] = gtProfiles[0]->getValueTop(SWAVE); }

	double getWaterThick()
	{ return gtProfiles[NLAYERS]->getRadiusTop()-gtProfiles[NLAYERS-1]->getRadiusTop(); }

protected:

	GeoTessModelSLBM* model;

	// A 1D array of GeoTess Profile objects with NLAYERS elements.
	// Each Profile spans one of the layers of the model.
	GeoTessProfile** gtProfiles;

	bool hasLowVelocityZone();

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  INLINE FUNCTIONS
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

inline void GridProfileGeoTess::getData(double* d,
			double* pv, double* sv, double* g)
{
	for (int k=0; k<NLAYERS; k++)
	{
		d[k]  = getEarthRadius()-gtProfiles[NLAYERS-k]->getRadiusTop();
		pv[k] = gtProfiles[NLAYERS-k]->getValueTop(PWAVE);
		sv[k] = gtProfiles[NLAYERS-k]->getValueTop(SWAVE);
	}
	g[PWAVE] = gtProfiles[0]->getValueTop(PWAVE);
	g[SWAVE] = gtProfiles[0]->getValueTop(SWAVE);
}

inline void GridProfileGeoTess::getInterfaceDepths(vector<double>& depths)
{
	depths.resize(NLAYERS);
	for (int k=0; k<NLAYERS; k++)
		depths[k] = earthRadius - gtProfiles[NLAYERS-k]->getRadiusTop();
}

} // end slbm namespace

#endif // GRIDPROFILEGEOTESS_H_.h
