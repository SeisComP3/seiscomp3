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

#ifndef HORIZON_H_
#define HORIZON_H_
// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <string>
#include <fstream>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"
#include "CPPGlobals.h"
#include "GeoTessProfile.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

class GeoTessPosition;

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief An abstract class that represents a single "surface" within a
 * model.
 *
 * Horizon is an abstract class that represents a single "surface" within a
 * model. This might be a surface of constant radius, constant depth, the top or
 * bottom of a layer, etc. The surface can be constrained to a specified layer,
 * or can cross layer boundaries.  There are derived classes for each of these,
 * HorizonRadius, HorizonDepth, and HorizonLayer.  A Horizon class implements
 * a single basic function, getRadius().  That method can take either a
 * GeoTessPosition object or a vertex position and the 1D array of
 * Profiles associated with that vertex.
 *
 * @author sballar
 *
 */
class GEOTESS_EXP_IMP GeoTessHorizon
{

private:

protected:

	/**
	 * If layerIndex is >= 0 and < the number of layers represented in a model,
	 * then the returned radius will be constrained to be between the top and
	 * bottom of the specified layer.  Otherwise, the radius will not be
	 * so constrained.
	 */
	int layerIndex;


public:

	/**
	 * Default constructor.
	 */
	GeoTessHorizon(const int& lyrIndex) { layerIndex =  lyrIndex; } ;

	virtual ~GeoTessHorizon() {};

	virtual string class_name() { return "Horizon"; };

	/**
	 * HorizonDepth objects return depth, HorizonRadius
	 * object return radius, and HorizonLayer object return fraction.
	 */
	virtual double getValue() = ABSTRACT;

	/**
	 * Return the radius of the Horizon at the specified geographic position
	 * and constrained by the specified array of Profiles, all of which are
	 * assumed to reside at the specified position.
	 * @param position the unit vector representing the position where the
	 * radius is to be determined.  This should correspond to the position
	 * of the supplied array of Profiles.
	 * @param profiles a 1D array of profiles at the specified position.
	 * The number of elements must be equal to the number of layers in the
	 * model with the first layer being the deepest (closest to the center
	 * of the Earth) and the last layer being the shallowest (farthest from
	 * the center of the Earth).
	 * @return the radius of the Horizon at the specified position and
	 * perhaps constrained to reside in the specified layer.  Units are km.
	 */
	virtual double getRadius(const double* position, GeoTessProfile** profiles) = ABSTRACT;

	/**
	 * Return the radius of the Horizon at the position of the specified
	 * GeoTessPosition object.
	 * @param position
	 * @return the radius of the Horizon at the specified position and
	 * perhaps constrained to reside in the specified layer.  Units are km.
	 * @throws GeoTessException
	 */
	virtual double getRadius(GeoTessPosition& position) = ABSTRACT;

	/**
	 * Retrieve the index of the layer that was specified at construction.
	 * If >= 0 and < the number of layers in the model then the
	 * radius of this Horizon object will be constrained to be within the radii of
	 * the top and bottom of this layer.
	 * @return layer index, or -1.
	 */
	virtual int getLayerIndex() { return layerIndex; };

	virtual string str() = ABSTRACT;

};
// end class Horizon

}// end namespace geotess

#endif /* HORIZON_H_ */
