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
//- Module:$RCSfile: GridProfileSLBM.cc,v $
//- Revision:  $Revision: 1.3 $
//- Last Modified: $Date: 2012/12/14 23:08:50 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#include "Grid.h"
#include "GridProfile.h"
#include "GridProfileSLBM.h"
#include "InterpolatedProfile.h"
#include "SLBMException.h"

#include <string>
#include <iostream>

//using namespace std;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

// **** _STATIC INITIALIZATIONS_************************************************

GridProfileSLBM::GridProfileSLBM(Grid& g,
						 const int& nodeid, const double& lat,
						 const double& lon, const double& elev,
						 const double& zwater, GeoStack* gstack)
: GridProfile(nodeid, lat, lon, elev), grid(g), geoStack(gstack)
{
	// if earth surface is below sea level, and water thickness
	// is greater than zero, then set water thickness to the 
	// depth of earth surface below sea level.
	if (zwater > 0 && elev < 0)
		waterThick = abs(elev);
	else waterThick = 0;
}

GridProfileSLBM::GridProfileSLBM(const GridProfileSLBM &other)
: GridProfile(other.getNodeId(), other.getLat(), other.getLon(),
		-other.getDepth()),
  grid(other.grid),
	geoStack(other.geoStack),
	waterThick(other.waterThick)
{
	geoStack->incRefCount();
}

GridProfileSLBM::~GridProfileSLBM()
{
	geoStack->decRefCount();
}

void GridProfileSLBM::setData(double* depths,
						  double* pvelocities, double* svelocities,
								 double* gradients)
{
	// if this gridprofile is not the only one that references the geostack
	// then we need to make a new one.
	if (geoStack->getRefCount() > 1)
	{
		// decrement the reference count on the current geostack since this
		// grid profile will no longer be pointing to it.
		geoStack->decRefCount();

		// construct a new GeoStack and have this grid profile point to it.
		geoStack = new GeoStack(*geoStack);

		// add the new geostack to grid's array of geostacks, which returns
		// the new size of grid's geoStacks array.  Set the index of the new
		// geostack to it's index in grid's geoStacks array.
		geoStack->setIndex(grid.addGeoStack(geoStack)-1);
	}

	double z0[NLAYERS];
	setDepth(depths[0]);
	for(int i=0; i<NLAYERS; ++i)
		z0[i] = depths[i] - depths[0];

	// modifiy the data in the geostack that this grid profile references.
	geoStack->setData(z0, pvelocities, svelocities, gradients);

	//! throw an error if (1) any finite thickness layer above the middle crust
	//! has a velocity that exceeds the Pg/Lg velocity of the middle crust
	//! or (2) any finite thickness crustal layer has a velocity that exceeds
	//! the mantle velocity.
	if (geoStack->hasLowVelocityZone())
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(4);
		os << endl << "ERROR in GridProfileSLBM::setData" << endl
			<< "Geostack has low velocity zone."  << endl
			<< "Node ID, lat, lon = " << nodeId << ", "
			<< getLatDegrees() << ", " << getLonDegrees() << endl
			<< geoStack->toString() << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__
			<< " line " << __LINE__ << endl	<< endl;

		//throw SLBMException(os.str());
		cout << os.str();
	}
}

void GridProfileSLBM::setDepths(const vector<double>& depths)
{
	// if this gridprofile is not the only one that references the geostack
	// then we need to make a new one.
	if (geoStack->getRefCount() > 1)
	{
		// decrement the reference count on the current geostack since this
		// grid profile will no longer be pointing to it.
		geoStack->decRefCount();

		// construct a new GeoStack and have this grid profile point to it.
		geoStack = new GeoStack(*geoStack);

		// add the new geostack to grid's array of geostacks, which returns
		// the new size of grid's geoStacks array.  Set the index of the new
		// geostack to it's index in grid's geoStacks array.
		geoStack->setIndex(grid.addGeoStack(geoStack)-1);
	}

	vector<double> z(NLAYERS);
	setDepth(depths[0]);
	for(int i=0; i<NLAYERS; ++i)
		z[i] = depths[i] - depths[0];

	// modifiy the data in the geostack that this grid profile references.
	geoStack->setDepth(z);
}

void GridProfileSLBM::setVelocity(const int& waveType, const vector<double>& velocity)
{
	// if this gridprofile is not the only one that references the geostack
	// then we need to make a new one.
	if (geoStack->getRefCount() > 1)
	{
		// decrement the reference count on the current geostack since this
		// grid profile will no longer be pointing to it.
		geoStack->decRefCount();

		// construct a new GeoStack and have this grid profile point to it.
		geoStack = new GeoStack(*geoStack);

		// add the new geostack to grid's array of geostacks, which returns
		// the new size of grid's geoStacks array.  Set the index of the new
		// geostack to it's index in grid's geoStacks array.
		geoStack->setIndex(grid.addGeoStack(geoStack)-1);
	}
	// modifiy the data in the geostack that this grid profile references.
	geoStack->setVelocity(waveType, velocity);
}

void GridProfileSLBM::setGradient(const vector<double>& gradient)
{
	// if this gridprofile is not the only one that references the geostack
	// then we need to make a new one.
	if (geoStack->getRefCount() > 1)
	{
		// decrement the reference count on the current geostack since this
		// grid profile will no longer be pointing to it.
		geoStack->decRefCount();

		// construct a new GeoStack and have this grid profile point to it.
		geoStack = new GeoStack(*geoStack);

		// add the new geostack to grid's array of geostacks, which returns
		// the new size of grid's geoStacks array.  Set the index of the new
		// geostack to it's index in grid's geoStacks array.
		geoStack->setIndex(grid.addGeoStack(geoStack)-1);
	}
	// modifiy the data in the geostack that this grid profile references.
	geoStack->setGradient(gradient);
}

} // end slbm namespace
