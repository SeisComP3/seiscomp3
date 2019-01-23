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

#ifndef POLYGON3D_H_
#define POLYGON3D_H_

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
#include "GeoTessPolygon.h"
#include "GeoTessHorizon.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

class GeoTessPosition;

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Extends Polygon by including information and constraints about the
 * radial dimension.
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
 * @author sballar
 *
 */
class GEOTESS_EXP_IMP GeoTessPolygon3D : public GeoTessPolygon
{
  private:

	GeoTessHorizon* bottom;

	GeoTessHorizon* top;

  public:

	~GeoTessPolygon3D();

	GeoTessPolygon3D() : GeoTessPolygon(), bottom(NULL), top(NULL)
	{};

	/**
	 * Constructor that accepts a list of unit vectors that define the polygon.
	 * The polygon will be closed, i.e., if the first point and last point are
	 * not coincident then an edge will be created between them.
	 *
	 * @param points array of unit vectors specifying the positions that define
	 * the polygon.
	 * @param h_bottom Horizon object that defines the bottom of the Polygon3D.
	 * @param h_top Horizon object that defines the top of the Polygon3D.
	 */
	GeoTessPolygon3D(vector<double*> points, GeoTessHorizon* h_bottom, GeoTessHorizon* h_top)
	: GeoTessPolygon(points), bottom(h_bottom), top(h_top)
	{}

	/**
	 * Constructor that builds a circular polygon of a specified horizontal
	 * radius centered on position center.
	 *
	 * @param center unit vector of the position that defines the center of
	 * the circular Polygon.
	 * @param radius angular radius of the polygon, in radians.
	 * @param nEdges number of edges that define the polygon
	 * @param h_bottom Horizon object that defines the bottom of the Polygon3D.
	 * @param h_top Horizon object that defines the top of the Polygon3D.
	 */
	GeoTessPolygon3D(double* center, double radius, int nEdges, GeoTessHorizon* h_bottom, GeoTessHorizon* h_top)
	: GeoTessPolygon(center, radius, nEdges), bottom(h_bottom), top(h_top)
	{}

	/**
	 * Constructor that reads a Polygon from a file.
	 *
	 * @param filename the name of the file containing the polygon information.
	 *
	 * <p>throws a GeoTessException if specified file is a kml or kmz file.
	 * GeoTessExplorer can translate kml/kmz files to a compatible ascii format.
	 */
	GeoTessPolygon3D(string filename);

	/**
	 * Returns the class name.
	 */
	virtual string class_name() { return "Polygon3D"; };

	/**
	 * Retrieve a reference to the Horizon that defines the top of the active region.
	 */
	GeoTessHorizon* getTop()
	{
		return top;
	}

	/**
	 * Retrieve a reference to the Horizon that defines the bottom of the active region.
	 */
	GeoTessHorizon* getBottom()
	{
		return bottom;
	}

	/**
	 * Returns true if this Polygon3D contains the specified position.
	 *
	 * @param x the unit vector of the position
	 * @param radius the radius of the position in km
	 * @param layer the index of the layer in which the position resides
	 * @param profiles a 1D array of profiles at the specified position.
	 * The number of elements must be equal to the number of layers in the
	 * model with the first layer being the deepest (closest to the center
	 * of the Earth) and the last layer being the shallowest (farthest from
	 * the center of the Earth).
	 * @return true if this Polygon3D contains the specified position.
	 */
	bool contains(const double* x, const double& radius, const int& layer, GeoTessProfile** profiles)
		{
		return (bottom->getLayerIndex() < 0 || layer >= bottom->getLayerIndex())
				&& (top->getLayerIndex() < 0 || layer <= top->getLayerIndex())
				&& radius > bottom->getRadius(x, profiles) - 1e-4
				&& radius < top->getRadius(x, profiles) + 1e-4
				&& GeoTessPolygon::contains(x);
	}

	/**
	 * Returns true if this Polygon3D contains the specified position.
	 *
	 * @param x the unit vector of the position
	 * @param layer the index of the layer in which the position resides
	 * @return true if this Polygon3D contains the specified position.
	 */
	bool contains(const double* x, const int& layer)
	{
		return ( bottom->getLayerIndex() < 0 || layer >= bottom->getLayerIndex())
				&& ( top->getLayerIndex() < 0 || layer <= top->getLayerIndex())
				&& GeoTessPolygon::contains(x);
	}

	/**
	 * Returns true if this Polygon3D contains the specified position.
	 *
	 * @param position
	 * @return true if this Polygon3D contains the specified position.
	 */
	bool contains(GeoTessPosition& position);

	/**
	 * Returns true if this Polygon contains all of the supplied unit vectors
	 *
	 * @param points unit vectors of positions to be evaluated
	 * @param radii the radii of the positions to be evaluated.  radii.size()
	 * and points.size() must be equal.
	 * @param layers the indexes of the layers in which the positions reside.
	 * @param profiles the array of Profiles at the current geographic position.
	 * profiles.size() must equal number of layers in the model.
	 * @return true if this Polygon contains all of the supplied unit vectors
	 */
	bool containsAll(vector<double*>& points, vector<double>& radii, vector<int>& layers, vector<GeoTessProfile**>& profiles)
	{
		for (int i=0; i<(int)points.size(); ++i)
			if (!contains(points[i], radii[i], layers[i], profiles[i]))
				return false;
		return true;
	}

	/**
	 * Returns true if this Polygon contains any of the supplied unit vectors
	 *
	 * @param points unit vectors of positions to be evaluated
	 * @param radii the radii of the positions to be evaluated.  radii.size()
	 * and points.size() must be equal.
	 * @param layers the indexes of the layers in which the positions reside.
	 * @param profiles the array of Profiles at the current geographic position.
	 * profiles.size() must equal number of layers in the model.
	 * @return true if this Polygon contains any of the supplied unit vectors
	 */
	bool containsAny(vector<double*>& points, vector<double>& radii, vector<int>& layers, vector<GeoTessProfile**>& profiles)
	{
		for (int i=0; i<(int)points.size(); ++i)
			if (contains(points[i], radii[i], layers[i], profiles[i]))
				return true;
		return false;
	}

	virtual void write(const string& outputFileName);

	/// @cond PROTECTED  Turn off doxygen documentation until 'endcond' is found

	virtual void loadAscii(vector<string>& records);

	/// @endcond

}; // end class Polygon

} // end namespace geotess

#endif /* POLYGON3D_H_ */
