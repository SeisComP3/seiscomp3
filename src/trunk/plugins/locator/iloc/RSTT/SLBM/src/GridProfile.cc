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
//- Program:   GridProfile
//- Module:$RCSfile: GridProfile.cc,v $
//- Revision:  $Revision: 1.25 $
//- Last Modified: $Date: 2013/05/04 19:41:51 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#include "Grid.h"
#include "GridProfile.h"
#include "InterpolatedProfile.h"
#include "SLBMException.h"

#include <string>
#include <iostream>

//using namespace std;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

// **** _STATIC INITIALIZATIONS_************************************************

int GridProfile::gridProfileClassCount = 0;

GridProfile::GridProfile(const int& nid, const double& lat,
						 const double& lon, const double& elev)
: Location(lat, lon, -elev),
    nodeId(nid),
	activeNodeId(-1),
	weight(NA_VALUE), 
	nHits(0),
	earthRadius(-1.)
{
	earthRadius = Location::getEarthRadius();
	++gridProfileClassCount;
}

GridProfile::GridProfile(const int& nid, Location& location)
: Location(location),
    nodeId(nid),
	activeNodeId(-1),
	weight(NA_VALUE),
	nHits(0),
	earthRadius(-1.)
{
	earthRadius = Location::getEarthRadius();
	++gridProfileClassCount;
}

GridProfile::~GridProfile()
{
	--gridProfileClassCount;
}

int GridProfile::getClassCount()
{
  return gridProfileClassCount;
}

/**
 * Convert the supplied array of depths into a 2D vector of radii using the local
 * earth radius.  Returned array has dimensions 2 x NLAYERS+1
 * @param depths depths in km, relative ellipsoid.  Depths are in slbm order,
 * i.e., 0 is top of water, 1 is surface of solid earth, etc.
 * @param radii 2D array of radii with 2 x NLAYERS+1 elements. radii[0][i]
 * refers to the radius at the bottom or layer i, radii[1][i] is the radius
 * at the top of layer i.  Guaranteed that radii[1][i] == radii[0][i+1].
 * These are in geotess order, i.e, radii[i][0] refer to radii of top and
 * bottom of 'gradient' layer.  radii[i][1] refer to radii of 'mantle'
 * layer.  radii[i][2] refer to lower crust, and so on.
 * radii[i][NLAYERS] are the radii of the top and bottom of the water column.
 */
void GridProfile::depthsToRadii(double depths[NLAYERS], vector<vector<float> >& radii)
{
	radii.clear();
	radii.resize(NLAYERS+1);
	radii[0].resize(2, 0.001F*floor((earthRadius-depths[MANTLE])*1000.+.5));
	for (int k=1; k<=NLAYERS; ++k)
	{
		radii[k].clear();
		radii[k].push_back(radii[k-1][1]);
		radii[k].push_back(0.001F*floor((earthRadius-depths[NLAYERS-k])*1000.+.5));
		if (radii[k][0] > radii[k][1])
		{
			if (radii[k][0] - radii[k][1] < 0.002)
				radii[k][1] = radii[k][0];
			else
			{
				ostringstream os;
				os << endl << "ERROR in GridProfile::depthsToRadii()" << endl
						<< "radiusBottom > radiusTop" << endl
						<< "radiusBottom = " << setw(11) << radii[k][0] << endl
						<< "radiusTop    = " << setw(11) << radii[k][1] << endl
						<< "Version " << SlbmVersion << "  File " << __FILE__
						<< " line " << __LINE__ << endl	<< endl;

				throw SLBMException(os.str(),504);
			}
		}
	}
}

void GridProfile::depthsToRadii(const vector<double>& depths, vector<vector<float> >& radii)
{
	radii.clear();
	radii.resize(NLAYERS+1);
	radii[0].resize(2, 0.001F*floor((earthRadius-depths[MANTLE])*1000.+.5));
	for (int k=1; k<=NLAYERS; ++k)
	{
		radii[k].clear();
		radii[k].push_back(radii[k-1][1]);
		radii[k].push_back(0.001F*floor((earthRadius-depths[NLAYERS-k])*1000.+.5));
		if (radii[k][0] > radii[k][1])
		{
			if (radii[k][0] - radii[k][1] < 0.002)
				radii[k][1] = radii[k][0];
			else
			{
				ostringstream os;
				os << endl << "ERROR in GridProfile::depthsToRadii()" << endl
						<< "radiusBottom > radiusTop" << endl
						<< "radiusBottom = " << setw(11) << radii[k][0] << endl
						<< "radiusTop    = " << setw(11) << radii[k][1] << endl
						<< "Version " << SlbmVersion << "  File " << __FILE__
						<< " line " << __LINE__ << endl	<< endl;

				throw SLBMException(os.str(),504);
			}
		}
	}
}

} // end slbm namespace
