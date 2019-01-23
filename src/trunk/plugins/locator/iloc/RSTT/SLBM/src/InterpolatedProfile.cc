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
//- Program:       InterpolatedProfile
//- Module:        $RCSfile: InterpolatedProfile.cc,v $
//- Revision:      $Revision: 1.14 $
//- Last Modified: $Date: 2012/12/03 23:59:05 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#include "Grid.h"
#include "InterpolatedProfile.h"
#include "GridProfile.h"
#include <iostream>

//using namespace std;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

// **** _STATIC INITIALIZATIONS_************************************************

int InterpolatedProfile::interpolatedProfileClassCount = 0;


InterpolatedProfile::InterpolatedProfile()
{
	++interpolatedProfileClassCount;
}

InterpolatedProfile::InterpolatedProfile(Grid& grid, Location& location)
{
	++interpolatedProfileClassCount;
	grid.findProfile(location, nodes, nodeIds, coefficients);
}

InterpolatedProfile::InterpolatedProfile(const InterpolatedProfile& other)
{
	++interpolatedProfileClassCount;
	nodes.clear();
	nodes.reserve(other.nodes.size());
	nodeIds.clear();
	nodeIds.reserve(other.nodeIds.size());
	coefficients.clear();
	coefficients.reserve(other.coefficients.size());
	for (int i=0; i<(int)other.nodes.size(); i++)
	{
		nodes.push_back(other.nodes[i]);
		nodeIds.push_back(other.nodeIds[i]);
		coefficients.push_back(other.coefficients[i]);
	}
}

InterpolatedProfile::~InterpolatedProfile()
{
	--interpolatedProfileClassCount;
}

InterpolatedProfile& InterpolatedProfile::operator=(const InterpolatedProfile& other)
{
	nodes.clear();
	nodes.reserve(other.nodes.size());
	coefficients.clear();
	coefficients.reserve(other.coefficients.size());
	for (int i=0; i<(int)other.nodes.size(); i++)
	{
		nodes.push_back(other.nodes[i]);
		coefficients.push_back(other.coefficients[i]);
	}
	return *this;
}

bool InterpolatedProfile::operator==(const InterpolatedProfile& other)
{
	if (nodes.size() != other.nodes.size())
		return false;

	if (coefficients.size() != other.coefficients.size())
		return false;

	for (int i=0; i<(int)nodes.size(); i++)
		if (nodes[i]!= other.nodes[i] 
		 || coefficients[i] != other.coefficients[i]) 
			 return false;
	return true;
}

size_t InterpolatedProfile::memSize()
{
	return 
		sizeof(nodes) + nodes.size()*sizeof(GridProfile*)
		+ sizeof(coefficients) + coefficients.size()*sizeof(double)
		;
}

int InterpolatedProfile::getClassCount() 
{ 
	return interpolatedProfileClassCount; 
}

} // end slbm namespace
