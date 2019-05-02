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
//- Program:       CrustalProfile
//- Module:        $RCSfile: CrustalProfile.h,v $
//- Revision:      $Revision: 1.22 $
//- Last Modified: $Date: 2011/09/21 21:19:28 $
//- Last Check-in: $Author: avencar $
//-
//- ****************************************************************************
#ifndef CrustalProfile_H
#define CrustalProfile_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <vector>
#include <set>

using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "SLBMGlobals.h"
#include "InterpolatedProfile.h"
#include "Location.h"
#include "GreatCircle.h"
#include "TauPSite.h"
//#include "Grid.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

class GridProfile;

class SLBM_EXP_IMP CrustalProfile : public InterpolatedProfile
//! \brief A profile through the Earth model that stores interface radius, and 
//! interval P or S wave velocity information about
//! the crustal stack below either the source or the receiver.
//!
//! The CrustalProfile class  represents a Profile based on radius and P or S wave 
//! velocity values interpolated from values of nearby GridProfile objects.  
//! A CrustalProfile object should be acquired by calling
//! Grid::getCrustalProfile().
//!
//! CrustalProfile owns a Location object which defines a point in the Earth.
//! For CrustalProfile objects that represent a seismic source, the Location object
//! represents the latitude, longitude and depth of the event.  
//! For CrustalProfile objects that represent a seismic receiver, the Location object
//! represents the latitude, longitude and depth (negative elevation) of the receiver.  
//!
//! Each instance of a CrustalProfile is populated with only the information needed to 
//! compute crustal travel time for a particular phase.  Supported phases are Pn, Sn, 
//! Pg and Lg.  Since mantle velocity gradient information is not required to compute
//! crustal travel times, CrustalProfile objects do not store mantle gradient 
//! information.  CrustalProfile objects to support Pn or Pg store only P wave velocities while
//! those instantiated to support Sn or Lg only store S wave velocity information.
//! 
//! The xtCrust() function computes the travel time (in seconds) and the horizontal
//! offset (in radians) of the ray as it traverses the crust from the depth of the 
//! Location object down to the head wave interface (Moho or top of middle crust).  
//! xtCrust takes a ray parameter as an arguement.  
//! The function will always return an offset and travel time because if the ray
//! parameter exceeds the critical ray parameter for any of the velocity interfaces
//! that the ray has to interact with, the function will decrease the ray parameter
//! enough to successfully travel from the Location depth to the deepest interface it
//! knows about, using a constant ray parameter over the distance traveled.  
//! 
//! CrustalProfile objects can be members of multiple GreatCircle objects. 
//! Grid maintains a map that associates a phase/Location combination with a 
//! particular instance of a CruatalProfile.  The map can be cleared by calling
//! Grid::clearCrustalProfiles(), which is also accessible from 
//! SlbmInterface::clear().
{

public:

	//! \brief Default constructor.
	//!
	//! Default constructor.
	CrustalProfile();

	//! \brief Copy constructor.
	//!
	//! Copy constructor.
	CrustalProfile(const CrustalProfile& CrustalProfile);

	virtual ~CrustalProfile();

	//! \brief Equal operator.
	//!
	//! Equal operator.
	CrustalProfile& operator=(const CrustalProfile& other);

	//! \brief Equality operator.
	//!
	//! Equality operator.
	bool operator==(const CrustalProfile& other);

	//! \brief Inequality operator.
	//!
	//! Inequality operator.
	bool operator!=(const CrustalProfile& other) {return ! (*this == other);};

	//! \brief Parameterized constructor.
	//!
	//! Parameterized constructor that builds a CrustalProfile object based on values
	//! interpolated from nearby GridProfile objects.
	//! @param grid a reference to the Grid object.  Grid::findProfile() will
	//! be called to retrieve the neighbors and interpolation coefficients.
	//! @param phase the seismic phase that this CrustalProfile is to support.  Must be
	//! one of SLBMGlobals::Pn, SLBMGlobals::Sn, SLBMGlobals::Pg, SLBMGlobals::Lg.
	//! @param lat geographic latitude of the source or receiver in radians.
	//! @param lon geographic longitude of the source or receiver in radians.
	//! @param depth the depth of the source or receiver in km.
	void setup(Grid& grid, const int& phase, 
		const double& lat, const double& lon, const double& depth);

	//! \brief Retrieve a const reference to the Location associated with this Profile.
	//!
	//! Retrieve a const reference to the Location associated with this Profile.
	//!
	//! Since the Location retrieved is const, the calling application will not be
	//! allowed to modify the position of the Location.
	const Location& getLocation() const { return location; };

	double getEarthRadius() { return location.getEarthRadius(); };

	//! \brief Retrieve the number of intervals associated with this Profile.
	//! 
	//! Retrieve the number of intervals associated with this Profile.
	int  getNIntervals() { return NLAYERS; };

	//! \brief Retrieve all the interval depth and velocity information contained in
	//! this CrustalProfile object.
	//! 
	//! Retrieve all the interval depth and velocity information contained in
	//! this CrustalProfile object.
	void getData(vector<double>& depths, vector<double>& velocities);

	//! \brief Retrieve the radius of the top of the k'th interval, in km.
	//!
	//! Retrieve the radius of the top of the k'th interval, in km.
	double getInterfaceRadius(const int& k) { return radius[k]; };

	//! \brief Retrieve the depths of the top of each intervals, in km.
	//!
	//! Retrieve the depths of the top of each intervals, in km.
	void getDepths(double* depths); 

	//! \brief Retrieve the P or S wave velocity of each interval, in km/sec.
	//! 
	//! Retrieve the P or S wave velocity of each interval, in km/sec.
	double* getVelocities() { return velocity; };

	//! \brief Retrieve the depth of the top of the k'th interval, in km.
	//!
	//! Retrieve the depth of the top of the k'th interval, in km.
	double getDepth(const int& k); 

	//! \brief Retrieve the P or S wave velocity of the k'th interval, in km/sec.
	//! 
	//! Retrieve the P or S wave velocity of the k'th interval, in km/sec.
	double getVelocity(const int& k) { return velocity[k]; };

	//! \brief Retrieve the critical spherical ray parameter for this profile,
	//! in radians/seconds.
	//!
	//! Retrieve the critical spherical ray parameter for this profile, in 
	//! radians/seconds.
	//! This is R / V where R is the radius at the top of the bottom interval in 
	//! this profile and V is the velocity of the bottom interval.  For Pn and
	//! Sn, R will be the radius of the Moho.  For Pg and Lg, R will be the
	//! radius of the top of the middle crust.  For Pn and Pg, V will be the 
	//! P wave velocity, for Sn and Lg, V will be the S wave velocity.
	//! @param greatCircle the GreatCircle object that knows which head wave
	//! interface to use to compute the interface radius.
	double getPCrit(GreatCircle* greatCircle);

	//! \brief Compute the horizontal offset (radians)
	//! and the travel time (sec) of the ray through the crust.
	//! 
	//! Computes the travel time (in seconds) and the horizontal
	//! offset (in radians) of the ray as it traverses the crust from the depth of the 
	//! Location object down to the head wave interface (Moho for Pn and Sn,
	//! top of middle crust for Pg and Lg).  
	//!
	//! The horizontal offset is the angular separation of the Location object
	//! owned by this CrustalProfile and the pierce point of the ray on the 
	//! head wave interface.
	//! 
	//! The function will always return an offset and travel time because if the ray
	//! parameter exceeds the critical ray parameter for any of the velocity interfaces
	//! that the ray has to interact with, the function will decrease the ray parameter
	//! enough to successfully travel from the Location depth to the deepest interface it
	//! knows about, using a constant ray parameter over the distance traveled.  
	//! 
	//! @param greatCircle the GreatCircle object, of which this CrustalProfile is 
	//! a member, that is requesting the information.
	//! @param rayParameter the spherical ray parameter, in radians/sec.
	//! @param xTotal the total horizontal offset of the ray as it travelled through 
	//! the crust, in radians.
	//! @param tTotal the total travel time of the ray through the crust, in seconds.
	void   xtCrust(
				GreatCircle* greatCircle,
				const double& rayParameter,
				double &xTotal,
				double &tTotal);

	//! \brief Compute the horizontal offset (radians)
	//! and the travel time (sec) of the ray through the crust.
	//! 
	//! Computes the travel time (in seconds) and the horizontal
	//! offset (in radians) of the ray as it traverses the crust from the depth of the 
	//! Location object down to the head wave interface (Moho for Pn and Sn,
	//! top of middle crust for Pg and Lg). 
	//!
	//! Unlike the other version of xtCrust(), this version returns information
	//! about the horizontal offset and travel time at each interface of the crust
	//! that the ray interacts with.
	//!
	//! The horizontal offset is the angular separation of the Location object
	//! owned by this CrustalProfile and the pierce point of the ray on the 
	//! head wave interface.
	//! 
	//! The function will always return offset and travel time information because if 
	//! the ray parameter exceeds the critical ray parameter for any of the velocity 
	//! interfaces that the ray has to interact with, the function will 
	//! decrease the ray parameter enough to successfully travel from the 
	//! Location depth to the deepest interface it
	//! knows about, using a constant ray parameter over the distance traveled.  
	//! 
	//! @param greatCircle the GreatCircle object, of which this CrustalProfile is 
	//! a member, that is requesting the information.
	//! @param rayParameter the spherical ray parameter, in radians/sec.
	//! @param layid the layerid number of each interface that the ray
	//! interacted with.  Only includes layers below the depth of the Location
	//! object and excludes the layerids of zero thickness layers.
	//! @param x the horizontal offset of the point where the ray intersected 
	//! each interface, in radians.  The first element is zero, and the last 
	//! element is the same as the value of xTotal returned by the other version 
	//! of xtCrust().
	//! @param r the radii of the interfaces that the ray interacted with, in km.
	//! @param v the velocities of the layers that the ray interacted with, in km/sec.
	//! @param t the cumulative travel time of the ray across all the preceeding
	//! layers.  The first element is zero, and the last element is the same as 
	//! the value of tTotal returned by the other version of xtCrust().
	//! @param npoints the number of elements in arrays layid, x, r, v and t.
	void   xtCrust(
				GreatCircle* greatCircle,
				const double& rayParameter,
				int layid[],
				double x[],
				double r[],
				double v[],
				double t[],
				int& npoints);

	const int* getLayid() { return layid; };

	int getNLayid() { return nlay; };

	int getTopLayid() { return topLayid; };

	//! \brief Returns a formatted string containing detailed information about this Profile.
	//!
	//! Returns a formatted string containing detailed information about this Profile.
	//! @param greatCircle the GreatCircle object, of which this CrustalProfile is 
	//! a member, that is requesting the information.
	//! @param rayParameter the ray parameter to use to compute interval horizontal 
	//! This method calls xtCrust() to compute horizontal 
	//! offsets and travel times in each crustal layer.
	string toString(GreatCircle* greatCircle, double rayParameter);
	string toString();

	size_t memSize();

  //! \brief Set the input TauPSite into this CrustalProfile;
  void      setTauPSite(taup::TauPSite* tps);

  //! \brief Return this CrustalProfiles TauPSite object;
  taup::TauPSite* getTauPSite();

  static int getClassCount();

  //! \brief true location radius > moho radius.
  //!
  //! true if location radius > moho radius.
  bool isInCrust() { return inCrust; };

private:

	static int crustalProfileClassCount;

	//! \brief The Location object that defines this CrustalProfile object.  
	//! 
	//! The Location object that defines this CrustalProfile object.  
	//! For sources, the radius of this Location reflects the depth of the event.
	//! For receivers, the radius of this Location reflects the elevation of
	//! the receiver.
	Location location;

	//! \brief true location radius > moho radius.
	//!
	//! true if location radius > moho radius.
	bool inCrust;

	//! \brief The phase that this CrustalProfile represents.
	//!
	//! The phase that this CrustalProfile represents.
	//! Will be one of SLBMGlobals::Pn, SLBMGlobals::Sn, SLBMGlobals::Pg, SLBMGlobals::Lg.
	int phase;

	//! \brief Radius of the top of each interval, in km.
	//!
	//! Radius of the top of each interval, in km.  
	double radius[NLAYERS];

	//! \brief The P or S velocity of each layer, in km/sec.
	//! 
	//! The P or S velocity of each layer, in km/sec.
	double velocity[NLAYERS];

	//! \brief Indexes of non-zero thickness layers 
	//!
	//! Indexes of non-zero thickness layers 
	int layid[NLAYERS];

	//! \brief Number of entries in layid.
	//!
	//! Number of entries in layid.
	int nlay;

	//! \brief Index of entry in layid that represents the layer that contains
	//! location radius.
	//!
	//! Index of entry in layid that represents the layer that contains
	//! location radius.
	int topLayid;

	//! \brief The index of the entry in layid that corresponds to the
	//! SLBMGlobals::MIDDLE_CRUST
	//!
	//! The index of the entry in layid that corresponds to the
	//! SLBMGlobals::MIDDLE_CRUST
	int nMiddleCrust;

	//! \brief For Pg/Lg, fudgeFactor is a factor applied to all velocities above
	//! the middle crust to ensure that the velocity of the upper crust is 
	//! less than or equal to the Pg/Lg velocity of the middle crust minus 
	//! 0.1 km/sec.
	//!
	//! For Pg/Lg, fudgeFactor is a factor applied to all velocities above
	//! the middle crust to ensure that the velocity of the upper crust is 
	//! less than or equal to the Pg/Lg velocity of the middle crust minus 
	//! 0.1 km/sec.  fudgeFactor will be <= 1.0.
	double fudgeFactor;

	//! \brief A single TaupSite object used to calculate travel time
	//! near a receiver when the SLBM method fails or is inaccurate.
	//!
	//! The attribute (tps) is initialized to null and is only assigned
	//! for cases where the CrustalProfile objects is created for a
	//! receiver.
	taup::TauPSite* cpTPS;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  INLINE FUNCTIONS 
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

inline void CrustalProfile::getData(vector<double>& depths, 
			vector<double>& velocities)
{
	depths.clear();
	velocities.clear();
	double R = getEarthRadius();
	for (int i=0; i<getNIntervals(); i++)
	{
		depths.push_back(R - radius[i]);
		velocities.push_back(velocity[i]);
	}
}

inline double CrustalProfile::getDepth(const int& k)
{
	return getEarthRadius() - radius[k];
}

inline void CrustalProfile::getDepths(double* depths)
{
	double R = getEarthRadius();
	for (int i=0; i<getNIntervals(); i++)
		depths[i] = R - radius[i];
}

inline double CrustalProfile::getPCrit(GreatCircle* greatCircle)
{ 
	return radius[greatCircle->getHeadWaveInterface()] 
		/ velocity[greatCircle->getHeadWaveInterface()]; 
}

} // end slbm namespace

#endif // CrustalProfile.h
