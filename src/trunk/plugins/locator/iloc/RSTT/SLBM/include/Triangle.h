//- ****************************************************************************
//- 
//- Copyright 2009 Sandia Corporation. Under the terms of Contract 
//- DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains 
//- certain rights in this software.
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
//-
//- Program:       Triangle
//- Module:        $RCSfile: Triangle.h,v $
//- Revision:      $Revision: 1.14 $
//- Last Modified: $Date: 2012/12/03 15:21:50 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef Triangle_H
#define Triangle_H

// **** _SYSTEM INCLUDES_ ******************************************************
#include <vector>
#include <map>
#include <cmath>
#include <string>
#include <iostream>
#include <fstream>


using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "SLBMGlobals.h"
#include "GridProfile.h"
#include "SLBMException.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

class SLBM_EXP_IMP Triangle
{

public:

	//! \brief Default constructor.
	//!
	//! Default constructor.
	Triangle();

	Triangle(const int& index, GridProfile* loc0, 
		GridProfile* loc1, GridProfile* loc2);

	//! \brief Default destructor.
	//!
	//! Default destructor.
	~Triangle();

	//! \brief Copy constructor.
	//! 
	//! Copy constructor.
	Triangle(const Triangle &other);

	//! \brief Equal operator.
	//! 
	//! Equal operator.
	Triangle& operator=(const Triangle& other);

	int getIndex() { return index; };

	//! \brief Retrieve a handle to one of the 3 nodes that defines the corners
	//! of this triangle.
	//!
	//! Retrieve a handle to one of the 3 nodes that defines the corners
	//! of this triangle.
	//! @param i the index of the desired node.  Must be in range 0 to 2 inclusive.
	GridProfile* getNode(const int& i) { return nodes[i]; };

	//! Specify a reference to the triangle which lies on the other side
	//! of the edge of this triangle defined by nodes i and (i+1)%3
	//!
	//! Specify a reference to the triangle which lies on the other side
	//! of the edge of this triangle defined by nodes i and (i+1)%3
	void setNeighbor(const int& i, Triangle* neighbor) { neighbors[i] = neighbor; };

	//! Retrieve a reference to the triangle which lies on the other side
	//! of the edge of this triangle defined by nodes i and (i+1)%3
	//!
	//! Retrieve a reference to the triangle which lies on the other side
	//! of the edge of this triangle defined by nodes i and (i+1)%3
	Triangle* getNeighbor(const int& i) { return neighbors[i]; };

	/**
	* Search through this Triangle's neighbors and find
	* the first one such that Location position is on the other side of the
	* great circle that contains the edge that separates this Triangle from
	* its neighbor.  If no such triangle is identified, then position must
	* reside within the boundaries of this triangle.  In that case, compute
	* the interpolation coefficients.
	* @param position GeoVector
	* @param coefficients
	* @return Triangle
	*/
	Triangle* walk(const Location& position, vector<double>& coefficients);

	//! \brief Find the set of all nodes that are directly connected to
	//! node node0.
	//!
	//! Find the set of all nodes that are directly connected to
	//! nodenode0.  It is assumed that the node with nodeId node0 ise 
	//! on of the corners of this Triangle.  This method will search all
	//! this Triangle's neighbors (and their neighbors if necessary)
	//! and add all nodes that are directly linked to node0 to the
	//! set neighborNodes.
	void findNodeNeighbors(const int& node0, set<int>& neighborNodes);

	/**
	 * If this triangle is triangle T with neighbor N, it must be true that
	 * T is one of N's neighbors.  Return the index of triangle T in N's list
	 * of neighbors.  Returns -1 if T is not a member of N's list of neighbors.
	 * @param neighbor Triangle
	 * @return int
	 */
	int findNeighborIndex(Triangle* neighbor);

private:

	int index;

	GridProfile* nodes[3];

	Triangle* neighbors[3];

	void findNodeNeighbors(const int& node0, 
						  set<int>& neighborNodes,
						  set<Triangle*> visitedTriangles);

};

inline Triangle* Triangle::walk(const Location& position, vector<double>& coeff)
{
    int i,j;
	for (i=0; i<3; i++)
    {
        j = (i + 1) % 3;
		// compute the scalar triple product: 
		// position dot (nodes[j] cross nodes[j+1])
		coeff[i] = position.scalarTripleProduct(*nodes[j], *nodes[(j+1)%3]);
        if (coeff[i] > 1e-15)
        {
            // positive means that position does not reside in this triangle.
            // It resides on the other side of edge[i] of this triangle.
			// Recursively walk into triangle neighbors[j]
            return neighbors[j]->walk(position, coeff);
        }
    }

	// this is the triangle in which position resides.
    double sum = 0;
    for (i=0; i<3; i++)
	{
		if (coeff[i] > 0.) coeff[i] = 0.;
        sum += coeff[i];
	}

    for (i=0; i<3; i++)
        coeff[i] /= sum;

    return this;
}

inline void Triangle::findNodeNeighbors(const int& node0, set<int>& neighborNodes)
{
	set<Triangle*> visitedTriangles;
	findNodeNeighbors(node0, neighborNodes, visitedTriangles);
}

inline void Triangle::findNodeNeighbors(const int& node0, set<int>& neighborNodes,
						  set<Triangle*> visitedTriangles)
{
	visitedTriangles.insert(this);
	// iterate over the 3 nodes of this triangle
	for (int i=0; i<3; i++)
	{
		// if the node we are searching for is one of the corners
		// of this node, then add the other 2 nodes to neighborNodes
		// and then recursively visit each of this triangles neighbors
		// to see they also contain the node being sought.
		if (nodes[i]->getNodeId() == node0)
		{
			for (int j=0; j<3; j++)
				if (nodes[j]->getNodeId() != node0)
					neighborNodes.insert(nodes[j]->getNodeId());
			for (int j=0; j<3; j++)
				if (visitedTriangles.find(neighbors[j]) == visitedTriangles.end())
					neighbors[j]->findNodeNeighbors(node0, neighborNodes, visitedTriangles);
		}
	}
}

inline int Triangle::findNeighborIndex(Triangle* neighbor)
{
    for (int i=0; i<3; i++)
		if (neighbor->getNeighbor(i) == this)
            return i;
    return -1;
}

} // end slbm namespace

#endif // Grid_H
