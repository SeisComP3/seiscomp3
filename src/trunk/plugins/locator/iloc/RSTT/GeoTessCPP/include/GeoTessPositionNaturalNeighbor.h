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

#ifndef GEOTESSPOSITIONNATURALNEIGHBOR_OBJECT_H
#define GEOTESSPOSITIONNATURALNEIGHBOR_OBJECT_H

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
#include "GeoTessInterpolatorType.h"
#include "ArrayReuse.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

class GeoTessModel;
class GeoTessProfile;

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Implements the Natural Neighbor Interpolation algorithm for the geographic
 * dimensions of the grid.
 *
 * Implements the Natural Neighbor Interpolation algorithm of Sibson (1980, 1981). This technique
 * interpolates values stored on a Delaunay triangulation. Returned values are continuous everywhere
 * and smooth everywhere except the vertices of the triangulation. Implementation of the algorithm
 * is described in detail in Hipp et al. (1999).
 *
 * <p>
 * Hipp, J. R., C. J. Young, S. G. Moore, E. R. Shepherd, C. A. Schultz and S. C. Myers (1999),
 * Parametric Grid Information in the DOE Knowledge Base: Data Preparation, Storage, and Access,
 * SAND99-2277, Sandia National Laboratories Report.
 *
 * <p>
 * Sibson, R., (1980) A Vector Identity for the Dirichlet Tessellation, Proc. Cambridge
 * Philosophical Society, 87, 151-155.
 *
 * <p>
 * Sibson, R. (1981). "A brief description of natural neighbor interpolation (Chapter 2)". In V.
 * Barnett. Interpreting Multivariate Data. Chichester: John Wiley. pp. 21�36.
 *
 * @author Sandy Ballard
 *
 */
class GEOTESS_EXP_IMP GeoTessPositionNaturalNeighbor: public GeoTessPosition {

private:

	vector<bool> marked;
	vector<int> nnTriangles;
	vector<Edge*> edges;

	/**
	 * a reference to the grid.getVertices().  No new memory involved.
	 */
	double const* const* gridVrtcs;

	bool isNNTriangle(const int& t)
	{
		const double* center = grid.getCircumCenter(t);
		// if the distance from the interpolation point to the
		// circumcenter is less than the radius of the circumcircle
		return GeoTessUtils::dot(center, unitVector) > center[3];
		  //GeoTessUtils::dot(center, grid.getTriangleVertex(triangle, 0));

	}


protected:

	/**
	 * Update the vertices and their associated interpolation coefficients, that will be used to
	 * interpolate new values. Uses the natural neighbor interpolation algorithm originally proposed
	 * by Sibson, R. (1980), A Vector Identity For Dirichlet Tessellation, Proc. Cambridge
	 * Philosophical Society, 87, 151-155. The algorithm is described in detail in Hipp, J. R., C.
	 * J. Young, S. G. Moore, E. R. Shepherd, C. A. Schultz and S. C. Myers (1999), Parametric Grid
	 * Information in the DOE Knowledge Base: Data Preparation, Storage, and Access, SAND99-2277,
	 * Sandia National Laboratories Report.
	 */
	virtual void update2D(int tid);

public:

	/**
	 * Standard Constructor.
	 */
	GeoTessPositionNaturalNeighbor(GeoTessModel* model,
			const GeoTessInterpolatorType& radialType);

	/**
	 * Destructor.
	 */
	virtual ~GeoTessPositionNaturalNeighbor();

	/**
	 * Retrieve the type of interpolation that this GeoTessPosition object is configured
	 * to perform. Either InterpolatorType.LINEAR or InterpolatorType.NATURAL_NEIGHBOR.
	 */
	virtual const GeoTessInterpolatorType& getInterpolatorType() const {
		return GeoTessInterpolatorType::NATURAL_NEIGHBOR;
	}

	virtual long getMemory()
	{
		return (long)sizeof(GeoTessPositionNaturalNeighbor) + memory()
				+ (long)(marked.capacity() * sizeof(bool) + nnTriangles.capacity() * sizeof(int)
				+ edges.capacity() * sizeof(Edge*));
	}

};
// end class GeoTessModelNaturalNeighbor

}// end namespace geotess

#endif  // GEOTESSPOSITIONNATURALNEIGHBOR_OBJECT_H
