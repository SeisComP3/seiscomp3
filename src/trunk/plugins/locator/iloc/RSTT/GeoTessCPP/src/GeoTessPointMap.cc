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

#include "GeoTessPointMap.h"
#include "GeoTessModel.h"
#include "GeoTessHorizon.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess
{

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

/**
 * protected constructor that builds the new PointMap and assigns the point index
 * to all profile nodes.
 */
GeoTessPointMap::GeoTessPointMap(GeoTessModel& model)
: grid(model.getGrid()), profiles(model.getProfiles()), metaData(model.getMetaData()),
  polygon(NULL), populated(false)
{
}

GeoTessPointMap::GeoTessPointMap(GeoTessPointMap& other)
: grid(other.grid), profiles(other.profiles), metaData(other.metaData),
  polygon(other.polygon), populated(other.populated)
{
	if (polygon != NULL)
		polygon->addReference();

	if (populated)
	{
		vector<int> v3(3,0);
		pointMap.resize(other.pointMap.size(), v3);

		for (int i=0; i<(int)other.pointMap.size(); ++i)
		{
			pointMap[i][0] = other.pointMap[i][0];
			pointMap[i][1] = other.pointMap[i][1];
			pointMap[i][2] = other.pointMap[i][2];
		}
	}
}


/**
 * Overloaded assignment operator
 */
GeoTessPointMap& GeoTessPointMap::operator=(const GeoTessPointMap& other)
{
	grid = other.grid;
	metaData = other.metaData;
	profiles = other.profiles;

	polygon = other.polygon;
	polygon->addReference();

	populated = other.populated;

	if (populated)
	{
		vector<int> v3(3,0);
		pointMap.resize(other.pointMap.size(), v3);

		for (int i=0; i<(int)other.pointMap.size(); ++i)
		{
			pointMap[i][0] = other.pointMap[i][0];
			pointMap[i][1] = other.pointMap[i][1];
			pointMap[i][2] = other.pointMap[i][2];
		}
	}
	else
		pointMap.clear();

	return *this;
}

/**
 * Overloaded equality operator
 */
bool GeoTessPointMap::operator==(const GeoTessPointMap& other)
																								{
	//cout << endl << "PointMap::operator==(const PointMap& other)" << endl;

	if (grid != other.grid) return false;
	if (metaData != other.metaData) return false;

	if (!other.populated)
	{
		ostringstream os;
		os << endl << "ERROR in PointMap::operator==(const PointMap& other)" << endl
				<< "other has not been populated." << endl
				<< "Call other.setActiveRegions() to populate other."
				<< endl;
		throw GeoTessException(os, __FILE__, __LINE__, 8001);
	}

	if (!populated)
		setActiveRegion();

	if (pointMap.size() != other.pointMap.size()) return false;

	for (int i=0; i<(int)pointMap.size(); ++i)
	{
		if (pointMap[i][0] != other.pointMap[i][0]) return false;
		if (pointMap[i][1] != other.pointMap[i][1]) return false;
		if (pointMap[i][2] != other.pointMap[i][2]) return false;
	}

	return true;
																								}

GeoTessPointMap::~GeoTessPointMap()
{
	clear();
}

void GeoTessPointMap::clear()
{
	pointMap.clear();
	populated = false;

	if (polygon != NULL)
	{
		polygon->removeReference();
		if (polygon->isNotReferenced())
			delete polygon;
		polygon = NULL;
	}

	GeoTessProfile** pp;
	for (int vertex = 0; vertex < grid.getNVertices(); ++vertex)
	{
		pp = profiles[vertex];
		for (int layer = 0; layer < metaData.getNLayers(); ++layer)
			pp[layer]->resetPointIndices();
	}
}

void GeoTessPointMap::setActiveRegion()
{
	clear();

	GeoTessProfile** pp;
	GeoTessProfile* p;
	for (int vertex = 0; vertex < grid.getNVertices(); ++vertex)
	{
		pp = profiles[vertex];
		for (int layer = 0; layer < metaData.getNLayers(); ++layer)
		{
			p = pp[layer];
			for (int node = 0; node < p->getNData(); ++node)
			{
				p->setPointIndex(node, (int)pointMap.size());
				vector<int> v(3,0);
				v[0] = vertex;
				v[1] = layer;
				v[2] = node;
				pointMap.push_back(v);
			}
		}
	}
	populated = true;
}

void GeoTessPointMap::setActiveRegion(GeoTessPolygon* _polygon)
{
	clear();

	polygon = _polygon;
	polygon->addReference();

	GeoTessProfile** pp;
	GeoTessProfile* p;

	if (polygon->class_name() == "Polygon")
	{
		for (int vertex = 0; vertex < grid.getNVertices(); ++vertex)
			if (polygon->contains(grid.getVertex(vertex)))
			{
				pp = profiles[vertex];
				for (int layer = 0; layer < metaData.getNLayers(); ++layer)
				{
					p = pp[layer];
					for (int node = 0; node < p->getNData(); ++node)
					{
						p->setPointIndex(node, (int)pointMap.size());
						vector<int> v(3,0);
						v[0] = vertex;
						v[1] = layer;
						v[2] = node;
						pointMap.push_back(v);
					}
				}
			}

	}
	else
	{
		GeoTessHorizon* top = ((GeoTessPolygon3D*)polygon)->getTop();
		GeoTessHorizon* bottom = ((GeoTessPolygon3D*)polygon)->getBottom();

		double rTop, rBottom;

		int layerTop = top->getLayerIndex();
		if (layerTop < 0 || layerTop >= metaData.getNLayers())
			layerTop = metaData.getNLayers()-1;

		int layerBottom = bottom->getLayerIndex();
		if (layerBottom < 0)
			layerBottom = 0;

		for (int vertex = 0; vertex < grid.getNVertices(); ++vertex)
			if (polygon->contains(grid.getVertex(vertex)))
			{
				pp = profiles[vertex];

				rBottom = bottom->getRadius(grid.getVertex(vertex), profiles[vertex]);
				rTop = top->getRadius(grid.getVertex(vertex), profiles[vertex]);

				for (int layer = layerBottom; layer <= layerTop; ++layer)
				{
					p = pp[layer];
					for (int node = 0; node < p->getNData(); ++node)
						if (p->getRadius(node) >= rBottom && p->getRadius(node) <= rTop)
						{
							p->setPointIndex(node, (int)pointMap.size());
							vector<int> v(3,0);
							v[0] = vertex;
							v[1] = layer;
							v[2] = node;
							pointMap.push_back(v);
						}
				}
			}
	}
	populated = true;
}

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
 *
 * @param pointIndex
 * @return the pointIndexes that are first order neighbors of the vertex of
 *         the supplied point.
 */
void GeoTessPointMap::getPointNeighbors(set<int>& pointNeighbors, int pointIndex)
{
	// find the vertexID, layerID and nodeID of point in question

	int vertex = pointMap[pointIndex][0];
	int layer = pointMap[pointIndex][1];
	int node = pointMap[pointIndex][2];

	// find the tessID, levelID and radius of point in question
	int tessid = metaData.getTessellation(layer);
	int level = grid.getNLevels(tessid) - 1;
	double radius = profiles[vertex][layer]->getRadius(node);

	// get the vertex neighbors

	pointNeighbors.clear();
	set<int> vertexNeighbors;
	grid.getVertexNeighbors(tessid, level, vertex, vertexNeighbors);

	// loop over all vertex neighbors and find the closest point index at
	// the input points radius ... add those to the neighbor set

	GeoTessProfile* p;
	int ptId;
	set<int>::iterator it;
	for (it = vertexNeighbors.begin(); it != vertexNeighbors.end(); ++it)
	{
		p = profiles[*it][layer];
		ptId = p->getPointIndex(p->findClosestRadiusIndex(radius));
		if (ptId > 0)
			pointNeighbors.insert(ptId);
	}
}

} // end namespace geotess
