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

#ifndef POLYGON_H_
#define POLYGON_H_

// **** _SYSTEM INCLUDES_ ******************************************************

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <vector>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"
#include "GeoTessUtils.h"
#include "GeoTessGreatCircle.h"
#include "IFStreamAscii.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief An ordered list of points on the surface of a unit sphere that define a
 * closed polygon.
 *
 * An ordered list of points on the surface of a unit sphere that define a
 * closed polygon. Polygons have the ability to test whether or not an
 * arbitrary test point on the sphere is inside or outside the polygon.
 *
 * <p>
 * Besides the set of points that define the boundary of the polgon, a Polygon
 * object has a single private instance of a reference point which is known to
 * be either 'inside' or 'outside' the polygon. By default, the reference point
 * is computed from the supplied boundary points in the following manner:
 * <ul>
 * <li>Compute the normalized vector mean of all the points that define the
 * polygon and call it <i>center</i>.
 * <li>set the referencePoint to the anti-pode of <i>center</i>.
 * <li>set referencePointIn to false
 * <li>test <i>center</i> to see if it is inside or outside the
 * polygon and set the value of <i>referencePointIn</i> to this value.
 * <li>change the value of referencePoint to <i>center</i>
 * </ul>
 * This method will work fine provided that polygons are
 * 'smaller' than approximately a hemisphere.  It is possible to override this
 * behavior.  The first method is to call the invert() method which simply
 * reverses the value of whether the reference point is 'inside' or 'outside'
 * the polygon.  There are also methods that allow applications to specify
 * both the position of the reference point and whether it is 'inside' or
 * 'outside' the polygon.
 *
 * <p>A test point that is located very close to a polygon boundary point is
 * deemed to be 'inside' the polygon.  This means that if two adjacent,
 * non-overlapping polygons share a boundary point, a test point near that
 * boundary point will be deemed to be 'inside' both polygons.  In this
 * context two points are 'very close' if they are separated by less than
 * 1e-7 radians or 5.7e-6 degrees.  For a sphere with the radius of the
 * Earth (6371 km), this corresponds to a linear distance of about 60 cm.
 *
 * <p>Polygon implements reference counting. This means that when some other
 * object obtains a reference to a Polygon object, it should increment the
 * polygon's reference count.  When it is done with the polygon it should
 * decrement the polygon's reference count and delete the polygon if the
 * reference count is equal to zero.
 *
 * @author sballar
 *
 */
class GEOTESS_EXP_IMP GeoTessPolygon
{
protected:

	/**
	 * A GreatCircle object for each edge of the polygon.
	 */
	vector<GeoTessGreatCircle*> edges;

	/**
	 * A point on the surface of the unit sphere that is used as
	 * a reference point.  The status of this point relative to
	 * the polygon is known, i.e., it is known if this point is
	 * inside or outside the polygon.
	 */
	double* referencePoint;

	/**
	 * true if the referencePoint is inside the polygon.
	 */
	bool referenceIn;

	/**
	 * Tolerance value in radians used when comparing locations of two points.
	 */
	 static double TOLERANCE;

	/**
	 * If global is true this polygon encompasses the entire Earth and
	 * method contains() will always return the value of referenceIn.
	 */
	 bool global;

	/**
	 * When reading/writing lat,lon data, should order be lat,lon or lon,lat.
	 */
	 bool lonFirst;

	/**
	 * @param points a list of unit vectors
	 */
	void setup(vector<double*>& points);

	int edgeCrossings(GeoTessGreatCircle& gcRef);

	/**
	 * Reference count.
	 */
	int refCount;

  public:

	virtual ~GeoTessPolygon();

	/**
	 * Some unspecified information that applications can attach to this
	 * polygon.  This information is not processed in anyway by Polygon.
	 */
	void* attachment;

	GeoTessPolygon();

	/**
	 * Constructor that accepts a list of unit vectors that define the polygon.
	 * The polygon will be closed, i.e., if the first point and last point are
	 * not coincident then an edge will be created between them.
	 * <p>
	 * Polygon assumes ownership of the supplied points and will delete their
	 * memory when it is done with them.  Callers should not use the points
	 * in the supplied vector after the call to this constructor.
	 *
	 * @param points Collection
	 * @throws PolygonException
	 */
	GeoTessPolygon(vector<double*>& points);

	/**
	 * Constructor that builds a circular polygon of a specified horizontal
	 * radius centered on position center.
	 *
	 * <p>Polygon does not assume ownership of the supplied unit vector.
	 *
	 * @param center unit vector of the position of the center of the polygon.
	 * @param radius double angular radius of the polygon, in radians.
	 * @param nEdges number of points that define the border of the polygon.
	 * @throws PolygonException
	 */
	GeoTessPolygon(const double* center, double radius, int nEdges);

	/**
	 * Constructor that reads a Polygon from a file.  If the supplied
	 * filename is 'global' or 'GLOBAL' then the polygon.contains(unit_vector)
	 * will always return true.
	 *
	 * <p>throws a GeoTessException if specified file is a kml or kmz file.
	 * GeoTessExplorer can translate kml/kmz files to a compatible ascii format.
	 */
	GeoTessPolygon(string filename);

	/**
	 * Returns the class name.
	 */
	virtual string class_name() { return "Polygon"; };

	/**
	 * Add reference count;
	 */
	void addReference() { ++refCount; }

	/**
	 * Remove reference count;
	 */
	void removeReference()
	{
		if (isNotReferenced())
		{
			ostringstream os;
			os << endl << "ERROR in Polygon::removeReference" << endl
					<< "Reference count (" << refCount << ") is already zero." << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 10001);
		}

		--refCount;
	}

	/**
	 * Returns true if reference count is zero.
	 */
	bool isNotReferenced() { return refCount == 0; }

	int getRefCount() { return refCount; }

	/**
	 * Returns the number of edges that define the polygon. Equals the number of
	 * unique GeoVectors that define the polygon.
	 *
	 * @return the number of edges that define the polygon. Equals the number of
	 *         unique GeoVectors that define the polygon.
	 */
	int size()
	{
		return edges.size();
	}

	/**
	 * Retrieve the tolerance value in radians used when comparing locations of two points.
	 */
	static double getTolerance()
	{
		return TOLERANCE;
	}

	/**
	 * Retrieve a reference to the referencePoint.
	 *
	 * <p>Caller should not delete this array
	 * @return
	 */
	const double* const getReferencePoint()
	{
		return referencePoint;
	}

	/**
	 * Retrieve a copy to the referencePoint.
	 *
	 * @param u 3-element array that will be populated
	 * with the unit vector of the referece point.
	 */
	const void getReferencePoint(double *u)
	{
		u[0] = referencePoint[0];
		u[1] = referencePoint[1];
		u[2] = referencePoint[2];
	}

	/**
	 * Retrieve the value of referenceIn which indicates whether or not
	 * the referencePoint is inside or outside the Polygon.
	 */
	bool getReferencePointIn()
	{
		return referenceIn;
	}

	/**
	 * Invert the current polygon. What used to be in will be out and what used
	 * to be out will be in.
	 */
	void invert()
	{
		referenceIn = !referenceIn;
	}

	/**
	 * Specify the reference point for this polygon and whether or not the
	 * specified point is inside or outside the polygon.
	 *
	 * <p>Polygon copies the supplied refPoint array to internal variable.
	 * No reference to the supplied array is stored.
	 *
	 * @param refPoint a unit vector of a position known to be either
	 * inside or outside the polygon
	 * @param inside true if the supplied reference point is known to be
	 * inside the polygon, false if known to be outside.
	 */
	void setReferencePoint(const double* refPoint, const bool &inside)
	{
		if (referencePoint == NULL)
			referencePoint = new double[3];

		referencePoint[0] = refPoint[0];
		referencePoint[1] = refPoint[1];
		referencePoint[2] = refPoint[2];
		referenceIn = inside;
	}

	/**
	 * Specify the reference point for this polygon and whether or not the
	 * specified point is inside or outside the polygon.
	 *
	 * @param lat geographic latitude of reference point in degrees
	 * @param lon longitude of reference point in degrees
	 * @param inside true if the supplied reference point is known to be
	 * inside the polygon, false if known to be outside.
	 */
	void setReferencePoint(double lat, double lon, bool inside)
	{
		double r[3];
		GeoTessUtils::getVectorDegrees(lat, lon, r);
		setReferencePoint(r, inside);
	}

	/**
	 * Returns true if this Polygon contains any of the supplied unit vectors
	 *
	 * @param points
	 *            array of unit vectors
	 * @return true if this Polygon contains any of the supplied unit vectors
	 * @throws PolygonException
	 */
	bool containsAny(const vector<double*>& points)
	{
		for (int i=0; i<(int)points.size(); ++i)
			if (contains(points[i]))
				return true;
		return false;
	}

	/**
	 * Returns true if this Polygon contains all of the supplied unit vectors
	 *
	 * @param positions unit vectors of positions to be evaluated
	 * @return true if this Polygon contains all of the supplied unit vectors
	 */
	bool containsAll(const vector<double*>& positions)
	{
		for (int i=0; i<(int)positions.size(); ++i)
			if (!contains(positions[i]))
				return false;
		return true;
	}

	/**
	 * return true if point x is located inside the polygon
	 *
	 * @param x unit vector of position to be evaluated
	 * @return true if point x is located inside the polygon
	 */
	bool contains(const double* x)
	{
		if (global || GeoTessUtils::dot(referencePoint, x) > cos(TOLERANCE))
			return referenceIn;

		GeoTessGreatCircle gcRef(referencePoint, x);

		return onBoundary(gcRef) || ((edgeCrossings(gcRef) % 2 == 0) == referenceIn);
	}

	/**
	 * Return true if evaluation point is very close to being on the boundary of the polygon.
	 *
	 * @param gcRef the great circle from the reference point to the evaluation
	 * point.
	 * @return true if evaluation point is very close to being on the boundary of the polygon.
	 */
	bool onBoundary(GeoTessGreatCircle& gcRef)
	{
		// point being evaluated is gcRef.getLast()
		for (int i=0; i<(int)edges.size(); ++i)
		{
			// if point is very close to a polygon point, return true
			if (GeoTessUtils::dot(gcRef.getLast(), edges[i]->getFirst()) >= cos(TOLERANCE))
				return true;

			// if gcRef and edge are coincident, i.e., their normals are parallel or
			// anti-parallel, and the point being evaluated is between first and last
			// point of the edge, then return true;
			if (abs(GeoTessUtils::dot(gcRef.getNormal(), edges[i]->getNormal())) >= cos(TOLERANCE)
					&& edges[i]->getDistance(gcRef.getLast()) <= edges[i]->getDistance())
				return true;
		}
		return false;
	}

	/**
	 * Return true if evaluation point is very close to being on the boundary of the polygon.
	 *
	 * @param x the evaluation  point.
	 * @return true if x is very close to being on the boundary of the polygon.
	 * @throws PolygonException
	 */
	bool onBoundary(const double* x)
	{
		GeoTessGreatCircle gc(referencePoint, x);
		return onBoundary(gc);
	}

	/**
	 * Retrieve a deep copy of the points on the polygon.
	 *
	 * <p>vector of points is not cleared by this method.
	 *
	 * <p>It is the caller's responsibility to delete the points
	 * retrieved with this method.
	 *
	 * @param points a vector of unit vectors in which to store
	 * the points of this polygon.
	 *
	 * @param repeatFirstPoint
	 *            if true, last point will be equal to the first point.
	 */
	void getPoints(vector<double*> &points, const bool &repeatFirstPoint)
	{
		points.reserve(points.size()+edges.size()+1);

		for (int i = 0; i < (int)edges.size(); ++i)
		{
			double* point = new double[3];
			point[0] = edges[i]->getFirst()[0];
			point[1] = edges[i]->getFirst()[1];
			point[2] = edges[i]->getFirst()[2];
			points.push_back(point);
		}

		if (repeatFirstPoint)
		{
			double* point = new double[3];
			point[0] = edges[0]->getFirst()[0];
			point[1] = edges[0]->getFirst()[1];
			point[2] = edges[0]->getFirst()[2];
			points.push_back(point);
		}
	}

	/**
	 * Retrieve a deep copy of the points on the polygon.
	 *
	 * <p>It is the caller's responsibility to delete the points
	 * retrieved with this method.
	 *
	 * @param points a vector of unit vectors in which to store
	 * the points of this polygon.
	 *
	 * @param repeatFirstPoint
	 *            if true, last point will be equal to the first point.
	 * @param maxSpacing maximum spacing between points in radians.
	 * Extra points will be inserted as necessary so as to ensure that this is the case.
	 * @return a deep copy of the points on the polygon.
	 */
	void getPoints(vector<double*> &points, const bool &repeatFirstPoint, const double &maxSpacing)
	{
		points.reserve(points.size()+edges.size()+1);

		int n;
		double dx;
		for (int i = 0; i < (int)edges.size(); ++i)
		{
			n = (int)ceil(edges[i]->getDistance()/maxSpacing);
			dx = edges[i]->getDistance()/n;
			for (int j=0; j<n; ++j)
				points.push_back(edges[i]->getPoint(j*dx));
		}

		if (repeatFirstPoint)
		{
			double* point = new double[3];
			point[0] = edges[0]->getFirst()[0];
			point[1] = edges[0]->getFirst()[1];
			point[2] = edges[0]->getFirst()[2];
			points.push_back(point);
		}
	}

	/**
	 * Retrieve a reference to one point on the polygon boundary
	 *
	 * <p>Caller should not delete this array.
	 *
	 * @return a reference to a point on the polygon
	 */
	const double* getPoint(int index)
	{
		return edges[index]->getFirst();
	}

	/**
	 * Retrieve the area of this polygon.  This is the unitless area
	 * (radians squared).  It must be multiplied by R^2 where R is
	 * the radius of the sphere.
	 * <p>
	 * The area is computed assuming that the points on the polygon
	 * are listed in clockwise order when viewed from outside the unit
	 * sphere.  If the compliment of this area is desired, simply
	 * subtract the reported area from the surface area of the entire
	 * sphere (4*PI).
	 * @return
	 */
	double getArea()
	{
		double a, area = 0;
		GeoTessGreatCircle* edge;
		GeoTessGreatCircle* previous = edges[edges.size()-1];

		for (int i=0; i<(int)edges.size(); ++i)
		{
			edge = edges[i];
			a = PI - GeoTessUtils::angle(previous->getNormal(), edge->getNormal());

			if (GeoTessUtils::scalarTripleProduct(previous->getNormal(), edge->getNormal(),
					edge->getFirst()) < 0)
				area += a;
			else
				area += 2*PI - a;

			previous = edge;
		}
		area -= (edges.size()-2)*PI;
		return area;
	}

	/**
	 * Retrieve the area of this polygon.  This is the unitless area
	 * (radians squared).  It must be multiplied by R^2 where R is
	 * the radius of the sphere.
	 * <p>
	 * The area is computed assuming that the polygon area is less
	 * than half the area of the entire sphere.
	 * @return
	 */
	double getAreaSmall()
	{
		double area = getArea();
		return area <= 2*PI ? area : 4*PI-area;
	}

	/**
	 * Retrieve the area of this polygon.  This is the unitless area
	 * (radians squared).  It must be multiplied by R^2 where R is
	 * the radius of the sphere.
	 * <p>
	 * The area is computed assuming that the polygon area is greater
	 * than half the area of the entire sphere.
	 * @return
	 */
	double getAreaLarge()
	{
		double area = getArea();
		return area >= 2*PI ? area : 4*PI-area;
	}

	/**
	 * Returns a String containing all the points that define the polygon with
	 * one lon, lat pair per record. lats and lons are in degrees.
	 * <p>
	 * If latFirst is true, points are listed as lat, lon. If false, order is
	 * lon, lat.
	 * <p>
	 * Longitudes will be adjusted so that they fall in the range minLongitude
	 * to (minLongitude+360).
	 *
	 * @param repeatFirstPoint
	 *
	 * @return String
	 * @param latFirst
	 *            boolean
	 * @param minLongitude
	 *            double
	 */
	string str(const bool& repeatFirstPoint, const bool& latFirst,
			const double& minLongitude = -180);

	virtual void write(const string& outputFileName);

	/// @cond PROTECTED  Turn off doxygen documentation until 'endcond' is found

	virtual void loadAscii(vector<string>& records);

	/// @endcond

}; // end class Polygon

} // end namespace geotess

#endif /* POLYGON_H_ */
