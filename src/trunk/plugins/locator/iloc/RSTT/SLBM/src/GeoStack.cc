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
//- Program:   GeoStack
//- Module:$RCSfile: GeoStack.cc,v $
//- Revision:  $Revision: 1.14 $
//- Last Modified: $Date: 2013/05/04 19:41:51 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#include "Grid.h"
#include "GeoStack.h"
#include "InterpolatedProfile.h"
#include "SLBMException.h"

#include <string>
#include <iostream>

//using namespace std;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

// **** _STATIC INITIALIZATIONS_************************************************

int GeoStack::geoStackClassCount = 0;

GeoStack::GeoStack()
{
	++geoStackClassCount;
}

GeoStack::GeoStack(const int& i, double* depths, double* pvelocities, 
				   double* svelocities, double* gradients) 
				   : index(i), refCount(0)
{
	++geoStackClassCount;

	for (int i=0; i<NLAYERS; i++)
	{
		depth[i] = depths[i];
		pvelocity[i] = pvelocities[i];
		svelocity[i] = svelocities[i];
	}
	gradient[PWAVE] = gradients[PWAVE];
	gradient[SWAVE] = gradients[SWAVE];

	thicknessTest();
}

GeoStack::GeoStack(const GeoStack &other) : index(-1), refCount(0)
{
	++geoStackClassCount;

	for (int i=0; i<NLAYERS; i++)
	{
		depth[i] = other.depth[i];
		pvelocity[i] = other.pvelocity[i];
		svelocity[i] = other.svelocity[i];
	}
	gradient[PWAVE] = other.gradient[PWAVE];
	gradient[SWAVE] = other.gradient[SWAVE];
}

GeoStack::~GeoStack()
{
	--geoStackClassCount;
}

GeoStack& GeoStack::operator=(const GeoStack& other)
{
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

bool GeoStack::operator==(const GeoStack& other)
{
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

bool GeoStack::hasLowVelocityZone()
{
	double vp = 0, vs = 0;
	for (int i=SEDIMENT1; i<=UPPER_CRUST; ++i)
		if (depth[i+1] - depth[i] > 1e-6)
		{
			vp = max(vp, pvelocity[i]);
			vs = max(vs, svelocity[i]);
		}

	if (vp > pvelocity[MIDDLE_CRUST_G]) return true;

	//if (vs > svelocity[MIDDLE_CRUST_G]) return true;

	for (int i=MIDDLE_CRUST_N; i<=LOWER_CRUST; ++i)
		if (depth[i+1] - depth[i] > 1e-6)
		{
			vp = max(vp, pvelocity[i]);
			vs = max(vs, svelocity[i]);
		}

	if (vp > pvelocity[MANTLE]) return true;

	if (vs > svelocity[MANTLE]) return true;

	return false;
}

string GeoStack::toString()
{
	ostringstream os;

	os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(4);

	string layers[NLAYERS] = {"water", "sediment1", "sediment2", "sediment3",
			"upper_crust", "middle_crust_n", "middle_crust_g", "lower_crust", "mantle"};

	int w = 9;

	os << " " << setw(5) << "#" 
		<< left 
		<< " " << setw(15) << "Layer" 
		<< right
		<< setw(w-1) << "P_Vel" 
		<< setw(w-1) << "S_Vel" 
		<< setw(w) << "Top" 
		<< setw(w) << "Thick" 
		<< endl;

	--w;
	for (int i=0; i<NLAYERS; i++)
	{
		// layer number
		os << " " << setw(5) << i+1;

		// layer name
		os << " " << setw(15) << left << layers[i] << right;

		// velocity
		os << " " << setw(w-1) << pvelocity[i];

		// velocity
		os << " " << setw(w-1) << svelocity[i];

		// depth
		os << " " << setw(w) << depth[i];

		// thickness
		if (i < NLAYERS-1)
		{
		if (i == MIDDLE_CRUST_N)
			os << " " << setw(w) << depth[LOWER_CRUST]-depth[MIDDLE_CRUST_N];
		else 
			os << " " << setw(w) << depth[i+1]-depth[i];
		}

		os << endl;
	}

	return os.str();
}

int GeoStack::getClassCount()
{
  return geoStackClassCount;
}

bool GeoStack::thicknessTest()
{
	bool modified = false;
	for (int i=1; i<NLAYERS; i++)
		if (depth[i-1] > depth[i])
		{
			if (depth[i-1] - depth[i] < 0.002)
			{
				depth[i] = depth[i-1];
				modified = true;
			}
			else
			{
				ostringstream os;
				os << endl << "ERROR in GridSLBM::saveVelocityModel()" << endl
						<< "Layer " << i << " has negative thickness" << endl
						<< "depths[" << i-1 << "] = " << setw(11) << depth[i-1] << endl
						<< "depths[" << i   << "] = " << setw(11) << depth[i] << endl
						<< "Version " << SlbmVersion << "  File " << __FILE__
						<< " line " << __LINE__ << endl	<< endl;

				throw SLBMException(os.str(), 999);
			}

		}
	return modified;
}

} // end slbm namespace
