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
//- Module:        $RCSfile: GridProfile.h,v $
//- Revision:      $Revision: 1.25 $
//- Last Modified: $Date: 2012/12/18 15:12:18 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef GridProfile_H
#define GridProfile_H

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

class SLBM_EXP_IMP GridProfile : public Location
//! \brief Manages all information related to a single node in a Grid object.
//!
//! Manages all information related to a single node in a Grid object
//! including: the Location of the node, the depths of all model interfaces, 
//! the P and S velocities of each model interval, and the P and S velocity 
//! gradients in the mantle.
//!
//! GridProfile objects are created in Grid::loadVelocityModel() and deleted 
//! in Grid::~Grid.
{

public:

	GridProfile() { ++gridProfileClassCount; };

	GridProfile(const int& i, const double& lat, const double& lon, const double& elev);

	GridProfile(const int& i, Location& location);

	//! \brief Destructor.
	//!
	//! Destructor.
	virtual ~GridProfile();

	/**
	 * Returns the class name.
	 */
	static string class_name() {return "GridProfile"; };

	//! \brief Retrieve the node index of this node in the model grid.
	//!
	//! Retrieve the node index of this node in the model grid.
	const int getNodeId() const { return nodeId; };

	//! \brief Retrieve the index of the GeoStack that holds the 
	//! information about model interfaces, velocities and 
	//! gradients.
	//!
	//! Retrieve the index of the GeoStack that holds the 
	//! information about model interfaces, velocities and 
	//! gradients.
	virtual int getGeoStackId() = ABSTRACT;

	virtual double getEarthRadius() { return earthRadius; }

	//! \brief Retrieve all the data associated with this Profile.
	//!
	//! Retrieve all the data associated with this Profile.
	//! @param depths the depths of the top of 
	//! each interval associated with this Profile.
	//! @param pvelocity the P velocities of  each interval, in km/sec.
	//! @param svelocity the S velocities of  each interval, in km/sec.
	//! @param gradient a 2-element array specifying the P and S velocity
	//! gradient in the mantle, in 1/sec.
	virtual void getData(double* depths, double* pvelocity, double* svelocity, double* gradient) = ABSTRACT;

	//! \brief Set the P and S velocities and gradients associated with this
	//! GridProfile object to specified values.
	//!
	//! Set the P and S velocities and gradients associated with this
	//! GridProfile object to specified values.
	//! @param depths the depths of the interfaces, in km below sea level.
	//! @param pvelocities the P velocities stored in this GridProfile, in km/sec.
	//! @param svelocities the S velocities stored in this GridProfile, in km/sec.
	//! @param gradients a 2-element vector specifying the P and S velocity
	//! gradients in the mantle, in 1/sec.
	virtual void setData(double* depths, double* pvelocities,
		double* svelocities,	double* gradients) = ABSTRACT;

	//! \brief Set the layer depths to specified values.
	//!
	//! Set the layer depths to specified values.
	//! @param depths the layer depths, in km below sea level.
	virtual void setDepths(const vector<double>& depths) = ABSTRACT;
	
	//! \brief Set the P or S velocity to specified values.
	//!
	//! Set the P or S velocity to specified values.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @param velocity the P or S velocity, in km/sec.
	virtual void setVelocity(const int& waveType, const vector<double>& velocity) = ABSTRACT;

	//! \brief Set the P and S velocity gradients to specified values.
	//!
	//! Set the P and S velocity gradients to specified values.
	//! @param gradient the P and S velocity gradients, in km/sec.
	virtual void setGradient(const vector<double>& gradient) = ABSTRACT;

	//! \brief Retrieve the radius of the k'th interval, in km.
	//! 
	//! Retrieve the radius of the k'th interval, in km.
	virtual double getInterfaceRadius(const int& k) = ABSTRACT;

	//! \brief Retrieve the depth of the k'th interval, in km
	//! relative sea level.
	//! 
	//! brief Retrieve the depth of the k'th interval, in km
	//! relative sea level.
	virtual double getInterfaceDepth(const int& k) = ABSTRACT;

	//! \brief Retrieve the P or S velocity of the k'th interval, in km/sec.
	//! 
	//! Retrieve the P or S velocity of the k'th interverroral, in km/sec.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @param k the index of the desired interval.  The shallowest interval
	//! is index 0.
	virtual double getVelocity(const int& waveType, const int& k) = ABSTRACT;

	//! \brief Retrieve the P or S velocity gradient in the mantle, in 1/sec.
	//! 
	//! Retrieve the P or S velocity gradient in the mantle, in 1/sec.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	virtual double getMantleGradient(const int& waveType) = ABSTRACT;

	//! \brief Retrieve the depths of all intervals, in km.
	//! 
	//! Retrieve the depths of all intervals, in km.
	virtual void getInterfaceDepths(vector<double>& depths) = ABSTRACT;

	//! \brief Retrieve the P or S velocities of all intervals, in km/sec.
	//! 
	//! Retrieve the P or S velocities of all intervals, in km/sec.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @param velocity (output) the P or S velocity of all the intervals, in km/sec
	virtual void getVelocity(const int& waveType, double* velocity) = ABSTRACT;

	//! \brief Retrieve the P and S velocity gradients in the mantle, in 1/sec.
	//! 
	//! Retrieve the P and S velocity gradients in the mantle, in 1/sec.
	virtual void getMantleGradient(double* gradients) = ABSTRACT;

	virtual double getWaterThick() = ABSTRACT;

  //! \brief A temporary weight storage assignment function (returned
  //! reference) used exclusively by the GreatCircle::getWeights(...)
  //! function.
  //!
  //! A temporary weight storage assignment function (returned
  //! reference) used exclusively by the GreatCircle::getWeights(...)
  //! function.
	double getWeight() { return weight; };

	void addWeight(const double& w) { weight += w; };

	void setWeight(const double& w) { weight = w; };

	void clearHitCount() { nHits = 0; };

	void incrementHitCount() { ++nHits; };

	int getHitCount() { return nHits; };

	int getActiveNodeId() { return activeNodeId; };

	void setActiveNodeId(int id) { activeNodeId = id; };

	static int getClassCount();

	void depthsToRadii(double depths[NLAYERS], vector<vector<float> >& radii);
	void depthsToRadii(const vector<double>& depths, vector<vector<float> >& radii);

protected:

	//! \brief The node id of this GridProfile.
	//!
	//! The node id of this GridProfile.  
	int nodeId;

	int activeNodeId;

	//! \brief A temporary weight storage location used exclusively by the
	//! GreatCircle::getWeights(...) function.
	//!
	//! A temporary weight storage location used exclusively by the
	//! GreatCircle::getWeights(...) function.
	double weight;

	//! \brief The number of times this node has been 'touched' by a 
	//! GreatCircle. 
	//!
	//! The number of times this node has been 'touched' by a 
	//! GreatCircle. This attribute is incremented in getWeights()
	//! once each time this node contributes any weight to any
	//! profile object in a GreatCircle.
	int nHits;

	double earthRadius;

	static int gridProfileClassCount;

};

} // end slbm namespace

#endif // GridProfile.h
