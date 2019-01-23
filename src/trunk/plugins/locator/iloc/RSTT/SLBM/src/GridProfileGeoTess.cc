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
//- Module:$RCSfile: GridProfileGeoTess.cc,v $
//- Revision:  $Revision: 1.14 $
//- Last Modified: $Date: 2013/07/26 14:37:51 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#include "Grid.h"
#include "GridGeoTess.h"
#include "GridProfile.h"
#include "GridProfileGeoTess.h"
#include "InterpolatedProfile.h"
#include "SLBMException.h"
#include "GeoTessData.h"
#include "GeoTessProfileThin.h"
#include "GeoTessProfileConstant.h"

#include <string>
#include <iostream>
#include <map>

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

// **** _STATIC INITIALIZATIONS_************************************************

GridProfileGeoTess::GridProfileGeoTess(Grid& g, const int& nodeId, Location& location)
: GridProfile(nodeId, location),
  model(((GridGeoTess&)g).getModel()),
  gtProfiles(((GridGeoTess&)g).getModel()->getProfiles(nodeId))
{
}

GridProfileGeoTess::GridProfileGeoTess(const GridProfileGeoTess &other)
: GridProfile(other.getNodeId(), other.getLat(), other.getLon(), -other.getDepth()),
		model(other.model), gtProfiles(other.gtProfiles)
{
}

GridProfileGeoTess::~GridProfileGeoTess()
{
}

void GridProfileGeoTess::setData(double depths[NLAYERS], double pvelocities[NLAYERS],
		double svelocities[NLAYERS], double gradients[2])
{
	vector<vector<float> > radii;
	depthsToRadii(depths, radii);

	gtProfiles[0]->setRadii(radii[0]);
	for (int k=1; k<=NLAYERS; ++k)
	{
		if (radii[k][1] == radii[k][0])
		{
			if (gtProfiles[k]->getType() == GeoTessProfileType::THIN)
				gtProfiles[k]->setRadii(radii[k]);
			else
			{
				GeoTessProfile* profile = new GeoTessProfileThin(radii[k][1], gtProfiles[k]->getData(0)->copy());
				model->setProfile(nodeId, k, profile);
			}
		}
		else if (radii[k][1] > radii[k][0])
		{
			if (gtProfiles[k]->getType() == GeoTessProfileType::CONSTANT)
				gtProfiles[k]->setRadii(radii[k]);
			else
			{
				GeoTessProfile* profile = new GeoTessProfileConstant(radii[k][0], radii[k][1], gtProfiles[k]->getData(0)->copy());
				model->setProfile(nodeId, k, profile);
			}
		}
		else
		{
			ostringstream os;
			os << fixed << setprecision(3);
			os << endl << "GridProfileGeoTess::setData().  nodeId=" << nodeId << endl
				<< "Depths include layer with negative thickness." << endl;
			for (int d=0; d<NLAYERS; ++d)
				os << (d==0 ? "" : ", ") << depths[d];
			os << endl;

			os << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
			throw SLBMException(os.str(),109);
		}
	}


	gtProfiles[0]->getData(0)->setValue(PWAVE, gradients[PWAVE]);
	gtProfiles[0]->getData(0)->setValue(SWAVE, gradients[SWAVE]);
	for (int k=1; k<=NLAYERS; ++k)
	{
		gtProfiles[k]->getData(0)->setValue(PWAVE, pvelocities[NLAYERS-k]);
		gtProfiles[k]->getData(0)->setValue(SWAVE, svelocities[NLAYERS-k]);
	}
}

void GridProfileGeoTess::setDepths(const vector<double>& depths)
{
	vector<vector<float> > radii;
	depthsToRadii(depths, radii);

//	cout << setprecision(3);
//	for (int k=NLAYERS; k > 0; --k)
//	{
//		cout << setw(3) << k << setw(3) << NLAYERS-k  <<
//				setw(9) << depths[NLAYERS-k]  <<
//				setw(9) << radii[k][0] <<
//				setw(9) << radii[k][1]  <<
//				setw(9) << radii[k][1]-radii[k][0] << endl;
//	}
//	cout << endl;

	gtProfiles[0]->setRadii(radii[0]);
	for (int k=1; k<=NLAYERS; ++k)
	{
		if (radii[k][1] == radii[k][0])
		{
			if (gtProfiles[k]->getType() == GeoTessProfileType::THIN)
				gtProfiles[k]->setRadii(radii[k]);
			else
			{
				GeoTessProfile* profile = new GeoTessProfileThin(radii[k][1], gtProfiles[k]->getData(0)->copy());
				model->setProfile(nodeId, k, profile);
			}
		}
		else if (radii[k][1] > radii[k][0])
		{
			if (gtProfiles[k]->getType() == GeoTessProfileType::CONSTANT)
				gtProfiles[k]->setRadii(radii[k]);
			else
			{
				GeoTessProfile* profile = new GeoTessProfileConstant(radii[k][0], radii[k][1], gtProfiles[k]->getData(0)->copy());
				model->setProfile(nodeId, k, profile);
			}
		}
	}
}

void GridProfileGeoTess::setVelocity(const int& waveType, const vector<double>& velocity)
{
	for (int k=1; k<=NLAYERS; ++k)
		gtProfiles[k]->getData(0)->setValue(PWAVE, velocity[NLAYERS-k]);
}

void GridProfileGeoTess::setGradient(const vector<double>& gradient)
{
	gtProfiles[0]->getData(0)->setValue(PWAVE, gradient[PWAVE]);
	gtProfiles[0]->getData(0)->setValue(SWAVE, gradient[SWAVE]);
}

bool GridProfileGeoTess::hasLowVelocityZone()
{
	double vp = 0, vs = 0;
	for (int i=SEDIMENT1; i<=UPPER_CRUST; ++i)
		if (getInterfaceDepth(i-1) - getInterfaceDepth(i) > 1e-6)
		{
			vp = max(vp, getVelocity(PWAVE, i)); // pvelocity[i]);
			vs = max(vs, getVelocity(SWAVE, i)); // svelocity[i]);
		}

	if (vp > getVelocity(PWAVE, MIDDLE_CRUST_G)) return true;

	//if (vs > svelocity[MIDDLE_CRUST_G]) return true;

	for (int i=MIDDLE_CRUST_N; i<=LOWER_CRUST; ++i)
		if (getInterfaceDepth(i-1) - getInterfaceDepth(i) > 1e-6)
		{
			vp = max(vp, getVelocity(PWAVE, i)); // pvelocity[i]);
			vs = max(vs, getVelocity(SWAVE, i)); // svelocity[i]);
		}

	if (vp > getVelocity(PWAVE, MANTLE) || vs > getVelocity(SWAVE, MANTLE))
		return true;

	return false;
}

} // end slbm namespace
