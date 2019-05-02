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
//- Program:       QueryProfile
//- Module:        $RCSfile: QueryProfile.cc,v $
//- Revision:      $Revision: 1.14 $
//- Last Modified: $Date: 2012/12/03 23:59:06 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#include "Grid.h"
#include "QueryProfile.h"
#include "GridProfile.h"
#include <iostream>

//using namespace std;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

// **** _STATIC INITIALIZATIONS_************************************************

int QueryProfile::queryProfileClassCount = 0;

QueryProfile::QueryProfile(Grid& grid, Location& loc)
		: 
		InterpolatedProfile(grid, loc)
{
	++queryProfileClassCount;

	location = loc;

	double R = location.getEarthRadius();

	for (int k=0; k<NLAYERS; k++)
	{
		interpRadius(k, depth[k]);
		depth[k] = R-depth[k];
		interpVelocity(PWAVE, k, pvelocity[k]);
		interpVelocity(SWAVE, k, svelocity[k]);
	}
    
	interpGradient(PWAVE, gradient[PWAVE]);
	interpGradient(SWAVE, gradient[SWAVE]);

	nodeIds.reserve((int)nodes.size());
	for (int i=0; i<(int)nodes.size(); ++i)
		nodeIds.push_back(nodes[i]->getNodeId());
}

QueryProfile::QueryProfile(const QueryProfile &other) 
: InterpolatedProfile(other)
{
	++queryProfileClassCount;

	for (int i=0; i<NLAYERS; i++)
	{
		depth[i] = other.depth[i];
		pvelocity[i] = other.pvelocity[i];
		svelocity[i] = other.svelocity[i];
	}
	gradient[PWAVE] = other.gradient[PWAVE];
	gradient[SWAVE] = other.gradient[SWAVE];
}

QueryProfile::~QueryProfile()
{
	--queryProfileClassCount;
}

QueryProfile& QueryProfile::operator=(const QueryProfile& other)
{
	InterpolatedProfile::operator = (other);
	for (int i=0; i<NLAYERS; i++)
	{
		depth[i] = other.depth[i];
		pvelocity[i] = other.pvelocity[i];
		svelocity[i] = other.svelocity[i];
	}
	gradient[PWAVE] = other.gradient[PWAVE];
	gradient[SWAVE] = other.gradient[SWAVE];
    return *this;
}

bool QueryProfile::operator==(const QueryProfile& other)
{
	if (InterpolatedProfile::operator != (other)) return false;
	for (int i=0; i<NLAYERS; i++)
	{
		if (depth[i] != other.depth[i]) return false;
		if (pvelocity[i] != other.pvelocity[i]) return false;
		if (svelocity[i] != other.svelocity[i]) return false;
	}
	if (gradient[PWAVE] != other.gradient[PWAVE]) return false;
	if (gradient[SWAVE] != other.gradient[SWAVE]) return false;

	return true;
}

string QueryProfile::toString()
{
	return "QueryProfile.toString() not implemented yet.\n";
}

int QueryProfile::getClassCount()
{
  return queryProfileClassCount;
}

} // end slbm namespace
