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
//- Module:        $RCSfile: Grid.cc,v $
//- Revision:      $Revision: 1.62 $
//- Last Modified: $Date: 2013/08/14 22:33:24 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#include "Grid.h"
#include "GridSLBM.h"
#include "GridGeoTess.h"
#include "SLBMGlobals.h"
#include "CPPUtils.h"
#include "GeoTessPolygon.h"

#include <sys/stat.h>
#include <errno.h>

#if defined(_WIN32) || defined(WIN32)
// Windows compiler
#include <direct.h>       // Windows directory creation/removal
#include <io.h>           // Windows file/directory existence checking
#define PGL_MKDIR_OPTIONS // Windows - no options
#else
// Sun compiler
#include <dirent.h>       // Sun directory creation/removal
#define PGL_MKDIR_OPTIONS , S_IRWXU | S_IRWXG | S_IRWXO  // Sun mkdir param 2
#endif

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

//using namespace std;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

// **** _FUNCTION DESCRIPTION_ *************************************************
//
// Parameterized Grid Constructor
//
// *****************************************************************************
Grid::Grid() : polygon(NULL)
{
	sources = new CrustalProfileStore(*this, 10);
	receivers = new CrustalProfileStore(*this, 1000);

	uncertainty.resize(4);
	for (int i=0; i<(int)uncertainty.size(); ++i)
		uncertainty[i].resize(3, NULL);

}


// **** _FUNCTION DESCRIPTION_ *************************************************
//
// Grid Destructor
//
// *****************************************************************************
Grid::~Grid()
{
	if (polygon != NULL)
	{
		delete polygon;
		polygon = NULL;
	}

	delete sources;
	delete receivers;

	sources = NULL;
	receivers = NULL;

	for (int i=0; i<(int)uncertainty.size(); ++i)
		for (int j=0; j<(int)uncertainty[i].size(); ++j)
			delete uncertainty[i][j];
}

void Grid::clear()
{
	for (unsigned i=0; i<profiles.size(); i++)
		if (profiles[i]) delete profiles[i];
	profiles.clear();

	clearCrustalProfiles();

}

Grid* Grid::getGrid(const string& modelname)
{
	fstream dataFile;

	string modelFileName = CPPUtils::insertPathSeparator(modelname, "geostacks");

	dataFile.open(modelFileName.c_str(), ios::in);
	if (dataFile.is_open())
	{
		dataFile.close();
		GridSLBM* grid = new GridSLBM();
		grid->loadFromDirectory(modelname);
		return grid;
	}

	modelFileName = CPPUtils::insertPathSeparator(modelname, "geotessmodel");

	dataFile.open(modelFileName.c_str(), ios::in);
	if (dataFile.is_open())
	{
		dataFile.close();
		GridGeoTess* grid = new GridGeoTess();
		grid->loadFromDirectory(modelname);
		return grid;
	}

	if (!fileExists(modelname))
	{
		ostringstream os;
		os << endl << "ERROR in Grid::getGrid(const string& modelname)." << endl
				<< modelname << " does not exist." << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),105);
	}

	if (GeoTessModel::isGeoTessModel(modelname))
	{
		GridGeoTess* grid = new GridGeoTess();
		grid->loadFromFile(modelname);
		return grid;
	}

	GridSLBM* grid = new GridSLBM();
	grid->loadFromFile(modelname);
	return grid;

}

Grid* Grid::getGrid(util::DataBuffer& buffer)
{
	string modelType;

	// read 12 characters into a string.
	buffer.readString(modelType, 12);
	// restore the pointer to original position
	buffer.decrementPos(12);

	if (modelType == "GEOTESSMODEL")
	{
		GridGeoTess* grid = new GridGeoTess();
		grid->loadFromDataBuffer(buffer);
		return grid;
	}
	else
	{
		GridSLBM* grid = new GridSLBM();
		grid->loadFromDataBuffer(buffer);
		return grid;
	}
	return NULL;
}

bool Grid::operator==(const Grid& other)
{
	if (other.class_name() != class_name())
		return false;

	if (other.profiles.size() != profiles.size()) return false;

	for (int i=0; i< (int)profiles.size(); i++)
		if (profiles[i] != other.profiles[i])
			return false;

	return true;
}

GreatCircle* Grid::getGreatCircle(
		const int& phase,
		const double& latSource,
		const double& lonSource,
		const double& depthSource,
		const double& latReceiver,
		const double& lonReceiver,
		const double& depthReceiver,
		const double& delta,
		const double& ch_max)
{
	return GreatCircleFactory::create(
			phase, this,
			latSource, lonSource, depthSource,
			latReceiver, lonReceiver, depthReceiver,
			ch_max);
}

void Grid::getNodeNeighborInfo(const int& nid, int neighbors[],
		double distance[], double azimuth[], int& nNeighbors)
{
	getNodeNeighbors(nid, neighbors, nNeighbors);
	for (int i=0; i<nNeighbors; i++)
	{
		getNodeSeparation(nid, neighbors[i], distance[i]);
		getNodeAzimuth(nid, neighbors[i], azimuth[i]);
	}
}

void Grid::getActiveNodeNeighborInfo(const int& nid, int neighbors[],
		double distance[], double azimuth[], int& nNeighbors)
{
	getActiveNodeNeighbors(nid, neighbors, nNeighbors);
	int id = getGridNodeId(nid);
	for (int i=0; i<nNeighbors; i++)
	{
		getNodeSeparation(id, getGridNodeId(neighbors[i]), distance[i]);
		getNodeAzimuth(id, getGridNodeId(neighbors[i]), azimuth[i]);
	}
}

void Grid::getNodeNeighborInfo(const int& nid, vector<int>& neighbors,
		vector<double>& distance, vector<double>& azimuth)
{
	getNodeNeighbors(nid, neighbors);
	distance.resize(neighbors.size());
	azimuth.resize(neighbors.size());
	for (int i=0; i<(int)neighbors.size(); i++)
	{
		getNodeSeparation(nid, neighbors[i], distance[i]);
		getNodeAzimuth(nid, neighbors[i], azimuth[i]);
	}
}

void Grid::getActiveNodeNeighborInfo(const int& nid, vector<int>& neighbors,
		vector<double>& distance, vector<double>& azimuth)
{
	getActiveNodeNeighbors(nid, neighbors);
	int id = getGridNodeId(nid);
	distance.resize(neighbors.size());
	azimuth.resize(neighbors.size());
	for (int i=0; i<(int)neighbors.size(); i++)
	{
		getNodeSeparation(id, getGridNodeId(neighbors[i]), distance[i]);
		getNodeAzimuth(id, getGridNodeId(neighbors[i]), azimuth[i]);
	}
}

void Grid::getNodeSeparation(const int& node1, const int& node2, double& distance)
{
	distance = profiles[node1]->distance(*profiles[node2]);
}

void Grid::getNodeAzimuth(const int& node1, const int& node2, double& azimuth)
{
	azimuth = profiles[node1]->azimuth(*profiles[node2]);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns true if the directory exists.
//
// *****************************************************************************
bool Grid::is_directory(const string& dir)
{
#if defined(_WIN32) || defined(WIN32)  // Windows

	// check if the directory exists
	if((_access(dir.c_str(), 0)) != -1)
		return true;
	else
		return false;

#else  // Sun

	struct stat st;

	// read directory information into st
	if (stat(dir.c_str(), &st) != 0)
		return false;

	// return true if it's a directory by checking the mode
	if (S_ISDIR(st.st_mode) != 0) // returns non-zero if it's a directory
		return true;
	else
		return false;

#endif
}

void Grid::specifyOutputDirectory(const string& dirname)
{
	util::DataBuffer buffer;
	outputDirectory = "";

	if (!is_directory(dirname))
		mkdir(dirname.c_str() PGL_MKDIR_OPTIONS);

	string s = "delete me";
	buffer.writeString(s);

	writeBufferToFile(buffer, dirname+"/deleteme.buf");

	remove((dirname+"/deleteme.buf").c_str());

	outputDirectory = dirname;
}

void Grid::writeBufferToFile(util::DataBuffer& buffer, string fileName)
{
	// Open file.
	std::ofstream outFile;
	outFile.open(fileName.c_str(), ios::out | ios::binary);
	if (!outFile.is_open())
	{
		ostringstream os;
		os << endl << "ERROR in GridSLBM::writeBufferToFile()." << endl
				<<"Cannot open file " << fileName << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),105);
	}

	// Write the contents of the buffer to the file.
	buffer.writeToFile(outFile);

	// Clean up.
	outFile.close();
}

void Grid::reaDataBuffererFromFile(util::DataBuffer& buffer, string dirname, string fileName)
{
	string pathFileName = dirname + fileName;

	ifstream dataFile(pathFileName.c_str(), ios::in | ios::binary);

	if (dataFile.fail() || !dataFile.is_open())
	{
		ostringstream os;
		os << endl << "ERROR in GridSLBM::reaDataBuffererFromFile" << endl
				<<"Could not open file " << pathFileName << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__
				<< " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),106);
	}

	// Get size of file.
	dataFile.seekg(0, ios::end);
	int fileSize = dataFile.tellg();

	// Read entire file into a buffer.
	buffer.readFromFile(dataFile, 0, fileSize);

	//reverse byte order on little endian machines
	if(!util::MD50::isBigEndian()) buffer.setByteOrderReverse(true);

	// reset buffer position
	buffer.resetPos();
	dataFile.close();
}

void Grid::initializeActiveNodes(double activeNodeLatMin, double activeNodeLonMin,
		double activeNodeLatMax, double activeNodeLonMax)
{
	clearActiveNodes();

	// ensure that lonMin is >= -PI and < PI
	while (activeNodeLonMin < -PI) activeNodeLonMin += 2*PI;
	while (activeNodeLonMin >= PI) activeNodeLonMin -= 2*PI;
	// ensure that lonMax > lonMin and <= lonMin+2*PI
	while (activeNodeLonMax <=  activeNodeLonMin) activeNodeLonMax += 2*PI;
	while (activeNodeLonMax > activeNodeLonMin+2*PI) activeNodeLonMax -= 2*PI;

	double lat, lon;

	// iterate over all the vertices of the grid
	for (int v=0; v<(int)profiles.size(); ++v)
	{
		lon = profiles[v]->getLon();
		if (lon < activeNodeLonMin) lon += 2*PI;

		if (lon <= activeNodeLonMax)
		{
			lat = profiles[v]->getLat();
			if (lat >= activeNodeLatMin && lat <= activeNodeLatMax)
			{
				// this vertex is inside the polygon and it is not currently an active node.
				// Make it an active node.
				profiles[v]->setActiveNodeId(activeNodes.size());
				activeNodes.push_back(v);
			}
		}
	}

	// search for nodes that are active, but have neighbors that are not active.
	// Add the nodeids of the inactive neighbors to a set. This set contains
	// the indices of marginal nodes.  They are marginal in the sense that they
	// are members of triangles that span the boundary of the active region.
	vector<int> neighbors;
	set<int> marginalNodes;
	for (int v=0; v<(int)profiles.size(); ++v)
		if (profiles[v]->getActiveNodeId() >= 0)
		{
			// find all of its neighbors (vertices connected by a single edge).
			getNodeNeighbors(v, neighbors);
			for (int n=0; n<(int)neighbors.size(); ++n)
				if (profiles[neighbors[n]]->getActiveNodeId() < 0)
					marginalNodes.insert(neighbors[n]);
		}

	for (set<int>::iterator m = marginalNodes.begin(); m != marginalNodes.end(); ++m)
	{
		profiles[*m]->setActiveNodeId(activeNodes.size());
		activeNodes.push_back(*m);
//		cout << "Grid::initializeActiveNodes() marginal node  " << setw(6) <<
//				*m << "  " << setw(10) << profiles[*m]->getLatDegrees() << " "
//				<< setw(10) << profiles[*m]->getLonDegrees() << " " << endl;
	}
}

void Grid::initializeActiveNodes(GeoTessPolygon* p)
{
	clearActiveNodes();

	if (polygon != NULL)
	{
		delete polygon;
		polygon = NULL;
	}
	polygon = p;

	vector<int> neighbors;

	// iterate over all the vertices of the grid
	for (int v=0; v<(int)profiles.size(); ++v)
		if (profiles[v]->getActiveNodeId() < 0 && polygon->contains(profiles[v]->getUnitVector()))
		{
			// this vertex is inside the polygon and it is not currently an active node.
			// Make it an active node.
			profiles[v]->setActiveNodeId(activeNodes.size());
			activeNodes.push_back(v);

			// find all of its neighbors (vertices connected by a single edge).
			getNodeNeighbors(v, neighbors);
			for (vector<int>::iterator neighbor=neighbors.begin(); neighbor!=neighbors.end(); ++neighbor)
				if (profiles[*neighbor]->getActiveNodeId() < 0)
				{
					// this is a vertex that is a neighbor of a vertex in the polygon.
					// Set it active even if it is not in the polygon. This ensures that
					// if any vertex of a triangle is inside the polygon then all the
					// vertices of the triangle are active.
					profiles[*neighbor]->setActiveNodeId(activeNodes.size());
					activeNodes.push_back(*neighbor);
				}
		}
}

void Grid::clearActiveNodes()
{
	activeNodes.clear();
	for (int i=0; i<(int)profiles.size(); ++i)
		profiles[i]->setActiveNodeId(-1);
}


} // end slbm namespace
