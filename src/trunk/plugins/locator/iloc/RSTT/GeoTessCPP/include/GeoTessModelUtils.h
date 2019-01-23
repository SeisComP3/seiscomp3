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

#ifndef GEOTESSMODELUTILS_OBJECT_H
#define GEOTESSMODELUTILS_OBJECT_H

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
#include "GeoTessInterpolatorType.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

class	GeoTessModel;
class GeoTessPosition;

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief A collection of static utilities that extract organized information from a GeoTessModel.
 *
 * A collection of static utilities that extract organized information from a GeoTessModel. There
 * are utilities to retrieve:
 * <ul>
 * <li>a map of attribute values at a constant specified depth on a regular latitude-longitude grid.
 * <li>a map of attribute values at top or bottom of a layer on a regular latitude-longitude grid.
 * <li>a map of the depth of the top or bottom of a specified layer on a regular latitude-longitude
 * grid.
 * <li>attribute values interpolated on a vertical slice through a model.
 * <li>attribute values along a radial 'borehole' at a specified position.
 * </ul>
 * 
 * @author sballar
 * 
 */
class GEOTESS_EXP_IMP GeoTessModelUtils
{
private:


public:

	GeoTessModelUtils() {};
	virtual							~GeoTessModelUtils() {};

	/**
	 * Evaluates maximum number of nodes per layer. On input, pointsPerLayer is an array of length
	 * nLayers where each element contains a current estimate of the number of nodes that must be
	 * deployed on the corresponding layer so that the node spacing will be no greater than
	 * maxSpacing (in km). The values in pointsPerLayer will be evaluated at the specified position
	 * and increased if necessary. Only layers between firstLayer and lastLayer, inclusive, will be
	 * evaluated.
	 */
	static int					updatePointsPerLayer(GeoTessPosition& pos, int firstLayer,
			int lastLayer, double maxSpacing,
			vector<int>& pointsPerLayer);

	/**
	 * Retrieve interpolated attribute values along a radial 'borehole' at the specified 
	 * latitude and longitude, in degrees.
	 */
	static string				getBoreholeString(GeoTessModel& pos, double lat, double lon);

	/**
	 * Retrieve interpolated attribute values along a radial 'borehole' at the specified position.
	 * @param pos the position where the borehole is to be generated
	 * @param maxSpacing the maximum radial spacing in km.  Actual spacing will generally somewhat
	 * less than this value so that an integer number of equally spaced nodes will span each layer.
	 * @param firstLayer
	 * @param lastLayer
	 * @param convertToDepth if true, depths are reported otherwise radii are reported.
	 * @param reciprocal if true, 1/value are reported, otherwise values are reported as-is.
	 * @param attributes the indexes of the attributes to include.
	 */
	static string				getBoreholeString(GeoTessPosition& pos, double maxSpacing,
			int firstLayer, int lastLayer, bool convertToDepth, bool reciprocal,
			vector<int>& attributes);

	/**
	 * Retrieve interpolated attribute values along a radial 'borehole' at the specified position.
	 */
	static void			getBorehole(GeoTessPosition& pos,
			double maxSpacing, int firstLayer,
			int lastLayer, bool convertToDepth,
			bool reciprocal, vector<int>& attributes,
			vector<vector<double> >& borehole);

	/**
	 * Retrieve interpolated attribute values along a radial 'borehole' at the specified position.
	 */
	static void			getBorehole(GeoTessPosition& pos,
			vector<int>& pointsPerLayer, bool convertToDepth,
			bool reciprocal, const vector<int>& attributes,
			vector<vector<double> >& borehole);

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
	 * @param latitudes array of latitude values in degrees.
	 * @param longitudes array of longitude values in degrees.
	 * @param layerId layer index
	 * @param depth the depth at which samples should be interpolated, in km.
	 * @param horizontalType either InterpolatorType.LINEAR or
	 *            InterpolatorType.NATURAL_NEIGHBOR
	 * @param radialType either InterpolatorType.LINEAR or
	 *            InterpolatorType.CUBIC_SPLINE
	 * @param reciprocal if false, return value; if true, return 1./value.
	 * @param attributes index(es) of the attributes to interpolate. If omitted, all
	 *            attributes are reported.
	 * @param values 3D vector of doubles: [nlat][nlon][nAttributes]
	 * @throws GeoTessException
	 */
	static void getMapValuesDepth(GeoTessModel& model,
			vector<double>& latitudes, vector<double>& longitudes, int layerId, double depth,
			const GeoTessInterpolatorType& horizontalType,
			const GeoTessInterpolatorType& radialType, bool reciprocal, vector<int>& attributes,
			vector<vector<vector<double> > >& values);

	static void getSlice(GeoTessModel& model, const double* const x0,
			const double* const x1, int nx,
			double maxRadialSpacing, int firstLayer,
			int lastLayer, const GeoTessInterpolatorType& horizontalType,
			const GeoTessInterpolatorType& radialType,
			const string& spatialCoordinates, bool reciprocal,
			const vector<int>& attributes,
			vector<vector<vector<double> > >& transect);

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
	 * @param horizontalType either InterpolatorType.LINEAR or
	 *            InterpolatorType.NATURAL_NEIGHBOR
	 * @param radialType either InterpolatorType.LINEAR or
	 *            InterpolatorType.CUBIC_SPLINE
	 * @param reciprocal if false, return value; if true, return 1./value.
	 * @param attributes index(es) of the attributes to interpolate.
	 * @param values a 3D array of doubles: [nlat][nlon][nAttributes]
	 * @throws GeoTessException
	 */
	static void getMapValuesLayer(GeoTessModel& model,
			vector<double>& latitudes, vector<double>& longitudes, int layerId, double fractionalRadius,
			const GeoTessInterpolatorType& horizontalType,
			const GeoTessInterpolatorType& radialType, bool reciprocal, vector<int>& attributes,
			vector<vector<vector<double> > >& values);

	/**
	 * Retrieve a map of the depth or radius of the top or bottom of a
	 * specified layer.
	 *
	 * @param model the GeoTessModel to be interrogated.
	 * @param latitudes array of latitude values in degrees.
	 * @param longitudes array of longitude values in degrees.
	 * @param layerId layer index
	 * @param top if true return top of layer otherwise bottom.
	 * @param convertToDepth if true, return depth, otherwise radius.
	 * @param horizontalType either InterpolatorType.LINEAR or
	 *            InterpolatorType.NATURAL_NEIGHBOR
	 * @param radialType either InterpolatorType.LINEAR or
	 *            InterpolatorType.CUBIC_SPLINE
	 * @param values 2D vector of values: [nlat][nlon]
	 * @throws GeoTessException
	 */
	static void getMapLayerBoundary(GeoTessModel& model,
			vector<double>& latitudes, vector<double>& longitudes, int layerId,
			bool top, bool convertToDepth, const GeoTessInterpolatorType& horizontalType,
			const GeoTessInterpolatorType& radialType,
			vector<vector<double> >& values);

}; // end class GeoTessModelUtils

} // end namespace geotess

#endif  // GEOTESSMODELUTILS_OBJECT_H
