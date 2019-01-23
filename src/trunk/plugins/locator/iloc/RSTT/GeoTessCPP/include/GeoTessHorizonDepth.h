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

#ifndef HORIZONDEPTH_H_
#define HORIZONDEPTH_H_

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <fstream>
#include <string>
#include <climits>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"
#include "GeoTessHorizon.h"
#include "GeoTessProfile.h"
#include "GeoTessPosition.h"

// **** _BEGIN GEOTESS NAMESPACE_ *********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Defines a "surface" in a model that resides at a constant depth.
 *
 * Defines a "surface" in a model that resides at a constant depth.
 *
 * @author sballar
 *
 */
class GEOTESS_EXP_IMP GeoTessHorizonDepth : public GeoTessHorizon
{

private:

	/**
	 * The depth in the model, in km.
	 */
	double depth;

public:

	/**
	 * Constructor for a Horizon object that represents a constant
	 * depth within the Earth.  Units are km below the surface of the
	 * GRS80 ellipsoid.
	 * <p>Since the layerIndex is not specified, the depth is not
	 * constrained to be within any particular layer.
	 * @param dpth depth in km below the surface of the GRS80 ellipsoid.
	 */
	GeoTessHorizonDepth(const double& dpth) : GeoTessHorizon(-1), depth(dpth)
	{};

	/**
	 * Constructor for a Horizon object that represents a constant
	 * depth in the Earth, in km.  Depth is measured relative to the
	 * surface of the WGS84 ellipsoid in km.
	 * <p>Since the layerIndex is specified, the depth will be
	 * constrained to be within the specified layer.
	 * @param dpth depth below the surface of WGS84 ellipsoid, in km.
	 * @param lyrIndex the index of the layer within which
	 * the radius will be constrained.
	 */
	GeoTessHorizonDepth(const double& dpth, const int& lyrIndex) : GeoTessHorizon(lyrIndex), depth(dpth)
	{};

	/**
	 * Destructor.
	 */
	virtual ~GeoTessHorizonDepth() {};

	/**
	 * Copy constructor.
	 */
	GeoTessHorizonDepth(GeoTessHorizonDepth& other) : GeoTessHorizon(other.getLayerIndex()), depth(other.depth)
	{
	}

	/**
	 * Overloaded assignment operator
	 */
	GeoTessHorizonDepth& operator=(const GeoTessHorizonDepth& other)
	{
		layerIndex = other.layerIndex;
		depth = other.depth;
		return *this;
	}

	virtual string class_name() { return "HorizonDepth"; };

	virtual double getValue() { return depth; };

	virtual double getRadius(const double* position, GeoTessProfile** profiles)
	{
		double radius = GeoTessUtils::getEarthRadius(position)-depth;

		if (layerIndex < 0)
			return radius;
		double bottom = profiles[layerIndex]->getRadiusBottom();
		if (radius <= bottom)
			return bottom;
		double top = profiles[layerIndex]->getRadiusTop();
		if (radius >= top)
			return top;
		return radius;
	}

	virtual double getRadius(GeoTessPosition& position)
	{
		double radius = position.getEarthRadius()-depth;
		if (layerIndex < 0)
			return radius;
		double bottom = position.getRadiusBottom(layerIndex);
		if (radius <= bottom)
			return bottom;
		double top = position.getRadiusTop(layerIndex);
		if (radius >= top)
			return top;
		return radius;
	}

	virtual string str()
	{
		string s = "depth " + CPPUtils::dtos(depth) + " " + CPPUtils::itos(layerIndex);
		return s;
	}

}; // end class DataValue

} // end namespace geotess

#endif /* HORIZONDEPTH_H_ */
