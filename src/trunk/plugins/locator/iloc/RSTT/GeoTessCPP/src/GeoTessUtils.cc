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

#include "GeoTessUtils.h"
#include "GeoTessException.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

/**
 * If approximateLatitudes is true, then an approximate algorithm will be used to
 * convert back and forth between geocentric and geographic latitudes.
 * The approximation incurs an error of about 0.1 meters in latitude
 * calculations but is faster than the correct calculation.
 * <p>Note that the existence of this static, mutable variable technically
 * violates thread-safety of this class. But given the assumption that
 * the approximation is just as good as the true conversions, this is
 * not a significant violation.
 */
bool 			GeoTessUtils::approximateLatitudes = false;


// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

/**
 * @param vector
 * @return a String of lat,lon in degrees formatted with "%10.6f %11.6f"
 */
string GeoTessUtils::getLatLonString(const double* const v)
{
  char s[300];
  string frmt("%9.5f %10.5f");
  sprintf(s, frmt.c_str(), getLatDegrees(v), getLonDegrees(v));
  return s;
}

/**
 * @param vector
 * @return a String of lat,lon in degrees formatted with "%10.6f %11.6f"
 */
string GeoTessUtils::getLonLatString(const double* const v)
{
  char s[300];
  string frmt("%11.6f %10.6f");
  sprintf(s, frmt.c_str(), getLonDegrees(v), getLatDegrees(v));
  return s;
}

double GeoTessUtils::azimuth(const double* const v1, const double* const v2, double errorValue)
{
  double* temp = new double [6];
  double* temp0 = &temp[0];
  double* temp1 = &temp[3];

  // set temp[0] = the cross product of this x other.
  if (crossNormal(v1, v2, temp0) > 0.)
  // if the cross product has zero length then the two vectors are
  // coincident
  // (do nothing in that case; returns errorValue).
  {
    // set temp[1] = this x north pole
    // if the cross product has zero length then this == north_pole
    // or south pole and azimuth is indeterminant.
    if (crossNorth(v1, temp1) > 0.)
    {
      // set azimuth to the angle between (this x north pole) and
      // (this x other).
      errorValue = angle(temp1, temp0);
      // if the dot product of (this x other) . northPole < 0
      if (temp0[2] < 0.) errorValue = -errorValue;
    }
  }

  delete [] temp;
  return errorValue;
}

double GeoTessUtils::azimuthDegrees(const double* const v1, const double* const v2, double errorValue)
{
  double* temp = new double [6];
  double* temp0 = &temp[0];
  double* temp1 = &temp[3];

  if (crossNormal(v1, v2, temp0) > 0.)
  // if the cross product has zero length then the two vectors are
  // coincident
  // (do nothing in that case; returns NaN).
  {
    // set temp[1] = this x north pole
    // if the cross product has zero length then this == north_pole
    // or south pole and azimuth is indeterminant.
    if (crossNorth(v1, temp1) > 0.)
    {
      // set azimuth to the angle between (this x north pole) and
      // (this x other).
      errorValue = angleDegrees(temp1, temp0);
      // if the dot product of (this x other) . northPole < 0
      if (temp0[2] < 0.) errorValue = -errorValue;
    }
  }

  delete [] temp;
  return errorValue;
}

void GeoTessUtils::rotate(const double* const x, const double* const p,
													double a, double* const z)
{
  if (abs(a) < 1e-15)
  {
    z[0] = x[0];
    z[1] = x[1];
    z[2] = x[2];
    return;
  }

  double d = x[0] * p[0] + x[1] * p[1] + x[2] * p[2]; // dot product
  // if x and p are parallel, x needs no rotation.
  if (abs(d) > 1. - 1e-15)
  {
    z[0] = x[0];
    z[1] = x[1];
    z[2] = x[2];
    return;
  }

  double cosa = cos(a);
  double sina = sin(a);
  d *= (1 - cosa);
  double z0 = cosa * x[0] + d * p[0] - sina * (p[1] * x[2] - p[2] * x[1]);
  double z1 = cosa * x[1] + d * p[1] - sina * (p[2] * x[0] - p[0] * x[2]);
  double z2 = cosa * x[2] + d * p[2] - sina * (p[0] * x[1] - p[1] * x[0]);
  double len = sqrt(z0 * z0 + z1 * z1 + z2 * z2);
  z[0] = z0 / len;
  z[1] = z1 / len;
  z[2] = z2 / len;
}

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
 * @param gc  The 2x3 array containing the 2 great circle unit vectors.
 * 						The first one is a clone of unit vector v0 passed as first
 * 						argument to this method. The second is located 90 degrees
 * 						away from v0.
 * @throws GeoTessException
 *             if v0 and v1 are parallel.
 */
void GeoTessUtils::getGreatCircle(const double* const v0,
																	const double* const v1,
																	double** const gc)
{
  if (parallel(v0, v1))
		throw GeoTessException("Cannot create a GreatCicle with two vectors that are parallel.",
													 __FILE__, __LINE__, 7001);

  gc[0][0] = v0[0];
  gc[0][1] = v0[1];
  gc[0][2] = v0[2];
  vectorTripleProduct(v0, v1, v0, gc[1]);
}

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
double** GeoTessUtils::getGreatCircle(const double* const v0,const double* const v1)
{
  if (parallel(v0, v1))
		throw GeoTessException("Cannot create a GreatCicle with two vectors that are parallel.",
													 __FILE__, __LINE__, 7002);

  double** greatCircle = new double* [2];
  greatCircle[0] = new double [6];
  greatCircle[1] = &greatCircle[0][3];

  greatCircle[0][0] = v0[0];
  greatCircle[0][1] = v0[1];
  greatCircle[0][2] = v0[2];
  vectorTripleProduct(v0, v1, v0, greatCircle[1]);
  return greatCircle;
}

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
 * <p>Caller assumes ownership of the resulting double** array 
 * and should delete it when it is no longer needed.
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
double** GeoTessUtils::getGreatCircle(const double* const v, double azimuth)
{
  if (isPole(v))
		throw GeoTessException("Cannot create a GreatCicle with north/south pole and an azimuth.",
													 __FILE__, __LINE__, 7003);

  double** greatCircle = new double* [2];
  greatCircle[0] = new double [6];
  greatCircle[1] = &greatCircle[0][3];

  greatCircle[0][0] = v[0];
  greatCircle[0][1] = v[1];
  greatCircle[0][2] = v[2];
  moveNorth(v, PI * 0.5, greatCircle[1]);
  rotate(greatCircle[1], v, -azimuth, greatCircle[1]);
  return greatCircle;
}

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
 * @param gc a 2 x 3 array specifying two unit vectors. The first one is a
 *         clone of unit vector v passed as an argument to this method. The
 *         second is located 90 degrees away from v in the direction
 *         specified by azimuth.
 * @throws GeoTessException
 *             if v is located at north or south pole.
 */
void GeoTessUtils::getGreatCircle(const double* const v, double azimuth,
		double** const gc)
{
  if (isPole(v))
		throw GeoTessException("Cannot create a GreatCicle with north/south pole and an azimuth.",
													 __FILE__, __LINE__, 7004);

  gc[0][0] = v[0];
  gc[0][1] = v[1];
  gc[0][2] = v[2];
  moveNorth(v, PI * 0.5, gc[1]);
  rotate(gc[1], v, -azimuth, gc[1]);
}

int GeoTessUtils::getGreatCirclePoints(double* ptA, double* ptB, const double& delta, const bool& onCenters)
{
	if (delta <= 0.)
		throw GeoTessException("ERROR in GeoTessUtils::getGreatCirclePoints(). delta <= 0.", __FILE__, __LINE__, 7007);

	// find distance from a to b, in radians
	double distance = angle(ptA, ptB);

	if (distance == 0.) return onCenters ? 1 : 2;

	// at this point both distance and delta are > 0.
	// find number of intervals (minimum of 1)
	return onCenters ? (int)ceil(distance/delta) : ((int)ceil(distance/delta))+1;

}

double GeoTessUtils::getGreatCirclePoints(double* ptA, double* ptB, const double& delta, const bool& onCenters,
		double** points, int& npoints)
{
	if (delta <= 0.)
		throw GeoTessException("ERROR in GeoTessUtils::getGreatCirclePoints(). delta <= 0.", __FILE__, __LINE__, 7007);

	// find distance from a to b, in radians
	double distance = angle(ptA, ptB);

	if (distance == 0.)
	{
		points[0][0] = ptA[0];
		points[0][1] = ptA[1];
		points[0][2] = ptA[2];
		npoints = 1;
		if (!onCenters)
		{
			points[1][0] = ptB[0];
			points[1][1] = ptB[1];
			points[1][2] = ptB[2];
			npoints = 2;
		}
		return 0.;
	}

	// at this point both distance and delta are > 0.
	// find number of intervals (minimum of 1)
	npoints = (int)ceil(distance/delta);

	// dx0 is distance from ptA to first point in points array.
	double dx0 = 0.;

	// dx is the actual spacing between points.
	double dx = distance/npoints;

	if (onCenters)
		// npoints equals number of intervals and distance to
		// first point is half the point spacing
		dx0 = dx/2;
	else
		// npoint is number of intervals + 1, and distance to
		// first point is zero.
		++npoints;

	double** gc = CPPUtils::new2DArray<double>(2,3);
	getGreatCircle(ptA, ptB, gc);

	for (int i=0; i<npoints; ++i)
		getGreatCirclePoint(gc, dx0+i*dx, points[i]);

	CPPUtils::delete2DArray(gc);

	return dx;
}

double GeoTessUtils::getGreatCirclePoints(double* ptA, double* ptB, const int& npoints,
		const bool& onCenters, double** points)
{
	if (npoints <= 0) return 0.;

	double distance = angle(ptA, ptB);

	if (distance <= 0.)
	{
		for (int i=0; i<npoints; ++i)
		{
			points[i][0] = ptA[0];
			points[i][1] = ptA[1];
			points[i][2] = ptA[2];
		}
		return 0.;
	}

	double dx = onCenters ? angle(ptA, ptB) / npoints
			: npoints == 1 ? 0. : angle(ptA, ptB)/(npoints-1);

	double dx0 = onCenters ? dx/2 : 0.;

	double** gc = CPPUtils::new2DArray<double>(2,3);
	getGreatCircle(ptA, ptB, gc);

	for (int i=0; i<npoints; ++i)
		getGreatCirclePoint(gc, dx0+i*dx, points[i]);

	CPPUtils::delete2DArray(gc);

	return dx;
}

/**
 * Transform is a 3 x 3 matrix such that when a vector is multiplied by
 * transform, the vector will be projected onto the plane of this
 * GreatCircle. The z direction will point out of the plane of the great
 * circle in the direction of the observer (lastPoint cross firstPoint;
 * parallel to normal). The y direction will correspond to the mean of
 * firstPoint and lastPoint. The x direction will correspond to y cross z,
 * forming a right handed coordinate system.
 *
 * @throws GeoTessException
 */
void GeoTessUtils::getTransform(const double* const u, const double* const v,
																double** const t)
{
	// t[0] will be x direction -- observer's right
	// t[1] will be y direction -- observer's up
	// t[2] will be z direction -- points toward the observer

	// set t[2] equal to unit vector normal to plan containing u and v
	if (crossNormal(v, u, t[2]) == 0.)
		throw GeoTessException("u and v are parallel: |v x u| == 0",
													 __FILE__, __LINE__, 7005);

	// set t[1] to mean of vectors u and v, normalized to unit length.
	t[1][0] = u[0] + v[0];
	t[1][1] = u[1] + v[1];
	t[1][2] = u[2] + v[2];
	if (normalize(t[1]) == 0.)
		throw GeoTessException("u and v are anti-parallel",
													 __FILE__, __LINE__, 7006);

	// set t[0] equal to t[1] cross t[2]
	crossNormal(t[1], t[2], t[0]);
}

} // end namespace geotess
