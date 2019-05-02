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
//- Module:        $RCSfile: Triangle.cc,v $
//- Revision:      $Revision: 1.8 $
//- Last Modified: $Date: 2011/10/07 13:15:00 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#include <limits>

#include "Triangle.h"
#include "Location.h"
#include <iostream>

//using namespace std;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

Triangle::Triangle() : index(-1)
{
	nodes[0] = NULL;
	nodes[1] = NULL; 
	nodes[2] = NULL;
	neighbors[0] = NULL;
	neighbors[1] = NULL;
	neighbors[2] = NULL;
}

Triangle::Triangle(const int& i, GridProfile* loc0, 
				   GridProfile* loc1, GridProfile* loc2)
				   : index(i)
{
	nodes[0] = loc0;
	nodes[1] = loc1; 
	nodes[2] = loc2;
	neighbors[0] = NULL;
	neighbors[1] = NULL;
	neighbors[2] = NULL;
}

Triangle::~Triangle()
{
}

//! \brief Copy constructor.
//! 
//! Copy constructor.
Triangle::Triangle(const Triangle &other) : index(other.index)
{
	nodes[0] = other.nodes[0];
	nodes[1] = other.nodes[1]; 
	nodes[2] = other.nodes[2];
	neighbors[0] = other.neighbors[0];
	neighbors[1] = other.neighbors[1];
	neighbors[2] = other.neighbors[2];
}

//! \brief Equal operator.
//! 
//! Equal operator.
Triangle& Triangle::operator=(const Triangle& other)
{
	index = other.index;
	nodes[0] = other.nodes[0];
	nodes[1] = other.nodes[1]; 
	nodes[2] = other.nodes[2];
	neighbors[0] = other.neighbors[0];
	neighbors[1] = other.neighbors[1];
	neighbors[2] = other.neighbors[2];
	return *this;
}

} // end slbm namespace
