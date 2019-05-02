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
//- Program:       GridSLBM
//- Module:        $RCSfile: GridSLBM.h,v $
//- Revision:      $Revision: 1.16 $
//- Last Modified: $Date: 2013/08/13 22:51:25 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef GRIDSLBM_H
#define GRIDSLBM_H

// **** _SYSTEM INCLUDES_ ******************************************************
#include <vector>
#include <map>
#include <cmath>

using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "SLBMGlobals.h"
#include "GridProfile.h"
#include "InterpolatedProfile.h"
#include "LayerProfile.h"
#include "LayerProfileG.h"
#include "QueryProfile.h"
#include "CrustalProfile.h"
#include "CrustalProfileStore.h"
#include "GreatCircle.h"
#include "GreatCircleFactory.h"
#include "Location.h"
#include "SLBMException.h"
#include "Triangle.h"

#include "DataBuffer.h"
#include "MD50.h"
#include "CPPUtils.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

//!
//! \brief A 2 dimensional, horizontal grid of GirdProfile objects.
//!
//! A 2 dimensional, horizontal grid of GridProfile objects.
//!
//! When an application or another object in the slbm library
//! requests a CrustalProfile object by calling GridSLBM::getCrustalProfile()
//! GridSLBM will check to see if it already has a pointer to a
//! CrustalProfile object for the same phase and at the same
//! Location.  If it does, it will return a pointer to the
//! existing CrustalProfile object.  If it does not, it will
//! create a new CrustalProfile object and return that.
//! The map of crustalProfiles can be cleared by calling
//! clearCrustalProfiles().  The CrustalProfile objects owned by a GridSLBM
//! object are deleted in the GridSLBM destructor.
class SLBM_EXP_IMP GridSLBM : public Grid
{

public:

	//! \brief Default constructor.
	//!
	//! Default constructor.
	GridSLBM();

	//! \brief Destructor.
	//!
	//! Destructor.
	~GridSLBM();

	/**
	 * Returns the class name.
	 */
	static  string				class_name() { return "GridSLBM"; };

	//! \brief Clears and releases all memory held by this GridSLBM object.
	//!
	//! Clears and releases all memory held by this GridSLBM object.
	void clear();

	//! \brief Load the depth, velocity and gradient information from an
	//! ascii flat file.
	//!
	//! Load the Earth model information from an ascii flat file.
	//! @param filename the name of the ascii flat file containing the data.
	void loadFromFile(const string& filename);

	//! \brief Load the depth, velocity and gradient information from
	//! binary files in specified directory.
	//!
	//! Load the Earth model information from binary files in specified directory
	//! @param dirName the name of directory from which to load the model
	void loadFromDirectory(const string& dirName);

	void loadFromDataBuffer(util::DataBuffer& buffer);

	//! \brief Save the Earth model currently in memory, to file(s)
	//!
	//! Save the Earth model currently in memory, to to file(s)
	//! An exception will be thrown if the speficied filename is the
	//! same as the name of the file from which the Earth model was
	//! originally loaded with loadVelocityModel()
	//! @param destination the name of the file or directory where the
	//! model information is to be saved.
	//! @param format integer in range 1 to 4
	void saveVelocityModel(const string& destination, const int& format);

	//! \brief Save the Earth model currently in memory, to a DataBuffer.
	//!
	//! Save the Earth model currently in memory, to to a DataBuffer.
	//! @param buffer the DataBuffer to which to save the model
	void saveVelocityModel(util::DataBuffer& buffer);

	int getBufferSize() const;

    //! \brief Retreive the average P or S wave velocity of the mantle, in km/sec.
	//!
	//! Retreive the average P or S wave velocity of the mantle.  These values are
	//! retrieved from the header information in the velocity model file.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @return the average P or S wave mantle velocity, in km/sec.
	double getAverageMantleVelocity(const int& waveType) {return V0[waveType]; };

	//! \brief Retreive the average P or S wave velocity of the mantle, in km/sec.
	//!
	//! Retreive the average P or S wave velocity of the mantle.  These values are
	//! retrieved from the header information in the velocity model file.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @param velocity (output) the average P or S wave mantle velocity, in km/sec.
	void setAverageMantleVelocity(const int& waveType, const double& velocity)
	{V0[waveType] = velocity; };

	//! \brief Find the neighboring GridProfile objects and associated
	//! interpolation coefficients at a specified Location.
	//!
	//! Given a Location, find the GridProfile objects that surround the
	//! Location and compute the interpolation coefficient for each
	//! of those surrounding GridProfile objects.  Interpolated values
	//! will equal the sum of the values at the GridProfile objects times
	//! the corresponding interpolation coefficient.
	//!
	//! This method is called by getCrustalProfile() and getMantleProfile()
	//! to compute InterpolatedProfile objects.
	//! @param location the Location of the desired profile.
	//! @param neighbors the GridProfile objects which surround the specified
	//! Location.
	//! @param nodeIds the node indices of the neighbors
	//! @param coefficients the interpolation coefficient corresponding
	//! to each neighbor.
	//! @return true if successful, false if location is outside valid range of
	//! the model.
	virtual bool findProfile(Location& location,
			vector<GridProfile*>& neighbors, vector<int>& nodeIds,
			vector<double>& coefficients);

	//! \brief Retrieve the grid node id of the nodes that are direct neighbors
	//! of the specified grid node.
	//!
	//! Retrieve the grid node id of the nodes that are direct neighbors
	//! of the specified grid node.
	void getNodeNeighbors(const int& nodeId, int neighbors[], int& nNeighbors);

	//! \brief Retrieve the grid node ids of the nodes that are direct neighbors
	//! of the specified grid node.
	//!
	//! Retrieve the grid node ids of the nodes that are direct neighbors
	//! of the specified grid node.
	void getNodeNeighbors(const int& nodeId, vector<int>& neighbors);

	void getActiveNodeNeighbors(const int& nodeid, int neighbors[], int& nNeighbors);

	void getActiveNodeNeighbors(const int& nodeid, vector<int>& neighbors);

	size_t memSize();

	string getTessId() { return tessId; }

	int addGeoStack(GeoStack* geoStack)
	{ geoStacks.push_back(geoStack); return geoStacks.size(); };

	string toString();

	void setInterpolatorType(const string& interpolatorType);

	string getInterpolatorType() { return "linear"; }

private:

	int getGeoStacksSize() { return geoStacks.size(); };

	vector<GeoStack*>& getGeoStacks() { return geoStacks; };

	vector<Triangle*>& getTriangles() { return triangles; };

	void readGeoStacks(util::DataBuffer& buffer);

	void readConnectivity(util::DataBuffer& buffer, int& nNodes,
			vector<float>& elev, vector<float>& waterThick,
			vector<int>& stackId);

	void readTessellationData(util::DataBuffer& buffer, int nNodes,
			const vector<float>& elev,
			const vector<float>& waterThick,
			const vector<int>& stackId,
			vector< vector<Triangle*> >& triset);

	void defineTessAdjacency(int nNodes,
			const vector< vector<Triangle*> >& triset);

	string tessId;

	vector<GeoStack*> geoStacks;

	vector<Triangle*> triangles;

	vector<Triangle*> specialTriangles;

	double cos_min;

	//! \brief The average P and S wave velocity of the mantle.
	//!
	//! The average P and S wave velocity of the mantle.  These values are
	//! retrieved from the header information in the velocity model file.
	double V0[2];

	Triangle* findTriangle(const Location& location,
			vector<double>& coefficients);

	void findSpecialTriangles();

	void saveSlbmFile(const string& fileName);
	void saveSlbmDirectory(const string& directoryName);

	void saveGeotessFile(const string& fileName);
	void saveGeotessDirectory(const string& directoryName);

	void saveGeotess(const string& path, const string& pathToGrid, const string& gridFilePath);

};

inline Triangle* GridSLBM::findTriangle(const Location& location,
		vector<double>& coefficients)
{
	Triangle* tr = NULL;

	double dot, dmax = -1e30;
	// iterate over all specialTriangles.
	for (int i=0; i<(int)specialTriangles.size(); i++)
	{
		// find the dot product of special triangle node 0
		// with desired position.
		dot = location.dot(*specialTriangles[i]->getNode(0));

		// large dot product corresponds to small separation.
		// if this is closest special triangle so far, save it
		if (dot > dmax)
		{
			// set best triangle to this special triangle.
			tr = specialTriangles[i];
			// if this triangle is closer than about 16 degrees
			// then no closer triangle will be found, so quit.
			if (dot > cos_min) break;
			dmax = dot;
		}
	}

	// search for the triangle that contains position, starting
	// from the triangle identified above.
	tr = tr->walk(location, coefficients);

	// set first special triangle to the triangle that was just found so that
	// it will be checked first on the next call to findTriangle().
	specialTriangles[0] = tr;

	return tr;
}

inline bool GridSLBM::findProfile(Location& location,
		vector<GridProfile*>& neighbors, vector<int>& nodeIds,
		vector<double>& coefficients)
{
	neighbors.resize(3);
	coefficients.resize(3);
	nodeIds.resize(3);

	Triangle* tr = findTriangle(location, coefficients);

	for (int i=0; i<3; i++)
	{
		neighbors[i] = tr->getNode(i);
		nodeIds[i] = neighbors[i]->getNodeId();
	}

	return true;
}

inline void GridSLBM::setInterpolatorType(const string& interpolatorType)
{
	if (geotess::CPPUtils::uppercase_string(interpolatorType) != "LINEAR")
	{
			ostringstream os;
			os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
			  os << endl << "ERROR in GridSLBM::setInterpolatorType()" << endl
				<< interpolatorType << " is not a recognized interpolator type." << endl
				<< "The only interpolator type recognized by the old style SLBM grids is LINEAR"
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
			throw SLBMException(os.str(),114);
	}
}

} // end slbm namespace

#endif // GRIDSLBM_H
