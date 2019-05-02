//- ****************************************************************************
//-
//- Copyright 2009 Sandia Corporation. Under the terms of Contract
//- DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
//- retains certain rights in this software.
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

#ifndef EARTH_SHAPE_H
#define EARTH_SHAPE_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <string>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"
#include "GeoTessException.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess
{

// **** _FORWARD REFERENCES_ ***************************************************

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Defines the ellipsoid that is to be used to convert between geocentric and
 *  geographic latitude and between depth and radius.
 *
 *  EarthShape defines the ellipsoid that is to be used to convert between geocentric and
 *  geographic latitude and between depth and radius.  The default is WGS84.
 *  The following EarthShapes are defined:
 * <ul>
 * <li>SPHERE - Geocentric and geographic latitudes are identical and
 * conversion between depth and radius assume the Earth is a sphere
 * with constant radius of 6371 km.
 * <li>GRS80 - Conversion between geographic and geocentric latitudes, and between depth
 * and radius are performed using the parameters of the GRS80 ellipsoid.
 * <li>GRS80_RCONST - Conversion between geographic and geocentric latitudes are performed using
 * the parameters of the GRS80 ellipsoid.  Conversions between depth and radius
 * assume the Earth is a sphere with radius 6371.
 * <li>WGS84 - Conversion between geographic and geocentric latitudes, and between depth
 * and radius are performed using the parameters of the WGS84 ellipsoid.
 * <li>WGS84_RCONST - Conversion between geographic and geocentric latitudes are performed using
 * the parameters of the WGS84 ellipsoid.  Conversions between depth and radius
 * assume the Earth is a sphere with radius 6371.
 * </ul>
 *
 */
class GEOTESS_EXP_IMP EarthShape
{
private:

	/**
	 * The name of the ellipsoid (SPHERE, WGS84, etc)
	 */
	string shapeName;

	/**
	 * The equatorial radius of the Earth in km
	 */
	double equatorialRadius;

	/**
	 * The inverse flattening of the Earth.  This is infinity for a sphere,
	 * and is equal to numbers like ~298 for modern ellipsoids.
	 */
	double inverseFlattening;

	/**
	 * Eccentricity squared.
	 * <p>
	 * Equals [ flattening * (2. - flattening) ]
	 * <p>
	 * Also equals [ 1 - sqr(b)/sqr(a) ] where a is the
	 * equatorial radius and b is the polar radius.
	 */
	double eccentricitySqr;

	/**
	 * Equals [ 1 - eccentricitySqr ]
	 */
	double e1;

	/**
	 * Equals [ eccentricitySqr / (1 - eccentricitySqr) ]
	 */
	double e2;

	/**
	 * True if conversions between depth and radius should assume that
	 * the Earth is a sphere with radius equal to equatorial radius.
	 */
	bool constantRadius;

	/**
	 * True if the shape is a sphere.
	 */
	bool sphere;

public:

	/**
	 *  Define the shape of the Earth that is to be used to convert between geocentric and
	 *  geographic latitude and between depth and radius.  The default is WGS84.
	 * <ul>
	 * <li>SPHERE - Geocentric and geographic latitudes are identical and
	 * conversion between depth and radius assume the Earth is a sphere
	 * with constant radius of 6371 km.
	 * <li>GRS80 - Conversion between geographic and geocentric latitudes, and between depth
	 * and radius are performed using the parameters of the GRS80 ellipsoid.
	 * <li>GRS80_RCONST - Conversion between geographic and geocentric latitudes are performed using
	 * the parameters of the GRS80 ellipsoid.  Conversions between depth and radius
	 * assume the Earth is a sphere with radius 6371.
	 * <li>WGS84 - Conversion between geographic and geocentric latitudes, and between depth
	 * and radius are performed using the parameters of the WGS84 ellipsoid.
	 * <li>WGS84_RCONST - Conversion between geographic and geocentric latitudes are performed using
	 * the parameters of the WGS84 ellipsoid.  Conversions between depth and radius
	 * assume the Earth is a sphere with radius 6371.
	 * </ul>
	 * @param earthShape one of SPHERE, GRS80, GRS80_RCONST, WGS84, WGS84_RCONST
	 */
	EarthShape(const string& earthShape = "WGS84") { setEarthShape(earthShape); }

	/**
	 *  Define the shape of the Earth that is to be used to convert between geocentric and
	 *  geographic latitude and between depth and radius.  The default is WGS84.
	 * <ul>
	 * <li>SPHERE - Geocentric and geographic latitudes are identical and
	 * conversion between depth and radius assume the Earth is a sphere
	 * with constant radius of 6371 km.
	 * <li>GRS80 - Conversion between geographic and geocentric latitudes, and between depth
	 * and radius are performed using the parameters of the GRS80 ellipsoid.
	 * <li>GRS80_RCONST - Conversion between geographic and geocentric latitudes are performed using
	 * the parameters of the GRS80 ellipsoid.  Conversions between depth and radius
	 * assume the Earth is a sphere with radius 6371.
	 * <li>WGS84 - Conversion between geographic and geocentric latitudes, and between depth
	 * and radius are performed using the parameters of the WGS84 ellipsoid.
	 * <li>WGS84_RCONST - Conversion between geographic and geocentric latitudes are performed using
	 * the parameters of the WGS84 ellipsoid.  Conversions between depth and radius
	 * assume the Earth is a sphere with radius 6371.
	 * <li>IERS2003 - Conversion between geographic and geocentric latitudes, and between depth
	 * and radius are performed using the parameters of the IERS2003 ellipsoid.
	 * <li>IERS2003_RCONST - Conversion between geographic and geocentric latitudes are performed using
	 * the parameters of the IERS2003 ellipsoid.  Conversions between depth and radius
	 * assume the Earth is a sphere with radius 6371.
	 * </ul>
	 * @param earthShape one of SPHERE, GRS80, GRS80_RCONST, WGS84, WGS84_RCONST, IERS2003, IERS2003_RCONST
	 */
	void setEarthShape(const string& earthShape)
	{
		shapeName = earthShape;

		if (earthShape == "SPHERE")
		{
			equatorialRadius = 6371;
			inverseFlattening = 1e99;
			constantRadius = true;
		}
		else if (earthShape == "GRS80")
		{
			equatorialRadius = 6378.137;
			inverseFlattening = 298.257222101;
			constantRadius = false;
		}
		else if (earthShape == "GRS80_RCONST")
		{
			equatorialRadius = 6371.;
			inverseFlattening = 298.257222101;
			constantRadius = true;
		}
		else if (earthShape == "WGS84")
		{
			equatorialRadius = 6378.137;;
			inverseFlattening = 298.257223563;
			constantRadius = false;
		}
		else if (earthShape == "WGS84_RCONST")
		{
			equatorialRadius = 6371.;
			inverseFlattening = 298.257223563;
			constantRadius = true;
		}
		else if (earthShape == "IERS2003")
		{
			equatorialRadius = 6378.1366;
			inverseFlattening = 298.25642;
			constantRadius = false;
		}
		else if (earthShape == "IERS2003_RCONST")
		{
			equatorialRadius = 6371.;
			inverseFlattening = 298.25642;
			constantRadius = true;
		}
		else
		{
			ostringstream os;
			os << endl << "ERROR in EarthShape::setEarthShape" << endl
					<< earthShape << " is not a recognized EarthShape" << endl
					<< "Valid EarthShapes include SPHERE, GRS80, GRS80_RCONST, WGS84, WGS84_RCONST, IERS2003 and IERS2003_RCONST" << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 9001);
		}

		sphere = shapeName == "SPHERE";
		eccentricitySqr = (2.- 1./inverseFlattening)/inverseFlattening;
		e1 = 1.-eccentricitySqr;
		e2 = eccentricitySqr/(1.-eccentricitySqr);
	}

	/**
	 * Destructor.
	 */
	virtual ~EarthShape()
	{
	}

	/**
	 * Returns the class name.
	 * @return class name
	 */
	static string class_name()
	{ return "EarthShape"; }

	/**
	 * Returns the class size.
	 * @return class size
	 */
	virtual int class_size() const
	{ return (int) sizeof(EarthShape); }

	/**
	 * Retrieve the radius of the Earth in km at the position specified by an
	 * Earth-centered unit vector.  Uses the WGS84 ellipsoid.
	 *
	 * @param v Earth-centered unit vector
	 * @return radius of the Earth in km at specified position.
	 */
	double getEarthRadius(const double* const v)
	{ return constantRadius ? equatorialRadius : equatorialRadius / sqrt(1 + e2 * v[2] * v[2]); }

	/**
	 * Return geocentric latitude given a geographic latitude
	 *
	 * @param lat
	 *            geographic latitude in radians
	 * @return geocentric latitude in radians
	 */
	double getGeocentricLat(const double& lat)
	{ return sphere ? lat : atan(tan(lat) * e1); }

	/**
	 * Return geographic latitude given a geocentric latitude
	 *
	 * @param lat
	 *            geocentric latitude in radians
	 * @return geographic latitude in radians
	 */
	double getGeographicLat(const double& lat)
	{ return sphere ? lat : atan(tan(lat) / e1); }

	/**
	 * Return geocentric latitude given a geographic latitude
	 *
	 * @param lat
	 *            geographic latitude in degrees
	 * @return geocentric latitude in degrees
	 */
	double getGeocentricLatDegrees(const double& lat)
	{ return sphere ? lat : CPPUtils::toDegrees(atan(tan(CPPUtils::toRadians(lat)) * e1)); }

	/**
	 * Return geographic latitude given a geocentric latitude
	 *
	 * @param lat
	 *            geocentric latitude in degrees
	 * @return geographic latitude in degrees
	 */
	double getGeographicLatDegrees(const double& lat)
	{ return sphere ? lat : CPPUtils::toDegrees(atan(tan(CPPUtils::toRadians(lat)) / e1)); }

	/**
	 * Convert a 3-component unit vector to geographic latitude, in radians.
	 *
	 * @param v 3-component unit vector
	 * @return geographic latitude in radians.
	 */
	double getLat(const double* const v)
	{ return getGeographicLat(asin(v[2])); }

	/**
	 * Convert a 3-component unit vector to geographic latitude, in degrees.
	 *
	 * @param v 3-component unit vector
	 * @return geographic latitude in degrees.
	 */
	double getLatDegrees(const double* const v)
	{ return CPPUtils::toDegrees(getLat(v)); }

	/**
	 * Convert a 3-component unit vector to a longitude, in radians.
	 *
	 * @param v 3 component unit vector
	 * @return longitude in radians.
	 */
	double getLon(const double* const v)
	{ return atan2(v[1], v[0]); }

	/**
	 * Convert a 3-component unit vector to a longitude, in degrees.
	 *
	 * @param v 3 component unit vector
	 * @return longitude in degrees.
	 */
	double getLonDegrees(const double* const v)
	{ return CPPUtils::toDegrees(atan2(v[1], v[0])); }

	/**
	 * Convert geographic lat, lon into a geocentric unit vector. The
	 * x-component points toward lat,lon = 0, 0. The y-component points toward
	 * lat,lon = 0, 90. The z-component points toward north pole.
	 *
	 * @param lat geographic latitude in degrees.
	 * @param lon longitude in degrees.
	 * @param v 3 component unit vector.
	 * @return pointer to v
	 */
	void getVectorDegrees(const double& lat, const double& lon, double* v)
	{ getVector(CPPUtils::toRadians(lat), CPPUtils::toRadians(lon), v); }

	/**
	 * Convert geographic lat, lon into a geocentric unit vector. The
	 * x-component points toward lat,lon = 0, 0. The y-component points toward
	 * lat,lon = 0, 90. The z-component points toward north pole.
	 *
	 * @param lat geographic latitude in degrees.
	 * @param lon longitude in degrees.
	 * @return pointer to v
	 */
	double* getVectorDegrees(const double& lat, const double& lon)
	{ return getVector(CPPUtils::toRadians(lat), CPPUtils::toRadians(lon)); }

	/**
	 * Convert geographic lat, lon into a geocentric unit vector. The
	 * x-component points toward lat,lon = 0, 0. The y-component points toward
	 * lat,lon = 0, PI/2. The z-component points toward north pole.
	 *
	 * @param lat
	 *            geographic latitude in radians.
	 * @param lon
	 *            longitude in radians.
	 * @return 3 component unit vector.
	 */
	double* getVector(const double& lat, const double& lon)
	{
		double* v = new double[3];
		getVector(lat, lon, v);
		return v;
	}

	/**
	 * Convert geographic lat, lon into a geocentric unit vector. The
	 * x-component points toward lat,lon = 0, 0. The y-component points toward
	 * lat,lon = 0, PI/2 The z-component points toward north pole.
	 *
	 * @param lat geographic latitude in radians.
	 * @param lon longitude in radians.
	 * @param v 3-component unit vector.
	 * @return a pointer to v
	 */
	double* getVector(const double& lat, const double& lon, double* v)
	{
		// convert lat from geographic to geocentric latitude.
		double temp = getGeocentricLat(lat);

		// z component of v is sin of geocentric latitude.
		v[2] = sin(temp);

		// set lat = to cos of geocentric latitude
		temp = cos(temp);

		// compute x and y components of v
		v[0] = temp * cos(lon);
		v[1] = temp * sin(lon);

		return v;
	}

	/**
	 * @param v unit vector to be converted to lat, lon string
	 * @return a String of lat,lon in degrees formatted with "%10.6f %11.6f"
	 */
	string getLatLonString(const double* const v)
	{
	  char s[300];
	  string frmt("%9.5f %10.5f");
	  sprintf(s, frmt.c_str(), getLatDegrees(v), getLonDegrees(v));
	  return s;
	}

	bool isConstantRadius() const { return constantRadius; }

	double getEccentricitySqr() const { return eccentricitySqr; }

	double getEquatorialRadius() const { return equatorialRadius; }

	double getInverseFlattening() const { return inverseFlattening; }

	bool isSphere() const { return sphere; }

	const string& getShapeName() const { return shapeName; }

};
// end class EarthShape

} // end namespace geotess

#endif  // EARTH_SHAPE_H
