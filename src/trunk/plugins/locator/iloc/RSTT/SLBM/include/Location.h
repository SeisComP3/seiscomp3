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
//- Module:        $RCSfile: Location.h,v $
//- Revision:      $Revision: 1.22 $
//- Last Modified: $Date: 2012/12/18 15:12:17 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef Location_H
#define Location_H

// **** _SYSTEM INCLUDES_ ******************************************************
#include <vector>
#include <cmath>
#include <sstream>
#include <iomanip>

#include "SLBMGlobals.h"

#include "CPPUtils.h"

using namespace std;

using namespace geotess;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

class SLBM_EXP_IMP Location
// **** _CLASS DEFINITION_ *****************************************************
//
//! \brief The Location Class manages a single point in/on the Earth, which is
//! described by the GRS80 ellipsoid.
//
//! The Location Class manages a single point in/on the Earth, which is
//! described by the GRS80 ellipsoid.  
//! See <A href="../../../doc/geovectors.pdf">geovectors.pdf</A> for a thorough 
//! mathematical description of Location.
//! 
//! There is a parameterized constructor that takes a geographic latitude, 
//! longitude and depth as parameters and converts them to internal representation.  
//! There are also many methods for computing the distance between 2 Location 
//! objects, the azimuth from one Location to another, the radius of the Earth 
//! at a given location, retreiving points along a great circle path between
//! two Location objects, etc.
//
// *****************************************************************************
{
 public:

	 // Default constructor 
	 Location();

	 //destructor
	 virtual ~Location();
   
	 // copy constructor
	 Location(const Location& location);

	// constructor specifying latitude, longitude (in radians), and depth in km
	// Depth defaults to zero if not specified.  
	Location(const double &lat, const double &lon, const double &depth=0);

	// constructor specifying 3D unit vector with origin at center of earth.
	Location(const double v[], const double &radius);

	/**
	  * Parameterized constructor. Returns a new Location which is 
	  * the mean of the two specified Locations.
	 */
	Location(const Location& loc1, const Location& loc2);

	// equal operator
	Location& operator=(const Location& other);

	// equality operator
	bool operator==(const Location& other) const;

	/**
	 * Equality operator. Returns false if other Location object has the exact same 
	 * horizontal position and radius as this Location object.
	 */
	bool operator!=(const Location& other) {return ! (*this == other);};

	/**
	 * Returns a convenient string representation of this Location.
	 */
	string toString() const;

	/**
	 * Set the location of this Location.
	 * @param lat the geographic latitude in radians.
	 * @param lon the geographic longitude in radians.
	 * @param depth the depth in km.
	 */
	void setLocation(const double& lat, const double& lon, const double& depth);

	/**
	 * Set the location of this Location.
	 * @param u unit vector of new position.
	 * @param r radius of new position in km.
	 */
	void setLocation(const double* u, const double& r)
	{ v[0]=u[0]; v[1]=u[1]; v[2]=u[2]; radius=r; }

	/**
	 * Set the radius of this Location.
	 * @param r the radius in km.
	 */
	void setRadius(const double &r);

	/**
	 * Retrieve the radius of this Location object.
	 * @return the radius in km.
	 */
	double getRadius() const;

	/**
	 * Retrieve the radius of the Earth at the latitude 
	 * of this Location.
	 @return the radius of the Earth in km.
	 */
	double getEarthRadius() const;

	/**
	 * Retrieve the depth of this Location.
	 @return the depth of this Location in km.
	 */
	double getDepth() const;

	/**
	 * Set the depth of this Location.
	 @param depth the desired depth in km.
	 */
	void setDepth(const double &depth);

	/**
	 * Returns the distance, in radians, from this Location to some other Location.
	 * @param other the other Location to which this Location is to
	 * be compared.
	 * @return double separation of the locations in radians.
	 */
	double distance(const Location& other) const;

	// returns the distance in km between this Location and some other Location,
	// measured at the surface of the ellipsoid.
	// The product of distance * getEarthRadius() is integrated along the great 
	// circle path from source to receiver (in approximately 1 degree increments).
	double distanceKm(Location& other) const;

	/**
	 * Return the distance, in degrees, from this Location to some other Location.
	 * @param other the other Location to which this Location is to be compared.
	 * @return double separation of the Locations in degrees.
	 */
	double distanceDegrees(const Location& other) const;

	//! \brief Find the azimuth from this Location to some other Location. 
	//! Result will be between 0 and 2*PI radians.
	//!
	//! Find the azimuth from this Location to some other Location. 
	//! Result will be between 0 and 2*PI radians.
	//! @param other the other location to which the azimuth is requested.
	//! @return the azimuth from this to other, in radians clockwise from north.
	double azimuth(const Location& other) const;

	//! \brief Find the azimuth from this Location to some other Location. 
	//! Result will be between 0 and 360 degrees.
	//!
	//! Find the azimuth from this Location to some other Location. 
	//! Result will be between 0 and 360 degrees.
	//! @param other the other location to which the azimuth is requested.
	//! @return the azimuth from this to other, in degrees clockwise from north.
	double azimuthDegrees(const Location& other) const;
 
	//! \brief Find the azimuth from this Location to some other Location. 
	//! Result will be between 0 and 2*PI radians.  If current position is
	//! the north or south pole, or if current position and other position
	//! are conincident, returns specified errorValue.
	//!
	//! Find the azimuth from this Location to some other Location. 
	//! Result will be between 0 and 2*PI radians.  If current position is
	//! the north or south pole, or if current position and other position
	//! are conincident, returns specified errorValue.
	//! @param other the other location to which the azimuth is requested.
	//! @param errorValue the value to return when things go badly.
	//! @return the azimuth from this to other, in radians clockwise from north.
	//! If current position is
	//! the north or south pole, or if current position and other position
	//! are conincident, returns specified errorValue.
	double azimuth(const Location& other, const double& errorValue) const;

	//! \brief Find the azimuth from this Location to some other Location. 
	//! Result will be between 0 and 360 degrees.  If current position is
	//! the north or south pole, or if current position and other position
	//! are conincident, returns specified errorValue.
	//!
	//! Find the azimuth from this Location to some other Location. 
	//! Result will be between 0 and 360 degrees.  If current position is
	//! the north or south pole, or if current position and other position
	//! are conincident, returns specified errorValue.
	//! @param other the other location to which the azimuth is requested.
	//! @param errorValue the value to return when things go badly.
	//! @return the azimuth from this to other, in degrees clockwise from north.
	//! If current position is
	//! the north or south pole, or if current position and other position
	//! are conincident, returns specified errorValue.
	double azimuthDegrees(const Location& other, const double& errorValue) const;
 
	//! \brief Retrieve the geographic latitude of this Location, radians.
	//!
	//! Retrieve the latitude of this Location.
	//! @return geographic latitude, in radians.
	double getLat() const;

	//! \brief Retrieve the geocentric latitude of this Location, radians.
	//!
	//! Retrieve the latitude of this Location.
	//! @return geocentric latitude, in radians.
	double getGeocentricLat() const
	{ return asin(v[2]); }

	//! \brief Retrieve the geocentric latitude of this Location, degrees.
	//!
	//! Retrieve the latitude of this Location.
	//! @return geocentric latitude, in degrees.
	double getGeocentricLatDegrees() const
	{ return asin(v[2]) * RAD_TO_DEG; }

	//! \brief Retrieve the longitude of this Location.  Value
	//! will be between -PI and PI radians.
	//!
	//! Retrieve the longitude of this Location.  Value
	//! will be between -PI and PI radians.
	//! @return longitude, in radians.
	double getLon() const;

	//! \brief Retrieve the geographic latitude of this Location, degrees.
	//!
	//! Retrieve the latitude of this Location.
	//! @return geographic latitude, in degrees.
	double getLatDegrees() const;

	//! \brief Retrieve the longitude of this Location.  Value
	//! will be between -180 and 180 degrees.
	//!
	//! Retrieve the longitude of this Location.  Value
	//! will be between -180 and 180 degrees.
	//! @return longitude, in degrees.
	double getLonDegrees() const;

	const double* getUnitVector() { return v; };

	void getUnitVector(double x[3]) {x[0] = v[0]; x[1] = v[1]; x[2] = v[2];};

	void setUnitVector(double x[3]) {v[0] = x[0]; v[1] = x[1]; v[2] = x[2];};

	//! \brief Retrieve a Location that is a specified distance away from this 
	//! Location, in a specified direction.
	//!
	//! Retrieve a Location that is a specified distance away from this 
	//! Location, in a specified direction.  The returned Location will have 
	//! the same radius as this location.
	//! @param azimuth the azimuth from this Location to the desired Location, 
	//! in radians, measured clockwise from north.
	//! @param distance the distance from this Location to the desired Location,
	//! in radians.
	//! @param loc the Location that has been moved relative to this Location.
	void move(const double &azimuth, const double &distance, Location &loc) const;

	/**
	 * Retrieve a Location which is normal to the plane containing the great
	 * circle path from this Location to another Location.  Considering the 
	 * Locations to be unit vectors, loc will be set to this cross x.  The
	 * radius of the resulting vector will be set to radius of this Location.
	 * If this Location and Location x are parallel, then the resulting Location
	 * will be invalid (vector will have zero length) and the radius will be -1 km.
	 @param x the other Location which, together with this Location, define the 
	 great circle path.
	 @param loc the Location normal to the plane containing the great circle path.
	 Invalid if this and loc are parallel.
	 @return true if loc is valid, false if this and x are parallel.
	 */
	bool cross(const Location& x, Location& loc) const;

	/**
	 * Rotate this Location around Location pole by angle a.  When looking in
	 * direction of pole's unit vector, clockwise rotation is positive.
	 @param pole the pole around which this Location is to be rotated.
	 @param angle the angular distance by which this Location is to be
	 rotated around pole, in radians.
	 @param loc the Location that results from rotating this Location around
	 pole.
	 */
	void rotate(Location& pole, double angle, Location& loc) const;

	//! \brief Compute the vector triple product (this x other) x this, 
	//! normalized to unit length. Returns true if valid, false if triple
	//! product has zero length, which will happen when this and other are
	//! coincident or PI radians apart.
	//!
	//! Compute the vector triple product (this x other) x this, 
	//! normalized to unit length. Returns true if valid, false if triple
	//! product has zero length, which will happen when this and other are
	//! coincident or PI radians apart.
	bool vectorTripleProduct(const Location& other, double vtp[]) const;

	//! \brief Move this Location object a specified angular distance (radians) 
	//! in the direction specified by vtp.  
	//!
	//! Move this Location object a specified angular distance (radians) 
	//! in the direction specified by vtp, which is assumed to be a unit vector 
	//! normal to this Location object's unit vector.  vtp values are typically
	//! obtained by calling Location::vectorTripleProduct().
	//!
	//! @param vtp a 3 component unit vector normal to this Location.
	//! @param a the angular distance from this Location to the desired 
	//! Location, in radians.
	//! @param loc the Location object which has been relocated as requested.
	void move(const double vtp[], const double& a, Location& loc) const;

	/**
	 * Return a location on the earth that is distance radians due north of
	 * positon x.  If x is already at the north or south pole, then it
	 * is returned unmodified.
	 * @param x the position to be moved.
	 * @param distance the distance, in radians, that x is
	 * to be moved toward the north.
	 * @param z the 3-element unit vector representing the position
	 * after having moved distance north.
	 */
	void move_north(const double x[], const double &distance, double z[]) const;

	/**
	 * Return a location on the earth that is distance radians due north of
	 * this Location.  If this is already at the north or south pole, then it
	 * is returned unmodified.
	 * @param distance the distance, in radians, that x is
	 * to be moved toward the north.
	 * @param loc the Location after having moved distance north.
	 */
	void move_north(const double &distance, Location &loc) const;

    /**
	 * rotate 3d vector x clockwise around 3d vector p, by angle a.
	 * x and p are assumed to be unit vectors on input.
	 * @param x vector to be rotated
	 * @param p pole about which rotation is to occur.
	 * @param a double the amount of rotation, in radians.
	 * @param z the rotated vector, normalized to unit length.
	 */
	void rotate (const double x[], const double p[], const double &a,
               double z[]) const;


	/**
	 * Return the scalar triple product of loc1, loc2 and this.  
	 * In other words (loc1 cross loc2) dot this.  Since these are all unit
	 * vectors, the the result is cos(phi) where phi is the angle between 
	 * this Location and loc1 cross loc2.
	 */
	double scalarTripleProduct(const Location& loc1, const Location& loc2) const;

	/**
	 * Return the scalar triple product of u, w, and this Location objects
	 * unit vector.  In other words (u cross w) dot v.  If u and w are unit
	 * vectors, then the result is cos(phi) where phi is the angle between 
	 * this Location and u cross w.
	 */
	double scalarTripleProduct(const double u[], const double w[]) const;

	/**
	 * Return the scalar triple product (u cross v) dot w.  If all are unit
	 * vectors, then the result is cos(phi) where phi is the angle between 
	 * w and u cross c.  
	 */
	double scalarTripleProduct(const double u[], const double v[],
                           const double w[]) const;

	//! \brief Compute the normalized vector triple product (u x v) x u and 
	//! and store result in vtp.
	//!
	//! Compute the normalized vector triple product (u x v) x u and 
	//! and store result in vtp.  Returns true if vtp has finite length, false if 
	//! length(vtp) is zero.
	bool vectorTripleProduct(const double u[], const double v[], double vtp[]) const;

	//! \brief Compute the normalized vector triple product (u x northPole) x u 
	//! and store result in w.  Returns true if w has finite length, false if 
	//! length(w) is zero.
	//!
	//! Compute the normalized vector triple product (u x northPole) x u 
	//! and store result in w.  Returns true if w has finite length, false if 
	//! length(w) is zero.
	bool vectorTripleProductNorthPole(const double u[],  double w[]) const;

	//! \brief Move unit vector v in direction of vtp by distance a and store result in u.
	//! vtp is assumed to be a unit vector normal to v.
	//!
	//! Move unit vector v in direction of vtp by distance a and store result in u.
	//! vtp is assumed to be a unit vector normal to v.
	void move(const double v[], const double vtp[], const double& a,
            double u[]) const;

	/**
     * Angular distance between two 3-component unit vectors, in radians.
     * Vectors are assumed to be unit length on input.
     * @param u unit vector one.
     * @param v unit vector two.
     * @return angular separation in radians.
     */
    double angle(const double u[], const double v[]) const;

	/**
     * Dot product of the unit vector that backs this Location
     * and the unit vector that backs Location other.
     * @param other the other Location
     * @return dot product.
     */
	double dot(const Location& other) const;

	/**
     * Dot product of two 3-component unit vectors.
     * @param u vector one.
     * @param v vector two.
     * @return dot product.
     */
    double dot(const double u[], const double v[]) const;

    /**
     * normalized cross product of two 3-component unit vectors.
     * @param u vector<double> vector one.
     * @param v vector<double> vector two.
	 * @param w set to u cross v, normalized to unit length.  If u cross v
	 * has zero length, w will equal (0,0,0).
     * @return the length of u cross v prior to normalization.  Guaranteed >= 0.
     */
    double cross(const double u[], const double v[], double w[]) const;

    /**
     * normalized cross product of a 3-component unit vector with the north pole.
     * @param u vector<double> vector one.
	 * @param w set to u cross north, normalized to unit length.  If u cross north
	 * has zero length, w will equal (0,0,0).
     * @return the length of u cross north prior to normalization.  Guaranteed >= 0.
     */
    double crossNorth(const double u[], double w[]) const;

    /**
     * Normalizes the input vector to unit length.  Returns the length of the 
	 * vector prior to normalization.
     * @param v vector<double>
     * @return length of the vector prior to normalization ( >= 0.)
     */
    double normalize(double v[]) const;

	/**
	 * Find the length of a 3-element vector.
	 @return the length of the vector.  Guaranteed to be >= 0.
	 */
	double length(const double v[]) const;

	/**
	 * This variable is initialized to -1 in Location.cc, outside the class
	 * definition.  There is a method SlbmInterface::fixEarthRadius(double) 
	 * that will modify this value.  If this value is less than zero, then
	 * Location::getEarthRadius() will return the radius of the earth that
	 * is a function of geocentric latitude (larger at the equator, smaller
	 * at the poles).  If this value is > 0, then getEarthRadius() returns
	 * this value.
	 */
	static double EARTH_RADIUS;

	static int getClassCount();

protected:

	static int locationClassCount;

	/**
	 * v is the geocentric unit vector that describes the position on the
	 * earth.  The origin of the vector is at the center of the earth.  The
	 * x-component points to lon,lat = 0,0.  y-component points to lon,lat =
	 * PI/2,0 and the z-component points to lon,lat = 0,PI/2.
	 */
	double v[3];

	/**
	 * The distance from the center of the earth to this location in km.
	 */
	double radius;

};

// INLINE FUNCTIONS

inline void Location::setLocation(const double& latitude, 
		const double& longitude, const double& depth)
{
	// set x to the geocentric latitude in radians
	double x = atan((1.0 - EARTH_E) * tan(latitude));
	// z component of v is sin of geocentric latitude.
	v[2] = sin(x);
	// reuse x by setting it to cos of geocentric latitude
	x = cos(x);
	// compute x and y components of v
	v[0] = x * cos(longitude);
	v[1] = x * sin(longitude);
	radius = getEarthRadius() - depth;
}

inline double Location::getLat() const
{
	// Reference: Snyder, eq. 3-28, p. 17.
    return atan(tan(asin(v[2]))/(1-EARTH_E));
}

inline double Location::getLon() const
{
  return atan2(v[1],v[0]);
}

inline double Location::getLatDegrees() const
{
  return RAD_TO_DEG * getLat();
}

inline double Location::getLonDegrees() const
{
  return RAD_TO_DEG * getLon();
}

inline double Location::getRadius() const
{
  return radius;
}

inline void Location::setRadius(const double &r)
{
	radius = r;
}

inline double Location::getDepth() const
{
	return getEarthRadius() - radius;
}

inline void Location::setDepth(const double &depth)
{
	radius = getEarthRadius() - depth;
}

inline double Location::distance(const Location& other) const
{
  return angle(v, other.v);
}

inline double Location::distanceDegrees(const Location& other) const
{
  return angle(v, other.v) * RAD_TO_DEG;
}

inline double Location::azimuth(const Location& other) const
{
  return azimuth(other, 0.);
}

inline double Location::azimuth(const Location& other, const double& errorValue) const
{
  double azim=errorValue;
  double c2[3];

  // set c2 = the cross product of this x other.
  if (cross(v, other.v, c2) > 0.)
  // if the cross product has zero length then the two vectors are coincident
  // (do nothing in that case; returns zero).
  {
    double c[3];
    // set c = this x north pole
    // if the cross product has zero length then this == north_pole
    // or south pole and azimuth is indeterminant.
	  if (crossNorth(v, c) > 0.)
	  {
		  // set azimuth to the angle between (this x north pole) and 
		  // (this x other).
		  azim = angle(c, c2);
		  // if the dot product of (this x other) . northPole < 0
		  if (c2[2] < 0.) azim = 2.0 * PI - azim;
	  }
  }
  return azim;
}

inline double Location::azimuthDegrees(const Location& other) const
{
	return azimuth(other) * RAD_TO_DEG;
}

inline double Location::azimuthDegrees(const Location& other, const double& errorValue) const
{
	double az = azimuth(other, errorValue);
	if (az != errorValue)
	  az *= RAD_TO_DEG;
	return az;
}

// find a location a specified distance and azimuth away from this.
inline void Location::move(const double &azimuth, const double &distance,
                           Location &loc) const
{
  //find a point distance radians due north of v
  double x[3];
  move_north(v, distance, x);
  //x is now a point distance due north of v
  //rotate x around v in direction of positive longitude
  rotate(x, v, -azimuth, loc.v);
  loc.radius=radius;
}

// given a vector x, find a vector z which is a specified distance north.
inline void Location::move_north(const double x[], const double &distance,
                                 double z[]) const
{
	double c[3];
	vectorTripleProductNorthPole(x, c);
	move(x, c, distance, z);
}

// find a vector z which is a specified distance north of this.
inline void Location::move_north(const double &distance, Location &loc) const
{
	double vtp[3];
	vectorTripleProductNorthPole(v, vtp);
	move(v, vtp, distance, loc.v);
}

// given a precomputed normalized vector triple product, vtp, move this 
// a specified distance in the direction specified by vtp and return the 
// result in Location loc
inline void Location::move(const double vtp[], const double& a, Location& loc) const
{
	move(v, vtp, a, loc.v);
	loc.radius = radius;
}

// given a vector w, and a precomputed normalized vector triple product, vtp, 
// move w a specified distance in the direction specified by vtp and return the 
// result in vector u.
inline void Location::move(const double w[], const double vtp[],
                           const double& a, double u[]) const
{
	double cosa = cos(a);
	double sina = sin(a);
	u[0] = cosa * w[0] + sina * vtp[0];
	u[1] = cosa * w[1] + sina * vtp[1];
	u[2] = cosa * w[2] + sina * vtp[2];
}

// find the normalized vector triple product using Location objects.
// Compute vtp = (this x other) x this and return vtp.
inline bool Location::vectorTripleProduct(const Location& other, double vtp[]) const
{
	return vectorTripleProduct(v, other.v, vtp);
}

// find the normalized vector triple product using vectors.
// Compute vtp = (u x v) x u and return vtp.
inline bool Location::vectorTripleProduct(const double u[], const double s[],
                                          double w[]) const
{
	// set q = u cross s
	double q0, q1, q2;
	q0 = u[1] * s[2] - u[2] * s[1];
	q1 = u[2] * s[0] - u[0] * s[2];
	q2 = u[0] * s[1] - u[1] * s[0];
	// set w = q cross u
	w[0] = q1 * u[2] - q2 * u[1];
	w[1] = q2 * u[0] - q0 * u[2];
	w[2] = q0 * u[1] - q1 * u[0];
	// normalize w to unit length.  if length
	// of w before normalization was zero, then
	// w will = {0,0,0} and return false;
	return normalize(w) != 0.;
}

// find the normalized vector triple product of vector u with north pole
// Compute w = (u x n) x u and return w.
inline bool Location::vectorTripleProductNorthPole(const double u[],
                                                   double w[]) const
{
	w[0] = -u[0] * u[2];
	w[1] = -u[1] * u[2];
	w[2] =  u[1] * u[1] + u[0] * u[0];
	return normalize(w) != 0.;
}

inline double Location::scalarTripleProduct(const Location& loc1, const Location& loc2) const
{
	return scalarTripleProduct(loc1.v, loc2.v, v);
}

inline double Location::scalarTripleProduct(const double u[], const double p[],
                                          const double w[]) const
{
	return u[0] * p[1] * w[2] + p[0] * w[1] * u[2] + w[0] * u[1] * p[2] -
			  w[0] * p[1] * u[2] - u[0] * w[1] * p[2] - p[0] * u[1] * w[2];
}

inline double Location::scalarTripleProduct(const double v1[],
                                            const double v2[]) const
{
	return scalarTripleProduct(v1, v2, v);
}

// rotate this Location around Location p by angle a and return result 
// in Location loc.
inline void Location::rotate(Location& p, double a, Location &loc) const
{
	rotate(v, p.v, a, loc.v);
	loc.radius=radius;
}

// rotate vector x around vector p by angle a and return the result in vector z.
inline void Location::rotate (const double x[], const double p[],
                              const double &a, double z[]) const
{
  double d = x[0] * p[0] + x[1] * p[1] + x[2] * p[2]; // dot product
  // if x and p are parallel, x needs no rotation.
  if (d <= -1. || d >= 1.0)
  {
	  z[0] = x[0]; z[1] = x[1]; z[2] = x[2];
  }
  else
  {
	  double cosa = cos(a);
	  double sina = sin(a);
	  d *= (1-cosa);
	  z[0] = cosa * x[0] + d * p[0] + sina * (p[1] * x[2] - p[2] * x[1]);
	  z[1] = cosa * x[1] + d * p[1] + sina * (p[2] * x[0] - p[0] * x[2]);
	  z[2] = cosa * x[2] + d * p[2] + sina * (p[0] * x[1] - p[1] * x[0]);
	  double len = sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
	  z[0] /= len;
	  z[1] /= len;
	  z[2] /= len;
  }
}

// find angular separation of two unit vectors, in radians.
inline double Location::angle(const double u[], const double w[]) const
{
    return acos(max(min(dot(u, w), 1.0), -1.0));
}

inline double Location::dot(const Location& other) const
{
	return v[0] * other.v[0] + v[1] * other.v[1] + v[2] * other.v[2];
}

// dot product
inline double Location::dot(const double u[], const double s[]) const
{
    return u[0] * s[0] + u[1] * s[1] + u[2] * s[2];
}

// cross product of this Location with other Location, result stored in
// Location loc.
// return true if length > zero.
inline bool Location::cross(const Location& x, Location &loc) const
{
	if (cross(v, x.v, loc.v) > 0.)
        loc.radius = radius;
	else
		loc.radius = -1.;
	return loc.radius > 0.;
}

// cross product of two unit vectors, normalized to unit length.  
// return the length of cross product before normalization.
inline double Location::cross(const double u[], const double s[],
                              double w[]) const
{
   w[0] = u[1] * s[2] - u[2] * s[1];
   w[1] = u[2] * s[0] - u[0] * s[2];
   w[2] = u[0] * s[1] - u[1] * s[0];
   return normalize(w);
}

// cross product of vector u with north pole, normalized to unit length.  
// result returned in w.  returns length prior to normalization.
inline double Location::crossNorth(const double u[], double w[]) const
{
	double len = u[0] * u[0] + u[1] * u[1];
    if (len <= 0.) 
	{
		len  = 0.;
		w[0] = 0.;
		w[1] = 0.;
		w[2] = 0.;
	}
	else
	{
		len  = sqrt(len);
		w[0] =  u[1]/len;
		w[1] = -u[0]/len;
		w[2] = 0.;
	}
	return len;
}

// normalize vector to unit length.  returns length prior to 
// normalization.
inline double Location::normalize(double u[]) const
{
	double len = length(u);
	if (len == 0.)
	{
		u[0] = u[1] = u[2] = 0.0;
	}
	else
	{
		u[0] /= len;
		u[1] /= len;
		u[2] /= len;
	}
	return len;
}

// find the length of a vector.
inline double Location::length(const double u[]) const
{
    double l = dot(u, u);
    if (l > 0.) return sqrt(l);
    return 0.;
}

inline string Location::toString() const
{
	ostringstream os;
  os << std::fixed
    << std::showpoint
    << std::setprecision(4)
    << std::setw(9) << getLatDegrees()
		<< " "
	  << std::setw(10) << getLonDegrees() 
		<< " "
		<< std::setprecision(3)
	  << std::setw(10) << getDepth();
	return os.str();
}

} // end slbm namespace

#endif // Location_H
