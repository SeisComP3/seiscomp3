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

#include <cmath>
#include <sstream>

// **** _LOCAL INCLUDES_ *******************************************************

#include "GeoTessModelUtils.h"
#include "GeoTessModel.h"
#include "GeoTessMetaData.h"
#include "GeoTessPosition.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess
{

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

/**
 * Evaluates maximum number of nodes per layer. On input, pointsPerLayer is an array of length
 * nLayers where each element contains a current estimate of the number of nodes that must be
 * deployed on the corresponding layer so that the node spacing will be no greater than
 * maxSpacing (in km). The values in pointsPerLayer will be evaluated at the specified position
 * and increased if necessary. Only layers between firstLayer and lastLayer, inclusive, will be
 * evaluated.
 * 
 * @param firstLayer first layer to be considered
 * @param lastLayer last layer to be considered
 * @param maxSpacing maximum radial node spacing in km
 * @param pointsPerLayer number of nodes per layer such that the node spacing will be no larger
 *            than maxSpacing.
 * @return returns the total number of points in all layers, i.e., the sum of the elements of
 *         pointsPerLayer.
 * @throws GeoTessException 
 */
int GeoTessModelUtils::updatePointsPerLayer(GeoTessPosition& pos, int firstLayer,
																						int lastLayer, double maxSpacing,
																						vector<int>& pointsPerLayer)
{
	int nLayers = pos.getModel()->getNLayers();
	int n;
	int nTotal = 0;
	for (int i = 0; i < nLayers; ++i)
	{
		if (i >= firstLayer && i <= lastLayer)
		{
			n = 1 + (int) ceil(pos.getLayerThickness(i) / maxSpacing);
			if (n < 2) n = 2;
			if (n > pointsPerLayer[i])
				pointsPerLayer[i] = n;
		}

		nTotal += pointsPerLayer[i];
	}

	return nTotal;
}

/**
 * Retrieve a map of attribute values at a constant specified depth on a
 * regular latitude-longitude grid. For every point on the map, if the
 * specified depth is greater than the depth of the bottom of the specified
 * layer, then attribute values are interpolated at the bottom of the
 * specified layer. Similarly, if the specified depth is less than the depth
 * of the top of the specified layer, then attribute values at the top of
 * the specified layer are interpolated.
 *
 * @param model the GeoTessModel to be interrogated.
 * @param latitudes  array of latitude values in degrees.
 * @param longitudes array of longitude values in degrees.
 * @param layerId layer index
 * @param depth the depth at which samples should be interpolated, in km.
 * @param interpType either InterpolatorType.LINEAR or
 *            InterpolatorType.NATURAL_NEIGHBOR
 * @param reciprocal if false, return value; if true, return 1./value.
 * @param attributes index(es) of the attributes to interpolate.
 * @return double[nlat][nlon][nAttributes]
 * @throws GeoTessException
 */
void GeoTessModelUtils::getMapValuesDepth(GeoTessModel& model,
		vector<double>& latitudes, vector<double>& longitudes, int layerId,
		double depth, const GeoTessInterpolatorType& horizontalType,
		const GeoTessInterpolatorType& radialType, bool reciprocal,
		vector<int>& attributes, vector<vector<vector<double> > >& values)
{
	int nlat = latitudes.size();
	int nlon = longitudes.size();

	GeoTessPosition* pos = model.getPosition(horizontalType, radialType);

	values.resize(nlat);
	for (int i = 0; i < nlat; ++i)
	{
		values[i].resize(nlon);
		for (int j = 0; j < nlon; ++j)
		{
			values[i][j].resize(attributes.size());
			pos->set(layerId, latitudes[i], longitudes[j], depth);
			for (int k = 0; k < (int) attributes.size(); ++k)
				values[i][j][k] =
						reciprocal ?
								1. / pos->getValue(attributes[k]) :
								pos->getValue(attributes[k]);
		}
	}

	delete pos;
}

/**
 * Retrieve a map of attribute values at some fractional thickness with
 * a layer.  FractionalRadius <= 0. will result in samples at the bottom
 * of the layer while fractionalRadius >= 1. will result in samples at
 * the top of the layer.
 *
 * @param model the GeoTessModel to be interrogated.
 * @param latitudes array of latitude values in degrees.
 * @param longitudes array of longitude values in degrees.
 * @param layerId layer index
 * @param fractionalRadius the fractional radius within the layer at
 * which samples should be interpolated.
 * @param interpType either InterpolatorType.LINEAR or
 *            InterpolatorType.NATURAL_NEIGHBOR
 * @param reciprocal if false, return value; if true, return 1./value.
 * @param attributes index(es) of the attributes to interpolate.
 * @return double[nlat][nlon][nAttributes]
 * @throws GeoTessException
 */
void GeoTessModelUtils::getMapValuesLayer(GeoTessModel& model,
		vector<double>& latitudes, vector<double>& longitudes, int layerId,
		double fractionalRadius, const GeoTessInterpolatorType& horizontalType,
		const GeoTessInterpolatorType& radialType, bool reciprocal,
		vector<int>& attributes, vector<vector<vector<double> > >& values)
{
	int nlat = latitudes.size();
	int nlon = longitudes.size();

	GeoTessPosition* pos = model.getPosition(horizontalType, radialType);

	values.resize(nlat);
	for (int i = 0; i < nlat; ++i)
	{
		values[i].resize(nlon);
		for (int j = 0; j < nlon; ++j)
		{
			values[i][j].resize(attributes.size());
			pos->set(layerId, latitudes[i], longitudes[j], 0.);
			pos->setRadius(
					layerId,
					pos->getRadiusBottom()
							+ (fractionalRadius * pos->getLayerThickness()));
			for (int k = 0; k < (int) attributes.size(); ++k)
				values[i][j][k] =
						reciprocal ?
								1. / pos->getValue(attributes[k]) :
								pos->getValue(attributes[k]);
		}
	}

	delete pos;
}

void GeoTessModelUtils::getMapLayerBoundary(GeoTessModel& model,
		vector<double>& latitudes, vector<double>& longitudes, int layerId,
		bool top, bool convertToDepth, const GeoTessInterpolatorType& horizontalType,
		const GeoTessInterpolatorType& radialType,
		vector<vector<double> >& values)
{
	int nlat = latitudes.size();
	int nlon = longitudes.size();
	double u[3];
	values.resize(nlat);

	GeoTessPosition* pos = model.getPosition(horizontalType, radialType);

	for (int i = 0; i < nlat; ++i)
	{
		values[i].resize(nlon);
		for (int j = 0; j < nlon; ++j)
		{
			GeoTessUtils::getVectorDegrees(latitudes[i], longitudes[j], u);
			if (top)
				pos->setTop(layerId, u);
			else
				pos->setBottom(layerId, u);

			values[i][j] = convertToDepth ? pos->getDepth() : pos->getRadius();
		}
	}

	delete pos;
}

/**
 * Retrieve interpolated attribute values along a radial 'borehole' at the specified position.
 * 
 * @param pos the geographic position where the profile is desired.
 * @param maxSpacing maximum radial spacing in km of points along the radial profile. Actual
 *            radial spacing will generally be somewhat less than the requested value so that
 *            there will be an integral number of equally spaced points along the profile.
 * @param firstLayer index of first layer to be evaluated.
 * @param lastLayer index of last layer to be evaluated, plus 1.
 * @param reciprocal if false, return value; if true, return 1./value.
 * @param attributes index(es) of the attributes to interpolate. If omitted, all attributes are
 *            reported.
 * @return double[nPoints][nAttributes+1]. Points will be evenly spaced within each layer, with
 *         two points on each layer boundary, one associated with values for the top of the
 *         layer below the boundary and the other associated with values for the bottom of the
 *         layer above the boundary. The first element of each attribute array is either the
 *         radius or depth of the corresponding point. Subsequent elements are interpolated
 *         values of the attributes.
 * @throws GeoTessException
 */
string GeoTessModelUtils::getBoreholeString(GeoTessPosition& pos,
		double maxSpacing, int firstLayer, int lastLayer, bool convertToDepth,
		bool reciprocal, vector<int>& attributes)
{
	ostringstream os;

	vector<vector<double> > borehole;

	getBorehole(pos, maxSpacing, firstLayer, lastLayer,
			convertToDepth, reciprocal, attributes, borehole);

	char buf[1000];
	for (int i = 0; i < (int) borehole.size(); ++i)
	{
		sprintf(buf, " %9.3f", borehole[i][0]);
		os << buf;
		for (int j = 1; j < (int) attributes.size() + 1; ++j)
		{
			sprintf(buf, " %10.7g", borehole[i][j]);
			os << buf;
		}
		os << endl;
	}
	return os.str();
}

/**
 * Retrieve interpolated attribute values along a radial 'borehole' at the specified 
 * latitude and longitude, in degrees.
 */
string GeoTessModelUtils::getBoreholeString(GeoTessModel& model, double lat, double lon)
{
	double maxSpacing = 1e6;
	int firstLayer = 0;
	int lastLayer = model.getNLayers()-1;
	bool convertToDepth = true;
	bool reciprocal = false;
	vector<int> attributes;
	for (int i=0; i<model.getMetaData().getNAttributes(); ++i)
		attributes.push_back(i);

	GeoTessPosition* pos = model.getPosition();
	pos->set(lat, lon, 0.);

	string s = GeoTessModelUtils::getBoreholeString(*pos, maxSpacing, firstLayer, lastLayer, 
		convertToDepth, reciprocal, attributes);

	delete pos;

	return s;
}

/**
 * Retrieve interpolated attribute values along a radial 'borehole' at the specified position.
 * 
 * @param pos GeoTessPosition object where attribute values are to be interpolated
 * @param maxSpacing maximum radial spacing in km of points along the radial profile. Actual
 *            radial spacing will generally be somewhat less than the requested value so that
 *            there will be an integral number of equally spaced points along the profile.
 * @param convertToDepth if false, radii are retured. If true, depths are returned.
 * @param invert if false, return value; if true, return 1./value.
 * @param attributes index(es) of the attributes to interpolate. If omitted, all attributes are
 *            reported.
 * @return double[nPoints][nAttributes+1]. Points will be evenly spaced within each layer, with
 *         two points on each layer boundary, one associated with values for the top of the
 *         layer below the boundary and the other associated with values for the bottom of the
 *         layer above the boundary. The first element of each attribute array is either the
 *         radius or depth of the corresponding point. Subsequent elements are interpolated
 *         values of the attributes.
 * @throws GeoTessException
 */
void GeoTessModelUtils::getBorehole(GeoTessPosition& pos, double maxSpacing,
		int firstLayer, int lastLayer, bool convertToDepth, bool reciprocal,
		vector<int>& attributes, vector<vector<double> >& borehole)
{
	vector<int> ppl(pos.getModel()->getNLayers(), 0);

	updatePointsPerLayer(pos, firstLayer, lastLayer, maxSpacing, ppl);

	getBorehole(pos, ppl, convertToDepth, reciprocal, attributes, borehole);
}

/**
 * Retrieve interpolated attribute values along a radial 'borehole' at the specified position.
 * 
 * @param pos GeoTessPosition object where attribute values are to be interpolated
 * @param pointsPerLayer an int[] of length nLayers where each element specifies the number of
 *            nodes that are to be generated in that layer.
 * @param convertToDepth if false, the radii are retured. If true, depths are returned.
 * @param reciprocal if false, return value; if true, return 1./value.
 * @param attributes index(es) of the attributes to interpolate. If omitted, all attributes are
 *            reported.
 * @return double[nPoints][nAttributes+1]. Points will be evenly spaced within each layer, with
 *         two points on each layer boundary, one associated with values for the top of the
 *         layer below the boundary and the other associated with values for the bottom of the
 *         layer above the boundary. The first element of each attribute array is either the
 *         radius or depth of the corresponding point. Subsequent elements are interpolated
 *         values of the attributes.
 * @throws GeoTessException
 */
void GeoTessModelUtils::getBorehole(GeoTessPosition& pos, vector<int>& pointsPerLayer,
																				bool convertToDepth, bool invert,
																				const vector<int>& attributes,
																				vector<vector<double> >& borehole)
{
	int nPoints = 0;
	for (int i=0; i<(int)pointsPerLayer.size(); ++i)
		nPoints += pointsPerLayer[i];

	borehole.resize(nPoints);

	int nLayers = pos.getModel()->getNLayers();

	int index = 0;
	for (int layer = 0; layer < nLayers; ++layer)
	{
		if (pointsPerLayer[layer] >= 2)
		{
			double dr = pos.getLayerThickness(layer)
					/ (pointsPerLayer[layer] - 1);
			for (int p = 0; p < pointsPerLayer[layer]; ++p)
			{
				borehole[index].resize(attributes.size()+1);

				pos.setRadius(layer, p * dr + pos.getRadiusBottom(layer));
				borehole[index][0] =
						convertToDepth ? pos.getDepth() : pos.getRadius();
				for (int a = 0; a < (int) attributes.size(); ++a)
					borehole[index][a + 1] =	invert ?
																	1.0 / pos.getValue(attributes[a]) :
																	pos.getValue(attributes[a]);
				++index;
			}
		}
	}

	double tmp;
	if (convertToDepth)
	{
		for (int i = 0; i < (int)borehole.size() / 2; ++i)
		{
			int j = borehole.size() - 1 - i;
			for (int t=0; t < (int)borehole[i].size(); ++t)
			{
				tmp = borehole[i][t];
				borehole[i][t] = borehole[j][t];
				borehole[j][t] = tmp;
			}
		}
	}
}

/**
 * Retrieve attribute values interpolated on a vertical slice through a
 * model.
 *
 * @param model the GeoTessModel from which slice will be extracted
 * @param x0 unit vector specifying first point of great circle path
 * @param x1 unit vector specifying last point of great circle path
 * @param nx number of points along great circle path
 * @param maxRadialSpacing radial spacing of points will be less
 * than or equal to this value (km).
 * @param firstLayer index of the first layer to include (deepest)
 * @param lastLayer index of the last layer to include (shallowest)
 * @param interpType either InterpolatorType.LINEAR or
 *            InterpolatorType.NATURAL_NEIGHBOR
 * @param spatialCoordinates coordinate values to be output along with requested model
 *            attributes. A comma delineated String containing a subset of
 *            the following strings in any order:
 *            <ol start=0>
 *            <li>distance -- distance in degrees from x0
 *            <li>depth -- depth in km
 *            <li>radius -- radius in km
 *            <li>x -- observer's 'right' in km
 *            <li>y -- observer's 'up' in km
 *            <li>z -- direction pointing toward the observer, in km
 *            <li>lat -- latitude in degrees
 *            <li>lon -- longitude in degrees
 *            </ol>
 *
 * @param reciprocal
 *            if false, return value; if true, return 1./value.
 * @param attributes
 *            index(es) of the attributes to interpolate. If omitted, all
 *            attributes are reported.
 * @return double[nx][nPoints][spatialCoordinates.length + nAttributes]. The
 *         values of spatial cooridinates will be output first, followed
 *         attribute values. Points will be evenly spaced radially within
 *         each layer, with two points on each layer boundary, one
 *         associated with values for the top of the layer below the
 *         boundary and the other associated with values for the bottom of
 *         the layer above the boundary. The first element of each attribute
 *         array is either the radius or depth of the corresponding point.
 *         Subsequent elements are interpolated values of the attributes.
 * @throws Exception
 */
void GeoTessModelUtils::getSlice(GeoTessModel& model, const double* const x0, const double* const x1,
		               int nx, double maxRadialSpacing, int firstLayer,
		               int lastLayer, const GeoTessInterpolatorType& horizontalType,
		               const GeoTessInterpolatorType& radialType,
		               const string& spatialCoordinates, bool reciprocal,
		               const vector<int>& attributes, vector<vector<vector<double> > >& transect)
{
	int nLayers = model.getNLayers();

	if (lastLayer >= nLayers) lastLayer = nLayers - 1;

	vector<string> coordinates;
	CPPUtils::tokenizeString(spatialCoordinates, ",", coordinates);

	vector<int> pointsPerLayer;
	pointsPerLayer.resize(nLayers, 0);

	// delta is total distance in radians between x0 and x1
	double delta = GeoTessUtils::angle(x0, x1);

	// dx is path increment that yields nx equally spaced points
	// along the great circle (radians).
	double dx = delta / (nx - 1);

	// find great circle, which consists of two unit vectors.
	// First one is a copy of x0, and second one is on the
	// great circle defined by x0 and x1 but located PI/2 radians
	// away from x0.
	double gcs[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	double* greatCircle[2] = {&gcs[0], &gcs[3]};
	GeoTessUtils::getGreatCircle(x0, x1, greatCircle);

	double tf[9] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	double* transform[3] = {&tf[0], &tf[3], &tf[6]};
	for (int i = 0; i < (int) coordinates.size(); ++i)
	{
		string s = CPPUtils::trim(CPPUtils::lowercase_string(coordinates[i]), " ");
		if ((s == "x") || (s == "y") || (s == "z"))
		{
			GeoTessUtils::getTransform(x0, x1, transform);
		}
	}

	// intantiate a GeoTessPosition object to use for interpolation.
	GeoTessPosition* pos = model.getPosition(horizontalType, radialType);

	double u[3], g[3], tloc[3];

	// loop over points along great circle and figure out how many
	// nodes are required in each layer so that (1) the number of
	// nodes in a given layer will be constant along the slice, and
	// (2) the radial node spacing in a given layer will not exceed
	// maxSpacing.
	for (int i = 0; i < nx; ++i)
	{
		// find unit vector for current point.
		GeoTessUtils::getGreatCirclePoint(greatCircle, i * dx, u);

		// loop over the requested layers
		for (int j = firstLayer; j <= lastLayer; ++j)
		{
			// set the interpolation point
			pos->setTop(j, u);

			// update pointsPerLayer
			updatePointsPerLayer(*pos, j, j, maxRadialSpacing, pointsPerLayer);
		}
	}

	vector<double> output;
	int layerid = nLayers - 1;

	// loop over all the points along the great circle and populate the
	// data values.
	for (int i = 0; i < nx; ++i)
	{
		double distance = i * dx; // radians

		// find unit vector for current point.
		GeoTessUtils::getGreatCirclePoint(greatCircle, distance, u);

		// set the interpolation point
		pos->setTop(layerid, u);

		// get borehole at this position. First element is radius,
		// followed by attribute values.
		getBorehole(*pos, pointsPerLayer, false, reciprocal, attributes, transect[i]);

		for (int j = 0; j < (int)transect[i].size(); ++j)
		{
			pos->setRadius(layerid, transect[i][j][0]);

			// convert current position from unit vector to full vector
			pos->copyVector(tloc);
			for (int k = 0; k < 3; ++k)	tloc[k] *= pos->getRadius();
			// apply transform to tloc and put resulting vector in g
			GeoTessUtils::transform(tloc, transform, g);

			for (int k = 0; k < (int) coordinates.size(); ++k)
			{
				string coord = CPPUtils::trim(CPPUtils::lowercase_string(coordinates[i]), " ");
				if (coord == "x")
					output.push_back(g[0]);
				else if (coord == "y")
					output.push_back(g[1]);
				else if (coord == "z")
					output.push_back(g[2]);
				else if (coord == "distance")
					output.push_back(CPPUtils::toDegrees(distance));
				else if (coord == "depth")
					output.push_back(pos->getDepth());
				else if (coord == "radius")
					output.push_back(pos->getRadius());
				else if (coord == "lat")
					output.push_back(GeoTessUtils::getLatDegrees(pos->getVector()));
				else if (coord == "lon")
					output.push_back(GeoTessUtils::getLonDegrees(pos->getVector()));
				else
					output.push_back(NA_VALUE);
			}
			for (int k = 1; k < (int) attributes.size() + 1; ++k)
				output.push_back(transect[i][j][k]);

			transect[i][j].clear();
			for (int k=0; i< (int) output.size(); ++k)
				transect[i][j].push_back(output[k]);
			output.clear();
		}

	}
}

} // end namespace geotess
