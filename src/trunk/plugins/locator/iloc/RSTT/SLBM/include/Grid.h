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
//- Program:       Grid
//- Module:        $RCSfile: Grid.h,v $
//- Revision:      $Revision: 1.61 $
//- Last Modified: $Date: 2013/07/24 18:24:04 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef SLBM_GRID_H
#define SLBM_GRID_H

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
#include "Uncertainty.h"
#include "GeoTessPolygon.h"

#include "DataBuffer.h"
#include "MD50.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

//!
//! \brief A 2 dimensional, horizontal grid of GirdProfile objects.
//!
//! A 2 dimensional, horizontal grid of GridProfile objects. 
//!
//! When an application or another object in the slbm library
//! requests a CrustalProfile object by calling Grid::getCrustalProfile()
//! Grid will check to see if it already has a pointer to a 
//! CrustalProfile object for the same phase and at the same 
//! Location.  If it does, it will return a pointer to the 
//! existing CrustalProfile object.  If it does not, it will
//! create a new CrustalProfile object and return that.  
//! The map of crustalProfiles can be cleared by calling 
//! clearCrustalProfiles().  The CrustalProfile objects owned by a Grid
//! object are deleted in the Grid destructor.
class SLBM_EXP_IMP Grid
{

public:

	//! \brief Default constructor.
	//!
	//! Default constructor.
	Grid();

	//! \brief Destructor.
	//!
	//! Destructor.
	virtual ~Grid();

	static Grid* getGrid(const string& modelDirectory);

	static Grid* getGrid(util::DataBuffer& buffer);

	static bool fileExists(const string& fileName);

	//! \brief Equality operator.
	//!
	//! Equality operator.
	virtual bool operator == (const Grid& other);

	//! \brief Inequality operator
	//!
	//! Inequality operator
	virtual bool operator != (const Grid& other) {return !(*this == other);};

	/**
	 * Returns the class name.
	 */
	static string class_name() {return "Grid"; };

	//! \brief Clears and releases all memory held by this Grid object.
	//!
	//! Clears and releases all memory held by this Grid object.
	virtual void clear();

	//! \brief Load the depth, velocity and gradient information from an
	//! ascii flat file.
	//!
	//! Load the Earth model information from an ascii flat file.
	//! @param filename the name of the ascii flat file containing the data.
	virtual void loadFromFile(const string& filename) = ABSTRACT;

	//! \brief Load the depth, velocity and gradient information from
	//! binary files in specified directory.
	//!
	//! Load the Earth model information from binary files in specified directory
	//! @param dirName the name of directory from which to load the model
	virtual void loadFromDirectory(const string& dirName) = ABSTRACT;

	//! \brief Load the depth, velocity and gradient information from
	//! DataBuffer.
	//!
	//! Load the Earth model information from DataBuffer
	//! @param buffer the DataBuffer from which to load the model
	virtual void loadFromDataBuffer(util::DataBuffer& buffer) = ABSTRACT;

	//! \brief Save the Earth model currently in memory, to a DataBuffer.
	//!
	//! Save the Earth model currently in memory, to to a DataBuffer.
	//! @param buffer the DataBuffer to which to save the model
	virtual void saveVelocityModel(util::DataBuffer& buffer) = ABSTRACT;

	//! \brief Save the Earth model currently in memory, to an ascii flat file.
	//!
	//! Save the Earth model currently in memory, to an ascii flat file.
	//! An exception will be thrown if the speficied filename is the
	//! same as the name of the file from which the Earth model was
	//! originally loaded with loadVelocityModel()
	virtual void saveVelocityModel(const string& filename, const int& format) = ABSTRACT;

	virtual int getBufferSize() const = ABSTRACT;

	virtual bool is_directory(const string& dir);

	//! \brief Retrieve the number of nodes in the model.
	//!
	//! Retrieve the number of nodes in the model.
	virtual int getNNodes()      { return profiles.size(); }

	//! \brief Retrieve the number of intervals in each Profile.
	//!
	//! Retrieve the number of intervals in each Profile.  There will be
	//! one interval for each layer in the crust, plus one more for the mantle.
	virtual int  getNIntervals()    { return NLAYERS;};

	//! \brief Retrieve a pointer to the GridProfile that is node (i, j) in
	//! this Grid.
	//!
	//! Retrieve a pointer to the GridProfile that is node (i, j) in
	//! this Grid.
	//! @param nodeId the node index of the desired grid node.
	virtual GridProfile* getProfile(const int& nodeId);

	//! \brief Retrieve a GreatCircle object between a source and a receiver.
	//!
	//! Retrieve a GreatCircle object composed of two CrustalProfile
	//! objects, one at either end of the GreatCircle, and a number
	//! of equally spaced LayerProfile objects positioned along the
	//! interface where the head wave will travel.
	//! @param phase the phase that this GreatCircle is to support.
	//! Must be one of SLBMGlobals::Pn, SLBMGlobals::Sn, SLBMGlobals::Pg or
	//! SLBMGlobals::Lg.
	//! @param latSource the geographic latitude at the source end
	//! of the GreatCircle, in radians.
	//! @param lonSource the geographic longitude at the source end
	//! of the GreatCircle, in radians.
	//! @param depthSource the depth of the source, km
	//! @param latReceiver the geographic latitude at the receiver end
	//! of the GreatCircle, in radians.
	//! @param lonReceiver the geographic longitude at the receiver end
	//! of the GreatCircle, in radians.
	//! @param depthReceiver the depth of the receiver, in km.
	//! @param delta the desired horizontal separation of the LayerProfile
	//! objects along the head wave interface, in radians.  The actual
	//! separation will be reduced from the requested value somewhat
	//! in order that some number of equal sized increments will
	//! exactly fit between the source and receiver.
	//! @param ch_max c is the zhao c parameter and h is the turning depth of the
	//! ray below the moho.  Zhao method only valid for c*h << 1.
	//! When c*h > ch_max, then slbm will throw an exception.
	virtual GreatCircle* getGreatCircle(
			const int& phase,
			const double& latSource,
			const double& lonSource,
			const double& depthSource,
			const double& latReceiver,
			const double& lonReceiver,
			const double& depthReceiver,
			const double& delta,
			const double& ch_max);

	//! \brief Retrieve a CrustalProfile containing data appropriate
	//! for the specified phase at the specified location.
	//!
	//! Retrieve a CrustalProfile at the specified location.  For phases
	//! Pn and Sn the profile will include all intervals down to
	//! and including the mantle.  For Pg and Lg, the profile will
	//! include intervals down to and including the middle crust.
	//! For Pn and Pg, the velocities stored in the profile will be
	//! P wave velocities and for Sn and Lg the velocities will be
	//! S wave velocities.  No gradient information is stored in
	//! CrustalProfile objects.
	//! @param phase the phase for which the CrustalProfile is
	//! appropriate.  Must be one of Pn, Sn, Pg, Lg which are int
	//! constants defined in SLBMGlobals.h
	//! @param lat the geographic latitude in radians.
	//! @param lon the geographic longitude in radians.
	//! @param depth the depth of the source or receiver, in km.
	//CrustalProfile* getCrustalProfile(const int& phase,
	//	const double& lat, const double& lon, const double& depth);

	virtual CrustalProfile* getReceiverProfile(const int& phase,
			const double& lat, const double& lon, const double& depth);

	virtual CrustalProfile* getSourceProfile(const int& phase,
			const double& lat, const double& lon, const double& depth);

	//! \brief Delete all the CrustalProfile objects managed by this Grid.
	//!
	//! Delete all the CrustalProfile objects managed by this Grid.
	virtual void clearCrustalProfiles();

	//! \brief Retrieve a LayerProfile containing data appropriate for
	//! the specified GreatCircle, at the specified geographic latitude
	//! and longitude.
	//!
	//! Retrieve a LayerProfile containing data appropriate for
	//! the specified GreatCircle, at the specified geographic latitude
	//! and longitude.  For phases
	//! Pn and Sn the profile will include data for the mantle.
	//! For Pg and Lg, the profile will include data for the middle crust.
	//! For Pn and Pg, the velocity stored in the profile will be the
	//! P wave velocity and for Sn and Lg the velocity will be the
	//! S wave velocity.  For Pn and Sn, the gradient will be the
	//! P or S wave velocity gradient in the mantle.  For Pg and Lg
	//! the gradient will be SLBMGlobals::NA_VALUE.
	//! @param greatCircle the GreatCircle that the returned LayerProfile
	//! object is to support.
	//! @param location the Location of the LayerProfile
	virtual LayerProfile* getLayerProfile(GreatCircle* greatCircle,
			Location& location);

	//! \brief Retrieve a QueryProfile containing all available model
	//! information, at the latitude and longitude of the
	//! specified Location.
	//!
	//! Retrieve a QueryProfile containing all available model
	//! information, at the latitude and longitude of the
	//! specified Location.
	//! @param location the Location of the QueryProfile
	//! @return the QueryProfile
	virtual QueryProfile* getQueryProfile(Location& location);

	//!
	//! Retrieve an nPhases by nAttributes array of Uncertainty objects.
	//! The 4 phases are 0:Pn, 1:Sn, 2:Pg, 3:Lg.
	//! The 3 attributes are 0:TT, 1:SH, 2:AZ.
	//!
	vector<vector<Uncertainty*> >& getUncertainty() { return uncertainty; }

	//!
	//! Retrieve the uncertainty for specified phase and attribute
	//! @param phase one of 0:Pn, 1:Sn, 2:Pg, 3:Lg.
	//! @param attribute one of 0:TT, 1:SH, 2:AZ.
	//! @return reference to an uncertainty object.
	//!
	Uncertainty& getUncertainty(int phase, int attribute) { return *uncertainty[phase][attribute]; }

	//! \brief Retrieve the average P or S wave velocity of the mantle, in km/sec.
	//!
	//! Retrieve the average P or S wave velocity of the mantle.  These values are
	//! retrieved from the header information in the velocity model file.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @return the average P or S wave mantle velocity, in km/sec.
	virtual double getAverageMantleVelocity(const int& waveType) = ABSTRACT;

	//! \brief Retrieve the average P or S wave velocity of the mantle, in km/sec.
	//!
	//! Retrieve the average P or S wave velocity of the mantle.  These values are
	//! retrieved from the header information in the velocity model file.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @param velocity (output) the average P or S wave mantle velocity, in km/sec.
	virtual void setAverageMantleVelocity(const int& waveType, const double& velocity) = ABSTRACT;

	virtual int getNCrustalProfiles();

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
	//! @param location the Location of the desired profile
	//! @param neighbors the GridProfile objects which surround the specified
	//! Location.
	//! @param nodeIds the indices of the nodes correspondingn to each neighbor.
	//! @param coefficients the interpolation coefficient corresponding
	//! to each neighbor.
	//! @return true if successful, false if location is outside valid range of
	//! the model.
	virtual bool findProfile(Location& location,
			vector<GridProfile*>& neighbors, vector<int>& nodeIds,
			vector<double>& coefficients) = ABSTRACT;

	//! \brief Specify the latitude, longitude range in radians that will define
	//! which grid nodes are also active nodes.
	//!
	//! Specify the latitude, longitude range in radians that will define
	//! which grid nodes are also active nodes.  Active nodes are defined
	//! as follows:  Each triangle in the tessellation is visited.  If any
	//! one of the three nodes which define the triangle is located within
	//! the specified latitude, longitude range, then all three of the
	//! nodes are active nodes.
	void initializeActiveNodes(double activeNodeLatMin,
			double activeNodeLonMin,
			double activeNodeLatMax,
			double activeNodeLonMax);

	//! \brief Specify the Polygon that will define
	//! which grid nodes are also active nodes.
	//!
	//! Specify the Polygon that will define
	//! which grid nodes are also active nodes. Active nodes are defined
	//! as follows:  Visit every node in the grid.  If it is inside
	//! the polygon, then set it to be an active node and set all
	//! of its immediate neighbors active as well.
	void initializeActiveNodes(GeoTessPolygon* polygon);

	//! \brief Retrieve the number of active nodes.
	//!
	//! Retrieve the number of active nodes.
	virtual int getNActiveNodes() { return activeNodes.size(); };

	void clearActiveNodes();

	//! \brief Retrieve the grid node id that corresponds to a specified
	//! active node id.
	//!
	//! Retrieve the grid node id that corresponds to a specified
	//! active node id.
	virtual int getGridNodeId(int activeNodeId)
	{ return (activeNodeId < 0 ? -1 : activeNodes[activeNodeId]); };

	//! \brief Retrieve the grid node id that corresponds to a specified
	//! active node id.
	//!
	//! Retrieve the grid node id that corresponds to a specified
	//! active node id.
	virtual int getActiveNodeId(int nodeId)
	{ return profiles[nodeId]->getActiveNodeId(); };

	//! \brief Retrieve the grid node id of the nodes that are direct neighbors
	//! of the specified grid node.
	//!
	//! Retrieve the grid node id of the nodes that are direct neighbors
	//! of the specified grid node.
	virtual void getNodeNeighbors(const int& nodeId, int neighbors[], int& nNeighbors) = ABSTRACT;

	//! \brief Retrieve the grid node ids of the nodes that are direct neighbors
	//! of the specified grid node.
	//!
	//! Retrieve the grid node ids of the nodes that are direct neighbors
	//! of the specified grid node.
	virtual void getNodeNeighbors(const int& nodeId, vector<int>& neighbors) = ABSTRACT;

	//! \brief Retrieve the grid node id of the nodes that are direct neighbors
	//! of the specified grid node.
	//!
	//! Retrieve the grid node id of the nodes that are direct neighbors
	//! of the specified grid node.  Also returs the angular distance and
	//! azimuth from the specified grid node to each of its neighbors.
	//! Distances and azimuths are in radians.
	virtual void getNodeNeighborInfo(const int& nodeId, int neighbors[],
			double distance[], double azimuth[], int& nNeighbors);

	//! \brief Retrieve the grid node id of the nodes that are direct neighbors
	//! of the specified grid node.
	//!
	//! Retrieve the grid node id of the nodes that are direct neighbors
	//! of the specified grid node.  Also returns the angular distance and
	//! azimuth from the specified grid node to each of its neighbors.
	//! Distances and azimuths are in radians.
	virtual void getNodeNeighborInfo(const int& nodeId, vector<int>& neighbors,
			vector<double>& distance, vector<double>& azimuth);

	virtual void getActiveNodeNeighbors(const int& nodeid, int neighbors[], int& nNeighbors) = ABSTRACT;

	virtual void getActiveNodeNeighbors(const int& nodeid, vector<int>& neighbors) = ABSTRACT;

	virtual void getActiveNodeNeighborInfo(const int& nid, int neighbors[],
			double distance[], double azimuth[], int& nNeighbors);

	virtual void getActiveNodeNeighborInfo(const int& nid, vector<int>& neighbors,
			vector<double>& distance, vector<double>& azimuth);

	//! \brief Retrieve the angular separation in radians between any pair of
	// grid nodes.
	//!
	//! Retrieve the angular separation in radians between any pair of
	// grid nodes.
	virtual void getNodeSeparation(const int& node1, const int& node2, double& distance);

	//! \brief Retrieve the azimuth in radians from one grid node to another.
	//!
	//! Retrieve the azimuth in radians from one grid node to another.
	virtual void getNodeAzimuth(const int& node1, const int& node2, double& azimuth);

	virtual void getNodeHitCount(const int& nodeId, int& hitCount)
	{ hitCount = profiles[nodeId]->getHitCount(); };

	virtual void clearNodeHitCount();

	virtual int addGeoStack(GeoStack* geoStack) = ABSTRACT;

	virtual size_t memSize() = ABSTRACT;

	virtual size_t memSizeCrustalProfiles();

	virtual string getTessId() = ABSTRACT;

	string& getModelPath() { return modelPath; }

	virtual string toString() = ABSTRACT;

	virtual void setInterpolatorType(const string& interpolatorType) = ABSTRACT;

	virtual string getInterpolatorType() = ABSTRACT;

	const string getOutputDirectory() { return outputDirectory; }

	void specifyOutputDirectory(const string& outputDir);

protected:

	//! \brief The name of the file or directory from which the velocity model was loaded.
	//!
	//! The name of the file or directory from which the velocity model was loaded.
	string modelPath;

	//! \brief The GridProfile objects which constitute the nodes in a Grid object.
	//!
	//! The GridProfile objects which constitute the nodes in a Grid object.
	vector<GridProfile*> profiles;

	vector<int> activeNodes;

	GeoTessPolygon* polygon;

	CrustalProfileStore* sources;
	CrustalProfileStore* receivers;

	vector<vector<Uncertainty*> > uncertainty;

	void writeBufferToFile(util::DataBuffer& buffer, string fileName);

	void reaDataBuffererFromFile(util::DataBuffer& buffer, string dirname,
			string fileName);

private:

	string outputDirectory;


};

inline GridProfile* Grid::getProfile(const int& n)
{
	if (n < 0 || n >= (int)profiles.size())
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(4);
		os << endl << "ERROR in Grid::getProfile. NodeId "<< n << " is out of range.  " << endl
				<< "Valid range is >= 0 and < " << profiles.size() << endl
				<< "Version " << SlbmVersion << " File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),107);
	}
	return profiles[n];
}

inline 	LayerProfile* Grid::getLayerProfile(GreatCircle* gc,
		Location& location)
{
	if (gc->getPhase()/2 == 0)
		// for Pn or Sn, return a LayerProfile with gradient info.
		return new LayerProfileG(gc, location);
	else
		// for Pg or Lg, return a LayerProfile without gradient info.
		return new LayerProfile(gc, location);
}

inline 	QueryProfile* Grid::getQueryProfile(Location& location)
{
	return new QueryProfile(*this, location);
}

inline 	CrustalProfile* Grid::getSourceProfile(const int& phase,
		const double& lat, const double& lon, const double& depth)
{
	return sources->getCrustalProfile(phase, lat, lon, depth);
}

inline 	CrustalProfile* Grid::getReceiverProfile(const int& phase,
		const double& lat, const double& lon, const double& depth)
{
	return receivers->getCrustalProfile(phase, lat, lon, depth);
}

inline void Grid::clearCrustalProfiles()
{
	receivers->clear();
	sources->clear();
}

inline int Grid::getNCrustalProfiles()
{
	return receivers->getNCrustalProfiles()
			+ sources->getNCrustalProfiles();
}

inline size_t Grid::memSizeCrustalProfiles()
{
	return receivers->memSize() + sources->memSize();
}

inline void Grid::clearNodeHitCount()
{
	for (int i=0; i<(int)profiles.size(); ++i)
		profiles[i]->clearHitCount();
}

inline bool Grid::fileExists(const string& fileName)
{
	fstream f;

	f.open(fileName.c_str(), ios::in);
	if (f.is_open())
	{
		f.close();
		return true;
	}
	return false;
}


} // end slbm namespace

#endif // GRID_H
