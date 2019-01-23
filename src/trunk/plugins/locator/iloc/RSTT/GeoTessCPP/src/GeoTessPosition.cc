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

#include <sstream>
#include <climits>
#include <cmath>

// **** _LOCAL INCLUDES_ *******************************************************

#include "GeoTessPosition.h"
#include "GeoTessException.h"
#include "CpuTimer.h"
#include "GeoTessInterpolatorType.h"
#include "GeoTessPositionLinear.h"
#include "GeoTessPositionNaturalNeighbor.h"
#include "GeoTessProfile.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

const double GeoTessPosition::TWALK_TOLERANCE = -1e-15;

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

/**
 * Constructor that takes a reference to the 3D model that is to be interrogated by this
 * GeoTessPosition3D object.
 *
 * @param model
 * @throws GeoTessException
 */
GeoTessPosition::GeoTessPosition(GeoTessModel* m, const GeoTessInterpolatorType& radialType) :
		refCount(0), maxTessLevel(NULL), tessLevels(NULL), triangle(NULL),
		errorValue(NaN_DOUBLE),
		layerId(-1), tessid(-1),
		radialInterpolatorType(radialType),
		model(m),
		grid(m->getGrid()), modlProfiles(m->getProfiles()),
		gridVertices(grid.getVertices()),
		gridTriangles(grid.getTriangles()),
		gridDescendants(grid.getDescendants()),
		gridEdges(grid.getEdgeList()),
		layerTessIds(m->getMetaData().getLayerTessIds()),
		nLayers(m->getNLayers()),
		radiusOutOfRangeAllowed(true)
{
	unitVector[0] = unitVector[1] = unitVector[2] = 0.0;

	radius = -1.0;
	tessid = -1;

	layerRadii.resize(nLayers + 1);
	for (int i = 0; i < nLayers + 1; ++i) layerRadii[i] = -1.0;

	int ntess = grid.getNTessellations();
	triangle = new int [ntess];
	CPPUtils::resetArray<int>(ntess, triangle, -1);
	tessLevels = new int [ntess];
	maxTessLevel = new int [ntess];
	CPPUtils::resetArray<int>(ntess, maxTessLevel, INT_MAX-1);

	vertices.resize(ntess);
	hCoefficients.resize(ntess);
	linearCoefficients.resize(ntess);
	for (int i = 0; i < ntess; ++i) linearCoefficients[i].resize(3);
}

/**
 * Destructor.
 */
GeoTessPosition::~GeoTessPosition()
{
	if (refCount > 0)
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessPosition::~GeoTessPosition()" << endl
			 << "Reference count (" << refCount << ") is not zero." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 3001);
	}

	if (maxTessLevel != NULL) delete [] maxTessLevel;
	if (tessLevels != NULL) delete [] tessLevels;
	if (triangle != NULL) delete [] triangle;
}

/**
 * Static factory method to create a new interpolator.
 */
GeoTessPosition* GeoTessPosition::getGeoTessPosition(GeoTessModel* model)
{
	return new GeoTessPositionLinear(model, GeoTessInterpolatorType::LINEAR);
}

/**
 * Static factory method to create a new interpolator.
 */
GeoTessPosition*	GeoTessPosition::getGeoTessPosition(GeoTessModel* model,
		const GeoTessInterpolatorType& horizontalType)
{
	switch (horizontalType.ordinal())
	{
	case 0: // LINEAR
		return new GeoTessPositionLinear(model, GeoTessInterpolatorType::LINEAR);

	case 1: // NATURAL_NEIGHBOR
		return new GeoTessPositionNaturalNeighbor(model, GeoTessInterpolatorType::CUBIC_SPLINE);

	default:
		ostringstream os;
		os << endl << "ERROR in GeoTessPosition::getGeoTessPosition" << endl
				<< "Unsupported InterpolatorType " << horizontalType.name() << endl
				<< "Must specify either LINEAR or NATURAL_NEIGHBOR." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 3002);
	}
}

/**
 * Static factory method to create a new interpolator.
 */
GeoTessPosition* GeoTessPosition::getGeoTessPosition(GeoTessModel* model,
		const GeoTessInterpolatorType& horizontalType, const GeoTessInterpolatorType& radialType)
{
	switch (horizontalType.ordinal())
	{
	case 0: // LINEAR
		return new GeoTessPositionLinear(model, radialType);

	case 1: // NATURAL_NEIGHBOR
		return new GeoTessPositionNaturalNeighbor(model, radialType);

	default:
		ostringstream os;
		os << endl << "ERROR in GeoTessPosition::getGeoTessPosition" << endl
				<< "Unsupported InterpolatorType " << horizontalType.name() << endl
				<< "Must specify either LINEAR or NATURAL_NEIGHBOR." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 3003);
	}
}

/**
 * Replace the model that currently supports this GeoTessPosition object
 * with a new model. For this to work, the new model and the current model
 * must use the same grid.
 *
 * <p>
 * The benefit of calling this method is that if the application needs to
 * interpolate a value at the same position in multiple models
 * that share the same grid, then the walking triangle algorithm and the
 * calculation of geographic interpolation coefficients do not have to be
 * repeated.  The radial interpolation information is updated by this
 * method as well so after the call to this method, GeoTessPosition is ready
 * to interpolate values from the new model.
 *
 * @param newModel
 *            model that is to replace the currently supported model.
 * @throws GeoTessException
 *             if the new model and current model do not have GeoTessGrids
 *             that have the same gridID.
 */
void GeoTessPosition::setModel(GeoTessModel* newModel)
{
	if (newModel->getGrid().getGridID() != grid.getGridID())
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessPosition::setModel" << endl
				<< "Specified model and current model use different grids." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 3004);
	}

	model = newModel;
	modlProfiles = model->getProfiles();

	layerTessIds = model->getMetaData().getLayerTessIds();
	nLayers = model->getMetaData().getNLayers();

	double r = radius;

	layerRadii.clear();
	radius = -1.;
	for (int i = 0; i <= nLayers; ++i)
		layerRadii.push_back(-1);

	updateRadius(layerId, r);
}

/**
 * Returns true if none of the current interpolation vertices have empty profiles.
 */
bool GeoTessPosition::noEmptyProfiles()
{
	vector<int>& v = vertices[tessid];
	for (int i = 0; i < (int) v.size(); ++i)
		if (modlProfiles[v[i]][layerId]->getType() == GeoTessProfileType::EMPTY)
			return false;
	return true;
}

/**
 * Set the 2D position and ensure that the triangle, vertices and 2D
 * interpolation coefficients for the tessellation that supports layerid
 * have been updated. Sets radius = -1.  Must call updateRadius after
 * this call to ensure that radius is set properly.
 *
 * @param lid
 * @param uVector
 * @throws GeoTessException
 */
void GeoTessPosition::updatePosition2D(int lid, const double* const uVector)
{
	tessid  = layerTessIds[lid];

	if ((triangle[tessid] < 0) || (unitVector[0] != uVector[0]) ||
			(unitVector[1] != uVector[1]) || (unitVector[2] != uVector[2]))
	{
		// the vector position has changed. Update everything.

		// nullify the triangle index for all tessellations other than the
		// current one.
		int ntess = grid.getNTessellations();
		for (int tess = 0; tess < ntess; ++tess)
			if (tess != tessid)
				triangle[tess] = -1;

		// 0.961261696 is cos(16 degrees)
		// if new position is more than 16 degrees away from current
		// position then start walk from triangle zero, otherwise,
		// start walk from current triangle
		if ((triangle[tessid] < 0) || GeoTessUtils::dot(uVector, unitVector) < 0.961261696)
		{
			triangle[tessid] = grid.getTriangle(tessid, 0, 0);
			tessLevels[tessid] = 0;
		}

		unitVector[0] = uVector[0];
		unitVector[1] = uVector[1];
		unitVector[2] = uVector[2];

		// perform walking triangle algorithm, which will set
		// triangle[tessid], lienarCoefficients[tessid] and tessLevels[tessid]
		getContainingTriangle(tessid);

		// determine vertices and coefficients for interpolation in geographic
		// dimensions. This is an abstract method so the results depend on the
		// interpolator type.
		update2D(tessid);

		// nullify previously computed earth radius.
		earthRadius = -1;

		// set current radius to -1, forcing recalculation of radial
		// interpolation coefficients after this method is done.
		radius = -1.;

		// nullify all previously computed layer radii (layer boundaries).
		for (int i = 0; i < (int) layerRadii.size(); ++i) layerRadii[i] = -1.0;

		clearRadialCoefficients();
	}
	else
		// the 2D position did not change but the layerId/tessid might have.
		checkTessellation(tessid);
}

/**
 * Find the index of the triangle that contains position. Also computes the 3 interpolation
 * coefficients that can be used to interpolate data stored on the vertices of the returned
 * triangle.
 * <p>
 * A GeoTessPosition object has an attribute maxTessLevel that defaults to Integer.maxValue. The
 * search is limited to that tessellation level. So the triangle identified by this method will
 * reside either on x.maxTessLevel, or the largest tessellation level of the tessellation,
 * whichever is larger.
 */
void GeoTessPosition::getContainingTriangle(int tid)
{
	int t = triangle[tid];
	int tessLevel = tessLevels[tid];
	vector<double>& c = linearCoefficients[tid];
	int maxTess = maxTessLevel[tid];

	while (true)
	{
		c[0] = GeoTessUtils::dot(gridEdges[t][0]->normal, unitVector);
		if (c[0] > GeoTessPosition::TWALK_TOLERANCE)
		{
			c[1] = GeoTessUtils::dot(gridEdges[t][1]->normal, unitVector);
			if (c[1] > GeoTessPosition::TWALK_TOLERANCE)
			{
				c[2] = GeoTessUtils::dot(gridEdges[t][2]->normal, unitVector);
				if (c[2] > GeoTessPosition::TWALK_TOLERANCE)
				{
					if ((gridDescendants[t] < 0) || (tessLevel >= maxTess))
					{
						// the correct triangle has been found.
						// Normalize the coefficients such that they sum to one.
						double sum =	c[0] + c[1] + c[2];
						c[0] /= sum; c[1] /= sum; c[2] /= sum;
						triangle[tid] = t;
						tessLevels[tid] = tessLevel;
						return;
					}
					else { ++tessLevel; t = gridDescendants[t]; }
				}
				else t = gridEdges[t][2]->tLeft;
			}
			else t = gridEdges[t][1]->tLeft;
		}
		else t = gridEdges[t][0]->tLeft;
	}
}

/**
 * Retrieve an interpolated value of the specified model attribute.
 *
 * @param attribute
 * @return interpolated value of the specified model attribute.
 * @throws GeoTessException
 */
double GeoTessPosition::getValue(int attribute)
{
	vector<int>& v = vertices[tessid];
	vector<double>& h = hCoefficients[tessid];

	double value = 0;
	if (&radialInterpolatorType == &GeoTessInterpolatorType::CUBIC_SPLINE)
		for (int i = 0; i < (int)v.size(); ++i)
			value += modlProfiles[v[i]][layerId]->getValue(radialInterpolatorType, attribute,
					radius, radiusOutOfRangeAllowed) * h[i];
	else
	{
		updateRadialCoefficients();
		for (int i = 0; i < (int)v.size(); ++i)
			value += h[i] * modlProfiles[v[i]][layerId]->getValue(
					radialIndexes[i], radialCoefficients[i], attribute);
	}

	return isnan(value) ? getErrorValue() : value;
}

/**
 * Retrieve an interpolated value of the radius of the top of the specified layer, in km.
 *
 * @param layer
 * @return interpolated value of the radius of the top of the specified layer, in km.
 * @throws GeoTessException
 */
double GeoTessPosition::getRadiusTop(int layer)
{
	if (layerRadii[layer + 1] < 0)
	{
		int tid = layerTessIds[layer];

		if (layer < nLayers-1 && layerTessIds[layer+1] != tid)
		{
			// the next layer above the current layer is in a different
			// multi-level tessellation.  The containing triangle in the next layer
			// may be smaller and provide a more accurate estimate of
			// the radius at current position.  Have to evaluate this.

			int tid2 = layerTessIds[layer+1];

			int t1 = getTriangle(tid);
			int t2 = getTriangle(tid2);
			if (biggerTriangle(t1, t2) == t1)
				// triangle on next layer is smaller than triangle on current layer
				tid = tid2;
		}

		vector<int>& v = vertices[tid];
		vector<double>& c = hCoefficients[tid];
		layerRadii[layer + 1]=0.;
		for (int i = 0; i < (int) v.size(); ++i)
			layerRadii[layer + 1] += modlProfiles[v[i]][layer]->getRadiusTop() * c[i];
	}
	return isnan(layerRadii[layer + 1]) ? getErrorValue() : layerRadii[layer + 1];
}

/**
 * Retrieve an interpolated value of the radius of the bottom of the specified layer, in km.
 *
 * @param layer
 * @return interpolated value of the radius of the bottom of the specified layer, in km.
 * @throws GeoTessException
 */
double GeoTessPosition::getRadiusBottom(int layer)
{
	if (layerRadii[layer] < 0)
	{
		int tid = layerTessIds[layer];

		if (layer > 0 && layerTessIds[layer-1] != tid)
		{
			// the layer below the current layer is in a different
			// multi-level tessellation.  The containing triangle in the previous layer
			// may be smaller and provide a more accurate estimate of
			// the radius at current position.  Have to evaluate this.

			int tid2 = layerTessIds[layer-1];

			int t1 = getTriangle(tid);
			int t2 = getTriangle(tid2);
			if (biggerTriangle(t1, t2) == t1)
				// triangle on previous layer is smaller than triangle on current layer
				tid = tid2;
		}

		vector<int>& v = vertices[tid];
		vector<double>& c = hCoefficients[tid];
		layerRadii[layer]=0.;
		for (int i = 0; i < (int) v.size(); ++i)
			layerRadii[layer] += modlProfiles[v[i]][layer]->getRadiusBottom() * c[i];
	}
	return isnan(layerRadii[layer]) ? getErrorValue() : layerRadii[layer];
}

/**
 * Retrieve the index of the vertex with the highest interpolation
 * coefficient.
 */
int	GeoTessPosition::getIndexOfClosestVertex() const
{
	const vector<double>& c = hCoefficients[tessid];
	int vertex = 0;
	for (int i = 1;  i < (int) c.size(); ++i)
		if (c[i] > c[vertex])	vertex = i;
	return vertices[tessid][vertex];
}

/**
 * Output this GeoTessPosition as a string.
 */
string GeoTessPosition::toString()
{
	char s[300];
	sprintf(s, "Triangle %7d layer %2d  lat, lon, depth: %1.6f, %1.6f, %1.3f",
			triangle[tessid], layerId,
			GeoTessUtils::getLatDegrees(unitVector),
			GeoTessUtils::getLonDegrees(unitVector), getDepth());
	ostringstream os;
	os << endl << s << endl << endl;
	os << "Vertex       Lat        Lon      Coeff    Dist" << endl;

	vector<int>& v = vertices[tessid];
	vector<double>& c = hCoefficients[tessid];
	for (int i = 0; i < (int) v.size(); ++i)
	{
		string lls = GeoTessUtils::getLatLonString(grid.getVertex(v[i]));
		sprintf(s, "%6d %s %10.6f %7.3f", v[i], lls.c_str(),
				c[i], CPPUtils::toDegrees(GeoTessUtils::angle(unitVector,
						grid.getVertex(v[i]))));
		os << s << endl;
	}
	return os.str();
}

} // end namespace geotess
