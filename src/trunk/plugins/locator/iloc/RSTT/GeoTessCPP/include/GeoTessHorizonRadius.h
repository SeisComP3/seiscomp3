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

#ifndef HORIZONRADIUS_H_
#define HORIZONRADIUS_H_

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <fstream>
#include <string>
#include <climits>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "GeoTessHorizon.h"
#include "GeoTessProfile.h"
#include "GeoTessPosition.h"

// **** _BEGIN GEOTESS NAMESPACE_ *********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Defines a "surface" in a model that resides at a constant radius.
 *
 * Defines a "surface" in a model that resides at a constant radius.
 *
 * @author sballar
 *
 */
class GEOTESS_EXP_IMP GeoTessHorizonRadius : public GeoTessHorizon
{

private:

	/**
	 * The radius in the model, in km.
	 */
	double radius;

public:

	/**
	 * Constructor for a Horizon object that represents a constant
	 * radius within the Earth.  Units are km.
	 * <p>Since the layerIndex is not specified, the radius is not
	 * constrained to be within any particular layer.
	 * @param r radius in km.
	 */
	GeoTessHorizonRadius(const double& r) : GeoTessHorizon(-1), radius(r)
	{};

	/**
	 * Constructor for a Horizon object that represents a constant
	 * radius in the Earth, in km.
	 * <p>Since the layerIndex is specified, the radius will be
	 * constrained to be within the specified layer.
	 * @param r radius within the Earth, in km.
	 * @param lyrIndex the index of the layer within which
	 * the radius will be constrained.
	 */
	GeoTessHorizonRadius(const double& r, const int& lyrIndex) : GeoTessHorizon(lyrIndex), radius(r)
	{};

	/**
	 * Destructor.
	 */
	virtual ~GeoTessHorizonRadius() {};

	/**
	 * Copy constructor.
	 */
	GeoTessHorizonRadius(GeoTessHorizonRadius& other) : GeoTessHorizon(other.getLayerIndex()), radius(other.radius)
	{
	}

	/**
	 * Overloaded assignment operator
	 */
	GeoTessHorizonRadius& operator=(const GeoTessHorizonRadius& other)
	{
		layerIndex = other.layerIndex;
		radius = other.radius;
		return *this;
	}

	virtual string class_name() { return "HorizonRadius"; };

	virtual double getValue() { return radius; };

	virtual double getRadius(const double* position, GeoTessProfile** profiles)
	{
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
		string s = "radius " + CPPUtils::dtos(radius) + " " + CPPUtils::itos(layerIndex);
		return s;
	}

}; // end class DataValue

} // end namespace geotess

#endif /* HORIZONRADIUS_H_ */
