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

// **** _LOCAL INCLUDES_ *******************************************************

#include "GeoTessPositionNaturalNeighbor.h"
#include "CPPUtils.h"
#include "GeoTessUtils.h"
#include "GeoTessException.h"
#include "GeoTessProfile.h"
#include "GeoTessProfileType.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

/**
 * Standard Constructor.
 */
GeoTessPositionNaturalNeighbor::GeoTessPositionNaturalNeighbor(GeoTessModel* model,
		const GeoTessInterpolatorType& radialType) : GeoTessPosition(model, radialType)
{
	marked.resize(grid.getNTriangles(), false);
	nnTriangles.reserve(64);
	edges.reserve(64);
	gridVrtcs = grid.getVertices();
}

/**
 * Destructor.
 */
GeoTessPositionNaturalNeighbor::~GeoTessPositionNaturalNeighbor()
{
}

/**
 * Update the vertices and their associated interpolation coefficients, that will be used to
 * interpolate new values. Uses the natural neighbor interpolation algorithm originally proposed
 * by Sibson, R. (1980), A Vector Identity For Dirichlet Tessellation, Proc. Cambridge
 * Philosophical Society, 87, 151-155. The algorithm is described in detail in Hipp, J. R., C.
 * J. Young, S. G. Moore, E. R. Shepherd, C. A. Schultz and S. C. Myers (1999), Parametric Grid
 * Information in the DOE Knowledge Base: Data Preparation, Storage, and Access, SAND99-2277,
 * Sandia National Laboratories Report.
 */
void GeoTessPositionNaturalNeighbor::update2D(int tessId)
{
	int vertex;
	int nnTriangle = getTriangle(tessId);
	vector<int>& vt = vertices[tessId];
	vector<double>& hc = hCoefficients[tessId];

	const int* triangleVertices = grid.getTriangleVertexIndexes(nnTriangle);

	for (int corner = 0; corner < 3; ++corner)
	{
		vertex = triangleVertices[corner];

		// if the interpolation point falls on a grid node:
		if (GeoTessUtils::dot(unitVector, grid.getVertex(vertex)) > cos(1e-7))
		{
			// the interpolation point coincides with one of the corners of
			// the triangle in which the interpolation point resides.
			// Set the list of interpolation vertices to include only
			// the identified vertex, and the interpolation coefficient = 1.

			vt.clear(); vt.push_back(vertex);
			hc.clear(); hc.push_back(1.0);
			return;
		}
	}

	// The interpolation point does not coincide with a grid vertex.
	// Carry on.

	// get the tessellation level, relative to the first tessellation
	// level of the current tessellation, that was discovered the last
	// time triangle walk algorithm was run.
	int tessLevel = getTessLevel(tessId);

	// get the level relative to all levels in all tessellations of the grid.
	int level = grid.getLevel(tessId, tessLevel);

	bool leftIn, rightIn, neighborIn[3];

	// make sure that circumcenters of all triangles on current level have been computed.
	grid.computeCircumCenters(level);

	vector<Edge*>& gridSpokeList = grid.getSpokeList(level);

	Edge *edge, *spoke, *firstSpoke[3];

	marked[nnTriangle] = true;
	nnTriangles.push_back(nnTriangle);

	// iterate over the indices of the 3 vertices at the corners of the
	// containing triangle and determine whether or not the 3 neighboring
	// triangles are natural neighbor triangles.
	for (int vi = 0; vi < 3; ++vi)
	{
		vertex = triangleVertices[vi];

		// access a random spoke emanating from vertex.
		spoke = gridSpokeList[vertex];

		// iterate clockwise over the circular list of spokes until triangleLeft == triangle
		while (spoke->tLeft != nnTriangle) spoke = spoke->next;
		firstSpoke[vi] = spoke;
		neighborIn[vi] = isNNTriangle(spoke->tRight);
	}

	// iterate over the indices of the 3 vertices at the corners of the
	// containing triangle.
	for (int vi = 0; vi < 3; ++vi)
	{
		spoke = firstSpoke[vi];
		rightIn = neighborIn[vi];
		if (rightIn)
		{
			marked[spoke->tRight] = true;
			nnTriangles.push_back(spoke->tRight);
		}
		else
			edges.push_back(grid.getEdgeList()[nnTriangle][(vi+1)%3]);

		int nk = grid.getNeighbor(nnTriangle, (vi+2)%3);

		while(true)
		{
			spoke = spoke->next;
			leftIn = rightIn;

			if (spoke->tRight == nk)
			{
				rightIn = neighborIn[(vi+1)%3];

				if (leftIn && !rightIn)
					edges.push_back(grid.getEdgeList()[spoke->tLeft][(spoke->cornerj+1)%3]);
				else if (!leftIn && rightIn)
					edges.push_back(grid.getEdgeList()[spoke->tRight][(spoke->next->cornerj+2)%3]);

				break;
			}

			rightIn = isNNTriangle(spoke->tRight);

			if (leftIn && !rightIn)
				edges.push_back(grid.getEdgeList()[spoke->tLeft][(spoke->cornerj+1)%3]);
			else if (!leftIn && rightIn)
				edges.push_back(grid.getEdgeList()[spoke->tRight][(spoke->next->cornerj+2)%3]);

			if (rightIn)
			{
				marked[spoke->tRight] = true;
				nnTriangles.push_back(spoke->tRight);

				edges.push_back(grid.getEdgeList()[spoke->tRight][spoke->next->cornerj]);
			}
		}
	}

	vector<Edge*>::reverse_iterator it;

	int prev = edges[0]->vj;
	for (it=edges.rbegin(); it != edges.rend(); ++it)
	{
		edge = *it;
		if (edge->vk != prev)
		{
			ostringstream os;
			os << endl << "ERROR in GeoTessPositionNaturalNeighbor::update2D" << endl
					<< "Perimeter of the natural neighbor triangles do not form a closed loop." << endl
					<< "It is likely that this is not a Delaunay tessellation." << endl
					<< "TessID=" << tessid << "  Level=" << tessLevel << "  Triangle=" << nnTriangle << endl
					<< "Layer " << model->getMetaData().getLastLayer(tessid) << "  "
					<< "Interpolation point = " << GeoTessUtils::getLatLonString(unitVector) << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 1003);

		}
		prev = edge->vj;
	}

	double ip1[3], ip2[3], ip3[3], work1[3], work2[3], work3[3];

	vt.clear();
	vt.reserve(edges.size());
	hc.clear();
	hc.reserve(edges.size());

	Edge *preEdge = edges[0];
	double weight, totalWeight = 0.0;

	for (it=edges.rbegin(); it != edges.rend(); ++it)
	{
		vertex = preEdge->vj;
		weight = 0;

		// set ip1 to the virtual veronoi vertex of the triangle formed by interpolationPoint and preEdge
		GeoTessUtils::circumCenter(unitVector, gridVrtcs[vertex], gridVrtcs[preEdge->vk], ip1);

		// access a random spoke emanating from vertex.
		spoke = gridSpokeList[vertex];
		// iterate over the circular list of spokes until vertex neighbor is equal to preEdge->vk
		while (spoke->vk != preEdge->vk) spoke = spoke->next;

		// spoke is the first surrounding edge and corresponds to a reversed version of preEdge.
		// set ip2 to the circumCenter of the nnTriangle that is to the right of spoke
		grid.getCircumCenter(spoke->tRight, ip2);

		while (true)
		{
			// find the next spoke in clockwise direction
			spoke = spoke->next;

			if (marked[spoke->tRight])
			{
				// set ip3 to the circumcenter of the triangle to the right of the current edge.
				grid.getCircumCenter(spoke->tRight, ip3);
				weight += GeoTessUtils::getTriangleArea(ip1, ip2, ip3, work1, work2, work3);
			}
			else
			{
				// set ip3 to the virtual veronoi vertex of the triangle
				// formed by interpolationPoint and spoke
				GeoTessUtils::circumCenter(unitVector, gridVrtcs[spoke->vk],
						gridVrtcs[vertex], ip3);

				weight += GeoTessUtils::getTriangleArea(ip1, ip2, ip3, work1, work2, work3);
				break;
			}

			ip2[0] = ip3[0];
			ip2[1] = ip3[1];
			ip2[2] = ip3[2];
		}

		// sum coefficient to total weight, get next edge, and continue
		totalWeight += weight;

		vt.push_back(vertex);
		hc.push_back(weight);

		preEdge = *it;

	}

	// normalize the interpolation coefficients.
	for (int i = 0; i < (int) hc.size(); ++i) hc[i] /= totalWeight;

	for (int i=0; i<(int)nnTriangles.size(); ++i)
		marked[nnTriangles[i]]=false;
	nnTriangles.clear();
	edges.clear();

}

} // end namespace geotess
