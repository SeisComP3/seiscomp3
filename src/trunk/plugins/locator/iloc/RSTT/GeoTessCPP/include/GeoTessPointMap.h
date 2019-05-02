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

#ifndef POINTMAP_OBJECT_H
#define POINTMAP_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <cstdio>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"
#include "GeoTessGrid.h"
#include "GeoTessProfile.h"
#include "GeoTessMetaData.h"
#include "GeoTessPolygon.h"
#include "GeoTessPolygon3D.h"
#include "GeoTessPolygonFactory.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess
{

// **** _FORWARD REFERENCES_ ***************************************************

class GeoTessModel;

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Relationships between vertices (2D positions in a tessellation), nodes
 * (1D positions along a radial Profile) and points (3D positions in a model).
 *
 * Class that manages relationships between vertices (2D positions in a tessellation),
 * nodes (1D positions along a radial Profile) and points (3D positions in a model).
 */
class GEOTESS_EXP_IMP GeoTessPointMap
{
private:

	/**
	 * The GeoTessGrid object that supports the 2D components of the model grid.
	 */
	GeoTessGrid& grid;

	/**
	 * The data stored in the model. An nVertices x nLayers array of Profile objects. Each Profile
	 * consists of an array of radii and the associated Data.
	 */
	GeoTessProfile *** profiles;

	/**
	 * metaData stores basic information about a GeoTessModel including:
	 * <ul>
	 * <li>textual description of the model
	 * <li>the names of all the layers in the model, e.g., ["core", "mantle", "crust"]. Layer names
	 * are specified in order of increasing radius.
	 * <li>dataType; all the data values stored in the model are of this type. Must be one of
	 * DOUBLE, FLOAT, LONG, INT, SHORTINT, BYTE.
	 * <li>number of data attributes
	 * <li>the names of all the data attributes ["P Velocity", "S Velocity", "Density", etc]
	 * <li>the units of the data attributes ["km/sec", "km/sec", "g/cc", etc]
	 * <li>layerTessIds: an integer map from layer index to tessellation index.
	 * </ul>
	 * Each GeoTessModel has a single instance of MetaData that it passes around to wherever the
	 * information is needed.
	 */
	GeoTessMetaData& metaData;

	GeoTessPolygon* polygon;

	/**
	 * A nPoints by 3 array of indexes. For each point in the 3D grid, pointMap stores 3 indexes:
	 * the index of the 2D vertex, the layer index, and the node index within the layer. There is an
	 * entry for each data object, not each radius.
	 */
	vector<vector<int> >  pointMap;

	bool populated;

public:

	/**
	 * Constructor.  PointMap is initialized but not populated by
	 * this method.
	 */
	GeoTessPointMap(GeoTessModel& m);

	/**
	 * Copy constructor.
	 */
	GeoTessPointMap(GeoTessPointMap& other);

	/**
	 * Overloaded assignment operator
	 */
	GeoTessPointMap& operator=(const GeoTessPointMap& other);

	/**
	 * Overloaded equality operator.
	 * Throws and exception if other is not populated.
	 */
	bool operator==(const GeoTessPointMap& other);

	/**
	 * Overloaded inequality operator
	 */
	bool operator!=(const GeoTessPointMap& other) { return !(*this == other); }

	/**
	 * Destructor.
	 */
	~GeoTessPointMap();

	LONG_INT getMemory()
	{
		return (LONG_INT) sizeof(GeoTessPointMap)
				+ (LONG_INT)(pointMap.capacity() * sizeof(vector<int>)
				+ pointMap.size() * 3 * (LONG_INT)sizeof(int));
	}

	/**
	 * Populates the PointMap such that every node in the entire
	 * model is within the active region.  The pointIndex value
	 * of every node in the model will be set a unique positive
	 * value.
	 */
	void setActiveRegion();

	/**
	 * Populate the PointMap such that all nodes associated with
	 * Profiles attached to grid vertices that are within the
	 * 2D polygon are active. The pointIndex values of nodes
	 * within the polygon will be set to unique positive values.
	 * All nodes outside the polygon will have pointIndex
	 * values equal to -1.
	 * @param polygon a 2D Polygon object
	 */
	void setActiveRegion(GeoTessPolygon* polygon);

	/**
	 * Populate the PointMap such that nodes located within the
	 * specified Polygon are active and all others are inactive.
	 * The pointIndex values of nodes within the polygon will
	 * be set to unique positive values. All nodes outside the
	 * polygon will have pointIndex values equal to -1.
	 * @param polygonFileName the name of a file that contains
	 * a 2D or 3D polygon.
	 */
	void setActiveRegion(const string& polygonFileName)
	{
		GeoTessPolygon* plgn = GeoTessPolygonFactory::getPolygon(polygonFileName);
		setActiveRegion(plgn);
	}

	/**
	 * Retrieve the pointer to the Polygon or Polygon3D object
	 * that supports this PointMap.  May be NULL.
	 * <p>Polygon implements referenceCounting so if you wish
	 * to retain a copy of this polygon, be sure to
	 * addReference() and delete it when you are done with it.
	 */
	GeoTessPolygon* getPolygon() { return polygon; }

	/**
	 * Clears all the information in this pointMap and the
	 * pointIndex values of all nodes in the model.
	 * Following this call, isPopulated() will return false.
	 * The pointIndex values in all model nodes and profiles
	 * will be set to -1.
	 */
	void clear();

	/**
	 * Determine whether or not this PointMap is populated.
	 * @return true if populated
	 */
	bool isPopulated() { return populated; }

	/**
	 * Retrieve the number of points supported by this model.
	 *
	 * @return the number of points supported by this model.
	 */
	int size()
	{
		return pointMap.size();
	}

	/**
	 * Retrieve the index of the vertex that corresponds to the specified
	 * pointIndex.
	 *
	 * @param pointIndex
	 * @return the index of the vertex that corresponds to the specified
	 *         pointIndex.
	 */
	int getVertexIndex(int pointIndex)
	{
		return pointMap[pointIndex][0];
	}

	/**
	 * Retrieve the index of the tessellation that corresponds to the specified
	 * pointIndex.
	 *
	 * @param pointIndex
	 * @return the index of the tessellation that corresponds to the specified
	 *         pointIndex.
	 */
	int getTessId(int pointIndex)
	{
		return metaData.getTessellation(pointMap[pointIndex][1]);
	}

	/**
	 * Retrieve the index of the layer that corresponds to the specified
	 * pointIndex.
	 *
	 * @param pointIndex
	 * @return the index of the layer that corresponds to the specified
	 *         pointIndex.
	 */
	int getLayerIndex(int pointIndex)
	{
		return pointMap[pointIndex][1];
	}

	/**
	 * Retrieve the index of the node that corresponds to the specified
	 * pointIndex.  Nodes refer to Data object, not radii.
	 *
	 * @param pointIndex
	 * @return the index of the node that corresponds to the specified
	 *         pointIndex.
	 */
	int getNodeIndex(int pointIndex)
	{
		return pointMap[pointIndex][2];
	}

	/**
	 * Retrieve a reference to the index map for the specified pointIndex. This
	 * is a 3-element array consisting of 0:vertexIndex, 1:layerIndex,
	 * 2:nodeIndex.
	 *
	 * @param pointIndex
	 * @return the index map for the specified pointIndex.
	 */
	const vector<int>& getPointIndices(int pointIndex)
	{
		return pointMap[pointIndex];
	}

	/**
	 * Retrieve the pointIndex of the point that corresponds to the specified
	 * vertex, layer and node.
	 *
	 * @param vertex
	 * @param layer
	 * @param node
	 * @return the pointIndex of the point that corresponds to the specified
	 *         vertex, layer and node.
	 */
	int getPointIndex(int vertex, int layer, int node)
	{
		return profiles[vertex][layer]->getPointIndex(node);
	}

	/**
	 * Retrieve the pointIndex of the point that corresponds to the last node
	 * in profile[vertex][layer].  The last node is the one with the largest
	 * radius (i.e., the shallowest node).
	 *
	 * @param vertex
	 * @param layer
	 * @return the pointIndex of the point that corresponds to the last node
	 * in profile[vertex][layer].
	 */
	int getPointIndexLast(int vertex, int layer)
	{
		return profiles[vertex][layer]->getPointIndex(profiles[vertex][layer]->getNData()-1);
	}

	/**
	 * Retrieve the pointIndex of the point that corresponds to the first node
	 * in profile[vertex][layer].  The first node is the one with the smallest
	 * radius (i.e., the deepest node).
	 *
	 * @param vertex
	 * @param layer
	 * @return the pointIndex of the point that corresponds to the first node
	 * in profile[vertex][layer].
	 */
	int getPointIndexFirst(int vertex, int layer)
	{
		return profiles[vertex][layer]->getPointIndex(0);
	}

	/**
	 * Replace the Data object associated with the specified point.
	 */
	void setPointData(int pointIndex, GeoTessData* data)
	{
		vector<int>& map = pointMap[pointIndex];
		profiles[map[0]][map[1]]->setData(map[2], data);
	}

	/**
	 * Retrieve a reference to the Data object associated with the specified point.
	 * @param pointIndex
	 * @return
	 */
	GeoTessData* getPointData(int pointIndex)
	{
		vector<int>& map = pointMap[pointIndex];
		return profiles[map[0]][map[1]]->getData(map[2]);
	}

	/**
	 * Set the value of the specified attribute at the specified point to the
	 * specified value.
	 *
	 * @param pointIndex
	 * @param attributeIndex
	 * @param value
	 */
	template <typename T>
	void setPointValue(int pointIndex, int attributeIndex, T value)
	{
		vector<int>& map = pointMap[pointIndex];
		profiles[map[0]][map[1]]->getData(map[2])->setValue(attributeIndex,
				value);
	}

	/**
	 * Retrieve the value of the specified attribute at the specified point.
	 *
	 * @param pointIndex
	 * @param attributeIndex
	 * @return the value of the specified attribute at the specified point.
	 */
	double getPointValue(int pointIndex, int attributeIndex)
	{
		vector<int>& map = pointMap[pointIndex];
		return profiles[map[0]][map[1]]->getValue(attributeIndex, map[2]);
	}

	/**
	 * Retrieve the value of the specified attribute at the specified point
	 * cast to double if necessary.
	 *
	 * @param pointIndex
	 * @param attributeIndex
	 * @return the value of the specified attribute at the specified point
	 * cast to double if necessary.
	 */
	double getPointValueDouble(int pointIndex, int attributeIndex)
	{
		vector<int>& map = pointMap[pointIndex];
		return profiles[map[0]][map[1]]->getData(map[2])->getDouble(attributeIndex);
	}

	/**
	 * Retrieve the value of the specified attribute at the specified point
	 * cast to float if necessary.
	 *
	 * @param pointIndex
	 * @param attributeIndex
	 * @return the value of the specified attribute at the specified point
	 * cast to float if necessary.
	 */
	float getPointValueFloat(int pointIndex, int attributeIndex)
	{
		vector<int>& map = pointMap[pointIndex];
		return profiles[map[0]][map[1]]->getData(map[2])->getFloat(attributeIndex);
	}

	/**
	 * Retrieve the value of the specified attribute at the specified point
	 * cast to LONG_INT if necessary.
	 *
	 * @param pointIndex
	 * @param attributeIndex
	 * @return the value of the specified attribute at the specified point
	 * cast to LONG_INT if necessary.
	 */
	LONG_INT getPointValueLong(int pointIndex, int attributeIndex)
	{
		vector<int>& map = pointMap[pointIndex];
		return profiles[map[0]][map[1]]->getData(map[2])->getLong(attributeIndex);
	}

	/**
	 * Retrieve the value of the specified attribute at the specified point
	 * cast to int if necessary.
	 *
	 * @param pointIndex
	 * @param attributeIndex
	 * @return the value of the specified attribute at the specified point
	 * cast to int if necessary.
	 */
	int getPointValueInt(int pointIndex, int attributeIndex)
	{
		vector<int>& map = pointMap[pointIndex];
		return profiles[map[0]][map[1]]->getData(map[2])->getInt(attributeIndex);
	}

	/**
	 * Retrieve the value of the specified attribute at the specified point
	 * cast to short if necessary.
	 *
	 * @param pointIndex
	 * @param attributeIndex
	 * @return the value of the specified attribute at the specified point
	 * cast to short if necessary.
	 */
	short getPointValueShort(int pointIndex, int attributeIndex)
	{
		vector<int>& map = pointMap[pointIndex];
		return profiles[map[0]][map[1]]->getData(map[2])->getShort(attributeIndex);
	}

	/**
	 * Retrieve the value of the specified attribute at the specified point
	 * cast to byte if necessary.
	 *
	 * @param pointIndex
	 * @param attributeIndex
	 * @return the value of the specified attribute at the specified point
	 * cast to byte if necessary.
	 */
	byte getPointValueByte(int pointIndex, int attributeIndex)
	{
		vector<int>& map = pointMap[pointIndex];
		return profiles[map[0]][map[1]]->getData(map[2])->getByte(attributeIndex);
	}

	/**
	 * Return true if the value of the specified attribute at the specified
	 * point is NaN.
	 *
	 * @param pointIndex
	 * @param attributeIndex
	 * @return true if the value of the specified attribute at the specified
	 *         point is NaN.
	 */
	bool isNaN(int pointIndex, int attributeIndex)
	{
		vector<int>& map = pointMap[pointIndex];
		return profiles[map[0]][map[1]]->isNaN(map[2], attributeIndex);
	}

	/**
	 * Retrieve a vector representation of the specified point (not a unit
	 * vector). The length of the vector is in km. This is a new double[], not a
	 * reference to an existing variable.
	 *
	 * @param pointIndex
	 * @param v (output) a vector representation of the specified point
	 */
	void getPointVector(int pointIndex, double* v)
	{
		vector<int>& map = pointMap[pointIndex];
		const double* vv = grid.getVertex(map[0]);
		double r = profiles[map[0]][map[1]]->getRadius(map[2]);
		v[0] = vv[0] * r;
		v[1] = vv[1] * r;
		v[2] = vv[2] * r;
	}

	/**
	 * Retrieve a reference to the unit vector for the specified point.
	 *
	 * @param pointIndex
	 * @return a reference to the unit vector for the specified point.
	 */
	const double* getPointUnitVector(int pointIndex) const
	{
		return grid.getVertex(pointMap[pointIndex][0]);
	}

	/**
	 * Retrieve the radius of the specified point.
	 *
	 * @param pointIndex
	 * @return radius of specified point, in km.
	 */
	double getPointRadius(int pointIndex)
	{
		vector<int>& map = pointMap[pointIndex];
		return profiles[map[0]][map[1]]->getRadius(map[2]);
	}

	/**
	 * Retrieve the radius of the specified point.
	 *
	 * @param pointIndex
	 * @return radius of specified point, in km.
	 */
	double getPointDepth(int pointIndex)
	{
		vector<int>& map = pointMap[pointIndex];
		return GeoTessUtils::getEarthRadius(
				grid.getVertex(pointMap[pointIndex][0]))
				- profiles[map[0]][map[1]]->getRadius(map[2]);
	}

	/**
	 * Retrieve the straight-line distance between two points in km.
	 *
	 * @param pointIndex1
	 * @param pointIndex2
	 * @return the straight-line distance between two points in km.
	 */
	double getDistance3D(int pointIndex1, int pointIndex2)
	{
		vector<int>& m1 = pointMap[pointIndex1];
		vector<int>& m2 = pointMap[pointIndex2];
		return GeoTessUtils::getDistance3D(grid.getVertex(m1[0]),
				profiles[m1[0]][m1[1]]->getRadius(m1[2]), grid.getVertex(m2[0]),
				profiles[m2[0]][m2[1]]->getRadius(m2[2]));
	}

//	/**
//	 * Append the input new values array to the profile at pointIndex
//	 */
//	template<typename T>
//	void appendData(int pointIndex, T* newValues, int n)
//	{
//		vector<int>& map = pointMap[pointIndex];
//		profiles[map[0]][map[1]]->appendData<T>(map[2], newValues, n);
//	}


	/**
	 * Find all the points that are first-order neighbors of the specified
	 * point. First, find all the vertices that are first order neighbors of the
	 * vertex of the supplied point (vertices connected by a single triangle
	 * edge). For each of those vertices, find the Profile that occupies the
	 * same layer and find the index of the radius in that Profile that is
	 * closest to the radius of the supplied point. Build the set of all such
	 * node index values. Finally, convert the node indexes to point indexes.
	 * There will generally be 6 such points, but that number is not guaranteed.
	 *
	 * <p>
	 * Here is an example of using a HashSetInteger object:
	 * <p>
	 * import gov.sandia.gmp.util.containers.hash.sets.HashSetInteger; <br>
	 * import gov.sandia.gmp.util.containers.hash.sets.HashSetInteger.Iterator;
	 * <p>
	 * Iterator it = getPointNeighbors(pointIndex).iterator(); <br>
	 * while (it.hasNext()) <br>
	 * { <br>
	 * int pointIndex = it.next(); <br>
	 * // do stuff with pointIndex. <br>
	 * }
	 */
	void getPointNeighbors(set<int>& pointNeighbors, int pointIndex);

	/**
	 * Retrieve nicely formated string with lat, lon of the point in degrees.
	 * @param pointIndex
	 * @return string with lat, lon in degrees.
	 */
	string getPointLatLonString(int pointIndex)
	{
		return GeoTessUtils::getLatLonString(getPointUnitVector(pointIndex));
	}

	/**
	 * Retrieve a nicely formated string with lat, lon, depth of the point in degrees and km.
	 * @param pointIndex
	 * @return string with lat, lon, depth
	 */
	string toString(int pointIndex)
	{
		char s[100];
		string frmt = "%8.3f";
		sprintf(s, frmt.c_str(), getPointDepth(pointIndex));
		return GeoTessUtils::getLatLonString(getPointUnitVector(pointIndex))
				+ " " + s;
	}

};
// end class PointMap

}// end namespace geotess

#endif  // POINTMAP_OBJECT_H
