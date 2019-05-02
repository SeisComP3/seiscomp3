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

#ifndef HORIZONLAYER_H_
#define HORIZONLAYER_H_

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
 * \brief Defines a "surface" in a model that resides at a constant fractional
 * radius within a specified layer.
 *
 * Defines a "surface" in a model that resides at a constant fractional
 * radius within a specified layer.
 *
 * @author sballar
 *
 */
class GEOTESS_EXP_IMP GeoTessHorizonLayer : public GeoTessHorizon
{

private:

	/**
	 * The fractional position in the layer.
	 * 0 is the bottom, 1 is the top.
	 */
	double fraction;

public:

	/**
	 * Constructor for a Horizon object that represents a constant
	 * fractional radius within a layer.  Zero represents the bottom of
	 * the layer and 1. represents the top of the layer.
	 * <p>Since the layerIndex is specified, the radius will be
	 * constrained to be within the specified layer.
	 * @param fractionalPosition radius within the Earth, in km.
	 * @param lyrIndex the index of the layer within which
	 * the radius will be constrained.
	 */
	GeoTessHorizonLayer(const double& fractionalPosition, const int& lyrIndex)
	: GeoTessHorizon(lyrIndex), fraction(fractionalPosition)
	{};

	/**
	 * Copy constructor.
	 */
	GeoTessHorizonLayer(GeoTessHorizonLayer& other) : GeoTessHorizon(other.getLayerIndex()), fraction(other.fraction)
	{
	}

	/**
	 * Overloaded assignment operator
	 */
	GeoTessHorizonLayer& operator=(const GeoTessHorizonLayer& other)
	{
		layerIndex = other.layerIndex;
		fraction = other.fraction;
		return *this;
	}

	/**
	 * Destructor.
	 */
	virtual ~GeoTessHorizonLayer() {};

	virtual double getValue() { return fraction; };

	virtual double getRadius(const double* position, GeoTessProfile** profiles)
	{
		double bottom = profiles[layerIndex]->getRadiusBottom();
		if (fraction <= 0. )
			return bottom;
		double top = profiles[layerIndex]->getRadiusTop();
		if (fraction >= 1.)
			return top;
		return bottom + fraction*(top-bottom);
	}

	virtual string class_name() { return "HorizonLayer"; };

	virtual double getRadius(GeoTessPosition& position)
	{
		double bottom = position.getRadiusBottom(layerIndex);
		if (fraction <= 0. )
			return bottom;
		double top = position.getRadiusTop(layerIndex);
		if (fraction >= 1.)
			return top;
		return bottom + fraction*(top-bottom);
	}

	virtual string str()
	{
		string s = "layer " + CPPUtils::dtos(fraction) + " " + CPPUtils::itos(layerIndex);
		return s;
	}

}; // end class DataValue

} // end namespace geotess

#endif /* HORIZONLAYER_H_ */
