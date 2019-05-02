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
//- Program:       SlbmInterface
//- Module:        $RCSfile: SlbmInterface.cc,v $
//- Revision:      $Revision: 1.53 $
//- Last Modified: $Date: 2013/07/12 23:25:49 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#include "SlbmInterface.h"
#include "SLBMGlobals.h"
#include "Grid.h"
#include "GridSLBM.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

//using namespace std;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

double SlbmInterface::CH_MAX = 0.2;

SlbmInterface::SlbmInterface() : 
    grid(NULL), 
	greatCircle(NULL),
	valid(false),
	srcLat(NaN_DOUBLE), srcLon(NaN_DOUBLE),
	rcvLat(NaN_DOUBLE), rcvLon(NaN_DOUBLE)
{
}  // END SlbmInterface Default Constructor

SlbmInterface::SlbmInterface(const double& earthRadius) : 
    grid(NULL), 
	greatCircle(NULL), 
	valid(false),
	srcLat(NaN_DOUBLE), srcLon(NaN_DOUBLE),
	rcvLat(NaN_DOUBLE), rcvLon(NaN_DOUBLE)
{
	Location::EARTH_RADIUS = earthRadius;
}  // END SlbmInterface Default Constructor

SlbmInterface::~SlbmInterface()
{
	clear();
	if (grid) delete grid;

	valid = false;
}  // END SlbmInterface Destructor

void SlbmInterface::clear()
{
	clearGreatCircles();
	if (grid) 
		grid->clearCrustalProfiles();
	valid = false;
}

void SlbmInterface::clearGreatCircles()
{
	if (greatCircle) 
	{
		delete greatCircle;
		greatCircle = NULL;
	}
	srcLat=srcLon=rcvLat=rcvLon=NaN_DOUBLE;
}

void  SlbmInterface::loadVelocityModel(const string& modelFileName)
{
	if (grid)
	{
		delete grid;
		grid = NULL;
	}

	grid = Grid::getGrid(modelFileName);
}

void SlbmInterface::saveVelocityModel(const string& fname, const int& format)
{
	if (!grid)
	{
		ostringstream os;
		os << endl << "ERROR in SlbmInterface::saveVelocityModel" << endl
			<< "There is no grid in memory to save." << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),109);
	}
	grid->saveVelocityModel(fname, format);
}

void SlbmInterface::saveVelocityModelBinary()
{
	if (!grid)
	{
		ostringstream os;
		os << endl << "ERROR in SlbmInterface::saveVelocityModelBinary" << endl
			<< "There is no grid in memory to save." << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),109);
	}
	grid->saveVelocityModel(grid->getOutputDirectory(), 3);
}

void SlbmInterface::specifyOutputDirectory(const string& directoryName)
{
	if (!grid)
	{
		ostringstream os;
		os << endl << "ERROR in SlbmInterface::specifyOutputDirectory" << endl
			<< "There is no grid in memory to save." << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),109);
	}
	grid->specifyOutputDirectory(directoryName);
}

void  SlbmInterface::loadVelocityModelBinary(util::DataBuffer& buffer)
{
	if (grid)
		delete grid;

	grid = Grid::getGrid(buffer);
}

void SlbmInterface::saveVelocityModelBinary(util::DataBuffer& buffer)
{
	if (!grid)
	{
		ostringstream os;
		os << endl << "ERROR in SlbmInterface::saveVelocityModelBinary" << endl
			<< "There is no grid in memory to save." << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),109);
	}

	grid->saveVelocityModel(buffer);
}

int SlbmInterface::getBufferSize() const
{
	if (!grid)
	{
		ostringstream os;
		os << endl << "ERROR in SlbmInterface::getBufferSize()" << endl
			<< "There is no grid in memory." << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),109);
	}

	return grid->getBufferSize();
}

void SlbmInterface::createGreatCircle(
			const int& phase,
			const double& sourceLat,
			const double& sourceLon,
			const double& sourceDepth,
			const double& receiverLat,
			const double& receiverLon,
			const double& receiverDepth)
{
	clearGreatCircles();

	// save copies of source and receiver position (radians)
	srcLat = sourceLat;
	srcLon = sourceLon;
	rcvLat = receiverLat;
	rcvLon = receiverLon;

	valid = false;
	
	greatCircle = GreatCircleFactory::create(
		phase, grid, sourceLat, sourceLon, sourceDepth,
		receiverLat, receiverLon, receiverDepth, CH_MAX);

	valid = true;
}

void SlbmInterface::getGridData(
		const int& nodeId,
		double& latitude,
		double& longitude,
		double depth[NLAYERS],
		double pvelocity[NLAYERS],
		double svelocity[NLAYERS],
		double gradient[2])
{
	if (nodeId < 0 || nodeId >= grid->getNNodes())
	{
		ostringstream os;
		os << endl << "ERROR in SlbmInterface::getGridData" << endl
			<< "Specified grid nodeId, " << nodeId <<", "
			<< " is out of range.  Must be less than " 
			<< grid->getNNodes() << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),110);
	}
	GridProfile* profile = grid->getProfile(nodeId);
	latitude = profile->getLat();
	longitude = profile->getLon();
	profile->getData(depth, pvelocity, svelocity, gradient);
}

void SlbmInterface::getActiveNodeData(
		const int& nodeId,
		double& latitude,
		double& longitude,
		double depth[NLAYERS],
		double pvelocity[NLAYERS],
		double svelocity[NLAYERS],
		double gradient[2])
{
	getGridData(grid->getGridNodeId(nodeId), latitude, longitude, depth, 
		pvelocity, svelocity, gradient);
}

void SlbmInterface::setGridData(
		const int& nodeId,
		double depths[NLAYERS], 
		double pvelocity[NLAYERS], 
		double svelocity[NLAYERS],
		double gradient[2])
{
	if (nodeId < 0 || nodeId >= grid->getNNodes())
	{
		ostringstream os;
		os << endl << "ERROR in SlbmInterface::setGridData" << endl
			<< "Specified grid nodeId, " << nodeId <<", "
			<< " is out of range.  Must be less than " 
			<< grid->getNNodes() << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),111);
	}
	GridProfile* gridProfile = grid->getProfile(nodeId);
	gridProfile->setData(depths, pvelocity, svelocity, gradient);
}

void SlbmInterface::setActiveNodeData(
		const int& nodeId,
		double depths[NLAYERS], 
		double pvelocity[NLAYERS], 
		double svelocity[NLAYERS],
		double gradient[2])
{
	setGridData(grid->getGridNodeId(nodeId), depths, pvelocity, svelocity, gradient);
}

void SlbmInterface::setMaxDistance(const double& maxDistance)
{
  GreatCircle::MAX_DISTANCE = maxDistance;
}

void SlbmInterface::getMaxDistance(double& maxDistance)
{
  maxDistance = GreatCircle::MAX_DISTANCE;
}

void SlbmInterface::setMaxDepth(const double& maxDepth)
{
  GreatCircle_Xn::MAX_DEPTH = maxDepth;
}

void SlbmInterface::getMaxDepth(double& maxDepth)
{
  maxDepth = GreatCircle_Xn::MAX_DEPTH;
}

void SlbmInterface::setCHMax(const double& chMax)
{
  CH_MAX = chMax;
}

void SlbmInterface::getCHMax(double& chMax)
{
  chMax = CH_MAX;
}

void SlbmInterface::setDelDistance(const double& del_distance)
{ GreatCircle::setDelDistance(del_distance); }

void SlbmInterface::getDelDistance(double& del_distance)
{ del_distance = GreatCircle::getDelDistance(); }

void SlbmInterface::setDelDepth(const double& del_depth)
{ GreatCircle::setDelDepth(del_depth); }

void SlbmInterface::getDelDepth(double& del_depth)
{ del_depth = GreatCircle::getDelDepth(); }

void SlbmInterface::setPathIncrement(const double& path_increment)
{ GreatCircle::setPathIncrement(path_increment); }

void SlbmInterface::getPathIncrement(double& path_increment)
{ path_increment = GreatCircle::getPathIncrement(); }

double SlbmInterface::getPathIncrement()
{ return GreatCircle::getPathIncrement(); }

} // end slbm namespace
