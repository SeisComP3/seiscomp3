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
//- Module:        $RCSfile: GridSLBM.cc,v $
//- Revision:      $Revision: 1.31 $
//- Last Modified: $Date: 2014/04/28 13:44:06 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
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

#include "GridSLBM.h"
#include "SLBMGlobals.h"
#include "GridProfileSLBM.h"
#include "CPPUtils.h"
#include "GeoTessMetaData.h"
#include "GeoTessModelSLBM.h"
#include "CpuTimer.h"
#include "GeoTessProfile.h"
#include "GeoTessProfileThin.h"
#include "GeoTessProfileConstant.h"

//using namespace std;

using namespace geotess;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

// **** _FUNCTION DESCRIPTION_ *************************************************
//
// Parameterized Grid Constructor
//
// *****************************************************************************
GridSLBM::GridSLBM() : Grid()
{
}  // END Grid Default Constructor


// **** _FUNCTION DESCRIPTION_ *************************************************
//
// Grid Destructor
//
// *****************************************************************************
GridSLBM::~GridSLBM()
{
	clear();
}

void GridSLBM::clear()
{
	Grid::clear();

	for (unsigned i=0; i<triangles.size(); i++)
		if (triangles[i]) delete triangles[i];
	triangles.clear();

	specialTriangles.clear();

	for (unsigned i=0; i<geoStacks.size(); i++)
		if (geoStacks[i]) delete geoStacks[i];
	geoStacks.clear();

	specialTriangles.clear();

	activeNodes.clear();

}

size_t GridSLBM::memSize()
{
	return
			sizeof(*geoStacks[0])*geoStacks.size()
			+ sizeof(*profiles[0])*profiles.size()
			+ sizeof(*triangles[0])*triangles.size()
			+ sizeof(geoStacks)
			+ sizeof(profiles)
			+ sizeof(triangles)
			+ sizeof(specialTriangles)
			+ sizeof(modelPath)
			+ sizeof(cos_min)
			+ sizeof(V0)
			+ receivers->memSize()
			+ sources->memSize()
			;
}

void GridSLBM::loadFromFile(const string& filename)
{
	// save the name of the file from which the velocity model
	// is being loaded.
	modelPath = filename;

	ifstream fin;

	//open the file.  Return an error if cannot open the file.
	fin.open(filename.c_str());
	if (fin.fail() || !fin.is_open())
	{
		ostringstream os;
		os << endl << "ERROR in GridSLBM::loadFromFile" << endl
				<<"Could not open file " << filename << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),101);
	}

	int nStacks, nNodes, nTriangles, nData;
	fin >> nStacks >> nNodes >> nTriangles >> nData;

	if (nData != 24)
	{
		ostringstream os;
		os << endl << "ERROR in GridSLBM::loadFromFile" << endl
				<<"File " << filename << " specifies a model with " << nData << endl
				<<" data items per layer but the SLBM code expects models with 24 data items per layers."
				<< endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),503);
	}

	// read the average P and S velocities of the top of the mantle.  These
	// are averages over the part of the model that come from the Unified Model only.
	fin >> V0[PWAVE] >> V0[SWAVE];

	double lat, lon, elev, waterThick, depth[NLAYERS], vp[NLAYERS], vs[NLAYERS], g[2];
	int stackId;

	geoStacks.clear();
	geoStacks.resize(nStacks);

	for (int i=0; i<nStacks; i++)
	{
		for (int k=0; k<NLAYERS; k++)
		{
			if (k == 0)
			{
				// this is the water layer
				depth[k] = 0.;
				vp[k] = 1.5;
				vs[k] = 0.;
			}
			else
			{
				if (k == 1 || k == 6)
					depth[k] = depth[k-1];
				else
					fin >> depth[k];
				fin >> vp[k] >> vs[k];
			}
		}
		fin >> g[0] >> g[1];

		geoStacks[i] = new GeoStack(i, depth, vp, vs, g);
	}

	profiles.clear();
	profiles.resize(nNodes);
	for (int nodeid=0; nodeid<nNodes; ++nodeid)
	{
		fin >> lat >> lon >> elev >> waterThick >> stackId;
		lat *= DEG_TO_RAD;
		lon *= DEG_TO_RAD;
		GridProfileSLBM* p = new GridProfileSLBM(*this, nodeid, lat, lon, elev,
				abs(waterThick), geoStacks[stackId]);

		profiles[nodeid] = p;

		geoStacks[stackId]->incRefCount();
	}

	// create a temporary storage to hold a list of the triangles that
	// each node is a member of.  There will a vector<Triangle*> for
	// each node.  Each vector will contain pointers to the triangles
	// of which that node is a member.  There will almost always be 6
	// of these, but in 12 cases there will only be 5.
	vector< vector<Triangle*> > triset(profiles.size());
	for (int i=0; i<(int)triset.size(); i++)
		triset[i].reserve(6);

	triangles.resize(nTriangles);
	int a, b, c;
	for (int i=0; i<nTriangles; i++)
	{
		fin >> a >> b >> c;
		triangles[i] = new Triangle(i, profiles[a], profiles[b], profiles[c]);

		triset[a].push_back(triangles[i]);
		triset[b].push_back(triangles[i]);
		triset[c].push_back(triangles[i]);
	}

	// establish adjacency information for the triangles in the tessellation.
	defineTessAdjacency(nNodes, triset);

	// now load the uncertainties from files in modelPath.
	int np, na, iphase, iattribute;
	string phase, attribute;
	fin >> np >> na;
	for (int i=0; i<np; i++)
		for (int j=0; j<na; ++j)
		{
			fin >> phase >> attribute;
			iphase = Uncertainty::getPhase(phase);
			iattribute = Uncertainty::getAttribute(attribute);
			if (uncertainty[iphase][iattribute] != NULL)
				delete uncertainty[iphase][iattribute];
			uncertainty[iphase][iattribute] = Uncertainty::getUncertainty(fin, iphase, iattribute);
		}

	fin.close();
	activeNodes.clear();
	tessId = "";
}

void GridSLBM::saveVelocityModel(const string& destination, const int& format)
{
	if (destination == modelPath)
	{
		ostringstream os;
		os << endl << "ERROR in GridSLBM::saveVelocityModel" << endl
				<<"Output file name cannot equal input file name."<< endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),102);
	}

	if (format == 4)
		saveGeotessFile(destination);
	else if (format == 3)
		saveGeotessDirectory(destination);
	else if (format == 2)
		saveSlbmDirectory(destination);
	else if (format == 1)
		saveSlbmFile(destination);

}

void GridSLBM::saveVelocityModel(util::DataBuffer& buffer)
{
	// ******************************************************
	// write GeoStack file
	// ******************************************************

	string s;

	buffer.writeInt32(geoStacks.size());
	buffer.writeFloat((float)V0[PWAVE]);
	buffer.writeFloat((float)V0[SWAVE]);

	for (int i=0; i<(int)geoStacks.size(); i++)
	{
		for (int k=1; k<NLAYERS; k++)
		{
			if (k != 6) buffer.writeFloat((float)geoStacks[i]->getDepth(k));
			buffer.writeFloat((float)geoStacks[i]->getVelocity(PWAVE, k));
			buffer.writeFloat((float)geoStacks[i]->getVelocity(SWAVE, k));
		}
		buffer.writeFloat((float)geoStacks[i]->getMantleGradient(PWAVE));
		buffer.writeFloat((float)geoStacks[i]->getMantleGradient(SWAVE));
	}

	// ******************************************************
	// write Connectivity file
	// ******************************************************

	buffer.writeString(tessId);
	buffer.writeInt32(profiles.size());

	for (int i=0; i<(int)profiles.size(); i++)
	{
		buffer.writeInt32(profiles[i]->getGeoStackId());
		buffer.writeFloat((float)(-profiles[i]->getDepth()));
		buffer.writeFloat((float)profiles[i]->getWaterThick());
	}

	//******************************************************
	//write Tessellation file
	//******************************************************

	s = "SLBM Tessellation";
	buffer.writeString(s);

	s = "Parameter list";
	buffer.writeString(s);

	s = "Comment";
	buffer.writeString(s);

	buffer.writeInt32(profiles.size());

	for (int i=0; i<(int)profiles.size(); i++)
	{
		buffer.writeDouble(profiles[i]->getLatDegrees());
		buffer.writeDouble(profiles[i]->getLonDegrees());
	}

	buffer.writeInt32(triangles.size());

	for (int i=0; i<(int)triangles.size(); i++)
		for (int j=0; j<3; j++)
			buffer.writeInt32(triangles[i]->getNode(j)->getNodeId());

	buffer.writeInt32(0); // no additional metadata

	// uncertainty
	buffer.writeInt32(4);  // 4 phases: Pn, Sn, Pg, Lg
	buffer.writeInt32(3);  // 3 attributes: TT, AZ, SH
	for (int p=0; p<4; ++p)
		for (int a=0; a<3; ++a)
		{
			buffer.writeString(Uncertainty::getPhase(p));
			buffer.writeString(Uncertainty::getAttribute(a));
			if (uncertainty[p][a] != NULL)
				uncertainty[p][a]->serialize(buffer);
			else
				buffer.writeInt32(-1);
		}
}

int  GridSLBM::getBufferSize() const
{
	string s;
	int bsiz = 0;

	bsiz += sizeof(int) + 2*sizeof(float);

	bsiz += geoStacks.size() * ((NLAYERS-1)*3+1) * sizeof(float);

	bsiz += sizeof(int) + tessId.length();
	bsiz += sizeof(int);
	bsiz += profiles.size()*(sizeof(int)+2*sizeof(float));

	s = "SLBM Tessellation";
	bsiz += sizeof(int) + s.length();

	s = "Parameter list";
	bsiz += sizeof(int) + s.length();

	s = "Comment";
	bsiz += sizeof(int) + s.length();

	bsiz += sizeof(int) + profiles.size() * 2 * sizeof(double);

	bsiz += sizeof(int) + triangles.size() * 3 * sizeof(int);

	bsiz += sizeof(int);

	bsiz += 2*sizeof(int);
	for (int p=0; p<4; ++p)
		for (int a=0; a<3; ++a)
		{
			bsiz += sizeof(int)+Uncertainty::getPhase(p).length();
			bsiz += sizeof(int)+Uncertainty::getAttribute(a).length();
			if (uncertainty[p][a] != NULL)
				bsiz += uncertainty[p][a]->getBufferSize();
			else
				bsiz += sizeof(int);
		}
	return bsiz;
}

void GridSLBM::loadFromDirectory(const string& dirname)
{
	modelPath = dirname;

	geotess::CPPUtils::addPathSeparator(modelPath);

	util::DataBuffer buffer;
	string filename;

	clear();

	// ******************************************************
	// read GeoStack file
	// ******************************************************

	filename = "geostacks";

	reaDataBuffererFromFile(buffer, modelPath, filename);

	readGeoStacks(buffer);

	buffer.clear();

	// ******************************************************
	// read connectivity file
	// ******************************************************

	filename = "connectivity";

	reaDataBuffererFromFile(buffer, modelPath, filename);

	vector<float> elev, waterThick;
	vector<int> stackId;
	int nNodes = 0;
	readConnectivity(buffer, nNodes, elev, waterThick, stackId);

	buffer.clear();

	// ******************************************************
	// read Tessellation file
	// ******************************************************

	filename = modelPath+"../tess/"+tessId;

	if (!fileExists(filename))
	{
		ostringstream os;
		os << endl << "ERROR in GridSLBM::loadFromDirectory(const string& dirname)." << endl
				<< filename << " does not exist." << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),105);
	}

	filename = "../tess/"+tessId;

	reaDataBuffererFromFile(buffer, modelPath, filename);

	vector< vector<Triangle*> > triset;
	readTessellationData(buffer, nNodes, elev, waterThick,
			stackId, triset);

	//cout << endl << "Metadata length: " << buffer->readInt32() << endl;

	// establish adjacency information for the triangles in the tessellation.
	defineTessAdjacency(nNodes, triset);

	// now load the uncertainties from files in modelPath.
	for (int i=0; i<4; i++)
		for (int j=0; j<3; ++j)
		{
			if (uncertainty[i][j] != NULL)
				delete uncertainty[i][j];
			uncertainty[i][j] = Uncertainty::getUncertainty(modelPath, i, j);
		}

	if (uncertainty[Pn][TT] == NULL)
	{
		string fname = CPPUtils::insertPathSeparator(modelPath, "Uncertainty_Pn_TT.txt");

		ostringstream os;
		os << endl << "ERROR in GridSLBM::loadFromDirectory()" << endl
				<<"Trying to load model " << modelPath << endl
				<<"Uncertainty file does not exist:"<< endl
				<< fname << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),104);
	}

}

void GridSLBM::loadFromDataBuffer(util::DataBuffer& buffer)
{
	// ******************************************************
	// read geostack information
	// ******************************************************

    readGeoStacks(buffer);

	// ******************************************************
	// read connectivity information
	// ******************************************************

    vector<float> elev, waterThick;
	vector<int> stackId;
    int nNodes = 0;
    readConnectivity(buffer, nNodes, elev, waterThick, stackId);

	// ******************************************************
	// read Tessellation information
	// ******************************************************

    vector< vector<Triangle*> > triset;
    readTessellationData(buffer, nNodes, elev, waterThick,
	                     stackId, triset);

    // read and ignore metadata count

    buffer.readInt32();

	// establish adjacency information for the triangles in the tessellation.

    defineTessAdjacency(nNodes, triset);

	// now load the uncertainties from buffer.
	int np = buffer.readInt32();
	int na = buffer.readInt32();
	if (na > 0 && np > 0)
	{
		if (na != 3 || np != 4)
		{
			ostringstream os;
			os << endl << "ERROR in GridSLBM::loadFromDataBuffer(util::DataBuffer& buffer)." << endl <<
					"Expecting uncertainty information for 3 attributes and 4 phases." << endl
					<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
			throw SLBMException(os.str(),105);
		}
		for (int i=0; i<np; i++)
			for (int j=0; j<na; ++j)
			{
				int iphase = Uncertainty::getPhase(buffer.readString());
				int iattribute = Uncertainty::getAttribute(buffer.readString());
				if (uncertainty[iphase][iattribute] != NULL)
					delete uncertainty[iphase][iattribute];
				uncertainty[iphase][iattribute] = Uncertainty::getUncertainty(buffer, iphase, iattribute);
			}

	}
}

string GridSLBM::toString()
{
	ostringstream os;
	os << "GridSLBM" << endl;
	os << "ModelPath  " << modelPath << endl;
	os << "NNodes     " << profiles.size() << endl;
	os << "NTriangles " << triangles.size() << endl;
	for (int i=0; i<(int)uncertainty.size(); ++i)
		for (int j=0; j<(int)uncertainty[i].size(); ++j)
			if (uncertainty[i][j] != NULL)
				os << uncertainty[i][j]->toStringTable();
	return os.str();
}



void GridSLBM::readGeoStacks(util::DataBuffer& buffer)
{
	int nStacks = buffer.readInt32();
	//cout << endl << "GeoStack Count: " << nStacks << endl;

	// read the average P and S velocities of the top of the mantle.  These
	// are averages over the part of the model that come from the Unified Model only.
	V0[PWAVE] = buffer.readFloat();
	V0[SWAVE] = buffer.readFloat();

	//cout << endl << "Mantle velocities: " << V0[PWAVE] << ", "
	//	<< V0[SWAVE] << endl;

	double depth[NLAYERS], vp[NLAYERS], vs[NLAYERS], g[2];

	geoStacks.resize(nStacks);

	for (int i=0; i<nStacks; i++)
	{
		for (int k=0; k<NLAYERS; k++)
		{
			if (k == 0)
			{
				// this is the water layer
				depth[k] = 0.;
				vp[k] = 1.5;
				vs[k] = 0.;
			}
			else
			{
				if (k == 6)
					// this the Pg/Lg velocity of middle crust
					depth[k] = depth[k-1];
				else
					depth[k] = buffer.readFloat();
				vp[k] = buffer.readFloat();
				vs[k] = buffer.readFloat();
			}
		}
		g[0] = buffer.readFloat();
		g[1] = buffer.readFloat();

		geoStacks[i] = new GeoStack(i, depth, vp, vs, g);

	}
}

void GridSLBM::readConnectivity(util::DataBuffer& buffer, int& nNodes,
		vector<float>& elev, vector<float>& waterThick,
		vector<int>& stackId)
{
	tessId = buffer.readString();
	//cout << "tess id: " << tessId << endl;

	// Read number of pairs (points)
	nNodes = buffer.readInt32();
	//cout << "Node Count: " << nNodes << endl;

	elev.reserve(nNodes);
	waterThick.reserve(nNodes);
	stackId.reserve(nNodes);
	for (int i=0; i<nNodes; i++)
	{
		stackId.push_back(buffer.readInt32());
		elev.push_back(buffer.readFloat());
		waterThick.push_back(buffer.readFloat());
	}
}

void GridSLBM::readTessellationData(util::DataBuffer& buffer, int nNodes,
		const vector<float>& elev,
		const vector<float>& waterThick,
		const vector<int>& stackId,
		vector< vector<Triangle*> >& triset)
{
	// ******************************************************
	// read Tessellation file
	// ******************************************************

	// read header info
	string description = buffer.readString();
	//cout << "Description: " << description << endl;

	string parameters = buffer.readString();
	//cout << "Parameters: " << parameters << endl;

	string comments = buffer.readString();
	//cout << "Comments: " << comments << endl;

	// Read number of pairs (points)
	nNodes = buffer.readInt32();

	//cout << "Node Count: " << nNodes << endl;

	profiles.resize(nNodes);
	double lat, lon;

	for(int i = 0; i < nNodes; i++)
	{
		lat = buffer.readDouble();
		lon = buffer.readDouble();
		profiles[i] = new GridProfileSLBM(*this, i,
				lat * DEG_TO_RAD, // read latitude
				lon * DEG_TO_RAD, // read longitude
				(double)elev[i],
				(double)abs(waterThick[i]),
				geoStacks[stackId[i]]);

		geoStacks[stackId[i]]->incRefCount();

		//! throw an error if (1) any finite thickness layer above the middle crust
		//! has a velocity that exceeds the Pg/Lg velocity of the middle crust
		//! or (2) any finite thickness crustal layer has a velocity that exceeds
		//! the mantle velocity.
		if (geoStacks[stackId[i]]->hasLowVelocityZone())
		{
			ostringstream os;
			os << endl << "ERROR in GridSLBM::loadVelocityModelBinary" << endl
					<< "Geostack has low velocity zone."  << endl
					<< "Node ID, Stack Id = " << i << ", " << stackId[i] << endl
					<< "Node lat, lon = " << lat << ", " << lon << endl
					<< geoStacks[stackId[i]]->toString() << endl
					<< "Version " << SlbmVersion << "  File " << __FILE__
					<< " line " << __LINE__ << endl	<< endl;

			throw SLBMException(os.str(),504);
		}
	}

	int nTriangles = buffer.readInt32();
	//cout << endl << "Triangle Count: " << nTriangles << endl;

	// create a temporary storage to hold a list of the triangles that
	// each node is a member of.  There will a vector<Triangle*> for
	// each node.  Each vector will contain pointers to the triangles
	// of which that node is a member.  There will almost always be 6
	// of these, but in 12 cases there will only be 5.
	//vector< vector<Triangle*> > triset(profiles.size());
	triset.resize(profiles.size());
	for (int i=0; i<(int)triset.size(); i++)
		triset[i].reserve(6);

	triangles.resize(nTriangles);
	int a, b, c;
	for (int i=0; i<nTriangles; i++)
	{
		a = buffer.readInt32();
		b = buffer.readInt32();
		c = buffer.readInt32();
		triangles[i] = new Triangle(i, profiles[a], profiles[b], profiles[c]);

		triset[a].push_back(triangles[i]);
		triset[b].push_back(triangles[i]);
		triset[c].push_back(triangles[i]);
	}
}

void GridSLBM::defineTessAdjacency(int nNodes,
		const vector< vector<Triangle*> >& triset)
{
	// establish adjacency information for the triangles in the tessellation.
	bool found;
	GridProfile* nextNode;
	for (int i=0; i<nNodes; i++)
	{
		// for each node, iterate over the triangles of which it is a member.
		for (int j=0; j<(int)triset[i].size(); j++)
		{
			// search for the index of this node in the current triangle
			for (int k=0; k<3; k++)
			{
				if (triset[i][j]->getNode(k)->getNodeId() == profiles[i]->getNodeId())
				{
					// if the neighboring triangle for this edge has not yet been set
					if (triset[i][j]->getNeighbor(k) == NULL)
					{
						// identify the next node in the current triangle, moving
						// clockwise from profiles[i];
						nextNode = triset[i][j]->getNode((k+1)%3);
						found = false;
						// iterate over the other triangles of which profiles[i] is a member
						for (int m=0; m<(int)triset[i].size(); m++) if (m != j)
						{
							// search this triangle to see if it shares two nodes with the
							// original triangle.
							for (int n=0; n<3; n++)
								if (triset[i][m]->getNode(n) == nextNode)
								{
									// these two triangles share two nodes.  Make them
									// neighbors.
									triset[i][j]->setNeighbor(k, triset[i][m]);
									triset[i][m]->setNeighbor(n, triset[i][j]);
									found = true;
									break;
								}
							if (found) break;
						}
					}
					break;
				}
			}
		}
	}

	//	// 5/3/2013: test the triangle neighbors
	//	for (int i=0; i<triangles.size(); ++i)
	//	{
	//		for (int j=0; j<3; ++j)
	//		{
	//			Triangle* neighbor = triangles[i]->getNeighbor(j);
	//			if (neighbor == NULL)
	//			{
	//				ostringstream os;
	//				os << endl << "ERROR in GridSLBM::defineTessAdjacency" << endl
	//						<< "neighbor is NULL" << endl
	//						<< "Version " << SlbmVersion << "  File " << __FILE__
	//						<< " line " << __LINE__ << endl	<< endl;
	//
	//				throw SLBMException(os.str(),504);
	//			}
	//
	//		}
	//	}

	// define the special triangles.  after a successful triangle walk,
	// the first special triangle will be set to the triangle just found
	// so that the next triangle search will check that triangle first.
	// In addition, find the triangles which contain the first 42 nodes
	// in the model input file.  It is assumed that these nodes are
	// evenly distributed over the surface of the earth.  Those triangles
	// will be saved as special triangles.  When GridSLBM::findTriangle() is
	// called, the triangles in special triangles will be searched, in order,
	// for one whose node[0] is within 16 degrees of the location being searched
	// for.  If one is found, then the search will start there.  If none is found,
	// the walk will start at the closest one.
	findSpecialTriangles();
}

void GridSLBM::getNodeNeighbors(const int& nodeid, int neighbors[], int& nNeighbors)
{
	vector<double> coeff(3, 0.);
	Triangle* start = findTriangle(*profiles[nodeid], coeff);
	set<int> nbrs;
	start->findNodeNeighbors(nodeid, nbrs);

	nNeighbors = 0;
	for (set<int>::iterator it=nbrs.begin(); it != nbrs.end(); it++)
		neighbors[nNeighbors++] = profiles[*it]->getNodeId();
}

void GridSLBM::getActiveNodeNeighbors(const int& nodeid, int neighbors[], int& nNeighbors)
{
	vector<double> coeff(3, 0.);
	int id = getGridNodeId(nodeid);
	Triangle* start = findTriangle(*profiles[id], coeff);
	set<int> nbrs;
	start->findNodeNeighbors(id, nbrs);

	nNeighbors = 0;
	for (set<int>::iterator it=nbrs.begin(); it != nbrs.end(); it++)
	{
		id = getActiveNodeId(profiles[*it]->getNodeId());
		if (id >= 0)
			neighbors[nNeighbors++] = id;
	}
}

void GridSLBM::getNodeNeighbors(const int& nodeid, vector<int>& neighbors)
{
	vector<double> coeff(3, 0.);
	Triangle* start = findTriangle(*profiles[nodeid], coeff);
	set<int> nbrs;
	start->findNodeNeighbors(nodeid, nbrs);

	neighbors.clear();
	for (set<int>::iterator it=nbrs.begin(); it != nbrs.end(); it++)
		neighbors.push_back(profiles[*it]->getNodeId());
}

void GridSLBM::getActiveNodeNeighbors(const int& nodeid, vector<int>& neighbors)
{
	vector<double> coeff(3, 0.);
	int id = getGridNodeId(nodeid);
	Triangle* start = findTriangle(*profiles[id], coeff);
	set<int> nbrs;
	start->findNodeNeighbors(id, nbrs);

	neighbors.clear();
	for (set<int>::iterator it=nbrs.begin(); it != nbrs.end(); it++)
	{
		id = getActiveNodeId(profiles[*it]->getNodeId());
		if (id >= 0)
			neighbors.push_back(id);
	}
}

void GridSLBM::findSpecialTriangles()
{
	// specify the unit vectors of 42 positions on the earth that are approximately
	// evenly distributed on the surface.  These were generated by starting with 12 points
	// that define an icosahedron. Then add the midpoint of each of the 30 edges.
	int nSpecial = 42;
	double v[42][3];
	int i=0;
	v[i][0] =   0.00000000000000; v[i][1] =   0.00000000000000; v[i++][2] =   1.00000000000000;
	v[i][0] =   0.89442719099992; v[i][1] =  -0.00000000000000; v[i++][2] =   0.44721359549996;
	v[i][0] =   0.27639320225002; v[i][1] =   0.85065080835204; v[i++][2] =   0.44721359549996;
	v[i][0] =  -0.72360679774998; v[i][1] =   0.52573111211913; v[i++][2] =   0.44721359549996;
	v[i][0] =  -0.72360679774998; v[i][1] =  -0.52573111211913; v[i++][2] =   0.44721359549996;
	v[i][0] =   0.27639320225002; v[i][1] =  -0.85065080835204; v[i++][2] =   0.44721359549996;
	v[i][0] =   0.72360679774998; v[i][1] =  -0.52573111211913; v[i++][2] =  -0.44721359549996;
	v[i][0] =   0.72360679774998; v[i][1] =   0.52573111211913; v[i++][2] =  -0.44721359549996;
	v[i][0] =  -0.27639320225002; v[i][1] =   0.85065080835204; v[i++][2] =  -0.44721359549996;
	v[i][0] =  -0.89442719099992; v[i][1] =   0.00000000000000; v[i++][2] =  -0.44721359549996;
	v[i][0] =  -0.27639320225002; v[i][1] =  -0.85065080835204; v[i++][2] =  -0.44721359549996;
	v[i][0] =   0.00000000000000; v[i][1] =   0.00000000000000; v[i++][2] =  -1.00000000000000;
	v[i][0] =   0.16245984811645; v[i][1] =   0.50000000000000; v[i++][2] =   0.85065080835204;
	v[i][0] =   0.68819096023559; v[i][1] =   0.50000000000000; v[i++][2] =   0.52573111211913;
	v[i][0] =   0.52573111211913; v[i][1] =   0.00000000000000; v[i++][2] =   0.85065080835204;
	v[i][0] =  -0.42532540417602; v[i][1] =   0.30901699437495; v[i++][2] =   0.85065080835204;
	v[i][0] =  -0.26286555605957; v[i][1] =   0.80901699437495; v[i++][2] =   0.52573111211913;
	v[i][0] =  -0.42532540417602; v[i][1] =  -0.30901699437495; v[i++][2] =   0.85065080835204;
	v[i][0] =  -0.85065080835204; v[i][1] =   0.00000000000000; v[i++][2] =   0.52573111211913;
	v[i][0] =   0.16245984811645; v[i][1] =  -0.50000000000000; v[i++][2] =   0.85065080835204;
	v[i][0] =  -0.26286555605957; v[i][1] =  -0.80901699437495; v[i++][2] =   0.52573111211913;
	v[i][0] =   0.68819096023559; v[i][1] =  -0.50000000000000; v[i++][2] =   0.52573111211913;
	v[i][0] =   0.58778525229247; v[i][1] =   0.80901699437495; v[i++][2] =   0.00000000000000;
	v[i][0] =   0.95105651629515; v[i][1] =   0.30901699437495; v[i++][2] =   0.00000000000000;
	v[i][0] =  -0.58778525229247; v[i][1] =   0.80901699437495; v[i++][2] =   0.00000000000000;
	v[i][0] =   0.00000000000000; v[i][1] =   1.00000000000000; v[i++][2] =   0.00000000000000;
	v[i][0] =  -0.95105651629515; v[i][1] =  -0.30901699437495; v[i++][2] =   0.00000000000000;
	v[i][0] =  -0.95105651629515; v[i][1] =   0.30901699437495; v[i++][2] =   0.00000000000000;
	v[i][0] =   0.00000000000000; v[i][1] =  -1.00000000000000; v[i++][2] =   0.00000000000000;
	v[i][0] =  -0.58778525229247; v[i][1] =  -0.80901699437495; v[i++][2] =   0.00000000000000;
	v[i][0] =   0.95105651629515; v[i][1] =  -0.30901699437495; v[i++][2] =   0.00000000000000;
	v[i][0] =   0.58778525229247; v[i][1] =  -0.80901699437495; v[i++][2] =   0.00000000000000;
	v[i][0] =   0.85065080835204; v[i][1] =   0.00000000000000; v[i++][2] =  -0.52573111211913;
	v[i][0] =   0.26286555605957; v[i][1] =   0.80901699437495; v[i++][2] =  -0.52573111211913;
	v[i][0] =  -0.68819096023559; v[i][1] =   0.50000000000000; v[i++][2] =  -0.52573111211913;
	v[i][0] =  -0.68819096023559; v[i][1] =  -0.50000000000000; v[i++][2] =  -0.52573111211913;
	v[i][0] =   0.26286555605957; v[i][1] =  -0.80901699437495; v[i++][2] =  -0.52573111211913;
	v[i][0] =   0.42532540417602; v[i][1] =   0.30901699437495; v[i++][2] =  -0.85065080835204;
	v[i][0] =   0.42532540417602; v[i][1] =  -0.30901699437495; v[i++][2] =  -0.85065080835204;
	v[i][0] =  -0.16245984811645; v[i][1] =   0.50000000000000; v[i++][2] =  -0.85065080835204;
	v[i][0] =  -0.52573111211913; v[i][1] =   0.00000000000000; v[i++][2] =  -0.85065080835204;
	v[i][0] =  -0.16245984811645; v[i][1] =  -0.50000000000000; v[i++][2] =  -0.85065080835204;

	// define the special triangles.  after a successful triangle walk,
	// the first special triangle will be set to the triangle just found
	// so that the next triangle search will check that triangle first.
	// In addition, find the triangles which contain the 42 nodes
	// defined above.  It is assumed that these nodes are
	// evenly distributed over the surface of the earth.  Those triangles
	// will be saved as special triangles.  When GridSLBM::findTriangle() is
	// called, the triangles in special triangles will be searched, in order,
	// for one whose node[0] is within 16 degrees of the location being searched
	// for.  If one is found, then the search will start there.  If none is found,
	// the walk will start at the closest one.
	cos_min = cos(16*DEG_TO_RAD);
	vector<double> coeff(3, 0.);
	specialTriangles.reserve(nSpecial+1);
	specialTriangles.push_back(triangles[0]);
	Location node;
	for (int i=0; i<nSpecial; i++)
	{
		node.setUnitVector(v[i]);
		specialTriangles.push_back(findTriangle(node, coeff));
	}
}

void GridSLBM::saveSlbmDirectory(const string& directoryName)
{
	if (directoryName == "")
	{
		ostringstream os;
		os << endl << "ERROR in GridSLBM::saveVelocityModelBinary()" << endl
				<<"outputDirectory = <emptyString>.  Specify outputDirectory with call to method specifyOutpuDirectory()"<< endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),104);
	}

	util::DataBuffer buffer;
	string filename;

	// check the geostacks for layers with negative thickness.
	for (int i=0; i<(int)geoStacks.size(); i++)
	{
		double* depths = geoStacks[i]->getDepth();
		for (int i=1; i<NLAYERS; i++)
			if (depths[i-1] > depths[i])
			{
				if (depths[i-1] - depths[i] < 0.002)
					depths[i] = depths[i-1];
				else
				{
					ostringstream os;
					os << endl << "ERROR in GridSLBM::saveVelocityModel()" << endl
							<< "Layer " << i << " has negative thickness" << endl
							<< "depths[" << i-1 << "] = " << setw(11) << depths[i-1] << endl
							<< "depths[" << i   << "] = " << setw(11) << depths[i] << endl
							<< "Version " << SlbmVersion << "  File " << __FILE__
							<< " line " << __LINE__ << endl	<< endl;

					throw SLBMException(os.str(), 999);
				}

			}
	}

	if (!is_directory(directoryName))
		mkdir(directoryName.c_str() PGL_MKDIR_OPTIONS);

	string tessDir = directoryName+"/../tess";

	if (!is_directory(tessDir))
		mkdir(tessDir.c_str() PGL_MKDIR_OPTIONS);

	//***NOTE reverse byte order on little endian machines
	if(!util::MD50::isBigEndian())
		buffer.setByteOrderReverse(true);

	if (tessId.length() == 0)
	{
		//******************************************************
		//write Tessellation file
		//******************************************************

		buffer.reserve(profiles.size()*2*sizeof(double)
		+triangles.size()*3*sizeof(int) + 1000);

		string s = "SLBM Tessellation";
		buffer.writeString(s.c_str());

		s = "Parameter list";
		buffer.writeString(s.c_str());

		s = "Comment";
		buffer.writeString(s.c_str());

		buffer.writeInt32(profiles.size());

		for (int i=0; i<(int)profiles.size(); i++)
		{
			buffer.writeDouble(profiles[i]->getLatDegrees());
			buffer.writeDouble(profiles[i]->getLonDegrees());
		}

		buffer.writeInt32(triangles.size());

		for (int i=0; i<(int)triangles.size(); i++)
			for (int j=0; j<3; j++)
				buffer.writeInt32(triangles[i]->getNode(j)->getNodeId());

		buffer.writeInt32(0); // no additional metadata

		tessId = buffer.generateDataBufMD5HashKey();

		filename = tessDir+"/"+tessId;

		writeBufferToFile(buffer, filename);

		buffer.clear();

		//******************************************************
		//write node adjacency file
		//******************************************************
		buffer.reserve(profiles.size()*7*sizeof(int));

		buffer.writeString(tessId);

		buffer.writeInt32(profiles.size());

		vector<int> neighbors(profiles.size());
		vector<double> azimuth(profiles.size()), distance(profiles.size());

		map<double, int> nodeOrder;
		map<double, int>::iterator it;

		for (int i=0; i<(int)profiles.size(); ++i)
		{
			getNodeNeighborInfo(profiles[i]->getNodeId(), neighbors, distance, azimuth);
			nodeOrder.clear();
			for (int j=0; j<(int)neighbors.size(); ++j)
				nodeOrder[-azimuth[j]] = neighbors[j];

			buffer.writeInt32(neighbors.size());
			for (it = nodeOrder.begin(); it != nodeOrder.end(); ++it)
				buffer.writeInt32(it->second);
		}

		filename = tessDir+"/"+tessId+"_adjacency";

		writeBufferToFile(buffer, filename);

		buffer.clear();
	}

	// ******************************************************
	// write GeoStack file
	// ******************************************************
	buffer.reserve(geoStacks.size()*3*NLAYERS*4 + 1000);

	buffer.writeInt32(geoStacks.size());

	buffer.writeFloat((float)V0[PWAVE]);
	buffer.writeFloat((float)V0[SWAVE]);

	for (int i=0; i<(int)geoStacks.size(); i++)
	{
		for (int k=1; k<NLAYERS; k++)
		{
			if (k != 6)
				buffer.writeFloat((float)geoStacks[i]->getDepth(k));
			buffer.writeFloat((float)geoStacks[i]->getVelocity(PWAVE, k));
			buffer.writeFloat((float)geoStacks[i]->getVelocity(SWAVE, k));
		}
		buffer.writeFloat((float)geoStacks[i]->getMantleGradient(PWAVE));
		buffer.writeFloat((float)geoStacks[i]->getMantleGradient(SWAVE));
	}

	filename = directoryName+"/geostacks";

	writeBufferToFile(buffer, filename);

	buffer.clear();

	// ******************************************************
	// write Connectivity file
	// ******************************************************
	buffer.reserve(profiles.size()*12 + 1000);

	buffer.writeString(tessId);

	buffer.writeInt32(profiles.size());

	for (int i=0; i<(int)profiles.size(); i++)
	{
		buffer.writeInt32(profiles[i]->getGeoStackId());
		buffer.writeFloat((float)(-profiles[i]->getDepth()));
		buffer.writeFloat((float)profiles[i]->getWaterThick());
	}

	filename = directoryName+"/connectivity";

	writeBufferToFile(buffer, filename);

	buffer.clear();

	for (int i=0; i<(int)uncertainty.size(); ++i)
		for (int j=0; j<(int)uncertainty[i].size(); ++j)
			if (uncertainty[i][j])
				uncertainty[i][j]->writeFile(directoryName);

}

void GridSLBM::saveSlbmFile(const string& filename)
{
	// check the geostacks for layers with negative thickness.
	for (int i=0; i<(int)geoStacks.size(); i++)
	{
		double* depths = geoStacks[i]->getDepth();
		for (int i=1; i<NLAYERS; i++)
			if (depths[i-1] > depths[i])
			{
				if (depths[i-1] - depths[i] < 0.002)
					depths[i] = depths[i-1];
				else
				{
					ostringstream os;
					os << endl << "ERROR in GridSLBM::saveVelocityModel()" << endl
							<< "Layer " << i << " has negative thickness" << endl
							<< "depths[" << i-1 << "] = " << setw(11) << depths[i-1] << endl
							<< "depths[" << i   << "] = " << setw(11) << depths[i] << endl
							<< "Version " << SlbmVersion << "  File " << __FILE__
							<< " line " << __LINE__ << endl	<< endl;

					throw SLBMException(os.str(), 999);
				}
			}
	}

	ofstream fout;

	//open the file.  Return an error if cannot open the file.
	fout.open(filename.c_str());
	if (fout.fail() || !fout.is_open())
	{
		ostringstream os;
		os << endl << "ERROR in GridSLBM::saveVelocityModel" << endl
				<<"Could not open file " << filename << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),103);
	}

	//fout << setiosflags(ios::fixed) << setiosflags(ios::showpoint);

	fout << geoStacks.size() << "  " << profiles.size() <<
			" " << triangles.size() << " 24" << endl;

	fout  << setprecision(7) << V0[PWAVE] << " " << V0[SWAVE] << endl;

	for (int i=0; i<(int)geoStacks.size(); i++)
	{
		for (int k=1; k<NLAYERS; k++)
		{
			if (k != 1 && k != 6)
				fout  << " " << geoStacks[i]->getDepth(k);

			fout << " " << geoStacks[i]->getVelocity(PWAVE, k) <<
					" " << geoStacks[i]->getVelocity(SWAVE, k);
		}
		fout << " " << geoStacks[i]->getMantleGradient(PWAVE) <<
				" " << geoStacks[i]->getMantleGradient(SWAVE) << endl;
	}

	for (int i=0; i<(int)profiles.size(); i++)
		fout  << setprecision(10)
		<< profiles[i]->getLat() * RAD_TO_DEG << " "
		<< profiles[i]->getLon() * RAD_TO_DEG << " "
		<< setprecision(7)
		<< -profiles[i]->getDepth() << " "
		<< profiles[i]->getWaterThick() << " "
		<< profiles[i]->getGeoStackId()
		<< endl;

	for (int i=0; i<(int)triangles.size(); i++)
		fout << triangles[i]->getNode(0)->getNodeId() <<
		" " << triangles[i]->getNode(1)->getNodeId() <<
		" " << triangles[i]->getNode(2)->getNodeId() << endl;

	// uncertainty
	fout << "4 3" << endl;
	for (int p=0; p<4; ++p)
		for (int a=0; a<3; ++a)
		{
			fout << Uncertainty::getPhase(p) << " " << Uncertainty::getAttribute(a) << endl;
			if (uncertainty[p][a] == NULL)
				fout << "0" << endl;
			else
				fout << uncertainty[p][a]->toStringFile();
		}
	fout.close();
}

void GridSLBM::saveGeotessDirectory(const string& directoryName)
{

	if (directoryName == "")
	{
		ostringstream os;
		os << endl << "ERROR in GridSLBM::saveGeotessDirectory()" << endl
				<<"directoryName = <emptyString>.  "<< endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),104);
	}

	if (profiles.size() == 40962 && triangles.size() == 81920)
	{
		// the current slbm grid is compatible with the GeoTessGrid that has GridID below.

		string tessDir = CPPUtils::insertPathSeparator(directoryName, "..");
		tessDir = CPPUtils::insertPathSeparator(tessDir, "tess");

		if (!is_directory(directoryName))
			mkdir(directoryName.c_str() PGL_MKDIR_OPTIONS);

		if (!is_directory(tessDir))
			mkdir(tessDir.c_str() PGL_MKDIR_OPTIONS);

		string gridFilePath = CPPUtils::insertPathSeparator(tessDir, "808785948EB2350DD44E6C29BDEA6CAE");

		if (!fileExists(gridFilePath))
		{
			ostringstream os;
			os << endl << "ERROR in GridSLBM::saveGeotessDirectory(const string& directoryName)" << endl
					<<"The GeoTessGrid file that is compatible with the SLBM tessellation does not exist "<< endl
					<< gridFilePath << endl
					<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
			throw SLBMException(os.str(),104);
		}

		string outputFile = geotess::CPPUtils::insertPathSeparator(directoryName, "geotessmodel");

		saveGeotess(outputFile, gridFilePath, gridFilePath);

		for (int i=0; i<(int)uncertainty.size(); ++i)
			for (int j=0; j<(int)uncertainty[i].size(); ++j)
				if (uncertainty[i][j])
					uncertainty[i][j]->writeFile(directoryName);
	}
	else
	{
		ostringstream os;
		os << endl << "ERROR in GridSLBM::saveGeotessDirectory()" << endl
				<<"Cannot save to GeoTessModel because the grids are incompatible  "<< endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),104);
	}



}

void GridSLBM::saveGeotessFile(const string& fileName)
{
	if (modelPath == "")
	{
		ostringstream os;
		os << endl << "ERROR in GridSLBM::saveVelocityModelGeotessFile()" << endl
				<<"modelPath = <emptyString>.  "<< endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),104);
	}

	if (profiles.size() == 40962 && triangles.size() == 81920)
	{
		// the current slbm grid is compatible with the GeoTessGrid that has GridID below.

		string pathToGrid = CPPUtils::insertPathSeparator(modelPath, "..");
		pathToGrid = CPPUtils::insertPathSeparator(pathToGrid, "tess");
		pathToGrid = CPPUtils::insertPathSeparator(pathToGrid, "808785948EB2350DD44E6C29BDEA6CAE");

		saveGeotess(fileName, pathToGrid, "*");

	}
	else
	{
		ostringstream os;
		os << endl << "ERROR in GridSLBM::saveVelocityModelGeotessFile()" << endl
				<<"Can only save version 2 models to file if they have tessid 90d8a9fc0e8b2fd62009b621013cf51b"<< endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),104);
	}
}

void GridSLBM::saveGeotess(const string& path, const string& pathToGrid, const string& gridFilePath)
{
	try
	{
		// Create a MetaData object in which we can specify information
		// needed for model construction.
		GeoTessMetaData* metaData = new GeoTessMetaData();

		// Specify a description of the model. This information is not
		// processed in any way by GeoTess. It is carried around for
		// information purposes.
		metaData->setDescription("RSTT model");

		vector<string> layerNames;
		layerNames.push_back("mantle_gradient");
		layerNames.push_back("mantle");
		layerNames.push_back("lower_crust");
		layerNames.push_back("middle_crust_G");
		layerNames.push_back("middle_crust_N");
		layerNames.push_back("upper_crust");
		layerNames.push_back("sediment3");
		layerNames.push_back("sediment2");
		layerNames.push_back("sediment1");
		layerNames.push_back("water");

		// Specify a list of layer names delimited by semi-colons
		metaData->setLayerNames(layerNames);

		// specify the names of the attributes and the units of the
		// attributes in two Strings delimited by semi-colons.
		metaData->setAttributes("pvelocity; svelocity", "km/sec; km/sec");

		// specify the DataType for the data. All attributes, in all
		// profiles, will have the same data type.  Note that this
		// applies only to the data; radii are always stored as floats.
		metaData->setDataType(GeoTessDataType::FLOAT);

		// specify the name of the software that is going to generate
		// the model.  This gets stored in the model for future reference.
		string version = SlbmVersion;
		version = "SLBM."+version;
		metaData->setModelSoftwareVersion(version);

		// specify the date when the model was generated.  This gets
		// stored in the model for future reference.
		CpuTimer cpu;
		metaData->setModelGenerationDate(cpu.now());

		double vAvg[2];
		vAvg[PWAVE] = getAverageMantleVelocity(PWAVE);
		vAvg[SWAVE] = getAverageMantleVelocity(SWAVE);

		// call a GeoTessModel constructor to build the model. This will
		// load the grid, and initialize all the data structures to null.
		// To be useful, we will have to populate the data structures.
		GeoTessModelSLBM model(pathToGrid, metaData, uncertainty, vAvg);

		try
		{
			Location loc;
			vector<GridProfile*> neighbors;
			vector<int> nodeIds;
			vector<double> coefficients;
			int n;
			double depths[NLAYERS];
			double pvelocity[NLAYERS];
			double svelocity[NLAYERS];
			double gradients[2];
			vector<vector<float> > radii;
			vector<vector<float> > values;
			values.resize(1);
			values[0].resize(2);

			// now loop over every vertex in the grid
			for (int vtx = 0; vtx < model.getNVertices(); ++vtx)
			{
				// retrieve a reference to the unit vector corresponding to the i'th vertex
				loc.setLocation(model.getGrid().getVertex(vtx), 6371.);

				findProfile(loc, neighbors, nodeIds, coefficients);
				n=0;
				if (coefficients[1] > coefficients[0]) n=1;
				if (coefficients[2] > coefficients[n]) n=2;
				if (coefficients[n] < 0.99999)
				{
					cout << "Did not find colocated node." << endl;
					cout << fixed << setprecision(15);
					for (int i=0; i<3; i++)
						cout << coefficients[i] << "  ";
					cout << endl;
					exit(1);
				}
				neighbors[n]->getData(depths, pvelocity, svelocity, gradients);
				neighbors[n]->depthsToRadii(depths, radii);

				values[0][0] = (float)gradients[0];
				values[0][1] = (float)gradients[1];
				model.setProfile(vtx, 0, radii[0], values);
				for (int layer=1; layer<=NLAYERS; ++layer)
				{
					values[0][0] = (float)pvelocity[NLAYERS-layer];
					values[0][1] = (float)svelocity[NLAYERS-layer];
					model.setProfile(vtx, layer, radii[layer], values);
				}
			}
		}
		catch (GeoTessException& ex)
		{
			cout << endl << ex.emessage << endl;
		}

		string gridFileName = "*";
		if (gridFilePath.length() > 0)
		{
			vector<string> tokens;
			string file_separator = "x";
			file_separator[0] = CPPUtils::FILE_SEP;
			CPPUtils::tokenizeString(gridFilePath, file_separator, tokens);
			gridFileName = tokens[tokens.size()-1];
		}

		// if writing the grid to model file, then write the
		// uncertainties to the file as well.
		if (gridFileName == "*")
		{
			model.setIOUncertainty(true);
			//model.uncertainty = uncertainty;
		}
		else
			model.setIOUncertainty(false);

		model.writeModel(path, gridFileName);

		// print a bunch of information about the model to the screen.
		//cout << model.toString() << endl << endl;
	}
	catch (GeoTessException& ex)
	{
		cout << endl << ex.emessage << endl;
	}
	catch (...)
	{
		cout << endl << "Unidentified error detected " << endl
				<<  __FILE__ << "  " << __LINE__ << endl;
	}
}

} // end slbm namespace
