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

#ifndef GREATCIRCLE_H_
#define GREATCIRCLE_H_

// **** _SYSTEM INCLUDES_ ******************************************************

//#include <cstdio>
#include <iostream>
#include <string>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"
#include "GeoTessUtils.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Manages information about a great circle path that extends from one
 * point to another point, both or which are located on the surface of a unit
 * sphere.
 *
 * The GreatCircle class manages the information about a great circle path that
 * extends from one point to another point, both or which are located on the
 * surface of a unit sphere. It supports great circles where the distance from
 * the firstPoint to the lastPoint are 0 to 2*PI radians apart, inclusive.
 * Either or both of the points may coincide with one of the poles of the Earth.
 *
 * There is a method to retrieve a point that is located on the great circle at
 * some specified distance from the first point of the great circle.
 *
 * The method getIntersection(other, inRange) will return a point that is
 * located at the intersection of two great circles. In general, two great
 * circles intersect at two points, and this method returns the one that is
 * encountered first as one moves away from the first point of the first
 * GreatCircle. If the Boolean argument <i>inRange</i> is true, then the method
 * will only return a point if the point falls within the range of both great
 * circles. In other words, the point of intersection has to reside in between
 * the first and last point of both great circles. If <i>inRange</i> is false,
 * then that constraint is not applied.
 *
 * GreatCircle has the ability to transform the coordinates of an input point so
 * that it resides in the plane of the great circle. This is useful for
 * extracting slices from a 3D model for plotting purposes. The z-coordinate of
 * the transformed point will point out of the plane of the great circle toward
 * the observer. The y-coordinate of the transformed point will be equal to the
 * normalized vector sum of the first and last point of the great circle and the
 * x-coordinate will be y cross z.
 *
 * The key to successfully defining a great circle path is successfully
 * determining the unit vector that is normal to the plane of the great circle
 * (firstPoint cross lastPoint, normalized to unit length). For great circles
 * where the distance from firstPoint to lastPoint is more than zero and less
 * than PI radians, this is straightforward. But for great circles longer than
 * PI radians, great circles of exactly zero, PI or 2*PI radians length, or
 * great circles where the first point resides on one of the poles,
 * complications arise.
 *
 * To determine the normal to the great circle, three constructors are provided
 * (besides the default constructor that does nothing).
 *
 * The first constructor is the most general. It takes four arguments:
 * firstPoint (unit vector), intermediatePoint (unit vector), lastPoint (unit
 * vector) and shortestPath (boolean). The normal is computed as firstPoint
 * cross lastPoint normalized to unit length. If the distance from firstPoint to
 * lastPoint is greater than zero and less than PI radians, then the resulting
 * normal will have finite length and will have been successfully computed. If,
 * however, the distance from firstPoint to lastPoint is exactly 0 or PI
 * radians, then normal will have zero length. In this case, a second attempt to
 * compute the normal is executed by computing firstPoint cross
 * intermediatePoint. If this is successful, the calculation proceeds. If not
 * successful, then the normal is computed as the first of: firstPoint cross Z,
 * firstPoint cross Y or firstPoint cross X, whichever produces a finite length
 * normal first. Z is the north pole, Y is (0N, 90E) and X is (0N, 0E). One of
 * these calculations is guaranteed to produce a valid normal.
 *
 * <p>Once the normal
 * has been computed, then the shortestPath argument is considered. If
 * shortestPath is true, then no further action is taken, resulting in a great
 * circle with length less than or equal to PI radians. If shortestPath is false
 * then the normal is negated, effectively forcing the great circle to go the
 * long way around the globe to get from firstPoint to lastPoint. When
 * shortestPath is false the length of the great circle will be >= PI and <=
 * 2*PI. For example, when shortestPath is true, a great circle path from (10N,
 * 0E) to (30N, 0E) will proceed in a northerly direction for a distance of 20
 * degrees to get from firstPoint to lastPoint. But if shortestPath is false,
 * the great circle will proceed in a southerly direction for 340 degrees to get
 * from firstPoint to lastPoint.
 *
 * The second constructor is a simplification of the first, taking only 3
 * arguments: firstPoint (unit vector), lastPoint (unit vector) and shortestPath
 * (boolean). It calls the first constructor with intermediatePoint set to NULL.
 * This is useful in cases where the calling application is certain that great
 * circles of length exactly 0 or PI radians will not happen or is willing to
 * accept an arbitrary path if it does happens.
 *
 * There is a third constructor that takes 3 arguments: firstPoint (unit
 * vector), distance (radians) and azimuth (radians). The lastPoint of the great
 * circle is computed by moving the first point the specified distance in the
 * specified direction. This constructor can produce great circles where the
 * distance from firstPoint to lastPoint is >= 0 and <= 2*PI, inclusive. It can
 * fail, however, if firstPoint coincides with either of the poles because the
 * notion of azimuth from a pole in undetermined.
 *
 * Memory management: The three parameterized constructors described above make
 * copies of all of the unit vectors that are passed to them and delete the
 * memory allocated for the copies when they are no longer needed. The calling
 * application retains ownership of the variables passed to the constructors.
 * There is an alternative strategy implemented by using the default GreatCircle
 * constructor and then calling one of the 3 set() methods. The arguments of
 * the set methods are the same as for the constructors, with one addition, and
 * the calculation of the great circle parameters is identical. The difference
 * is that the input unit vectors are not copied into internal variables but
 * rather the input pointers are used directly by the GreatCircle object.
 * Modification of these variables by the caller will have undesirable
 * consequences for the GreatCircle object. The extra argument passed to the
 * set() methods is deleteWhenDone (boolean). If true, then GreatCircle will
 * delete the memory allocated for the unit vectors when it is done with them,
 * otherwise it will not delete the memory and it is the responsibility of the
 * caller to delete the memory after the lifetime of the GreatCircle object has
 * expired.
 *
 * @author sballar
 */
class GEOTESS_EXP_IMP GeoTessGreatCircle
{
  private:

	/**
	 * The angular separation of the first and last point in radians.
	 * Measured in direction from first point to last point, which
	 * may be > PI radians.
	 * Lazy evaluation is used for this variable.
	 */
	double distance;

	/**
	 * Reference to the first unit vector on the great circle
	 */
	double* firstPoint;

	/**
	 * Reference to the last unit vector on the great circle
	 */
	double* lastPoint;

	/**
	 * The unit vector normal to the plane of this GreatCircle. Equals firstPoint
	 * cross lastPoint.
	 */
	double normal[3];

	/**
	 * The vector triple product of (firstPoint cross lastPoint) cross
	 * firstPoint, normalized to unit length.  FirstPoint, moveDirection
	 * and normal define a right-handed orthogonal coordinate system.
	 */
	double moveDirection[3];

	/**
	 * If true then memory allocated to firstPoint is deleted in the destructor.
	 */
	bool deleteFirst;

	/**
	 * If true then memory allocated to lastPoint is deleted in the destructor.
	 */
	bool deleteLast;

	/**
	 * trnsfrm is a 3 x 3 matrix such that when a vector is multiplied by
	 * trnsfrm, the vector will be projected onto the plane of this
	 * GreatCircle. The z direction will point out of the plane of the great
	 * circle in the direction of the observer (lastPoint cross firstPoint;
	 * anti-parallel to normal). The y direction will correspond to the mean of
	 * firstPoint and lastPoint. The x direction will correspond to y cross z,
	 * forming a right handed coordinate system.
	 */
	double** trnsfrm;

	void initialize(const double* intermediatePoint, const bool &shortestPath);

	void clear();

  public:

	virtual ~GeoTessGreatCircle();

	/**
	 * Default constructor initializes everything to NULL.
	 */
	GeoTessGreatCircle() : firstPoint(NULL), lastPoint(NULL), deleteFirst(true), deleteLast(true), trnsfrm(NULL)
	{}

	/**
	 * Constructor creates a great circle from firstPoint to another point located
	 * the specified distance and direction from the the first point.
	 * <p>
	 * GreatCircle makes a copy of first point for internal use.  The copy is
	 * deleted in the destructor.
	 *
	 * @param firstPoint unit vector.
	 * @param distance epicentral angular distance to lastPoint, in radians
	 * @param direction double direction to lastPoint, in radians
	 * @throws Exception if firstPoint is on one of the poles.
	 */
	GeoTessGreatCircle(const double* firstPoint, const double& distance, const double& direction);

	/**
	 * Constructor that takes three unit vectors at the beginning, middle and end
	 * of the great circle path. Will not fail even when building GreatCircles
	 * that are 0, PI or 2PI radians long.
	 *
	 * <p>The key is to successfully compute a valid unit vector that
	 * is normal to the plane of the GreatCircle even when
	 * the firstPoint and lastPoint are 0 or PI radians apart.
	 * The following calculations are performed until normal is successfully
	 * computed, i.e., it has unit length.
	 * <ul>
	 * <li>normal = firstPoint X lastPoint, normalized to unit length
	 * <li>intermediatePoint != null and normal = firstPoint X intermediatePoint,
	 * normalized to unit length.
	 * <li>normal = firstPoint X [0., 0., 1.]
	 * <li>normal = firstPoint X [0., 1., 0.]
	 * <li>normal = firstPoint X [1., 0., 0.]
	 * </ul>
	 * GreatCircle makes copies of the 3 unit vectors for internal use.  The copies are
	 * deleted when they are no longer needed.
	 *
	 * @param firstPoint
	 *            unit vector of the origin of the great circle path.
	 * @param intermediatePoint
	 *            unit vector of an intermediate point on the great circle path.
	 *            If null, code will try the three cardinal directions (x, y, z).
	 * @param lastPoint
	 *            unit vector the end of the great circle path.
	 * @param shortestPath if false, normal will be negated and distance from first to last will be
	 * >= PI.
	 */
	GeoTessGreatCircle(const double* firstPoint, const double* intermediatePoint, const double* lastPoint,
			const bool &shortestPath=true);

	/**
	 * Constructor that takes just the two GeoVectors at the beginning and end
	 * of the great circle path. GreatCircle stores references to these arrays;
	 * no copies are made.
	 *
	 * <p>If firstPoint and lastPoint are 0 or PI radians apart, then the
	 * GreatCirlce will pass through one of the following lat,lon pairs:
	 * (90N, 0E), (0N, 90E), or (0N, 0E).
	 *
	 * @param firstPoint
	 *            unit vector the origin of the great circle path.
	 * @param lastPoint
	 *            unit vector the end of the great circle path.
	 * @param shortestPath if false, direction from first to last is reversed and
	 * the distance from first to last will be greater than 180 degrees.
	 */
	GeoTessGreatCircle(const double* firstPoint, const double* lastPoint, const bool &shortestPath=true);

	/**
	 * Copy constructor.
	 * @param other copy contents of other into this.
	 */
	GeoTessGreatCircle(GeoTessGreatCircle& other);

	/**
	 * Equal operator.
	 * @param other copy contents of other into this
	 */
	void operator= (GeoTessGreatCircle& other);

	/**
	 * Return true if this and other are equal.
	 * @param other
	 */
	bool operator == (const GeoTessGreatCircle& other) const
		{
		return firstPoint[0] == other.firstPoint[0]
		&& firstPoint[1] == other.firstPoint[1]
		&& firstPoint[2] == other.firstPoint[2]
		&& lastPoint[0] == other.lastPoint[0]
		&& lastPoint[1] == other.lastPoint[1]
		&& lastPoint[2] == other.lastPoint[2]
		&& normal[0] == other.normal[0]
		&& normal[1] == other.normal[1]
		&& normal[2] == other.normal[2];
		};


	/**
	 * Set the components of this GreatCircle to specified values.
	 *
	 * <p>The key is to successfully compute a valid unit vector that
	 * is normal to the plane of the GreatCircle even when
	 * the firstPoint and lastPoint are 0 or PI radians apart.
	 * The following calculations are performed until normal is successfully
	 * computed, i.e., it has unit length.
	 * <ul>
	 * <li>normal = firstPoint X lastPoint, normalized to unit length
	 * <li>intermediatePoint != null and normal = firstPoint X intermediatePoint,
	 * normalized to unit length.
	 * <li>normal = firstPoint X [0., 0., 1.]
	 * <li>normal = firstPoint X [0., 1., 0.]
	 * <li>normal = firstPoint X [1., 0., 0.]
	 * </ul>
	 *
	 * Memory: This method stores references to firstPoint, intermediatePoint and
	 * lastPoint, not copies.  If deleteWhenDone is true, then GreatCircle will
	 * delete these references when it is done with them, otherwise it will not.
	 * The default is that GreatCircle will not delete these variables.
	 *
	 * @param firstPoint
	 *            unit vector of the origin of the great circle path.
	 * @param intermediatePoint
	 *            unit vector of an intermediate point on the great circle path.
	 *            If null, code will try the three cardinal directions (x, y, z).
	 * @param lastPoint
	 *            unit vector the end of the great circle path.
	 * @param shortestPath if false, normal will be negated and distance from first to last will be
	 * >= PI. Defaults to true.
	 * @param deleteWhenDone if true, references to firstPoint, intermediatePoint and lastPoint
	 * will be deleted when no longer needed, otherwise not deleted.  Default is false.
	 */
	void set(double* firstPoint, double* intermediatePoint, double* lastPoint,
			const bool& shortestPath=true, const bool& deleteWhenDone=false);

	/**
	 * Set the components of this GreatCircle to specified values.
	 *
	 * <p>If firstPoint and lastPoint are 0 or PI radians apart, then the
	 * GreatCirlce will pass through one of the following lat,lon pairs:
	 * (90N, 0E), (0N, 90E), or (0N, 0E).
	 *
	 * Memory: This method stores references to firstPoint and
	 * lastPoint, not copies.  If deleteWhenDone is true, then GreatCircle will
	 * delete these references when it is done with them, otherwise it will not.
	 * The default is that GreatCircle will not delete these variables.
	 *
	 * @param frstPoint
	 *            unit vector of the origin of the great circle path.
	 * @param lstPoint
	 *            unit vector the end of the great circle path.
	 * @param shortestPath if false, normal will be negated and distance from first to last will be
	 * >= PI. Defaults to true.
	 * @param deleteWhenDone if true, references to firstPoint, intermediatePoint and lastPoint
	 * will be deleted when no longer needed, otherwise not deleted.  Default is false.
	 */
	void set(double* frstPoint, double* lstPoint,
			const bool& shortestPath=true, const bool& deleteWhenDone=false)
	{ set (frstPoint, NULL, lstPoint, shortestPath, deleteWhenDone); }

	/**
	 * Set the components of this GreatCircle to specified values.
	 *
	 * Memory: This method stores a reference to firstPoint, not a copy.
	 * If deleteWhenDone is true, then GreatCircle will
	 * delete this reference when it is done with it, otherwise it will not.
	 * The default is that GreatCircle will not delete the reference.
	 *
	 * @param firstPoint unit vector.
	 * @param distance epicentral angular distance to lastPoint, in radians
	 * @param azimuth double direction to lastPoint, in radians
	 * @param deleteWhenDone if true, references to firstPoint, intermediatePoint and lastPoint
	 * will be deleted when no longer needed, otherwise not deleted.  Default is false.
	 * @throws Exception if firstPoint is on one of the poles.
	 */
	void set(double* firstPoint, const double& distance, const double& azimuth,
			const bool& deleteWhenDone=false);

	/**
	 * Retrieve the angular distance from firstPoint to lastPoint, in radians.
	 *
	 * @return double
	 */
	double getDistance()
	{
		if (distance < 0.)
		{
			distance = GeoTessUtils::angle(firstPoint, lastPoint);
			if (GeoTessUtils::scalarTripleProduct(firstPoint, lastPoint, normal) < 0.)
				distance = 2*PI - distance;
		}
		return distance;
	}

	/**
	 * Retrieve the angular distance from firstPoint to lastPoint, in degrees.
	 *
	 * @return double
	 */
	double getDistanceDegrees()
	{
		return CPPUtils::toDegrees(getDistance());
	}

	/**
	 * Retrieve the distance in radians measured from firstPoint to specified
	 * unit vector, measured in direction from firstPoint to lastPoint.
	 * Range is zero to 2*PI
	 *
	 * @param position an earth-centered unit vector
	 * @return double
	 * @throws GreatCircleException
	 */
	double getDistance(const double *position)
	{
		// find the shortest distance from firstPoint to unit vector
		double d = GeoTessUtils::angle(firstPoint, position);

		if (GeoTessUtils::scalarTripleProduct(firstPoint, position, normal) < 0.)
			d = 2 * PI - d;

		return d;
	}

	/**
	 * Retrieve the distance in degrees measured from firstPoint to specified
	 * unit vector, measured in direction from firstPoint to lastPoint.
	 * Range is zero to 360.
	 *
	 * @param position an earth-centered unit vector
	 * @return  the distance in degrees measured from firstPoint to specified
	 * unit vector, measured in direction from firstPoint to lastPoint
	 * @throws GreatCircleException
	 */
	double getDistanceDegrees(const double *position)
	{
		return CPPUtils::toDegrees(getDistance(position));
	}

	/**
	 * Retrieve a reference to the first unit vector on this GreatCircle.
	 *
	 * <p>Caller should not delete this array
	 *
	 * @return unit vector
	 */
	double* getFirst()
	{
		return firstPoint;
	}

	/**
	 * Retrieve a reference to the last unit vector on this GreatCircle.
	 *
	 * <p>Caller should not delete this array
	 *
	 * @return unit vector
	 */
	double* getLast()
	{
		return lastPoint;
	}

	/**
	 * Retrieve a reference to the unit vector that is normal to the plane of this great circle
	 * (firstPoint cross lastPoint normalized to unit length). If firstPoint on
	 * left and lastPoint on right, normal points away from the observer.
	 *
	 * <p>Caller should not delete this array
	 *
	 * @return double[]
	 */
	const double* getNormal()
	{
		return normal;
	}

	/**
	 * Retrieve a unit vector object located on the great circle path a specified
	 * distance from firstPoint.
	 *
	 * <p>It is the caller's responsibility to delete the returned array.
	 *
	 * @param dist
	 *            double the angular distance from firstPoint, in radians.
	 * @return unit vector
	 * @throws GreatCircleException
	 */
	double* getPoint(const double &dist)
	{
		double* location = new double[3];
		GeoTessUtils::move(firstPoint, moveDirection, dist, location);
		return location;
	}

	/**
	 * Retrieve a unit vector object located on the great circle path a specified
	 * distance from firstPoint.
	 *
	 * @param dist
	 *            double the angular distance from firstPoint, in radians.
	 * @param location (output) unit vector located on the great circle path a specified
	 * distance from firstPoint.
	 */
	void getPoint(const double &dist, double* location)
	{
		GeoTessUtils::move(firstPoint, moveDirection, dist, location);
	}

	/**
	 * Return number of points required to span specified great circle distance with
	 * points that have spacing not to exceed specified value.  If onCenters is true
	 * then points will reside in the centers of equal sized intervals.
	 * If onCenters is false (default), then first point coincides with start of
	 * great circle, last point coincides with end of great circle and other points
	 * are equally spaced in between.  Actual spacing of points will generally be
	 * less than requested spacing so that integral number of equally spaced points
	 * will span the great circle.
	 * @param dist length of great circle path in radians
	 * @param spacing maximum spacing between points in radians.
	 * @param onCenters if true, points in middle of intervals, other wise
	 * points at boundaries of intervals.
	 */
	static int getNPoints(const double& dist, const double& spacing, const bool& onCenters=false)
	{
		if (dist <= 0.) return onCenters ? 1 : 2;
		return onCenters ? (int)ceil(dist/spacing) : ((int)ceil(dist/spacing))+1;
	}

	/**
	 * Return number of points required to span this great circle with
	 * points that have spacing not to exceed specified value.  If onCenters is true
	 * then points will reside in the centers of equal sized intervals.
	 * If onCenters is false (default), then first point coincides with start of
	 * great circle, last point coincides with end of great circle and other points
	 * are equally spaced in between.  Actual spacing of points will generally be
	 * less than requested spacing so that integral number of equally spaced points
	 * will span the great circle.
	 * @param spacing maximum spacing between points in radians.
	 * @param onCenters if true, points in middle of intervals, other wise
	 * points at boundaries of intervals.
	 */
	int getNPoints(const double& spacing, const bool& onCenters=false)
	{
		return getNPoints(getDistance(), spacing, onCenters);
	}

	/**
	 * Retrieve a bunch of unit vectors equally spaced along the great circle
	 * between initial and final points that define the great circle.
	 *
	 * <p>Supplied array of points must be large enough to hold the specified
	 * number of points.  Caller owns the unit vectors and should delete them
	 * when done with them.  This method neither creates nor deletes any memory.
	 *
	 * @param points an array of unit vectors that will be populated
	 * with equally spaced unit vectors along the great circle.
	 * @param npoints the number of points desired.
	 * @param onCenters if true, the points are
	 * located at the centers of path increments of equal size.
	 * If onCenters is false, the first point is located at the starting point
	 * of the great circle, the last point is located at the final point of the
	 * great circle and the remaining points are equally spaced in between.
	 * @return actual spacing between the points in radians.
	 */
	double getPoints(double** points, const int &npoints, const bool& onCenters=false)
	{
		double dx;
		if (onCenters)
		{
			dx = getDistance()/npoints;
			for (int i=0; i<npoints; ++i) getPoint((i+0.5)*dx, points[i]);
		}
		else
		{
			dx = getDistance()/(npoints-1);
			for (int i=0; i<npoints; ++i) getPoint(i*dx, points[i]);
		}
		return dx;
	}

	/**
	 * Retrieve a bunch of unit vectors equally spaced along the great circle
	 * between initial and final points that define the great circle.
	 *
	 * <p>Supplied array of points must be large enough to hold the necessary
	 * number of points.  Caller owns the unit vectors and should delete them
	 * when done with them.  This method neither creates nor deletes any memory.
	 *
	 * @param spacing the desired point spacing in radians.  Actual spacing
	 * of points will generally be less than requested spacing so that integral
	 * number of equally spaced points will span the great circle.
	 * @param points an array of unit vectors that will be populated
	 * with equally spaced unit vectors along the great circle.
	 * @param npoints number of points along the great circle path
	 * @param onCenters if true, the points are
	 * located at the centers of path increments of equal size.
	 * If onCenters is false, the first point is located at the starting point
	 * of the great circle, the last point is located at the final point of the
	 * great circle and the remaining points are equally spaced in between.
	 * @return actual spacing between the points in radians.
	 */
	double getPoints(const double &spacing, double** points, int &npoints, const bool& onCenters=false)
	{
		npoints = getNPoints(spacing, onCenters);

		double dx;
		if (onCenters)
		{
			dx = getDistance()/npoints;
			for (int i=0; i<npoints; ++i) getPoint((i+0.5)*dx, points[i]);
		}
		else
		{
			dx = getDistance()/(npoints-1);
			for (int i=0; i<npoints; ++i) getPoint(i*dx, points[i]);
		}
		return dx;
	}

	/**
	 * Retrieve the unit vector that lies at the intersection of this GreatCircle
	 * and another GreatCircle. There are, in general, two such intersections
	 * that are 180 degrees apart. This method returns the first one that is
	 * encountered when traveling from firstPoint in the direction of lastPoint.
	 * The other intersection can be retrieved by negating every element of
	 * the unit vector that is returned by this method.
	 *
	 * <p>If inRange is true then the point of intersection must reside between
	 * the firstPoint and the lastPoint of both this and other GreatCircles.
	 *
	 * <p>It is the responsibility of the caller to delete the array returned
	 * by this method.
	 *
	 * @param other
	 *            GreatCircle
	 * @param inRange if true then the point of intersection must reside between
	 *        the firstPoint and the lastPoint of both this and other GreatCircles.
	 * @param intersection unit vector. returns NaN_DOUBLE if the this GreatCircle and other
	 *         GreatCircle are coincident, i.e., their normals are equal.
	 *         Also returns null if inRange is true and the point of intersection
	 *         does not reside between firstPoint and lastPoint of both
	 *         this and other GreatCircle.
	 * @return true if successful, false otherwise.
	 */
	bool getIntersection(GeoTessGreatCircle& other, const bool& inRange, double* intersection)
	{
		if (GeoTessUtils::crossNormal(normal, other.normal, intersection) == 0.)
		{
			intersection[0] = intersection[1] = intersection[2] = NaN_DOUBLE;
			return false;
		}

		if (GeoTessUtils::scalarTripleProduct(firstPoint, intersection, normal) < 0.)
		{
			intersection[0] = -intersection[0];
			intersection[1] = -intersection[1];
			intersection[2] = -intersection[2];
		}

		if (inRange && (getDistance(intersection) >= getDistance()
				|| other.getDistance(intersection) >= other.getDistance()))
		{
			intersection[0] = intersection[1] = intersection[2] = NaN_DOUBLE;
			return false;
		}

		return true;
	}

	/**
	 * Retrieve a reference to the transform matrix owned by this GreatCircle.
	 * Transform is a 3 x 3 matrix such that when a vector is multiplied by
	 * transform, the vector will be projected onto the plane of this
	 * GreatCircle. The z direction will point out of the plane of the great
	 * circle in the direction of the observer (lastPoint cross firstPoint;
	 * parallel to normal). The y direction will correspond to the mean of
	 * firstPoint and lastPoint. The x direction will correspond to y cross z,
	 * forming a right handed coordinate system.
	 *
	 * <p>Caller should not delete this array
	 *
	 * @return double**
	 */
	double** getTransform();

	/**
	 * Project vector x onto the plane of this GreatCircle. Returns a 3 element
	 * vector g such that g[2] is the component of x that points out of the
	 * plane of the GreatCircle (toward the observer).
	 * g[1] is the component of x parallel to the mean of firstPoint and lastPoint,
	 * and g[0] is the remaining part of x.
	 * For an observer viewing the great circle from the normal direction (firstPoint
	 * on the left and lastPoint on the right), g[2] will be the component of
	 * x that points toward the observer, g[1] will be 'up' and g[0] will be
	 * 'to the right'.  For plotting values on a great circle 'slice' through a
	 * model, g[0] will be the x-component, g[1] will be the y-component and
	 * g[2] should be ignored.
	 *
	 * @param x
	 *            double[] the 3 element array containing the vector to be
	 *            projected.
	 * @param v
	 *            double[] the projection of x onto plane of this GreatCircle
	 * @throws GreatCircleException
	 */
	void transform(const double* x, double* v)
	{
		// make sure that trnsfrm has been calculated
		getTransform();
		v[0] = x[0] * trnsfrm[0][0] + x[1] * trnsfrm[0][1] + x[2]
				* trnsfrm[0][2];
		v[1] = x[0] * trnsfrm[1][0] + x[1] * trnsfrm[1][1] + x[2]
				* trnsfrm[1][2];
		v[2] = x[0] * trnsfrm[2][0] + x[1] * trnsfrm[2][1] + x[2]
					* trnsfrm[2][2];
	}

	/**
	 * Project vector x onto the plane of this GreatCircle. Returns a 3 element
	 * vector g such that g[2] is the component of x that points out of the
	 * plane of the GreatCircle (toward the observer).
	 * g[1] is the component of x parallel to the mean of firstPoint and lastPoint,
	 * and g[0] is the remaining part of x.
	 * For an observer viewing the great circle from the normal direction (firstPoint
	 * on the left and lastPoint on the right), g[2] will be the component of
	 * x that points toward the observer, g[1] will be 'up' and g[0] will be
	 * 'to the right'.  For plotting values on a great circle 'slice' through a
	 * model, g[0] will be the x-component, g[1] will be the y-component and
	 * g[2] should be ignored.
	 *
	 *<p>Caller must delete the array returned by this method.
	 *
	 * @param x
	 *            double[] the 3 element array containing the vector to be
	 *            projected.
	 * @return double[] the projection of x onto plane of this GreatCircle
	 * @throws GreatCircleException
	 */
	double* transform(const double* x)
	{
		double* v = new double[3];
		transform(x, v);
		return v;
	}

	string toString();

}; // end class GreatCircle

} // end namespace geotess

#endif /* GREATCIRCLE_H_ */
