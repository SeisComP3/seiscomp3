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

#ifndef GEOTESSUTILS_OBJECT_H
#define GEOTESSUTILS_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess
{

// **** _FORWARD REFERENCES_ ***************************************************

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Collection of static functions to manipulate geographic information.
 *
 * The Utils class provides basic static utility functions for GeoTess
 * to manipulate geographic information.
 */
class GEOTESS_EXP_IMP GeoTessUtils
{
private:

	/*
	 * Private copy constructor. Not used.
	 */
	GeoTessUtils(const GeoTessUtils& gtu) { }

	/*
	 * Private assignment operator. Not used.
	 */
	GeoTessUtils& operator=(const GeoTessUtils& gtu) { return *this; }

public:

	/**
	 * If true, then an approximate algorithm will be used to
	 * convert back and forth between geocentric and geographic latitudes.
	 * The approximation incurs an error of about 0.1 meters in latitude
	 * calculations but is faster than the correct calculation.
	 * <p>Note that the existence of this static, mutable variable technically
	 * violates thread-safety of this class. But given the assumption that
	 * the approximation is just as good as the true conversions, this is
	 * not a significant violation.
	 */
	static bool approximateLatitudes;

	/**
	 * Default constructor.
	 */
	GeoTessUtils() { }

	/**
	 * Destructor.
	 */
	virtual ~GeoTessUtils() { }

	/**
	 * Returns the class name.
	 * @return class name
	 */
	static string class_name()
	{ return "GeoTessUtils"; }

	/**
	 * Returns the class size.
	 * @return class size
	 */
	virtual int class_size() const
	{ return (int) sizeof(GeoTessUtils); }

	/**
	 * The current GeoTess version.
	 * @return the current GeoTess version
	 */
	static string getVersion() { return "2.2.3"; }

	/**
	 * Return the dot product of two vectors.
	 *
	 * @param v0 a 3 component vector
	 * @param v1 a 3 component vector
	 * @return dot product
	 */
	static double dot(const double* const v0, const double* const v1)
	{ return v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2]; }

	/**
	 * Calculate the scalar triple product of 3 3-component vectors: (v0 cross
	 * v1) dot v2
	 *
	 * @param v0 double[]
	 * @param v1 double[]
	 * @param v2 double[]
	 * @return scalar triple product (v0 cross v1) dot v2
	 */
	static double scalarTripleProduct(const double* const v0,
			const double* const v1, const double* const v2)
	{
		return v0[0] * v1[1] * v2[2] + v1[0] * v2[1] * v0[2]
				+ v2[0] * v0[1] * v1[2] - v2[0] * v1[1] * v0[2]
				- v0[0] * v2[1] * v1[2] - v1[0] * v0[1] * v2[2];
	}

	/**
	 * Return geocentric latitude given a geographic latitude using the WGS84 ellipsoid
	 *
	 * @param lat
	 *            geographic latitude in radians
	 * @return geocentric latitude in radians
	 */
	static double getGeocentricLat(const double& lat);

	/**
	 * Return geographic latitude given a geocentric latitude using the WGS84 ellipsoid
	 *
	 * @param lat
	 *            geocentric latitude in radians
	 * @return geographic latitude in radians
	 */
	static double getGeographicLat(const double& lat);

	/// @cond PROTECTED  Turn off doxygen documentation until 'endcond' is found

	/**
	 * DEPRECATED: use getGeocentricLat() instead.
	 * Return geocentric latitude given a geographic latitude
	 *
	 * @param lat
	 *            geographic latitude in radians
	 * @return geocentric latitude in radians
	 */
	static double getGeoCentricLatitude(const double& lat) { return getGeocentricLat(lat); }

	/**
	 * DEPRECATED: use getGeographicLat() instead.
	 * Return geographic latitude given a geocentric latitude
	 *
	 * @param lat
	 *            geocentric latitude in radians
	 * @return geographic latitude in radians
	 */
	static double getGeoGraphicLatitude(const double& lat) { return getGeographicLat(lat); }

	///@endcond

	/**
	 * @param v unit vector to be converted to lat, lon string
	 * @return a String of lat,lon in degrees formatted with "%10.6f %11.6f"
	 */
	static string getLatLonString(const double* const v);

	/**
	 * @param v unit vector to be converted to lon,lat string
	 * @return a String of lon,lat in degrees formatted with "%11.6f %10.6f"
	 */
	static string getLonLatString(const double* const v);


	/**
	 * Find the azimuth from unit vectors v1 to v2. Result will be between -180
	 * and 180 degrees
	 *
	 * @param v1
	 *            The point from which the azimuth will be directed toward v2.
	 * @param v2
	 *            The point to which the azimuth will be directed from v1.
	 * @param errorValue
	 *            if v1 and v2 are parallel, or if v1 is either the north or
	 *            south pole, then return errorValue
	 * @return the azimuth from v1 to v2, in degrees clockwise from north, or
	 *         errorValue
	 */
	static double azimuthDegrees(const double* const v1, const double* const v2,
			double errorValue);

	/**
	 * Find the azimuth from unit vectors v1 to v2. Result will be between -PI
	 * and PI radians.
	 *
	 * @param v1
	 *            The point from which the azimuth will be directed toward v2.
	 * @param v2
	 *            The point to which the azimuth will be directed from v1.
	 * @param errorValue
	 *            if v1 and v2 are parallel, or if v1 is either the north or
	 *            south pole, then return errorValue
	 * @return the azimuth from v1 to v2, in radians clockwise from north, or
	 *         errorValue
	 */
	static double azimuth(const double* const v1, const double* const v2,
			double errorValue);

	/**
	 * Rotate unit vector x clockwise around unit vector p, by angle a. x and z
	 * may be references to the same array.  Clockwise rotation as viewed
	 * from outside the unit sphere is positive.
	 *
	 * @param x
	 *            vector to be rotated
	 * @param p
	 *            pole about which rotation is to occur.
	 * @param a
	 *            double the amount of rotation, in radians.
	 * @param z
	 *            the rotated vector, normalized to unit length.
	 */
static void rotate(const double* const x, const double* const p, double a,
			double* const z);

	/**
	 * A great circle is defined by two unit vectors that are 90 degrees apart.
	 * A great circle is stored in a double[2][3] array, which is the structure
	 * returned by this method. A great circle can be passed to the method
	 * getGreatCirclePoint() to retrieve a unit vector that is on the great
	 * circle and located some distance from the first point of the great
	 * circle.
	 * <p>
	 * This method returns a great circle that is computed from two unit vectors
	 * that are not necessarily 90 degrees apart.
	 *
	 * @param v0
	 *            the first point on the great circle
	 * @param v1
	 *            some other point that is also on the great circle but which is
	 *            not necessarily 90 degrees away from v0.
	 * @return a 2 x 3 array specifying two unit vectors. The first one is a
	 *         clone of unit vector v0 passed as first argument to this method.
	 *         The second is located 90 degrees away from v0.
	 * @throws GeoTessException
	 *             if v0 and v1 are parallel.
	 */
	static double** getGreatCircle(const double* const v0,
			const double* const v1);

	/**
	 * A great circle is defined by two unit vectors that are 90 degrees apart.
	 * A great circle is stored in a double[2][3] array, which is the structure
	 * returned by this method. A great circle can be passed to the method
	 * getGreatCirclePoint() to retrieve a unit vector that is on the great
	 * circle and located some distance from the first point of the great
	 * circle.
	 * <p>
	 * This method returns a great circle that is computed from two unit vectors
	 * that are not necessarily 90 degrees apart.
	 *
	 * @param v0
	 *            the first point on the great circle
	 * @param v1
	 *            some other point that is also on the great circle but which is
	 *            not necessarily 90 degrees away from v0.
	 * @param gc a 2 x 3 array specifying two unit vectors. The first one is a
	 *         clone of unit vector v0 passed as first argument to this method.
	 *         The second is located 90 degrees away from v0.
	 * @throws GeoTessException
	 *             if v0 and v1 are parallel.
	 */
	static void getGreatCircle(const double* const v0, const double* const v1,
			double** const gc);

	/**
	 * A great circle is defined by two unit vectors that are 90 degrees apart.
	 * A great circle is stored in a double[2][3] array, which is the structure
	 * returned by this method. A great circle can be passed to the method
	 * getGreatCirclePoint() to retrieve a unit vector that is on the great
	 * circle and located some distance from the first point of the great
	 * circle.
	 * <p>
	 * This method returns a great circle that is defined by an initial point
	 * and an azimuth.
	 *
	 * @param v
	 *            a unit vector that will be the first point on the great
	 *            circle.
	 * @param azimuth
	 *            a direction, in radians, in which to move relative to v in
	 *            order to define the great circle
	 * @return a 2 x 3 array specifying two unit vectors. The first one is a
	 *         clone of unit vector v passed as an argument to this method. The
	 *         second is located 90 degrees away from v in the direction
	 *         specified by azimuth.
	 * @throws GeoTessException
	 *             if v is located at north or south pole.
	 */
	static double** getGreatCircle(const double* const v, double azimuth);

	/**
	 * A great circle is defined by two unit vectors that are 90 degrees apart.
	 * A great circle is stored in a double[2][3] array, which is the structure
	 * returned by this method. A great circle can be passed to the method
	 * getGreatCirclePoint() to retrieve a unit vector that is on the great
	 * circle and located some distance from the first point of the great
	 * circle.
	 * <p>
	 * This method returns a great circle that is defined by an initial point
	 * and an azimuth.
	 *
	 * @param v
	 *            a unit vector that will be the first point on the great
	 *            circle.
	 * @param azimuth
	 *            a direction, in radians, in which to move relative to v in
	 *            order to define the great circle
	 * @param gc  a 2 x 3 array specifying two unit vectors. The first one is a
	 *         clone of unit vector v passed as an argument to this method. The
	 *         second is located 90 degrees away from v in the direction
	 *         specified by azimuth.
	 * @throws GeoTessException
	 *             if v is located at north or south pole.
	 */
	static void getGreatCircle(const double* const v, double azimuth,
			double** const gc);

	/**
	 * Retrieve the number of points that would be required to populate a great circle path from
	 * point A to point B with equally spaced points given that the spacing can be no greater than
	 * delta.
	 * @param ptA unit vector of first point on great circle (input)
	 * @param ptB unit vector of last point on great circle (input)
	 * @param delta desired point spacing in radians (input)
	 * @param onCenters if true, returned points will be located in the centers of equal size
	 * path increments.  If false, first point will coincide with ptA, last point will coincide
	 * with ptB and the remaining points will be equally spaced in between. (input)
	 * @return number of points required
	 */
	static int getGreatCirclePoints(double* ptA, double* ptB,
			const double& delta, const bool& onCenters);

	/**
	 * Retrieve the unit vectors of a bunch of points distributed along a great circle path between
	 * two points.
	 * @param ptA unit vector of first point on great circle (input)
	 * @param ptB unit vector of last point on great circle (input)
	 * @param npoints the number of points to distribute along the great circle path (input)
	 * @param onCenters if true, returned points will be located in the centers of equal size
	 * path increments.  If false, first point will coincide with ptA, last point will coincide
	 * with ptB and the remaining points will be equally spaced in between. (input)
	 * @param points an npoints by 3 array that will be populated with computed points.  Must be
	 * large enough to hold all the computed points.
	 * @return actual point spacing in radians
	 */
	static double getGreatCirclePoints(double* ptA, double* ptB,
			const int& npoints, const bool& onCenters, double** points);

	/**
	 * Retrieve the unit vectors of a bunch of points distributed along a great circle path between
	 * two points.  Caller specifies desired spacing of the points, not the number of points.  The
	 * acutal spacing will generally be somewhat less than the request spacing in order that an
	 * integral number of points can be equally spaced along the path.
	 * @param ptA unit vector of first point on great circle (input)
	 * @param ptB unit vector of last point on great circle (input)
	 * @param delta desired point spacing in radians (input)
	 * @param onCenters if true, returned points will be located in the centers of equal size
	 * path increments.  If false, first point will coincide with ptA, last point will coincide
	 * with ptB and the remaining points will be equally spaced in between. (input)
	 * @param points an npoints by 3 array that will be populated with computed points.  Must be
	 * large enough to hold all the computed points.
	 * @param npoints the number of points being returned.
	 * @return actual point spacing in radians
	 */
	static double getGreatCirclePoints(double* ptA, double* ptB,
			const double& delta, const bool& onCenters, double** points,
			int& npoints);

	/**
	 * Find the length of a 3-element vector.
	 *
	 * @param u double[]
	 * @return the length of the vector. Guaranteed to be >= 0.
	 */
	 static double length(const double* const u)
	{
		double l = u[0] * u[0] + u[1] * u[1] + u[2] * u[2];
		return l > 0 ? sqrt(l) : 0.;
	}

	/**
	 * Transform is a 3 x 3 matrix such that when a vector is multiplied by
	 * transform, the vector will be projected onto the plane of this
	 * GreatCircle. The z direction will point out of the plane of the great
	 * circle in the direction of the observer (lastPoint cross firstPoint;
	 * parallel to normal). The y direction will correspond to the mean of
	 * firstPoint and lastPoint. The x direction will correspond to y cross z,
	 * forming a right handed coordinate system.
	 * @param u a 3-component unit vector defining start of great circle
	 * @param v a 3-component unit vector defining end of great circle
	 * @param t 3 x 3 array that will be populated with the transform
	 */
	static void getTransform(const double* const u, const double* const v,
			double** const t);

	/**
	 * Project vector x onto the plane of a great circle. Consider a great
	 * circle defined by two unti vectors, u and v. Find the transform of x by
	 * calling t = getTransform(u, v). Then call this method: transform(x, t,
	 * g), which will calculate unit vector g such that
	 * <ul>
	 * <li>g[2] is the z direction, i.e., the component of x that points out of
	 * the plane of the great circle, toward the observer (v cross u).
	 * <li>g[1] is the y direction, i.e., the mean of u and v, and
	 * <li>g[0] is the x direction, i.e, g[1] cross g2.
	 * </ul>
	 * @param x unit vector to be transformed
	 * @param t transform obtained with call to getTransform()
	 * @param g transformed unit vector
	 */
	static void transform(const double* x,
			double const* const * const t, double* const g)
	{
		g[0] = x[0] * t[0][0] + x[1] * t[0][1] + x[2] * t[0][2];
		g[1] = x[0] * t[1][0] + x[1] * t[1][1] + x[2] * t[1][2];
		g[2] = x[0] * t[2][0] + x[1] * t[2][1] + x[2] * t[2][2];
	}

	/**
	 * Read a string from the input file stream and set into s
	 * @param s the string that will be populated with results
	 * @param ifs input stream
	 */
	static void readString(string& s, ifstream& ifs)
	{
		int sze;
		ifs >> sze;
		char* c = new char[sze];
		ifs.read(c, sze);
		s = c;
		delete[] c;
	}

	/**
	 * Write the string s to the output file stream.
	 * @param ofs output stream
	 * @param s the string to write the output stream.
	 */
	static void writeString(ofstream& ofs, const string& s)
	{
		ofs << s.length();
		ofs.write(s.c_str(), s.length());
	}

	/**
	 * Return the angular distance in radians between two unit vectors.
	 *
	 * @param v0 a 3 component unit vector
	 * @param v1 a 3 component unit vector
	 * @return angular distance in radians.
	 */
	static double angle(const double* const v0,
			const double* const v1)
	{
		double dot = v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];
		if (dot >= 1.0)
			return 0.0;
		if (dot <= -1.0)
			return PI;
		return acos(dot);
	}

	/**
	 * Return the angular distance in degrees between two unit vectors.
	 *
	 * @param v0 a 3 component unit vector
	 * @param v1 a 3 component unit vector
	 * @return angular distance in degrees.
	 */
	static double angleDegrees(const double* const v0,
			const double* const v1)
	{
		double dot = v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];
		if (dot >= 1.)
			return 0.;
		if (dot <= -1.)
			return 180.;
		return CPPUtils::toDegrees(acos(dot));
	}

	/**
	 * Given two unit vectors and their radii, return the straight line separation
	 * between their tips.  Assuming that the radii are in km, the result will
	 * also be in km.
	 *
	 * @param v0 Point 0 unit vector.
	 * @param r0 Point 0 length (km).
	 * @param v1 Point 1 unit vector.
	 * @param r1 Point 1 length (km).
	 * @return Distance between tip of v0*r0 and v1*r1, in km.
	 */
	static double getDistance3D(const double* const v0, double r0,
			const double* const v1, double r1)
	{
		double v[3] = { v0[0] * r0 - v1[0] * r1, v0[1] * r0 - v1[1] * r1, v0[2] * r0
				- v1[2] * r1 };
		return length(v);
	}

	/**
	 * Retrieve the radius of the Earth in km at the position specified by an
	 * Earth-centered unit vector.
	 * Uses the WGS84 ellipsoid.
	 *
	 * @param v Earth-centered unit vector
	 * @return radius of the Earth in km at specified position.
	 */
	static double getEarthRadius(const double* const v)
	{ return EARTH_A / sqrt(1. + EARTH_E / (1 - EARTH_E) * v[2] * v[2]); }

	/**
	 * Convert a 3-component unit vector to geographic latitude, in radians.
	 * Uses the WGS84 ellipsoid.
	 *
	 * @param v 3-component unit vector
	 * @return geographic latitude in radians.
	 */
	static double getLat(const double* const v)
	{ return getGeographicLat(asin(v[2])); }

	/**
	 * Convert a 3-component unit vector to a longitude, in radians.
	 *
	 * @param v 3 component unit vector
	 * @return longitude in radians.
	 */
	static double getLon(const double* const v)
	{ return atan2(v[1], v[0]); }

	/**
	 * Convert a 3-component unit vector to geographic latitude, in degrees.
	 * Uses the WGS84 ellipsoid.
	 *
	 * @param v 3-component unit vector
	 * @return geographic latitude in degrees.
	 */
	static double getLatDegrees(const double* const v)
	{
		// the constant 0.9933 is (1-e*e) where e is the eccentricity of the
		// earth as defined by the WGS84 ellipsoid.
		return CPPUtils::toDegrees(atan(tan(asin(v[2])) / 0.9933056199770992));
	}

	/**
	 * Convert a 3-component unit vector to a longitude, in degrees.
	 *
	 * @param v 3 component unit vector
	 * @return longitude in degrees.
	 */
	static double getLonDegrees(const double* const v)
	{ return CPPUtils::toDegrees(atan2(v[1], v[0])); }

	/**
	 * Convert geographic lat, lon into a geocentric unit vector. The
	 * x-component points toward lat,lon = 0, 0. The y-component points toward
	 * lat,lon = 0, 90. The z-component points toward north pole.
	 * Uses the WGS84 ellipsoid.
	 *
	 * @param lat
	 *            geographic latitude in degrees.
	 * @param lon
	 *            longitude in degrees.
	 * @return 3 component unit vector.
	 */
	static double* getVectorDegrees(const double& lat,
			const double& lon)
	{ return getVector(CPPUtils::toRadians(lat), CPPUtils::toRadians(lon)); }

	/**
	 * Convert geographic lat, lon into a geocentric unit vector. The
	 * x-component points toward lat,lon = 0, 0. The y-component points toward
	 * lat,lon = 0, 90. The z-component points toward north pole.
	 * Uses the WGS84 ellipsoid.
	 *
	 * @param lat geographic latitude in degrees.
	 * @param lon longitude in degrees.
	 * @param v 3 component unit vector.
	 * @return pointer to v
	 */
	static double* getVectorDegrees(const double& lat,
			const double& lon, double* v)
	{ return getVector(CPPUtils::toRadians(lat), CPPUtils::toRadians(lon), v); }

	/**
	 * Convert geographic lat, lon into a geocentric unit vector. The
	 * x-component points toward lat,lon = 0, 0. The y-component points toward
	 * lat,lon = 0, PI/2. The z-component points toward north pole.
	 * Uses the WGS84 ellipsoid.
	 *
	 * @param lat
	 *            geographic latitude in radians.
	 * @param lon
	 *            longitude in radians.
	 * @return 3 component unit vector.
	 */
	static double* getVector(const double& lat, const double& lon)
	{
		double* v = new double[3];
		getVector(lat, lon, v);
		return v;
	}

	/**
	 * Convert geographic lat, lon into a geocentric unit vector. The
	 * x-component points toward lat,lon = 0, 0. The y-component points toward
	 * lat,lon = 0, PI/2 The z-component points toward north pole.
	 * Uses the WGS84 ellipsoid.
	 *
	 * @param lat geographic latitude in radians.
	 * @param lon longitude in radians.
	 * @param v 3-component unit vector.
	 * @return a pointer to v
	 */
	static double* getVector(const double& lat, const double& lon, double* v)
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
	 * Normalize the input vector to unit length. Unlike normalize(),
	 * this method does not check to ensure that the length of the
	 * input vector is not zero.  Only call this version if
	 * certain that the length of the vector is > 0.
	 *
	 * @param u vector<double>
	 */
	static void normalizeFast(double* const u)
	{
		double len = u[0] * u[0] + u[1] * u[1] + u[2] * u[2];
		len = sqrt(len);
		u[0] /= len;
		u[1] /= len;
		u[2] /= len;
	}

	/**
	 * Normalize the input vector to unit length. Returns the length of the
	 * vector prior to normalization.
	 *
	 * @param u vector<double>
	 * @return length of the vector prior to normalization ( >= 0.)
	 */
	static double normalize(double* const u)
	{
		double len = u[0] * u[0] + u[1] * u[1] + u[2] * u[2];
		if (len > 0.)
		{
			len = sqrt(len);
			u[0] /= len;
			u[1] /= len;
			u[2] /= len;
		}
		else
		{
			len = u[0] = u[1] = u[2] = 0.0;
		}
		return len;
	}

	/**
	 * Cross product of two 3-component vectors. Result is not normalized.
	 *
	 * @param v1
	 *            vector<double> vector one.
	 * @param v2
	 *            vector<double> vector two.
	 * @param rslt
	 *            set to v1 cross v2 Result is not a unit vector.
	 */
	static void cross(const double* const v1, const double* const v2,
			double* const rslt)
	{
		rslt[0] = v1[1] * v2[2] - v1[2] * v2[1];
		rslt[1] = v1[2] * v2[0] - v1[0] * v2[2];
		rslt[2] = v1[0] * v2[1] - v1[1] * v2[0];
	}

	/**
	 * Normalized cross product of two 3-component unit vectors.
	 *
	 * @param u
	 *            vector one.
	 * @param v
	 *            vector two.
	 * @return Normalized cross product of u x v. Will be [0,0,0] if u and v are
	 *         parallel.
	 */
	static double* crossNormal(const double* const u,
			const double* const v)
	{
		double* w = new double[3];
		w[0] = u[1] * v[2] - u[2] * v[1];
		w[1] = u[2] * v[0] - u[0] * v[2];
		w[2] = u[0] * v[1] - u[1] * v[0];
		normalize(w);
		return w;
	}

	/**
	 * Normalized cross product of two 3-component unit vectors.
	 *
	 * @param u
	 *            vector one.
	 * @param v
	 *            vector two.
	 * @param w
	 *            set to u cross v, normalized to unit length. If u cross v has
	 *            zero length, w will equal (0,0,0).
	 * @return the length of u cross v prior to normalization. Guaranteed >= 0.
	 */
	static double crossNormal(const double* const u,
			const double* const v, double* const w)
	{
		w[0] = u[1] * v[2] - u[2] * v[1];
		w[1] = u[2] * v[0] - u[0] * v[2];
		w[2] = u[0] * v[1] - u[1] * v[0];
		return normalize(w);
	}

	/**
	 * Normalized cross product of a 3-component unit vector with the north
	 * pole.
	 *
	 * @param u
	 *            vector<double> vector one.
	 * @param w
	 *            set to u cross north, normalized to unit length. If u cross
	 *            north has zero length, w will equal (0,0,0).
	 * @return the length of u cross north prior to normalization. Guaranteed >=
	 *         0.
	 */
	static double crossNorth(const double* const u, double* const w)
	{
		double len = u[0] * u[0] + u[1] * u[1];
		if (len <= 0.)
		{
			len = w[0] = w[1] = w[2] = 0.;
		}
		else
		{
			len = sqrt(len);
			w[0] = u[1] / len;
			w[1] = -u[0] / len;
			w[2] = 0.;
		}
		return len;
	}

	/**
	 * Compute the normalized vector triple product (v0 x v1) x v2 and store
	 * result in rslt. It is ok if rslt is a reference to one of the input
	 * vectors. Local variables are used to ensure memory is not corrupted.
	 *
	 * @param v0
	 *            double[]
	 * @param v1
	 *            double[]
	 * @param v2
	 *            double[]
	 * @param rslt
	 *            double[]
	 * @return true if rslt has finite length, false if length(rslt) is zero.
	 */
	static bool vectorTripleProduct(const double* const v0,
			const double* const v1, const double* const v2, double* const rslt)
	{
		// set q = v0 cross v1
		double q0 = v0[1] * v1[2] - v0[2] * v1[1];
		double q1 = v0[2] * v1[0] - v0[0] * v1[2];
		double q2 = v0[0] * v1[1] - v0[1] * v1[0];

		// set w = q cross v2
		double w0 = q1 * v2[2] - q2 * v2[1];
		double w1 = q2 * v2[0] - q0 * v2[2];
		double w2 = q0 * v2[1] - q1 * v2[0];

		// set rslt = w
		rslt[0] = w0;
		rslt[1] = w1;
		rslt[2] = w2;

		// normalize rslt to unit length. if the length
		// of v1 or v2 is zero or they are nearly parallel then
		// rslt will = {0,0,0} and the function will return false;
		return normalize(rslt) != 0.;
	}

	/**
	 * Compute the normalized vector triple product (u x northPole) x u and
	 * store result in w. Returns false is u is north or south pole.
	 *
	 * @param u
	 *            double[]
	 * @param w
	 *            double[]
	 * @return true if w has finite length, false if length(w) is zero.
	 */
	static bool vectorTripleProductNorthPole(const double* const u,
			double* const w)
	{
		w[0] = -u[0] * u[2];
		w[1] = -u[1] * u[2];
		w[2] = u[1] * u[1] + u[0] * u[0];
		return normalize(w) != 0.;
	}

	/**
	 * Given three unit vectors, v0, v1 and v2, find the circumcenter, vs. The
	 * circumcenter is the unit vector of the center of a small circle that has
	 * all three unit vectors on its circumference.
	 *
	 * @param v0
	 * @param v1
	 * @param v2
	 * @param vs
	 */
	static void circumCenter(const double* const v0,
			const double* const v1, const double* const v2, double* const vs)
	{
		vs[0] = v0[1] * (v2[2] - v1[2]) + v2[1] * (v1[2] - v0[2]) + v1[1] * (v0[2] - v2[2]);
		vs[1] = v0[2] * (v2[0] - v1[0]) + v2[2] * (v1[0] - v0[0]) + v1[2] * (v0[0] - v2[0]);
		vs[2] = v0[0] * (v2[1] - v1[1]) + v2[0] * (v1[1] - v0[1]) + v1[0] * (v0[1] - v2[1]);
		double len = vs[0] * vs[0] + vs[1] * vs[1] + vs[2] * vs[2];
		len = sqrt(len);
		vs[0] /= len;
		vs[1] /= len;
		vs[2] /= len;
	}

	/**
	 * Given three unit vectors, v0, v1 and v2, find the circumcenter, vs. The
	 * circumcenter is the unit vector of the center of a small circle that has
	 * all three unit vectors on its circumference. The fourth element of vs is
	 * the dot product of the new circumcenter with one of the vertices.  In other
	 * words, cc[3] = cos(ccRadius).
	 *
	 * @param v0
	 * @param v1
	 * @param v2
	 * @param vs
	 */
	static void circumCenterPlus(const double* const v0,
			const double* const v1, const double* const v2, double* const vs)
	{
		vs[0] = v0[1] * (v2[2] - v1[2]) + v2[1] * (v1[2] - v0[2]) + v1[1] * (v0[2] - v2[2]);
		vs[1] = v0[2] * (v2[0] - v1[0]) + v2[2] * (v1[0] - v0[0]) + v1[2] * (v0[0] - v2[0]);
		vs[2] = v0[0] * (v2[1] - v1[1]) + v2[0] * (v1[1] - v0[1]) + v1[0] * (v0[1] - v2[1]);
		double len = vs[0] * vs[0] + vs[1] * vs[1] + vs[2] * vs[2];
		len = sqrt(len);
		vs[0] /= len;
		vs[1] /= len;
		vs[2] /= len;
		vs[3] = dot(vs, v0);
	}

	/**
	 * Given three unit vectors, v0, v1 and v2, find the circumcenter, vs. The
	 * circumcenter is the unit vector of the center of a small circle that has
	 * all three unit vectors on its circumference. Vectors must be specified in
	 * clockwise order.  The fourth element of returned circumcenter is
	 * the dot product of the new circumcenter with one of the vertices.  In other
	 * words, cc[3] = cos(ccRadius).
	 *
	 * <p>Caller is responsible for deleting the double*  returned by this method.
	 *
	 * @param v0
	 * @param v1
	 * @param v2
	 * @return the circumCenter: a unit vector plus cos(ccRadius)
	 */
	static double* circumCenterPlus(const double* const v0,
			const double* const v1, const double* const v2)
	{
		double* vs = new double[4];
		vs[0] = v0[1] * (v2[2] - v1[2]) + v2[1] * (v1[2] - v0[2]) + v1[1] * (v0[2] - v2[2]);
		vs[1] = v0[2] * (v2[0] - v1[0]) + v2[2] * (v1[0] - v0[0]) + v1[2] * (v0[0] - v2[0]);
		vs[2] = v0[0] * (v2[1] - v1[1]) + v2[0] * (v1[1] - v0[1]) + v1[0] * (v0[1] - v2[1]);
		double len = vs[0] * vs[0] + vs[1] * vs[1] + vs[2] * vs[2];
		len = sqrt(len);
		vs[0] /= len;
		vs[1] /= len;
		vs[2] /= len;
		vs[3] = dot(vs, v0);
		return vs;
	}

	/**
	 * Given the three unit vectors, t[0], t[1] and t[2], find the circumcenter,
	 * vs. The circumcenter is the unit vector of the center of a small circle
	 * that has all three unit vectors on its circumference.  The fourth element of
	 * returned circumcenter is the dot product of the new circumcenter with one
	 * of the vertices.  In other words, cc[3] = cos(ccRadius).
	 *
	 * @param t a 3 x 3 array of doubles that contains the three unit vectors
	 *            of a triangle.
	 * @param vs a 4 element array that will be populated with the unit vector
	 * and the cos(ccRadius).
	 * @return true if successful, false if the three input vectors do not
	 *         define a triangle.
	 */
	static void circumCenterPlus(double const* const * const t, double* const vs)
	{ circumCenterPlus(t[0], t[1], t[2], vs); }

	/**
	 * Move unit vector w specified distance in direction given by azimuth and
	 * return the result in u. If w is north or south pole, u will be equal to
	 * the same pole and method returns false.
	 *
	 * @param w
	 *            double[] unit vector of starting position
	 * @param distance
	 *            distance to move in radians
	 * @param azimuth
	 *            direction to move in radians
	 * @param u
	 *            double[] unit vector of resulting position.
	 * @return true if successful, false if w is north or south pole
	 */
	static bool moveDistAz(const double* const w, double distance,
			double azimuth, double* const u)
	{
		double n[3] = { 0.0, 0.0, 0.0 };
		if (moveNorth(w, distance, n))
		{
			rotate(n, w, azimuth, u);
			return true;
		}
		u[0] = w[0];
		u[1] = w[1];
		u[2] = w[2];
		return false;
	}

	/**
	 * Move unit vector w in direction of vtp by distance a and store result in
	 * u. vtp is assumed to be a unit vector normal to w on input.
	 *
	 * @param w
	 *            double[]
	 * @param vtp
	 *            double[]
	 * @param a
	 *            double
	 * @param u
	 *            double[]
	 */
	static void move(const double* const w, const double* const vtp,
			double a, double* const u)
	{
		double cosa = cos(a);
		double sina = sin(a);
		u[0] = cosa * w[0] + sina * vtp[0];
		u[1] = cosa * w[1] + sina * vtp[1];
		u[2] = cosa * w[2] + sina * vtp[2];
	}

	/**
	 * Return a unit vector that is distance radians due north of positon x. If
	 * x is the north or south pole, then z is set equal to x.
	 *
	 * @param x
	 *            the position to be moved.
	 * @param distance
	 *            the distance, in radians, that x is to be moved toward the
	 *            north.
	 * @param z
	 *            the 3-element unit vector representing the position after
	 *            having moved distance north.
	 * @return true if operation successful, false if x is north or south pole.
	 */
	static bool moveNorth(const double* const x, double distance,
			double* const z)
	{
		double vtp[3] = { 0.0, 0.0, 0.0 };
		if (vectorTripleProductNorthPole(x, vtp))
		{
			move(x, vtp, distance, z);
			return true;
		}

		z[0] = x[0];
		z[1] = x[1];
		z[2] = x[2];
		return false;
	}

	/**
	 * Returns true if unit vector u is very close to [0, 0, +/- 1]
	 *
	 * @param u
	 *            unit vector
	 * @return true if unit vector u is very close to [0, 0, +/- 1]
	 */
	static bool isPole(const double* const u)
	{
		return (u[0] * u[0] + u[1] * u[1]) < 1.0e-15;
	}

	/**
	 * Returns true if unit vector u and v are parallel or very close to it
	 *
	 * @param u
	 *            a unit vector
	 * @param v
	 *            another unit vector
	 * @return 1.-abs(dot(u,v)) < 2e-15
	 */
	static bool parallel(const double* const u, const double* const v)
	{
		return 1.0 - abs(u[0] * v[0] + u[1] * v[1] + u[2] * v[2]) < 2.0e-15;
	}

	/**
	 * A great circle is defined by two unit vectors that are 90 degrees apart.
	 * A great circle is stored in a double[2][3] array and one can be obtained
	 * by calling one of the getGreatCircle() methods.
	 * <p>
	 * In this method, a great circle and a distance are specified and a point
	 * is returned which is on the great circle path and is the specified
	 * distance away from the first point of the great circle.
	 *
	 * @param greatCircle
	 *            a great circle structure
	 * @param distance
	 *            distance in radians from first point of great circle
	 * @return unit vector of point which is on great circle and located
	 *         specified distance away from first point of great circle.
	 */
	static double* getGreatCirclePoint(
			double const* const * const greatCircle, double distance)
	{
		double* v = new double[3];
		getGreatCirclePoint(greatCircle, distance, v);
		return v;
	}

	/**
	 * A great circle is defined by two unit vectors that are 90 degrees apart.
	 * A great circle is stored in a double[2][3] array and one can be obtained
	 * by calling one of the getGreatCircle() methods.
	 * <p>
	 * In this method, a great circle and a distance are specified and a point
	 * is returned which is on the great circle path and is the specified
	 * distance away from the first point of the great circle.
	 *
	 * @param greatCircle
	 *            a great circle structure
	 * @param distance
	 *            distance in radians from first point of great circle
	 * @param v
	 *            unit vector of point which is on great circle and located
	 *            specified distance away from first point of great circle.
	 */
	static void getGreatCirclePoint(
			double const* const * const greatCircle, double distance,
			double* const v)
	{
		double cosa = cos(distance);
		double sina = sin(distance);
		v[0] = cosa * greatCircle[0][0] + sina * greatCircle[1][0];
		v[1] = cosa * greatCircle[0][1] + sina * greatCircle[1][1];
		v[2] = cosa * greatCircle[0][2] + sina * greatCircle[1][2];
	}

	/**
	 * @param v0 first corner of triangle
	 * @param v1 second corner of triangle
	 * @param v2 third corner of triangle
	 * @return the area of a triangle defined by three 3-component vectors
	 */
	static double getTriangleArea(const double* const v0,
			const double* const v1, const double* const v2)
	{
		double v10[3] = { v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2] };
		double v20[3] = { v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2] };
		double u[3] = { v10[1] * v20[2] - v10[2] * v20[1], v10[2] * v20[0]
				- v10[0] * v20[2], v10[0] * v20[1] - v10[1] * v20[0] };
		return sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]) / 2.;
	}

	/**
	 * @param v0 first corner of triangle
	 * @param v1 second corner of triangle
	 * @param v2 third corner of triangle
	 * @param work1 a double[3] used as work space
	 * @param work2 a double[3] used as work space
	 * @param work3 a double[3] used as work space
	 * @return the area of a triangle defined by three 3-component vectors
	 */
	static double getTriangleArea(const double* const v0,
			const double* const v1, const double* const v2, double* work1, double* work2, double* work3)
	{
		work1[0] = v1[0] - v0[0];
		work1[1] = v1[1] - v0[1];
		work1[2] = v1[2] - v0[2];

		work2[0] = v2[0] - v0[0];
		work2[1] = v2[1] - v0[1];
		work2[2] = v2[2] - v0[2];

		work3[0] = work1[1] * work2[2] - work1[2] * work2[1];
		work3[1] = work1[2] * work2[0] - work1[0] * work2[2];
		work3[2] = work1[0] * work2[1] - work1[1] * work2[0];

		return sqrt(work3[0] * work3[0] + work3[1] * work3[1] + work3[2] * work3[2]) / 2.;
	}

	/**
	 * Return the normalized vector sum of the supplied unit vectors.
	 *
	 * @param v one or more unit vectors
	 * @param n size of v
	 * @return the normalized vector sum of the supplied unit vectors.
	 */
	static double* center(double const * const * const v, int n)
	{
		double* x = new double[3];
		x[0] = x[1] = x[2] = 0.0;
		if (n == 0)
			return x;

		for (int i = 0; i < n; ++i)
		{
			x[0] += v[i][0];
			x[1] += v[i][1];
			x[2] += v[i][2];
		}
		normalize(x);
		return x;
	}

	/**
	 * Return the normalized vector sum of the supplied unit vectors.
	 *
	 * @param v one or more unit vectors
	 * @param x the 3-element array of double in which to place the
	 * computed unit vector.
	 */
	static void center(vector<double*> v, double* x)
	{
		x[0] = x[1] = x[2] = 0.0;

		for (size_t i = 0; i < v.size(); ++i)
		{
			x[0] += v[i][0];
			x[1] += v[i][1];
			x[2] += v[i][2];
		}
		normalize(x);
	}
};
// end class GeoTessUtils

// **** _INLINE FUNCTION IMPLEMENTATIONS_ **************************************

// conversion from geocentric to geographic latitude, in radians.
static const double geographic[] = {
-1.5707963267948966,
-1.5707963267948966, -1.5677489031921250, -1.5647014788238576, -1.5616540529246252,
-1.5586066247290158, -1.5555591934716986, -1.5525117583874544, -1.5494643187112020,
-1.5464168736780266, -1.5433694225232062, -1.5403219644822410, -1.5372744987908793,
-1.5342270246851464, -1.5311795414013718, -1.5281320481762162, -1.5250845442467005,
-1.5220370288502316, -1.5189895012246317, -1.5159419606081646, -1.5128944062395640,
-1.5098468373580602, -1.5067992532034090, -1.5037516530159180, -1.5007040360364745,
-1.4976564015065728, -1.4946087486683413, -1.4915610767645720, -1.4885133850387440,
-1.4854656727350555, -1.4824179390984470, -1.4793701833746320, -1.4763224048101213,
-1.4732746026522530, -1.4702267761492180, -1.4671789245500877, -1.4641310471048419,
-1.4610831430643942, -1.4580352116806217, -1.4549872522063898, -1.4519392638955806,
-1.4488912460031191, -1.4458431977850015, -1.4427951184983205, -1.4397470074012940,
-1.4366988637532907, -1.4336506868148570, -1.4306024758477454, -1.4275542301149386,
-1.4245059488806797, -1.4214576314104954, -1.4184092769712255, -1.4153608848310480,
-1.4123124542595058, -1.4092639845275337, -1.4062154749074850, -1.4031669246731584,
-1.4001183330998220, -1.3970696994642429, -1.3940210230447112, -1.3909723031210681,
-1.3879235389747300, -1.3848747298887172, -1.3818258751476769, -1.3787769740379130,
-1.3757280258474085, -1.3726790298658542, -1.3696299853846736, -1.3665808916970477,
-1.3635317480979430, -1.3604825538841352, -1.3574333083542363, -1.3543840108087190,
-1.3513346605499443, -1.3482852568821840, -1.3452357991116488, -1.3421862865465124,
-1.3391367184969374, -1.3360870942751000, -1.3330374131952152, -1.3299876745735633,
-1.3269378777285121, -1.3238880219805456, -1.3208381066522850, -1.3177881310685170,
-1.3147380945562173, -1.3116879964445736, -1.3086378360650126, -1.3055876127512245,
-1.3025373258391855, -1.2994869746671844, -1.2964365585758457, -1.2933860769081540,
-1.2903355290094787, -1.2872849142275975, -1.2842342319127213, -1.2811834814175174,
-1.2781326620971334, -1.2750817733092212, -1.2720308144139612, -1.2689797847740851,
-1.2659286837548998, -1.2628775107243113, -1.2598262650528476, -1.2567749461136817,
-1.2537235532826554, -1.2506720859383025, -1.2476205434618720, -1.2445689252373493,
-1.2415172306514815, -1.2384654590937990, -1.2354136099566380, -1.2323616826351630,
-1.2293096765273912, -1.2262575910342110, -1.2232054255594090, -1.2201531795096878,
-1.2171008522946918, -1.2140484433270270, -1.2109959520222835, -1.2079433777990571,
-1.2048907200789720, -1.2018379782867012, -1.1987851518499886, -1.1957322401996706,
-1.1926792427696968, -1.1896261589971520, -1.1865729883222764, -1.1835197301884879,
-1.1804663840424017, -1.1774129493338514, -1.1743594255159102, -1.1713058120449116,
-1.1682521083804690, -1.1651983139854960, -1.1621444283262286, -1.1590904508722426,
-1.1560363810964758, -1.1529822184752465, -1.1499279624882743, -1.1468736126186994,
-1.1438191683531016, -1.1407646291815210, -1.1377099945974760, -1.1346552640979840,
-1.1316004371835788, -1.1285455133583318, -1.1254904921298676, -1.1224353730093866,
-1.1193801555116805, -1.1163248391551530, -1.1132694234618365, -1.1102139079574118,
-1.1071582921712249, -1.1041025756363059, -1.1010467578893872, -1.0979908384709198,
-1.0949348169250928, -1.0918786927998494, -1.0888224656469052, -1.0857661350217649,
-1.0827097004837398, -1.0796531615959652, -1.0765965179254158, -1.0735397690429236,
-1.0704829145231949, -1.0674259539448256, -1.0643688868903185, -1.0613117129460985,
-1.0582544317025300, -1.0551970427539317, -1.0521395456985925, -1.0490819401387883,
-1.0460242256807961, -1.0429664019349100, -1.0399084685154565, -1.0368504250408095,
-1.0337922711334060, -1.0307340064197588, -1.0276756305304735, -1.0246171431002629,
-1.0215585437679590, -1.0184998321765302, -1.0154410079730933, -1.0123820708089293,
-1.0093230203394952, -1.0062638562244390, -1.0032045781276135, -1.0001451857170887,
-0.9970856786651652, -0.9940260566483880, -0.9909663193475587, -0.9879064664477488,
-0.9848464976383120, -0.9817864126128968, -0.9787262110694590, -0.9756658927102736,
-0.9726054572419468, -0.9695449043754286, -0.9664842338260234, -0.9634234453134026,
-0.9603625385616154, -0.9573015132991002, -0.9542403692586956, -0.9511791061776521,
-0.9481177237976417, -0.9450562218647695, -0.9419946001295835, -0.9389328583470855,
-0.9358709962767409, -0.9328090136824890, -0.9297469103327525, -0.9266846860004473,
-0.9236223404629921, -0.9205598735023176, -0.9174972849048763, -0.9144345744616503,
-0.9113717419681617, -0.9083087872244800, -0.9052457100352315, -0.9021825102096069,
-0.8991191875613709, -0.8960557419088682, -0.8929921730750334, -0.8899284808873974,
-0.8868646651780957, -0.8838007257838750, -0.8807366625461014, -0.8776724753107666,
-0.8746081639284953, -0.8715437282545516, -0.8684791681488458, -0.8654144834759410,
-0.8623496741050579, -0.8592847399100833, -0.8562196807695732, -0.8531544965667609,
-0.8500891871895607, -0.8470237525305747, -0.8439581924870967, -0.8408925069611178,
-0.8378266958593319, -0.8347607590931386, -0.8316946965786500, -0.8286285082366928,
-0.8255621939928134, -0.8224957537772827, -0.8194291875250981, -0.8163624951759885,
-0.8132956766744170, -0.8102287319695851, -0.8071616610154342, -0.8040944637706497,
-0.8010271401986641, -0.7979596902676580, -0.7948921139505637, -0.7918244112250671,
-0.7887565820736097, -0.7856886264833902, -0.7826205444463665, -0.7795523359592570,
-0.7764840010235420, -0.7734155396454647, -0.7703469518360321, -0.7672782376110162,
-0.7642093969909539, -0.7611404300011481, -0.7580713366716669, -0.7550021170373452,
-0.7519327711377832, -0.7488632990173464, -0.7457937007251657, -0.7427239763151360,
-0.7396541258459155, -0.7365841493809252, -0.7335140469883469, -0.7304438187411222,
-0.7273734647169510, -0.7243029849982892, -0.7212323796723475, -0.7181616488310889,
-0.7150907925712262, -0.7120198109942198, -0.7089487042062748, -0.7058774723183379,
-0.7028061154460953, -0.6997346337099688, -0.6966630272351121, -0.6935912961514075,
-0.6905194405934625, -0.6874474607006055, -0.6843753566168813, -0.6813031284910476,
-0.6782307764765696, -0.6751583007316160, -0.6720857014190538, -0.6690129787064436,
-0.6659401327660337, -0.6628671637747551, -0.6597940719142159, -0.6567208573706956,
-0.6536475203351388, -0.6505740610031495, -0.6475004795749841, -0.6444267762555459,
-0.6413529512543770, -0.6382790047856527, -0.6352049370681742, -0.6321307483253605,
-0.6290564387852420, -0.6259820086804524, -0.6229074582482208, -0.6198327877303641,
-0.6167579973732786, -0.6136830874279318, -0.6106080581498534, -0.6075329097991277,
-0.6044576426403837, -0.6013822569427861, -0.5983067529800269, -0.5952311310303151,
-0.5921553913763675, -0.5890795343053987, -0.5860035601091116, -0.5829274690836870,
-0.5798512615297728, -0.5767749377524746, -0.5736984980613438, -0.5706219427703677,
-0.5675452721979585, -0.5644684866669408, -0.5613915865045422, -0.5583145720423796,
-0.5552374436164493, -0.5521602015671145, -0.5490828462390925, -0.5460053779814434,
-0.5429277971475571, -0.5398501040951412, -0.5367722991862070, -0.5336943827870587,
-0.5306163552682779, -0.5275382170047123, -0.5244599683754603, -0.5213816097638592,
-0.5183031415574704, -0.5152245641480652, -0.5121458779316114, -0.5090670833082580,
-0.5059881806823221, -0.5029091704622723, -0.4998300530607160, -0.4967508288943822,
-0.4936714983841087, -0.4905920619548242, -0.4875125200355348, -0.4844328730593077,
-0.4813531214632545, -0.4782732656885168, -0.4751933061802482, -0.4721132433875999,
-0.4690330777637020, -0.4659528097656490, -0.4628724398544813, -0.4597919684951693,
-0.4567113961565957, -0.4536307233115377, -0.4505499504366511, -0.4474690780124505,
-0.4443881065232937, -0.4413070364573617, -0.4382258683066425, -0.4351446025669108,
-0.4320632397377119, -0.4289817803223404, -0.4259002248278240, -0.4228185737649034,
-0.4197368276480125, -0.4166549869952609, -0.4135730523284130, -0.4104910241728699,
-0.4074089030576480, -0.4043266895153610, -0.4012443840821984, -0.3981619872979068,
-0.3950794997057681, -0.3919969218525802, -0.3889142542886367, -0.3858314975677045,
-0.3827486522470051, -0.3796657188871915, -0.3765826980523292, -0.3734995903098728,
-0.3704163962306467, -0.3673331163888213, -0.3642497513618933, -0.3611663017306622,
-0.3580827680792097, -0.3549991509948770, -0.3519154510682416, -0.3488316688930969,
-0.3457478050664276, -0.3426638601883887, -0.3395798348622813, -0.3364957296945312,
-0.3334115452946638, -0.3303272822752831, -0.3272429412520464, -0.3241585228436426,
-0.3210740276717675, -0.3179894563610997, -0.3149048095392786, -0.3118200878368778,
-0.3087352918873839, -0.3056504223271693, -0.3025654797954709, -0.2994804649343629,
-0.2963953783887339, -0.2933102208062620, -0.2902249928373890, -0.2871396951352969,
-0.2840543283558812, -0.2809688931577279, -0.2778833902020855, -0.2747978201528420,
-0.2717121836764979, -0.2686264814421417, -0.2655407141214227, -0.2624548823885265,
-0.2593689869201489, -0.2562830283954686, -0.2531970074961227, -0.2501109249061789,
-0.2470247813121108, -0.2439385774027693, -0.2408523138693584, -0.2377659914054063,
-0.2346796107067404, -0.2315931724714588, -0.2285066773999049, -0.2254201261946395,
-0.2223335195604130, -0.2192468582041398, -0.2161601428348690, -0.2130733741637589,
-0.2099865529040473, -0.2068996797710264, -0.2038127554820124, -0.2007257807563200,
-0.1976387563152328, -0.1945516828819765, -0.1914645611816903, -0.1883773919413984,
-0.1852901758899828, -0.1822029137581537, -0.1791156062784226, -0.1760282541850719,
-0.1729408582141288, -0.1698534191033340, -0.1667659375921156, -0.1636784144215582,
-0.1605908503343753, -0.1575032460748804, -0.1544156023889568, -0.1513279200240304,
-0.1482401997290386, -0.1451524422544033, -0.1420646483519993, -0.1389768187751272,
-0.1358889542784822, -0.1328010556181263, -0.1297131235514570, -0.1266251588371797,
-0.1235371622352770, -0.1204491345069784, -0.1173610764147325, -0.1142729887221751,
-0.1111848721941013, -0.1080967275964339, -0.1050085556961953, -0.1019203572614753,
-0.0988321330614037, -0.0957438838661177, -0.0926556104467337, -0.0895673135753164,
-0.0864789940248483, -0.0833906525692003, -0.0803022899831004, -0.0772139070421047,
-0.0741255045225655, -0.0710370832016027, -0.0679486438570714, -0.0648601872675336,
-0.0617717142122262, -0.0586832254710305, -0.0555947218244430, -0.0525062040535433,
-0.0494176729399648, -0.0463291292658628, -0.0432405738138854, -0.0401520073671412,
-0.0370634307091703, -0.0339748446239121, -0.0308862498956756, -0.0277976473091088,
-0.0247090376491666, -0.0216204217010820, -0.0185318002503333, -0.0154431740826154,
-0.0123545439838071, -0.0092659107399420, -0.0061772751371762, -0.0030886379617588,
0.0000000000000000, 0.0030886379617588, 0.0061772751371762, 0.0092659107399420,
0.0123545439838071, 0.0154431740826154, 0.0185318002503333, 0.0216204217010820,
0.0247090376491666, 0.0277976473091088, 0.0308862498956756, 0.0339748446239121,
0.0370634307091703, 0.0401520073671412, 0.0432405738138854, 0.0463291292658628,
0.0494176729399648, 0.0525062040535433, 0.0555947218244430, 0.0586832254710305,
0.0617717142122262, 0.0648601872675336, 0.0679486438570714, 0.0710370832016027,
0.0741255045225655, 0.0772139070421047, 0.0803022899831004, 0.0833906525692003,
0.0864789940248483, 0.0895673135753164, 0.0926556104467337, 0.0957438838661177,
0.0988321330614037, 0.1019203572614753, 0.1050085556961953, 0.1080967275964339,
0.1111848721941013, 0.1142729887221751, 0.1173610764147325, 0.1204491345069784,
0.1235371622352770, 0.1266251588371797, 0.1297131235514570, 0.1328010556181263,
0.1358889542784822, 0.1389768187751272, 0.1420646483519993, 0.1451524422544033,
0.1482401997290386, 0.1513279200240304, 0.1544156023889568, 0.1575032460748804,
0.1605908503343753, 0.1636784144215582, 0.1667659375921156, 0.1698534191033340,
0.1729408582141288, 0.1760282541850719, 0.1791156062784226, 0.1822029137581537,
0.1852901758899828, 0.1883773919413984, 0.1914645611816903, 0.1945516828819765,
0.1976387563152328, 0.2007257807563200, 0.2038127554820124, 0.2068996797710264,
0.2099865529040473, 0.2130733741637589, 0.2161601428348690, 0.2192468582041398,
0.2223335195604130, 0.2254201261946395, 0.2285066773999049, 0.2315931724714588,
0.2346796107067404, 0.2377659914054063, 0.2408523138693584, 0.2439385774027693,
0.2470247813121108, 0.2501109249061789, 0.2531970074961227, 0.2562830283954686,
0.2593689869201489, 0.2624548823885265, 0.2655407141214227, 0.2686264814421417,
0.2717121836764979, 0.2747978201528420, 0.2778833902020855, 0.2809688931577279,
0.2840543283558812, 0.2871396951352969, 0.2902249928373890, 0.2933102208062620,
0.2963953783887339, 0.2994804649343629, 0.3025654797954709, 0.3056504223271693,
0.3087352918873839, 0.3118200878368778, 0.3149048095392786, 0.3179894563610997,
0.3210740276717675, 0.3241585228436426, 0.3272429412520464, 0.3303272822752831,
0.3334115452946638, 0.3364957296945312, 0.3395798348622813, 0.3426638601883887,
0.3457478050664276, 0.3488316688930969, 0.3519154510682416, 0.3549991509948770,
0.3580827680792097, 0.3611663017306622, 0.3642497513618933, 0.3673331163888213,
0.3704163962306467, 0.3734995903098728, 0.3765826980523292, 0.3796657188871915,
0.3827486522470051, 0.3858314975677045, 0.3889142542886367, 0.3919969218525802,
0.3950794997057681, 0.3981619872979068, 0.4012443840821984, 0.4043266895153610,
0.4074089030576480, 0.4104910241728699, 0.4135730523284130, 0.4166549869952609,
0.4197368276480125, 0.4228185737649034, 0.4259002248278240, 0.4289817803223404,
0.4320632397377117, 0.4351446025669111, 0.4382258683066425, 0.4413070364573617,
0.4443881065232935, 0.4474690780124507, 0.4505499504366511, 0.4536307233115377,
0.4567113961565955, 0.4597919684951695, 0.4628724398544813, 0.4659528097656488,
0.4690330777637022, 0.4721132433875999, 0.4751933061802482, 0.4782732656885166,
0.4813531214632548, 0.4844328730593077, 0.4875125200355348, 0.4905920619548240,
0.4936714983841087, 0.4967508288943822, 0.4998300530607158, 0.5029091704622726,
0.5059881806823221, 0.5090670833082580, 0.5121458779316113, 0.5152245641480654,
0.5183031415574704, 0.5213816097638592, 0.5244599683754600, 0.5275382170047123,
0.5306163552682779, 0.5336943827870585, 0.5367722991862072, 0.5398501040951412,
0.5429277971475571, 0.5460053779814433, 0.5490828462390928, 0.5521602015671145,
0.5552374436164493, 0.5583145720423793, 0.5613915865045422, 0.5644684866669408,
0.5675452721979584, 0.5706219427703679, 0.5736984980613438, 0.5767749377524745,
0.5798512615297726, 0.5829274690836871, 0.5860035601091116, 0.5890795343053986,
0.5921553913763673, 0.5952311310303152, 0.5983067529800269, 0.6013822569427860,
0.6044576426403839, 0.6075329097991278, 0.6106080581498534, 0.6136830874279315,
0.6167579973732787, 0.6198327877303641, 0.6229074582482207, 0.6259820086804526,
0.6290564387852421, 0.6321307483253605, 0.6352049370681740, 0.6382790047856529,
0.6413529512543770, 0.6444267762555458, 0.6475004795749840, 0.6505740610031496,
0.6536475203351388, 0.6567208573706955, 0.6597940719142161, 0.6628671637747552,
0.6659401327660337, 0.6690129787064435, 0.6720857014190540, 0.6751583007316160,
0.6782307764765694, 0.6813031284910472, 0.6843753566168814, 0.6874474607006055,
0.6905194405934625, 0.6935912961514077, 0.6966630272351122, 0.6997346337099688,
0.7028061154460952, 0.7058774723183380, 0.7089487042062748, 0.7120198109942197,
0.7150907925712261, 0.7181616488310891, 0.7212323796723475, 0.7243029849982890,
0.7273734647169512, 0.7304438187411223, 0.7335140469883469, 0.7365841493809251,
0.7396541258459156, 0.7427239763151360, 0.7457937007251656, 0.7488632990173463,
0.7519327711377833, 0.7550021170373452, 0.7580713366716668, 0.7611404300011482,
0.7642093969909540, 0.7672782376110162, 0.7703469518360320, 0.7734155396454648,
0.7764840010235420, 0.7795523359592569, 0.7826205444463663, 0.7856886264833903,
0.7887565820736097, 0.7918244112250670, 0.7948921139505639, 0.7979596902676581,
0.8010271401986641, 0.8040944637706496, 0.8071616610154343, 0.8102287319695851,
0.8132956766744170, 0.8163624951759882, 0.8194291875250982, 0.8224957537772827,
0.8255621939928133, 0.8286285082366929, 0.8316946965786500, 0.8347607590931386,
0.8378266958593317, 0.8408925069611180, 0.8439581924870967, 0.8470237525305746,
0.8500891871895606, 0.8531544965667610, 0.8562196807695732, 0.8592847399100831,
0.8623496741050583, 0.8654144834759411, 0.8684791681488458, 0.8715437282545514,
0.8746081639284954, 0.8776724753107666, 0.8807366625461013, 0.8838007257838747,
0.8868646651780958, 0.8899284808873974, 0.8929921730750333, 0.8960557419088684,
0.8991191875613709, 0.9021825102096069, 0.9052457100352312, 0.9083087872244802,
0.9113717419681617, 0.9144345744616502, 0.9174972849048760, 0.9205598735023178,
0.9236223404629921, 0.9266846860004472, 0.9297469103327527, 0.9328090136824891,
0.9358709962767409, 0.9389328583470853, 0.9419946001295836, 0.9450562218647695,
0.9481177237976417, 0.9511791061776519, 0.9542403692586957, 0.9573015132991002,
0.9603625385616152, 0.9634234453134028, 0.9664842338260234, 0.9695449043754285,
0.9726054572419467, 0.9756658927102736, 0.9787262110694590, 0.9817864126128968,
0.9848464976383122, 0.9879064664477489, 0.9909663193475587, 0.9940260566483880,
0.9970856786651654, 1.0001451857170889, 1.0032045781276135, 1.0062638562244390,
1.0093230203394952, 1.0123820708089293, 1.0154410079730933, 1.0184998321765304,
1.0215585437679590, 1.0246171431002629, 1.0276756305304735, 1.0307340064197590,
1.0337922711334060, 1.0368504250408095, 1.0399084685154563, 1.0429664019349100,
1.0460242256807961, 1.0490819401387883, 1.0521395456985927, 1.0551970427539317,
1.0582544317025300, 1.0613117129460985, 1.0643688868903185, 1.0674259539448256,
1.0704829145231949, 1.0735397690429234, 1.0765965179254158, 1.0796531615959652,
1.0827097004837398, 1.0857661350217650, 1.0888224656469052, 1.0918786927998494,
1.0949348169250925, 1.0979908384709200, 1.1010467578893872, 1.1041025756363059,
1.1071582921712246, 1.1102139079574118, 1.1132694234618365, 1.1163248391551530,
1.1193801555116807, 1.1224353730093866, 1.1254904921298676, 1.1285455133583315,
1.1316004371835790, 1.1346552640979840, 1.1377099945974760, 1.1407646291815206,
1.1438191683531016, 1.1468736126186994, 1.1499279624882743, 1.1529822184752467,
1.1560363810964758, 1.1590904508722426, 1.1621444283262283, 1.1651983139854962,
1.1682521083804690, 1.1713058120449116, 1.1743594255159100, 1.1774129493338514,
1.1804663840424017, 1.1835197301884879, 1.1865729883222769, 1.1896261589971520,
1.1926792427696968, 1.1957322401996704, 1.1987851518499888, 1.2018379782867012,
1.2048907200789720, 1.2079433777990570, 1.2109959520222835, 1.2140484433270270,
1.2171008522946918, 1.2201531795096880, 1.2232054255594090, 1.2262575910342110,
1.2293096765273910, 1.2323616826351633, 1.2354136099566380, 1.2384654590937990,
1.2415172306514812, 1.2445689252373493, 1.2476205434618720, 1.2506720859383025,
1.2537235532826556, 1.2567749461136817, 1.2598262650528476, 1.2628775107243113,
1.2659286837549000, 1.2689797847740851, 1.2720308144139612, 1.2750817733092210,
1.2781326620971334, 1.2811834814175174, 1.2842342319127213, 1.2872849142275977,
1.2903355290094787, 1.2933860769081540, 1.2964365585758455, 1.2994869746671847,
1.3025373258391855, 1.3055876127512245, 1.3086378360650124, 1.3116879964445736,
1.3147380945562173, 1.3177881310685170, 1.3208381066522852, 1.3238880219805456,
1.3269378777285121, 1.3299876745735630, 1.3330374131952154, 1.3360870942751000,
1.3391367184969374, 1.3421862865465122, 1.3452357991116488, 1.3482852568821840,
1.3513346605499441, 1.3543840108087193, 1.3574333083542363, 1.3604825538841352,
1.3635317480979428, 1.3665808916970479, 1.3696299853846736, 1.3726790298658542,
1.3757280258474083, 1.3787769740379130, 1.3818258751476769, 1.3848747298887170,
1.3879235389747302, 1.3909723031210681, 1.3940210230447112, 1.3970696994642426,
1.4001183330998221, 1.4031669246731584, 1.4062154749074850, 1.4092639845275339,
1.4123124542595058, 1.4153608848310480, 1.4184092769712255, 1.4214576314104956,
1.4245059488806797, 1.4275542301149386, 1.4306024758477451, 1.4336506868148573,
1.4366988637532907, 1.4397470074012940, 1.4427951184983208, 1.4458431977850015,
1.4488912460031191, 1.4519392638955804, 1.4549872522063900, 1.4580352116806217,
1.4610831430643942, 1.4641310471048417, 1.4671789245500880, 1.4702267761492180,
1.4732746026522530, 1.4763224048101216, 1.4793701833746320, 1.4824179390984470,
1.4854656727350553, 1.4885133850387442, 1.4915610767645720, 1.4946087486683413,
1.4976564015065725, 1.5007040360364747, 1.5037516530159180, 1.5067992532034090,
1.5098468373580605, 1.5128944062395640, 1.5159419606081646, 1.5189895012246315,
1.5220370288502318, 1.5250845442467005, 1.5281320481762162, 1.5311795414013716,
1.5342270246851466, 1.5372744987908793, 1.5403219644822408, 1.5433694225232064,
1.5464168736780266, 1.5494643187112020, 1.5525117583874541, 1.5555591934716988,
1.5586066247290158, 1.5616540529246252, 1.5647014788238574, 1.5677489031921252,
1.5707963267948966, 1.5707963267948966 };

// conversion from geographic to geocentric latitude, in radians.

static const double geocentric[] = {
-1.5707963267948966,
-1.5707963267948966, -1.5677076888331378, -1.5646190516577203, -1.5615304160549546,
-1.5584417828110895, -1.5553531527122813, -1.5522645265445634, -1.5491759050938145,
-1.5460872891457300, -1.5429986794857877, -1.5399100768992209, -1.5368214821709845,
-1.5337328960857262, -1.5306443194277555, -1.5275557529810110, -1.5244671975290338,
-1.5213786538549317, -1.5182901227413532, -1.5152016049704535, -1.5121131013238660,
-1.5090246125826705, -1.5059361395273627, -1.5028476829378250, -1.4997592435932938,
-1.4966708222723310, -1.4935824197527918, -1.4904940368117963, -1.4874056742256963,
-1.4843173327700483, -1.4812290132195800, -1.4781407163481628, -1.4750524429287788,
-1.4719641937334929, -1.4688759695334213, -1.4657877710987013, -1.4626995991984626,
-1.4596114546007952, -1.4565233380727216, -1.4534352503801642, -1.4503471922879183,
-1.4472591645596196, -1.4441711679577170, -1.4410832032434395, -1.4379952711767703,
-1.4349073725164143, -1.4318195080197693, -1.4287316784428974, -1.4256438845404933,
-1.4225561270658580, -1.4194684067708663, -1.4163807244059399, -1.4132930807200161,
-1.4102054764605212, -1.4071179123733384, -1.4040303892027810, -1.4009429076915625,
-1.3978554685807678, -1.3947680726098246, -1.3916807205164740, -1.3885934130367428,
-1.3855061509049138, -1.3824189348534980, -1.3793317656132063, -1.3762446439129201,
-1.3731575704796637, -1.3700705460385765, -1.3669835713128842, -1.3638966470238703,
-1.3608097738908493, -1.3577229526311376, -1.3546361839600276, -1.3515494685907568,
-1.3484628072344835, -1.3453762006002570, -1.3422896493949916, -1.3392031543234377,
-1.3361167160881562, -1.3330303353894903, -1.3299440129255382, -1.3268577493921272,
-1.3237715454827859, -1.3206854018887175, -1.3175993192987738, -1.3145132983994277,
-1.3114273398747476, -1.3083414444063700, -1.3052556126734738, -1.3021698453527548,
-1.2990841431183986, -1.2959985066420545, -1.2929129365928111, -1.2898274336371687,
-1.2867419984390154, -1.2836566316595996, -1.2805713339575073, -1.2774861059886347,
-1.2744009484061627, -1.2713158618605336, -1.2682308469994257, -1.2651459044677273,
-1.2620610349075128, -1.2589762389580188, -1.2558915172556180, -1.2528068704337967,
-1.2497222991231292, -1.2466378039512538, -1.2435533855428500, -1.2404690445196138,
-1.2373847815002328, -1.2343005971003653, -1.2312164919326152, -1.2281324666065077,
-1.2250485217284690, -1.2219646579017995, -1.2188808757266550, -1.2157971758000197,
-1.2127135587156865, -1.2096300250642344, -1.2065465754330034, -1.2034632104060754,
-1.2003799305642500, -1.1972967364850238, -1.1942136287425673, -1.1911306079077050,
-1.1880476745478916, -1.1849648292271922, -1.1818820725062600, -1.1787994049423160,
-1.1757168270891285, -1.1726343394969900, -1.1695519427126981, -1.1664696372795356,
-1.1633874237372486, -1.1603053026220267, -1.1572232744664834, -1.1541413397996356,
-1.1510594991468840, -1.1479777530299930, -1.1448961019670723, -1.1418145464725562,
-1.1387330870571848, -1.1356517242279858, -1.1325704584882540, -1.1294892903375349,
-1.1264082202716028, -1.1233272487824462, -1.1202463763582455, -1.1171656034833588,
-1.1140849306383010, -1.1110043582997270, -1.1079238869404153, -1.1048435170292479,
-1.1017632490311946, -1.0986830834072967, -1.0956030206146483, -1.0925230611063799,
-1.0894432053316420, -1.0863634537355888, -1.0832838067593618, -1.0802042648400725,
-1.0771248284107880, -1.0740454979005143, -1.0709662737341807, -1.0678871563326242,
-1.0648081461125745, -1.0617292434866386, -1.0586504488632850, -1.0555717626468313,
-1.0524931852374262, -1.0494147170310375, -1.0463363584194363, -1.0432581097901843,
-1.0401799715266187, -1.0371019440078382, -1.0340240276086896, -1.0309462226997554,
-1.0278685296473395, -1.0247909488134530, -1.0217134805558040, -1.0186361252277820,
-1.0155588831784472, -1.0124817547525171, -1.0094047402903543, -1.0063278401279556,
-1.0032510545969382, -1.0001743840245287, -0.9970978287335528, -0.9940213890424220,
-0.9909450652651237, -0.9878688577112096, -0.9847927666857850, -0.9817167924894978,
-0.9786409354185291, -0.9755651957645814, -0.9724895738148697, -0.9694140698521104,
-0.9663386841545130, -0.9632634169957689, -0.9601882686450431, -0.9571132393669648,
-0.9540383294216180, -0.9509635390645325, -0.9478888685466756, -0.9448143181144442,
-0.9417398880096545, -0.9386655784695360, -0.9355913897267224, -0.9325173220092438,
-0.9294433755405196, -0.9263695505393507, -0.9232958472199123, -0.9202222657917470,
-0.9171488064597577, -0.9140754694242008, -0.9110022548806807, -0.9079291630201415,
-0.9048561940288629, -0.9017833480884530, -0.8987106253758427, -0.8956380260632806,
-0.8925655503183270, -0.8894931983038491, -0.8864209701780152, -0.8833488660942911,
-0.8802768862014340, -0.8772050306434891, -0.8741332995597845, -0.8710616930849278,
-0.8679902113488013, -0.8649188544765588, -0.8618476225886219, -0.8587765158006768,
-0.8557055342236701, -0.8526346779638075, -0.8495639471225490, -0.8464933417966074,
-0.8434228620779456, -0.8403525080537744, -0.8372822798065497, -0.8342121774139714,
-0.8311422009489811, -0.8280723504797606, -0.8250026260697307, -0.8219330277775502,
-0.8188635556571134, -0.8157942097575512, -0.8127249901232296, -0.8096558967937485,
-0.8065869298039426, -0.8035180891838803, -0.8004493749588645, -0.7973807871494317,
-0.7943123257713546, -0.7912439908356396, -0.7881757823485300, -0.7851077003115063,
-0.7820397447212868, -0.7789719155698294, -0.7759042128443329, -0.7728366365272386,
-0.7697691865962324, -0.7667018630242468, -0.7636346657794624, -0.7605675948253116,
-0.7575006501204794, -0.7544338316189082, -0.7513671392697986, -0.7483005730176139,
-0.7452341328020831, -0.7421678185582038, -0.7391016302162466, -0.7360355677017578,
-0.7329696309355647, -0.7299038198337787, -0.7268381343078000, -0.7237725742643218,
-0.7207071396053357, -0.7176418302281357, -0.7145766460253233, -0.7115115868848132,
-0.7084466526898385, -0.7053818433189556, -0.7023171586460507, -0.6992525985403449,
-0.6961881628664013, -0.6931238514841299, -0.6900596642487952, -0.6869956010110216,
-0.6839316616168009, -0.6808678459074992, -0.6778041537198631, -0.6747405848860284,
-0.6716771392335258, -0.6686138165852896, -0.6655506167596651, -0.6624875395704165,
-0.6594245848267348, -0.6563617523332462, -0.6532990418900204, -0.6502364532925790,
-0.6471739863319046, -0.6441116407944493, -0.6410494164621440, -0.6379873131124075,
-0.6349253305181557, -0.6318634684478110, -0.6288017266653131, -0.6257401049301271,
-0.6226786029972549, -0.6196172206172444, -0.6165559575362010, -0.6134948134957965,
-0.6104337882332812, -0.6073728814814939, -0.6043120929688732, -0.6012514224194678,
-0.5981908695529498, -0.5951304340846230, -0.5920701157254376, -0.5890099141819997,
-0.5859498291565846, -0.5828898603471477, -0.5798300074473378, -0.5767702701465086,
-0.5737106481297314, -0.5706511410778079, -0.5675917486672830, -0.5645324705704574,
-0.5614733064554015, -0.5584142559859673, -0.5553553188218032, -0.5522964946183664,
-0.5492377830269375, -0.5461791836946337, -0.5431206962644228, -0.5400623203751378,
-0.5370040556614907, -0.5339459017540870, -0.5308878582794401, -0.5278299248599867,
-0.5247721011141006, -0.5217143866561084, -0.5186567810963040, -0.5155992840409649,
-0.5125418950923666, -0.5094846138487980, -0.5064274399045782, -0.5033703728500708,
-0.5003134122717017, -0.4972565577519730, -0.4941998088694810, -0.4911431651989314,
-0.4880866263111567, -0.4850301917731316, -0.4819738611479913, -0.4789176339950472,
-0.4758615098698037, -0.4728054883239767, -0.4697495689055093, -0.4666937511585906,
-0.4636380346236717, -0.4605824188374848, -0.4575269033330600, -0.4544714876397434,
-0.4514161712832160, -0.4483609537855100, -0.4453058346650290, -0.4422508134365649,
-0.4391958896113176, -0.4361410626969126, -0.4330863321974207, -0.4300316976133757,
-0.4269771584417951, -0.4239227141761973, -0.4208683643066221, -0.4178141083196501,
-0.4147599456984207, -0.4117058759226540, -0.4086518984686680, -0.4055980128094006,
-0.4025442184144278, -0.3994905147499850, -0.3964369012789863, -0.3933833774610454,
-0.3903299427524949, -0.3872765966064085, -0.3842233384726200, -0.3811701677977445,
-0.3781170840251998, -0.3750640865952259, -0.3720111749449079, -0.3689583485081953,
-0.3659056067159246, -0.3628529489958394, -0.3598003747726132, -0.3567478834678695,
-0.3536954745002046, -0.3506431472852087, -0.3475909012354876, -0.3445387357606854,
-0.3414866502675054, -0.3384346441597333, -0.3353827168382586, -0.3323308677010977,
-0.3292790961434151, -0.3262274015575475, -0.3231757833330247, -0.3201242408565939,
-0.3170727735122413, -0.3140213806812149, -0.3109700617420491, -0.3079188160705851,
-0.3048676430399968, -0.3018165420208114, -0.2987655123809355, -0.2957145534856753,
-0.2926636646977631, -0.2896128453773791, -0.2865620948821750, -0.2835114125672991,
-0.2804607977854179, -0.2774102498867426, -0.2743597682190509, -0.2713093521277122,
-0.2682590009557110, -0.2652087140436722, -0.2621584907298840, -0.2591083303503231,
-0.2560582322386794, -0.2530081957263792, -0.2499582201426115, -0.2469083048143511,
-0.2438584490663844, -0.2408086522213333, -0.2377589135996813, -0.2347092325197966,
-0.2316596082979593, -0.2286100402483842, -0.2255605276832478, -0.2225110699127127,
-0.2194616662449523, -0.2164123159861774, -0.2133630184406603, -0.2103137729107614,
-0.2072645786969535, -0.2042154350978488, -0.2011663414102229, -0.1981172969290423,
-0.1950683009474881, -0.1920193527569836, -0.1889704516472197, -0.1859215969061795,
-0.1828727878201665, -0.1798240236738284, -0.1767753037501854, -0.1737266273306537,
-0.1706779936950747, -0.1676294021217382, -0.1645808518874114, -0.1615323422673629,
-0.1584838725353908, -0.1554354419638487, -0.1523870498236709, -0.1493386953844011,
-0.1462903779142168, -0.1432420966799579, -0.1401938509471512, -0.1371456399800395,
-0.1340974630416059, -0.1310493193936025, -0.1280012082965759, -0.1249531290098951,
-0.1219050807917775, -0.1188570628993160, -0.1158090745885068, -0.1127611151142748,
-0.1097131837305024, -0.1066652796900548, -0.1036174022448089, -0.1005695506456786,
-0.0975217241426437, -0.0944739219847753, -0.0914261434202646, -0.0883783876964495,
-0.0853306540598411, -0.0822829417561525, -0.0792352500303247, -0.0761875781265552,
-0.0731399252883239, -0.0700922907584221, -0.0670446737789784, -0.0639970735914873,
-0.0609494894368364, -0.0579019205553327, -0.0548543661867321, -0.0518068255702648,
-0.0487592979446650, -0.0457117825481961, -0.0426642786186804, -0.0396167853935248,
-0.0365693021097503, -0.0335218280040174, -0.0304743623126556, -0.0274269042716904,
-0.0243794531168700, -0.0213320080836945, -0.0182845684074421, -0.0152371333231980,
-0.0121897020658807, -0.0091422738702712, -0.0060948479710391, -0.0030474236027716,
0.0000000000000000, 0.0030474236027716, 0.0060948479710391, 0.0091422738702712,
0.0121897020658807, 0.0152371333231980, 0.0182845684074421, 0.0213320080836945,
0.0243794531168700, 0.0274269042716904, 0.0304743623126556, 0.0335218280040174,
0.0365693021097503, 0.0396167853935248, 0.0426642786186804, 0.0457117825481961,
0.0487592979446650, 0.0518068255702648, 0.0548543661867321, 0.0579019205553327,
0.0609494894368364, 0.0639970735914873, 0.0670446737789784, 0.0700922907584221,
0.0731399252883239, 0.0761875781265552, 0.0792352500303247, 0.0822829417561525,
0.0853306540598411, 0.0883783876964495, 0.0914261434202646, 0.0944739219847753,
0.0975217241426437, 0.1005695506456786, 0.1036174022448089, 0.1066652796900548,
0.1097131837305024, 0.1127611151142748, 0.1158090745885068, 0.1188570628993160,
0.1219050807917775, 0.1249531290098951, 0.1280012082965759, 0.1310493193936025,
0.1340974630416059, 0.1371456399800395, 0.1401938509471512, 0.1432420966799579,
0.1462903779142168, 0.1493386953844011, 0.1523870498236709, 0.1554354419638487,
0.1584838725353908, 0.1615323422673629, 0.1645808518874114, 0.1676294021217382,
0.1706779936950747, 0.1737266273306537, 0.1767753037501854, 0.1798240236738284,
0.1828727878201665, 0.1859215969061795, 0.1889704516472197, 0.1920193527569836,
0.1950683009474881, 0.1981172969290423, 0.2011663414102229, 0.2042154350978488,
0.2072645786969535, 0.2103137729107614, 0.2133630184406603, 0.2164123159861774,
0.2194616662449523, 0.2225110699127127, 0.2255605276832478, 0.2286100402483842,
0.2316596082979593, 0.2347092325197966, 0.2377589135996813, 0.2408086522213333,
0.2438584490663844, 0.2469083048143511, 0.2499582201426115, 0.2530081957263792,
0.2560582322386794, 0.2591083303503231, 0.2621584907298840, 0.2652087140436722,
0.2682590009557110, 0.2713093521277122, 0.2743597682190509, 0.2774102498867426,
0.2804607977854179, 0.2835114125672991, 0.2865620948821750, 0.2896128453773791,
0.2926636646977631, 0.2957145534856753, 0.2987655123809355, 0.3018165420208114,
0.3048676430399968, 0.3079188160705851, 0.3109700617420491, 0.3140213806812149,
0.3170727735122413, 0.3201242408565939, 0.3231757833330247, 0.3262274015575475,
0.3292790961434151, 0.3323308677010977, 0.3353827168382586, 0.3384346441597333,
0.3414866502675054, 0.3445387357606854, 0.3475909012354876, 0.3506431472852087,
0.3536954745002046, 0.3567478834678695, 0.3598003747726132, 0.3628529489958394,
0.3659056067159246, 0.3689583485081953, 0.3720111749449079, 0.3750640865952259,
0.3781170840251998, 0.3811701677977445, 0.3842233384726200, 0.3872765966064085,
0.3903299427524949, 0.3933833774610454, 0.3964369012789863, 0.3994905147499850,
0.4025442184144278, 0.4055980128094006, 0.4086518984686680, 0.4117058759226540,
0.4147599456984207, 0.4178141083196501, 0.4208683643066221, 0.4239227141761973,
0.4269771584417949, 0.4300316976133759, 0.4330863321974207, 0.4361410626969126,
0.4391958896113174, 0.4422508134365651, 0.4453058346650290, 0.4483609537855100,
0.4514161712832158, 0.4544714876397437, 0.4575269033330600, 0.4605824188374846,
0.4636380346236719, 0.4666937511585906, 0.4697495689055093, 0.4728054883239765,
0.4758615098698039, 0.4789176339950472, 0.4819738611479913, 0.4850301917731313,
0.4880866263111567, 0.4911431651989314, 0.4941998088694807, 0.4972565577519732,
0.5003134122717017, 0.5033703728500708, 0.5064274399045780, 0.5094846138487981,
0.5125418950923666, 0.5155992840409649, 0.5186567810963038, 0.5217143866561084,
0.5247721011141006, 0.5278299248599865, 0.5308878582794403, 0.5339459017540870,
0.5370040556614907, 0.5400623203751377, 0.5431206962644230, 0.5461791836946337,
0.5492377830269375, 0.5522964946183662, 0.5553553188218032, 0.5584142559859673,
0.5614733064554014, 0.5645324705704576, 0.5675917486672830, 0.5706511410778078,
0.5737106481297312, 0.5767702701465087, 0.5798300074473378, 0.5828898603471476,
0.5859498291565843, 0.5890099141819998, 0.5920701157254376, 0.5951304340846229,
0.5981908695529500, 0.6012514224194680, 0.6043120929688731, 0.6073728814814937,
0.6104337882332813, 0.6134948134957965, 0.6165559575362007, 0.6196172206172447,
0.6226786029972549, 0.6257401049301271, 0.6288017266653130, 0.6318634684478113,
0.6349253305181557, 0.6379873131124074, 0.6410494164621438, 0.6441116407944494,
0.6471739863319046, 0.6502364532925788, 0.6532990418900205, 0.6563617523332463,
0.6594245848267348, 0.6624875395704165, 0.6655506167596653, 0.6686138165852896,
0.6716771392335257, 0.6747405848860282, 0.6778041537198634, 0.6808678459074992,
0.6839316616168007, 0.6869956010110218, 0.6900596642487954, 0.6931238514841299,
0.6961881628664012, 0.6992525985403450, 0.7023171586460507, 0.7053818433189555,
0.7084466526898383, 0.7115115868848134, 0.7145766460253233, 0.7176418302281355,
0.7207071396053360, 0.7237725742643220, 0.7268381343078000, 0.7299038198337786,
0.7329696309355649, 0.7360355677017578, 0.7391016302162465, 0.7421678185582037,
0.7452341328020832, 0.7483005730176139, 0.7513671392697985, 0.7544338316189083,
0.7575006501204795, 0.7605675948253116, 0.7636346657794623, 0.7667018630242469,
0.7697691865962324, 0.7728366365272384, 0.7759042128443326, 0.7789719155698295,
0.7820397447212868, 0.7851077003115062, 0.7881757823485303, 0.7912439908356397,
0.7943123257713546, 0.7973807871494316, 0.8004493749588646, 0.8035180891838803,
0.8065869298039425, 0.8096558967937484, 0.8127249901232297, 0.8157942097575512,
0.8188635556571133, 0.8219330277775504, 0.8250026260697310, 0.8280723504797606,
0.8311422009489808, 0.8342121774139715, 0.8372822798065497, 0.8403525080537743,
0.8434228620779455, 0.8464933417966075, 0.8495639471225490, 0.8526346779638074,
0.8557055342236705, 0.8587765158006768, 0.8618476225886219, 0.8649188544765586,
0.8679902113488013, 0.8710616930849278, 0.8741332995597844, 0.8772050306434889,
0.8802768862014341, 0.8833488660942911, 0.8864209701780152, 0.8894931983038492,
0.8925655503183271, 0.8956380260632806, 0.8987106253758426, 0.9017833480884531,
0.9048561940288629, 0.9079291630201414, 0.9110022548806804, 0.9140754694242010,
0.9171488064597577, 0.9202222657917469, 0.9232958472199125, 0.9263695505393509,
0.9294433755405196, 0.9325173220092435, 0.9355913897267225, 0.9386655784695360,
0.9417398880096545, 0.9448143181144439, 0.9478888685466759, 0.9509635390645325,
0.9540383294216178, 0.9571132393669650, 0.9601882686450431, 0.9632634169957687,
0.9663386841545127, 0.9694140698521105, 0.9724895738148697, 0.9755651957645813,
0.9786409354185294, 0.9817167924894980, 0.9847927666857850, 0.9878688577112096,
0.9909450652651239, 0.9940213890424222, 0.9970978287335528, 1.0001743840245285,
1.0032510545969382, 1.0063278401279556, 1.0094047402903543, 1.0124817547525173,
1.0155588831784472, 1.0186361252277820, 1.0217134805558040, 1.0247909488134532,
1.0278685296473395, 1.0309462226997554, 1.0340240276086894, 1.0371019440078382,
1.0401799715266187, 1.0432581097901843, 1.0463363584194365, 1.0494147170310375,
1.0524931852374262, 1.0555717626468313, 1.0586504488632853, 1.0617292434866386,
1.0648081461125745, 1.0678871563326240, 1.0709662737341807, 1.0740454979005143,
1.0771248284107880, 1.0802042648400727, 1.0832838067593618, 1.0863634537355888,
1.0894432053316419, 1.0925230611063800, 1.0956030206146483, 1.0986830834072967,
1.1017632490311944, 1.1048435170292479, 1.1079238869404153, 1.1110043582997270,
1.1140849306383012, 1.1171656034833588, 1.1202463763582455, 1.1233272487824457,
1.1264082202716030, 1.1294892903375349, 1.1325704584882540, 1.1356517242279855,
1.1387330870571848, 1.1418145464725562, 1.1448961019670723, 1.1479777530299933,
1.1510594991468840, 1.1541413397996356, 1.1572232744664834, 1.1603053026220270,
1.1633874237372486, 1.1664696372795356, 1.1695519427126980, 1.1726343394969900,
1.1757168270891285, 1.1787994049423160, 1.1818820725062600, 1.1849648292271922,
1.1880476745478916, 1.1911306079077049, 1.1942136287425678, 1.1972967364850238,
1.2003799305642500, 1.2034632104060752, 1.2065465754330034, 1.2096300250642344,
1.2127135587156865, 1.2157971758000200, 1.2188808757266550, 1.2219646579017995,
1.2250485217284688, 1.2281324666065080, 1.2312164919326152, 1.2343005971003653,
1.2373847815002326, 1.2404690445196138, 1.2435533855428500, 1.2466378039512538,
1.2497222991231294, 1.2528068704337967, 1.2558915172556180, 1.2589762389580186,
1.2620610349075130, 1.2651459044677273, 1.2682308469994257, 1.2713158618605336,
1.2744009484061627, 1.2774861059886347, 1.2805713339575073, 1.2836566316595999,
1.2867419984390154, 1.2898274336371687, 1.2929129365928110, 1.2959985066420547,
1.2990841431183986, 1.3021698453527548, 1.3052556126734736, 1.3083414444063700,
1.3114273398747476, 1.3145132983994277, 1.3175993192987740, 1.3206854018887175,
1.3237715454827859, 1.3268577493921270, 1.3299440129255384, 1.3330303353894903,
1.3361167160881562, 1.3392031543234375, 1.3422896493949916, 1.3453762006002570,
1.3484628072344833, 1.3515494685907570, 1.3546361839600276, 1.3577229526311376,
1.3608097738908491, 1.3638966470238705, 1.3669835713128842, 1.3700705460385765,
1.3731575704796635, 1.3762446439129201, 1.3793317656132063, 1.3824189348534979,
1.3855061509049140, 1.3885934130367428, 1.3916807205164740, 1.3947680726098244,
1.3978554685807680, 1.4009429076915625, 1.4040303892027810, 1.4071179123733386,
1.4102054764605212, 1.4132930807200161, 1.4163807244059394, 1.4194684067708665,
1.4225561270658580, 1.4256438845404933, 1.4287316784428970, 1.4318195080197695,
1.4349073725164143, 1.4379952711767703, 1.4410832032434397, 1.4441711679577170,
1.4472591645596196, 1.4503471922879179, 1.4534352503801644, 1.4565233380727216,
1.4596114546007952, 1.4626995991984624, 1.4657877710987015, 1.4688759695334213,
1.4719641937334929, 1.4750524429287790, 1.4781407163481628, 1.4812290132195800,
1.4843173327700480, 1.4874056742256965, 1.4904940368117963, 1.4935824197527918,
1.4966708222723308, 1.4997592435932940, 1.5028476829378250, 1.5059361395273627,
1.5090246125826707, 1.5121131013238660, 1.5152016049704535, 1.5182901227413530,
1.5213786538549319, 1.5244671975290338, 1.5275557529810110, 1.5306443194277553,
1.5337328960857264, 1.5368214821709845, 1.5399100768992207, 1.5429986794857880,
1.5460872891457300, 1.5491759050938145, 1.5522645265445632, 1.5553531527122815,
1.5584417828110895, 1.5615304160549546, 1.5646190516577200, 1.5677076888331380,
1.5707963267948966, 1.5707963267948966 };

// the two arrays above have 1027 elements each.  The latitude spacing of the points
// is GEOTESS_DLAT = PI/1024 radians.  The first value is at -PI/2-GEOTESS_DLAT and
// the last value is at PI/2+GEOTESS_DLAT.  The extra elements on the ends account
// for the possibility of small roundoff errors.

static const double GEOTESS_DLAT = PI/1024.;

static const double GEOTESS_LAT0 = -PI/2. - GEOTESS_DLAT;

// GEOTESS_E is ( 1 - eccentricity squared) for the WGS84 ellipsoid.
static const double GEOTESS_E = 0.99330562000985870000;

inline double GeoTessUtils::getGeographicLat(const double& lat)
{
	// convert lat from geocentric to geographic latitude.
	if (approximateLatitudes)
	{
		double tmp = (lat - GEOTESS_LAT0) / GEOTESS_DLAT;
		int i = (int) tmp;
		return geographic[i] + (tmp - i) * (geographic[i + 1] - geographic[i]);
	}
	return atan(tan(lat) / GEOTESS_E);
}

inline double GeoTessUtils::getGeocentricLat(const double& lat)
{
	// convert lat from geographic to geocentric latitude.
	if (approximateLatitudes)
	{
		double tmp = (lat - GEOTESS_LAT0) / GEOTESS_DLAT;
		int i = (int) tmp;
		return geocentric[i] + (tmp - i) * (geocentric[i + 1] - geocentric[i]);
	}
	return atan(tan(lat) * GEOTESS_E);
}

} // end namespace geotess

#endif  // GEOTESSUTILS_OBJECT_H

