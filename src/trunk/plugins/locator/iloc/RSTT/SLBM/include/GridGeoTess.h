/*
 * GridGeoTess.h
 *
 *  Created on: Sep 25, 2012
 *      Author: sballar
 */

#ifndef GRIDGEOTESS_H_
#define GRIDGEOTESS_H_

#include <map>

#include "Grid.h"
#include "GeoTessModelSLBM.h"
#include "GeoTessPosition.h"
#include "GridProfileGeoTess.h"

using namespace std;
using namespace geotess;

namespace slbm {

class SLBM_EXP_IMP GridGeoTess: public Grid
{
public:

	GridGeoTess();

	//! \brief Destructor.
	//!
	//! Destructor.
	virtual ~GridGeoTess();

	/**
	 * Returns the class name.
	 */
	static  string				class_name() { return "GridGeoTess"; };

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

	//! \brief Save the Earth model currently in memory, to an ascii flat file.
	//!
	//! Save the Earth model currently in memory, to an ascii flat file.
	//! An exception will be thrown if the speficied filename is the
	//! same as the name of the file from which the Earth model was
	//! originally loaded with loadVelocityModel()
	void saveVelocityModel(const string& filename, const int& format);

	//! \brief Save the Earth model currently in memory, to a DataBuffer.
	//!
	//! Save the Earth model currently in memory, to to a DataBuffer.
	//! @param buffer the DataBuffer to which to save the model
	void saveVelocityModel(util::DataBuffer& buffer);

	//! \brief Returns the size of a DataBuffer object required
	//! to store this Grid objects model data.
	//!
	//! Returns the size of a DataBuffer object required
	//! to store this Grid objects model data.
	int  getBufferSize() const { return model->getBufferSize(); };

	//! \brief Retreive the average P or S wave velocity of the mantle, in km/sec.
	//!
	//! Retreive the average P or S wave velocity of the mantle.  These values are
	//! retrieved from the header information in the velocity model file.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @return the average P or S wave mantle velocity, in km/sec.
	double getAverageMantleVelocity(const int& waveType)
	{return model->getAverageMantleVelocity(waveType); };

	//! \brief Retreive the average P or S wave velocity of the mantle, in km/sec.
	//!
	//! Retreive the average P or S wave velocity of the mantle.  These values are
	//! retrieved from the header information in the velocity model file.
	//! @param waveType either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @param velocity (output) the average P or S wave mantle velocity, in km/sec.
	void setAverageMantleVelocity(const int& waveType, const double& velocity)
	{model->setAverageMantleVelocity(waveType, velocity); };

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

//	//! \brief Specify the latitude, longitude range that will define
//	//! which grid nodes are also active nodes.
//	//!
//	//! Specify the latitude, longitude range that will define
//	//! which grid nodes are also active nodes.  Active nodes are defined
//	//! as follows:  Each triangle in the tessellation is visited.  If any
//	//! one of the three nodes which define the triangle is located within
//	//! the specified latitude, longitude range, then all three of the
//	//! nodes are active nodes.
//	void initializeActiveNodes(double activeNodeLatMin,
//			double activeNodeLonMin,
//			double activeNodeLatMax,
//			double activeNodeLonMax);

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

	//! \brief Retrieve the grid node id of the nodes that are direct neighbors
	//! of the specified grid node.
	//!
	//! Retrieve the grid node id of the nodes that are direct neighbors
	//! of the specified grid node.  Also returns the angular distance and
	//! azimuth from the specified grid node to each of its neighbors.
	//! Distances and azimuths are in radians.
	void getNodeNeighborInfo(const int& nodeid, int neighbors[],
				double distance[], double azimuth[], int& nNeighbors)
	{
		set<int> nbrs;
		model->getGrid().getVertexNeighbors(0, model->getGrid().getLastLevel(0),
				nodeid, nbrs);

		nNeighbors = (int)nbrs.size();
		int nid, i=0;
		for (set<int>::iterator it=nbrs.begin(); it != nbrs.end(); it++)
		{
			nid = profiles[*it]->getNodeId();

			neighbors[i]=nid;

			distance[i] = GeoTessUtils::angle(model->getGrid().getVertex(nodeid),
					model->getGrid().getVertex(nid));

			azimuth[i] = GeoTessUtils::azimuth(model->getGrid().getVertex(nodeid),
					model->getGrid().getVertex(nid), NA_VALUE);

			++i;
		}
	}

	void getNodeNeighborInfo(const int& nodeid, vector<int>& neighbors,
			vector<double>& distance, vector<double>& azimuth)
	{
		set<int> nbrs;
		model->getGrid().getVertexNeighbors(0, model->getGrid().getLastLevel(0),
				nodeid, nbrs);

		int nNeighbors = (int)nbrs.size();
		neighbors.clear();
		distance.clear();
		azimuth.clear();
		neighbors.reserve(nNeighbors);
		distance.reserve(nNeighbors);
		azimuth.reserve(nNeighbors);
		int nid;
		for (set<int>::iterator it=nbrs.begin(); it != nbrs.end(); it++)
		{
			nid = profiles[*it]->getNodeId();

			neighbors.push_back(nid);

			distance.push_back(GeoTessUtils::angle(model->getGrid().getVertex(nodeid),
					model->getGrid().getVertex(nid)));

			azimuth.push_back(GeoTessUtils::azimuth(model->getGrid().getVertex(nodeid),
					model->getGrid().getVertex(nid), NA_VALUE));
		}
	}

	//! \brief Retrieve the angular separation in radians between any pair of
	// grid nodes.
	//!
	//! Retrieve the angular separation in radians between any pair of
	// grid nodes.
	void getNodeSeparation(const int& node1, const int& node2, double& distance);

	//! \brief Retrieve the azimuth in radians from one grid node to another.
	//!
	//! Retrieve the azimuth in radians from one grid node to another.
	void getNodeAzimuth(const int& node1, const int& node2, double& azimuth);

	size_t memSize();

	string getTessId() { return model->getGrid().getGridID(); }

	int addGeoStack(GeoStack* geoStack)
	{ return 0; };

	GeoTessModelSLBM* getModel() { return model; }

	string toString();

	void setInterpolatorType(const string& interpolatorType);

	string getInterpolatorType() { return position->getInterpolatorType().toString(); }

private:

	GeoTessModelSLBM* model;

	GeoTessPosition* position;

};

inline void GridGeoTess::setInterpolatorType(const string& interpolatorType)
{
	string type = CPPUtils::uppercase_string(interpolatorType);
	if (position->getInterpolatorType().toString() != type)
	{
		if (type == "LINEAR")
		{
			delete position;
			position = model->getPosition(GeoTessInterpolatorType::LINEAR);
		}
		else if (type == "NATURAL_NEIGHBOR")
		{
			delete position;
			position = model->getPosition(GeoTessInterpolatorType::NATURAL_NEIGHBOR);
		}
		else
		{
			ostringstream os;
			os << endl << "ERROR in GridGeoTess::setInterpolatorType()" << endl
				<< interpolatorType << " is not a recognized interpolator type." << endl
				<< "Must be one of [ LINEAR | NATURAL_NEIGHBOR ]."
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
			throw SLBMException(os.str(),114);
		}
	}
}

inline void GridGeoTess::getNodeNeighbors(const int& nodeId, int neighbors[], int& nNeighbors)
{
	set<int> nbrs;
	model->getGrid().getVertexNeighbors(0, model->getGrid().getLastLevel(0),
			nodeId, nbrs);
	nNeighbors = (int)nbrs.size();
	set<int>::iterator it;
	int i=0;
	for (it=nbrs.begin(); it != nbrs.end(); ++it)
		neighbors[i++] = *it;
}

inline void GridGeoTess::getNodeNeighbors(const int& nodeId, vector<int>& neighbors)
{
	set<int> nbrs;
	const int level = model->getGrid().getLastLevel(0);
	model->getGrid().getVertexNeighbors(0, level, nodeId, nbrs);
	neighbors.resize(nbrs.size());
	set<int>::iterator it;
	int i=0;
	for (it=nbrs.begin(); it != nbrs.end(); ++it)
		neighbors[i++] = *it;
}

inline void GridGeoTess::getActiveNodeNeighbors(const int& activeNodeId, int neighbors[], int& nNeighbors)
{
	int nodeId = getGridNodeId(activeNodeId);
	if (nodeId < 0)
		nNeighbors = 0;
	else
	{
		set<int> nbrs;
		model->getGrid().getVertexNeighbors(0, model->getGrid().getLastLevel(0),
				nodeId, nbrs);
		nNeighbors = 0;
		int id;
		for (set<int>::iterator it=nbrs.begin(); it != nbrs.end(); it++)
		{
			id = getActiveNodeId(profiles[*it]->getNodeId());
			if (id >= 0)
				neighbors[nNeighbors++] = id;
		}
	}
}

inline void GridGeoTess::getActiveNodeNeighbors(const int& activeNodeId, vector<int>& neighbors)
{
	neighbors.clear();

	int nodeId = getGridNodeId(activeNodeId);
	if (nodeId >= 0)
	{
		set<int> nbrs;
		model->getGrid().getVertexNeighbors(0, model->getGrid().getLastLevel(0),
				nodeId, nbrs);
		int id;
		for (set<int>::iterator it=nbrs.begin(); it != nbrs.end(); it++)
		{
			id = getActiveNodeId(profiles[*it]->getNodeId());
			if (id >= 0)
				neighbors.push_back(id);
		}
	}
}

inline void GridGeoTess::getNodeSeparation(const int& node1, const int& node2, double& distance)
{
	distance = GeoTessUtils::angle(model->getGrid().getVertex(node1),
			model->getGrid().getVertex(node2));
}

inline void GridGeoTess::getNodeAzimuth(const int& node1, const int& node2, double& azimuth)
{
	azimuth = GeoTessUtils::azimuth(model->getGrid().getVertex(node1),
			model->getGrid().getVertex(node2), NaN_DOUBLE);
}

} /* namespace slbm */
#endif /* GRIDGEOTESS_H_ */
