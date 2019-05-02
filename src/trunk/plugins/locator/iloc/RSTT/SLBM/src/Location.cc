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
//- Program:       Location
//- Module:        $RCSfile: Location.cc,v $
//- Revision:      $Revision: 1.16 $
//- Last Modified: $Date: 2011/10/07 13:14:58 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
//#include <iostream>

#include "Location.h"

//using namespace std;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

// **** _STATIC INITIALIZATIONS_************************************************

int Location::locationClassCount = 0;

double Location::EARTH_RADIUS = -1;

/**
 * Default constructor.  Sets the location to lat, lon, depth = 0.
 */
Location::Location() : radius(EARTH_A)
{
	++locationClassCount;
  v[0] = 1.; 
  v[1] = v[2] = 0.0;
}  // END Location Default Constructor



/**
 * Destructor.  Performs no special actions.
 */
Location::~Location()
{
	--locationClassCount;
}  // END Destructor

/**
 * Copy constructor.
 @param other the Location object whose position is to be copied.
 */
Location::Location(const Location &other) : radius(other.radius)
{
	++locationClassCount;
	v[0] = other.v[0]; v[1] = other.v[1]; v[2] = other.v[2];
}

// constructor specifying 3D unit vector with origin at center of earth.
Location::Location(const double u[], const double &r)
{
	++locationClassCount;
	v[0] = u[0];
	v[1] = u[1]; 
	v[2] = u[2];
	radius = r;
}

/**
  * Parameterized constructor.
  @param latitude geographic latitude in radians.
  @param longitude geographic longitude in radians.
  @param depth depth in km.  Defaults to zero if not specified.  
 */
Location::Location(const double &latitude, const double &longitude, 
				   const double &depth)
{
	++locationClassCount;
  setLocation(latitude, longitude, depth);
}

/**
  * Parameterized constructor. Returns a new Location which is 
  * the mean of the two specified Locations.
 */
Location::Location(const Location& loc1, const Location& loc2)
{
	++locationClassCount;
	v[0] = loc1.v[0] + loc2.v[0];
	v[1] = loc1.v[1] + loc2.v[1]; 
	v[2] = loc1.v[2] + loc2.v[2];
	normalize(v);
	radius = 0.5*(loc1.radius + loc2.radius);
}

/**
 * Equal operator.  Returns a new Location object that is a copy of
 * this Location object.
 */
Location& Location::operator=(const Location& other)
{
	v[0] = other.v[0]; v[1] = other.v[1]; v[2] = other.v[2];
	radius = other.radius;
  return *this;
}

/**
 * Equality operator.  Returns true if other Location object has the 
 * exact same horizontal position and radius as this Location object.
 */
bool Location::operator==(const Location& other) const
{
	return ((v[0] == other.v[0]) && (v[1] == other.v[1]) &&
          (v[2] == other.v[2]) && (radius == other.radius));
}


/**
 * Returns the horizontal distance, in km, from this Location to some 
 * other Location, measured along the surface of the earth.
 * The distance from this to other is divided into approximately
 * 1 degree intervals and angular distance * local earthRadius() is summed 
 * along the great circle path from this to other.
 * @param other Location the other location to which this Location is to
 * be compared.
 * @return double separation of the locations in km.
 */
double Location::distanceKm(Location &other) const
{
	double distance = angle(v,other.v);

	if (EARTH_RADIUS > 0.)
		return distance*EARTH_RADIUS;

	// n = aproximate number of 1 degree increments in distance, 
	// minimum of 1.
	int n = (int) ceil(distance / DEG_TO_RAD);

	if (n == 1)
		return distance * 0.5 * (getEarthRadius() + other.getEarthRadius());

	double dx = distance/n;

	double vtp[3];
	Location loc;
	
	vectorTripleProduct(other, vtp);
	distance = 0;

	for (int i=0; i<n; i++)
	{
		move(vtp, dx * (((double) i) + 0.5), loc);
		distance += dx * loc.getEarthRadius();
	}
	return distance;
}

int Location::getClassCount()
{
  return locationClassCount;
}

double Location::getEarthRadius() const
{
	if (EARTH_RADIUS > 0.) return EARTH_RADIUS;
	return EARTH_A / sqrt(1. + v[2] * v[2] * EARTH_E / (1.-EARTH_E));
}

} // end slbm namespace
