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

#ifndef GEOTESSPOSITIONLINEAR_OBJECT_H
#define GEOTESSPOSITIONLINEAR_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"
#include "GeoTessUtils.h"
#include "GeoTessPosition.h"
#include "GeoTessGrid.h"
#include "GeoTessInterpolatorType.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess
{

// **** _FORWARD REFERENCES_ ***************************************************

class GeoTessModel;

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Implements linear interpolation in geographic dimensions of a grid.
 *
 * Implements linear interpolation in geographic dimensions of a grid.
 *
 * @author Sandy Ballard
 *
 */
class GEOTESS_EXP_IMP GeoTessPositionLinear: public GeoTessPosition
{
protected:

	/**
	 * Set vertices to the 3-element array that stores the corners of the triangle identified during
	 * the triangle walk algorithm. Horizontal coefficients are similarly set to the coefficients
	 * identified during triangle walk.
	 * @throws GeoTessException
	 */
	virtual void update2D(int tid)
	{
		vector<int>& vt = vertices[tid];
		vector<double>& hc = hCoefficients[tid];

		vt.clear();
		hc.clear();

		const int* trngl = grid.getTriangleVertexIndexes(getTriangle(tid));

		// if the interpolation point falls on a grid node set the list of interpolation
		// vertices to include only the identified vertex, and the interpolation coefficient = 1.
		if (GeoTessUtils::dot(unitVector, grid.getVertex(trngl[0])) > cos(1e-7))
		{ vt.push_back(trngl[0]); hc.push_back(1.0); return; }

		if (GeoTessUtils::dot(unitVector, grid.getVertex(trngl[1])) > cos(1e-7))
		{ vt.push_back(trngl[1]); hc.push_back(1.0); return; }

		if (GeoTessUtils::dot(unitVector, grid.getVertex(trngl[2])) > cos(1e-7))
		{ vt.push_back(trngl[2]); hc.push_back(1.0); return; }

		// interpolation point is not coincident with a grid node.
		vector<double>& lc = linearCoefficients[tid];
		vt.push_back(trngl[0]);
		hc.push_back(lc[0]);
		vt.push_back(trngl[1]);
		hc.push_back(lc[1]);
		vt.push_back(trngl[2]);
		hc.push_back(lc[2]);
}

public:

	/**
	 * Standard constructor.
	 */
	GeoTessPositionLinear(GeoTessModel* model,
			const GeoTessInterpolatorType& radialType);

	/**
	 * Destructor.
	 */
	virtual ~GeoTessPositionLinear();

	/**
	 * Retrieve the type of interpolation that this GeoTessPosition object is configured
	 * to perform. Either InterpolatorType.LINEAR or InterpolatorType.NATURAL_NEIGHBOR.
	 */
	virtual const GeoTessInterpolatorType& getInterpolatorType() const { return GeoTessInterpolatorType::LINEAR; }

	virtual long getMemory() { return (long) sizeof(GeoTessPositionLinear) + memory(); }

};
// end class GeoTessModelLinear

}// end namespace geotess

#endif  // GEOTESSPOSITIONLINEAR_OBJECT_H
