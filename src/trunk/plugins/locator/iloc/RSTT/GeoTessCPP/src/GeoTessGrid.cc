//- ****************************************************************************
//- 
//- Copyright 2009 Sandia Corporation. Under the terms of Contract
//- DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
//- retains certain rights in this software.
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

#include <cmath>

// **** _LOCAL INCLUDES_ *******************************************************

#include "GeoTessGrid.h"
#include "CPPUtils.h"
#include "IFStreamAscii.h"
#include "IFStreamBinary.h"
#include "CpuTimer.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

/**
 * Destructor.
 */
GeoTessGrid::~GeoTessGrid()
{
	if (refCount > 0)
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessGrid::~GeoTessGrid" << endl
				<< "Reference count (" << refCount << ") is not zero." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 2002);
	}

	if (vertices != NULL) CPPUtils::delete2DArray<double>(vertices);
	if (triangles != NULL) CPPUtils::delete2DArray<int>(triangles);
	if (tessellations != NULL) CPPUtils::delete2DArray<int>(tessellations);
	if (levels != NULL) CPPUtils::delete2DArray<int>(levels);
	if (descendants != NULL) delete[] descendants;

	for (int i=0; i<(int)circumCenters.size(); ++i)
		if (circumCenters[i] != NULL)
			delete[] circumCenters[i];
	circumCenters.clear();

	for (int t=0; t<nTriangles; ++t)
	{
		vector<Edge*>& edges = edgeList[t];
		delete edges[0];
		delete edges[1];
		delete edges[2];
	}
	spokeList.clear();
	edgeList.clear();
}

/**
 * Copy constructor.
 */
GeoTessGrid::GeoTessGrid(GeoTessGrid &other) :
												vertices(NULL), nVertices(0),
												triangles(NULL), nTriangles(0),
												levels(NULL), nLevels(0),
												tessellations(NULL), nTessellations(0),
												descendants(NULL), gridID(other.gridID),
												gridInputFile(other.gridInputFile), gridOutputFile(other.gridOutputFile),
												gridSoftwareVersion(other.gridSoftwareVersion),
												gridGenerationDate(other.gridGenerationDate), refCount(0)
{
	nVertices = other.nVertices;
	vertices = CPPUtils::new2DArray<double>(nVertices, 3);
	for (int i=0; i<nVertices; ++i)
	{
		vertices[i][0] = other.vertices[i][0];
		vertices[i][1] = other.vertices[i][1];
		vertices[i][2] = other.vertices[i][2];
	}

	nTriangles = other.nTriangles;
	triangles = CPPUtils::new2DArray<int>(nTriangles, 3);
	descendants = new int[nTriangles];
	for (int i=0; i<nTriangles; ++i)
	{
		triangles[i][0] = other.triangles[i][0];
		triangles[i][1] = other.triangles[i][1];
		triangles[i][2] = other.triangles[i][2];
		descendants[i] = other.descendants[i];
	}

	nLevels = other.nLevels;
	levels = CPPUtils::new2DArray<int>(nLevels, 2);
	for (int i=0; i<nLevels; ++i)
	{
		levels[i][0] = other.levels[i][0];
		levels[i][1] = other.levels[i][1];
	}

	nTessellations = other.nTessellations;
	tessellations = CPPUtils::new2DArray<int>(nTessellations, 2);
	for (int i=0; i<nTessellations; ++i)
	{
		tessellations[i][0] = other.tessellations[i][0];
		tessellations[i][1] = other.tessellations[i][1];
	}

	vtxTriangles.resize(nLevels);
	for (int level=0; level<nLevels; ++level)
	{
		vector<vector<int> >& a = vtxTriangles[level];
		vector<vector<int> >& b = other.vtxTriangles[level];
		a.resize(nVertices);
		for (int v=0; v<nVertices; ++v)
		{
			vector<int>& aa = a[v];
			vector<int>& bb = b[v];
			aa.resize(bb.size());
			for (int n=0; n<(int)aa.size(); ++n)
				aa[n] = bb[n];
		}
	}

	circumCenters.resize(other.circumCenters.size(), NULL);
	for (int i=0; i<(int)circumCenters.size(); ++i)
		if (other.circumCenters[i] != NULL)
		{
			double* b = other.circumCenters[i];
			double* a = new double[3];
			a[0]=b[0]; a[1]=b[1]; a[2]=b[2];
			circumCenters[i] = a;
		}

	edgeList.resize(nTriangles);
	Edge *edge, *otherEdge;
	for (int triangle = 0; triangle < nTriangles; ++triangle)
	{
		vector<Edge*>& tedges = edgeList[triangle];
		tedges.resize(3);

		const vector<Edge*>& otherEdges = other.edgeList[triangle];
		for (int v=0; v<3; ++v)
		{
			otherEdge = otherEdges[v];
			edge = new Edge;
			edge->vj = otherEdge->vj;
			edge->vk = otherEdge->vk;
			edge->tLeft = otherEdge->tLeft;
			edge->tRight = otherEdge->tRight;
			edge->normal[0] = otherEdge->normal[0];
			edge->normal[1] = otherEdge->normal[1];
			edge->normal[2] = otherEdge->normal[2];
			edge->next = NULL;
			tedges[v] = edge;
		}
	}
	spokeList.resize(nLevels);
	connectedVertices.resize(nLevels);
}

GeoTessGrid& GeoTessGrid::operator=(const GeoTessGrid& other)
{
	if (vertices != NULL) CPPUtils::delete2DArray<double>(vertices);
	if (triangles != NULL) CPPUtils::delete2DArray<int>(triangles);
	if (tessellations != NULL) CPPUtils::delete2DArray<int>(tessellations);
	if (levels != NULL) CPPUtils::delete2DArray<int>(levels);
	if (descendants != NULL) delete[] descendants;

	for (int i=0; i<(int)circumCenters.size(); ++i)
		if (circumCenters[i] != NULL)
			delete[] circumCenters[i];
	circumCenters.clear();


	nVertices = other.nVertices;
	vertices = CPPUtils::new2DArray<double>(nVertices, 3);
	for (int i=0; i<nVertices; ++i)
	{
		vertices[i][0] = other.vertices[i][0];
		vertices[i][1] = other.vertices[i][1];
		vertices[i][2] = other.vertices[i][2];
	}

	nTriangles = other.nTriangles;
	triangles = CPPUtils::new2DArray<int>(nTriangles, 3);
	descendants = new int[nTriangles];
	for (int i=0; i<nTriangles; ++i)
	{
		triangles[i][0] = other.triangles[i][0];
		triangles[i][1] = other.triangles[i][1];
		triangles[i][2] = other.triangles[i][2];
		descendants[i] = other.descendants[i];
	}

	nLevels = other.nLevels;
	levels = CPPUtils::new2DArray<int>(nLevels, 2);
	for (int i=0; i<nLevels; ++i)
	{
		levels[i][0] = other.levels[i][0];
		levels[i][1] = other.levels[i][1];
	}

	nTessellations = other.nTessellations;
	tessellations = CPPUtils::new2DArray<int>(nTessellations, 2);
	for (int i=0; i<nTessellations; ++i)
	{
		tessellations[i][0] = other.tessellations[i][0];
		tessellations[i][1] = other.tessellations[i][1];
	}

	vtxTriangles.resize(nLevels);
	for (int level=0; level<nLevels; ++level)
	{
		vector<vector<int> >& a = vtxTriangles[level];
		const vector<vector<int> >& b = other.vtxTriangles[level];
		a.resize(nVertices);
		for (int v=0; v<nVertices; ++v)
		{
			vector<int>& aa = a[v];
			const vector<int>& bb = b[v];
			aa.resize(bb.size());
			for (int n=0; n<(int)aa.size(); ++n)
				aa[n] = bb[n];
		}
	}

	circumCenters.resize(other.circumCenters.size(), NULL);
	for (int i=0; i<(int)circumCenters.size(); ++i)
		if (other.circumCenters[i] != NULL)
		{
			double* b = other.circumCenters[i];
			double* a = new double[3];
			a[0]=b[0]; a[1]=b[1]; a[2]=b[2];
			circumCenters[i] = a;
		}

	edgeList.resize(nTriangles);
	Edge *edge, *otherEdge;
	for (int triangle = 0; triangle < nTriangles; ++triangle)
	{
		vector<Edge*>& tedges = edgeList[triangle];
		tedges.resize(3);

		const vector<Edge*>& otherEdges = other.edgeList[triangle];
		for (int v=0; v<3; ++v)
		{
			otherEdge = otherEdges[v];
			edge = new Edge;
			edge->vj = otherEdge->vj;
			edge->vk = otherEdge->vk;
			edge->tLeft = otherEdge->tLeft;
			edge->tRight = otherEdge->tRight;
			edge->normal[0] = otherEdge->normal[0];
			edge->normal[1] = otherEdge->normal[1];
			edge->normal[2] = otherEdge->normal[2];
			edge->next = NULL;
			tedges[v] = edge;
		}
	}
	spokeList.resize(nLevels);
	connectedVertices.resize(nLevels);

	gridID = other.gridID;
	gridInputFile = other.gridInputFile;
	gridOutputFile = other.gridOutputFile;
	gridSoftwareVersion = other.gridSoftwareVersion;
	gridGenerationDate = other.gridGenerationDate;

	return *this;
}

/**
 * Test a file to see if it is a GeoTessGrid file.
 *
 * @param inputFile
 * @return true if inputFile is a GeoTessGrid file.
 */
bool GeoTessGrid::isGeoTessGrid(const string& fileName)
{
	string line = "";
	if (fileName.find(".ascii", fileName.length() - 6) != string::npos)
	{
		IFStreamAscii input;
		input.openForRead(fileName);
		line = input.readString();
		input.close();
	}
	else
	{
		IFStreamBinary ifs(fileName, 512);
		ifs.boundaryAlignmentOff();
		ifs.resetPos();
		ifs.readCharArray(line, 11);
	}

	return (line == "GEOTESSGRID");
}

/**
 * Open the specified file using the appropriate format, and read only enough of the file to
 * retrieve the gridID.
 *
 * @param fileName
 * @return String gridID
 * @throws IOException
 */
string GeoTessGrid::getGridID(const string& fileName)
{
	string gridID = "";
	if (fileName.find(".ascii", fileName.length() - 6) != string::npos)
	{
		int gridFileFormat;
		string gridSWVersion, fileCreationDate;

		IFStreamAscii input;
		input.openForRead(fileName);

		loadGridAsciiFront(input, gridFileFormat, gridSWVersion, fileCreationDate, gridID, "null");
		input.close();
	}
	else
	{
		int gridFileFormat;
		string grdSWVersion, fileCreationDate;

		IFStreamBinary ifs(fileName, 512);
		ifs.boundaryAlignmentOff();
		ifs.resetPos();

		loadGridBinaryFront(ifs, gridFileFormat, grdSWVersion, fileCreationDate, gridID, "null");
	}

	return gridID;
}

/**
 * Load GeoTessGrid object from a File.
 *
 * @param inputFile The name of the file from which the grid is to be read.  if the extension
 *                  is 'bin' the model is written to a binary file, otherwise it is written to
 *                  an ascii file.
 * @throws GeoTessException
 */
GeoTessGrid* GeoTessGrid::loadGrid(const string& inputFile)
{
	if (inputFile.find(".ascii", inputFile.length() - 6) != string::npos)
		loadGridAscii(inputFile);
	else
		loadGridBinary(inputFile);

	gridInputFile = inputFile;

	return this;
}

/**
 * Load the 2D grid from an Ascii File.
 *
 * @param file
 * @throws IOException
 */
GeoTessGrid* GeoTessGrid::loadGridAscii(const string& inputFile)
{
	IFStreamAscii input;
	input.openForRead(inputFile);
	loadGridAscii(input);
	input.close();

	return this;
}

void GeoTessGrid::loadGridAsciiFront(IFStreamAscii& input, int& gridFileFormat,
		string& gridSWVersion, string& fileCreationDate,
		string& gridid, const string& grdInptFile)
{
	string line = input.readString();
	if (line != "GEOTESSGRID")
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessModel::loadModelAscii" << endl
				<< "  expected file : " << grdInptFile << endl
				<< " to start with \"GEOTESSGRID\" as first line but found \"" << endl
				<< line << "\" instead" << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 2003);
	}

	// read file format version number

	gridFileFormat = input.readInteger();

	if (gridFileFormat != 2)
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessGrid::loadGridAscii" << endl
				<< "Grid file format version " << gridFileFormat
				<< " is not supported by this version of GeoTessGridAscii" << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 2004);
	}

	// read grid population software version and file creation date

	gridSWVersion = "";
	input.readLine(gridSWVersion);
	fileCreationDate = CPPUtils::trim(gridSWVersion);

	fileCreationDate = "";
	input.readLine(fileCreationDate);
	fileCreationDate = CPPUtils::trim(fileCreationDate);

	// read grid data

	input.readString(gridid);
}

/**
 * Read grid from a Scanner in ascii format. Scanner is not closed by this method.
 *
 * @param input Scanner from which to read grid definition.
 */
GeoTessGrid* GeoTessGrid::loadGridAscii(IFStreamAscii& input)
{
	int gridFileFormat;
	string gridSWVersion, fileCreationDate;

	loadGridAsciiFront(input, gridFileFormat, gridSWVersion, fileCreationDate,
			gridID, gridInputFile);

	setGridSoftwareVersion(gridSWVersion);
	setGridGenerationDate(fileCreationDate);

	// read nTessellations, nLevels, nTriangles and nVertices.
	nTessellations = input.readInteger();
	nLevels = input.readInteger();
	nTriangles = input.readInteger();
	nVertices = input.readInteger();

	tessellations = CPPUtils::new2DArray<int>(nTessellations, 2);
	levels = CPPUtils::new2DArray<int>(nLevels, 2);
	triangles = CPPUtils::new2DArray<int>(nTriangles, 3);
	vertices = CPPUtils::new2DArray<double>(nVertices, 3);

	//comment = input.nextLine();
	//comment = input.nextLine(); // skip comment
	// System.out.println(comment);
	for (int i = 0; i < nTessellations; ++i)
		for (int j = 0; j < 2; ++j)
			tessellations[i][j] = input.readInteger();

	//comment = input.nextLine();
	//comment = input.nextLine(); // skip comment
	for (int i = 0; i < nLevels; ++i)
		for (int j = 0; j < 2; ++j)
			levels[i][j] = input.readInteger();

	//comment = input.nextLine();
	//comment = input.nextLine(); // skip comment
	// System.out.println(comment);
	// for each vertex, read the geocentric unit vector
	for (int i = 0; i < nVertices; ++i)
		for (int j = 0; j < 3; ++j)
			vertices[i][j] = input.readDouble();

	//comment = input.nextLine();
	//comment = input.nextLine(); // skip comment
	// System.out.println(comment);
	// for each triangle read the indexes of the 3 vertices that form the
	// corners of the triangle,
	for (int i = 0; i < nTriangles; ++i)
		for (int j = 0; j < 3; ++j)
			triangles[i][j] = input.readInteger();

	initialize();

	return this;
}

/**
 * Load the 2D grid from a File.
 *
 * @param file
 * @throws IOException
 */
GeoTessGrid* GeoTessGrid::loadGridBinary(const string& inputFile)
{
	IFStreamBinary ifs(inputFile);
	ifs.boundaryAlignmentOff();
	ifs.resetPos();

	loadGridBinary(ifs);

	return this;
}

const set<int>&	GeoTessGrid::getVertexIndices(const int& level)
{
	set<int>& v = connectedVertices[level];
	if (v.size() == 0)
	{
		// if spokeList for this level has been computed, use it
		// but don't compute it just for this.
		if (!spokeList[level].empty())
		{
			vector<Edge*>& spokes = spokeList[level];
			for (int vtx=0; vtx<nVertices; ++vtx)
				if (spokes[vtx] != NULL)
					v.insert(spokes[vtx]->vj);
		}
		else
			for (int t = levels[level][0]; t < levels[level][1]; ++t)
			{
				int* triangle = triangles[t];
				v.insert(triangle[0]);
				v.insert(triangle[1]);
				v.insert(triangle[2]);
			}
	}
	return v;
}

void	GeoTessGrid::getVertices(const int& tessellation, const int& level,set<const double*>& vectors)
{
	vectors.clear();
	int lvl = tessellations[tessellation][0] + level;
	getVertexIndices(lvl);
	set<int>::iterator it;
	for (it = connectedVertices[lvl].begin(); it != connectedVertices[lvl].end(); ++it)
		vectors.insert(vertices[*it]);
}


/**
 * Retrieve a list of the indexes of all the vertexes that are connected to the specified vertex
 * by a single edge, considering only triangles in the specified tessellation and level. The
 * vertices will be arranged in clockwise order when viewed from outside the unit sphere.
 *
 * @param tessId tessellation index
 * @param level index of a level relative to the first level of the specified tessellation
 * @param vertex
 * @return list of vertex indices in clockwise order.
 * @throws GeoTessException
 */
void GeoTessGrid::getVertexNeighborsOrdered(const int& tessId, const int& level, const int& vertex,
		vector<int>& v)
{
	int lvl = getLevel(tessId, level);

	computeSpokeLists(lvl);

	v.clear();
	Edge* head = spokeList[lvl][vertex];
	if (head != NULL)
	{
		Edge *spoke = head;
		do { v.push_back(spoke->vk); spoke=spoke->next; } while (spoke != head);
	}
}

void GeoTessGrid::getVertexNeighbors(const int& tessId, const int& level, const int& vertex, const int& order,
		set<int>& nbrs)
{
	set<int> nghbrOrder;
	set<int>::iterator it;

	nbrs.clear();
	nbrs.insert(vertex);
	for (int o = 0; o < order; ++o)
	{
		nghbrOrder.clear();
		for (it = nbrs.begin(); it != nbrs.end(); ++it)
		{
			const vector<int>& tneighbors = getVertexTriangles(tessId, level, *it);
			for (int i = 0; i < (int) tneighbors.size(); ++i)
			{
				nghbrOrder.insert(getTriangleVertexIndex(tneighbors[i], 0));
				nghbrOrder.insert(getTriangleVertexIndex(tneighbors[i], 1));
				nghbrOrder.insert(getTriangleVertexIndex(tneighbors[i], 2));
			}
		}
		for (it = nghbrOrder.begin(); it != nghbrOrder.end(); ++it)
			nbrs.insert(*it);
	}
	nbrs.erase(vertex);
}

/**
 * @return summary information about this GeoTessGrid object.
 */
string GeoTessGrid::toString()
{
	ostringstream os;
	os << "GeoTessGrid" << endl
			<< "gridID = " << gridID << endl
			<< "memory : " << getMemory()/1024./1024. << " MB" << endl
			<< "input Grid File : "
			<< ((gridInputFile == "") ? "null" : gridInputFile) << endl
			<< "generated by software version : " << gridSoftwareVersion
			<< "  " << gridGenerationDate << endl;
	if (gridOutputFile != "null")
		os << "output Grid File : " << gridOutputFile << endl;

	os << endl
			<< "nTessellations = " << nTessellations << endl
			<< "nLevels = " << nLevels << endl
			<< "nVertices = " << nVertices << endl
			<< "nTriangles = " << nTriangles << endl << endl
			<< "    Tess    Level  LevelID     NTri    First   Last+1" << endl;

	char buf[300];
	for (int tess = 0; tess < nTessellations; ++tess)
	{
		for (int level = 0; level < getNLevels(tess); ++level)
		{
			sprintf(buf, " %6d %8d %8d %8d %8d %8d", tess, level,
					tessellations[tess][0] + level,
					getNTriangles(tess, level),
					getTriangle(tess, level, 0),
					getTriangle(tess, level, getNTriangles(tess, level)));
			os << buf << endl;
		}
		os << endl;
	}
	return os.str();
}

///**
// * For every edge of every triangle, compute the inward pointing normal to the edge, i.e., in a
// * triangle formed by nodes ni, nj and nk, set edge[i] = nk cross nj.
// * <p>
// * This variable can be large and hence it is optional. If it is computed then the triangle
// * walking algorithm will be much faster because it will only have to compute dot products
// * instead of scalar triple products. If it remains null (is never computed) then the walking
// * triangle algorithm will be slower.
// * <p>
// * Whether or not edges are computed is determined by the setting of variable
// * <it>optimization</it>. If set to SPEED, the edges are computed, otherwise they are not.
// */
//void GeoTessGrid::findEdges()
//{
//	if (edgeNormals != NULL) CPPUtils::delete3DArray<double>(edgeNormals);
//	edgeNormals = CPPUtils::new3DArray<double>(nTriangles, 3, 3);
//	for (int i = 0; i < nTriangles; ++i)
//	{
//		int* triangle = triangles[i];
//		double** n = edgeNormals[i];
//
//		GeoTessUtils::cross(vertices[triangle[2]],
//				vertices[triangle[1]], n[0]);
//		GeoTessUtils::cross(vertices[triangle[0]],
//				vertices[triangle[2]], n[1]);
//		GeoTessUtils::cross(vertices[triangle[1]],
//				vertices[triangle[0]], n[2]);
//	}
//}

/**
 * Perform walking triangle search to find the index of the triangle that contains position
 * defined by vector and which has no descendant.
 *
 * @param triangleIndex the index of a triangle from which to start the search.
 * @param vector the unit vector representing the position for which to search.
 * @return the index of the triangle containing the specified position.
 */
int GeoTessGrid::getTriangle(int triangleIndex, const double* vector)
{
	while (true)
	{
		if (GeoTessUtils::dot(edgeList[triangleIndex][0]->normal, vector) > -1e-15)
		{
			if (GeoTessUtils::dot(edgeList[triangleIndex][1]->normal, vector) > -1e-15)
			{
				if (GeoTessUtils::dot(edgeList[triangleIndex][2]->normal, vector) > -1e-15)
				{
					if (descendants[triangleIndex] < 0)
						return triangleIndex;
					else
						triangleIndex = descendants[triangleIndex];
				}
				else
					triangleIndex = edgeList[triangleIndex][2]->tLeft;
			}
			else
				triangleIndex = edgeList[triangleIndex][1]->tLeft;
		}
		else
			triangleIndex = edgeList[triangleIndex][0]->tLeft;
	}
	return -1; // this is impossible!
}

/**
 * Identify the neighbors and descendants of each triangle. This method is called during
 * construction of a GeoTessGrid object (i.e., when it is loaded from a file). Applications
 * should not call this method.
 *
 * <p>
 * If optimization is set to SPEED, then for each edge of a triangle the unit vector normal to
 * the plane of the great circle containing the edge will be computed during input of the grid
 * from file and stored in memory. With this information, the walking triangle algorithm can use
 * dot products instead of scalar triple products when determining if a point resides inside a
 * triangle. While much more computationally efficient, it requires alot of memory to store all
 * those unit vectors.
 *
 * @param optimization
 * @return
 */
void GeoTessGrid::initialize()
{
	// first, populate vtxTriangles, which is an
	// nLevels x nVertices x n array of triangle indices
	// that contains the list of triangles a vertex touches
	// as a function of tessellation level.
	int* count = new int[nVertices];
	vtxTriangles.resize(nLevels);
	for (int lvl=0; lvl < nLevels; ++lvl)
	{
		vector< vector<int> >& vtxT = vtxTriangles[lvl];
		vtxT.resize(nVertices);

		// count the number triangles each vertex touches.
		// reminder: triangles[t][c] is a vertex index
		for (int v=0; v<nVertices; ++v) count[v] = 0;
		for (int t = levels[lvl][0]; t < levels[lvl][1]; ++t)
			for (int c = 0; c < 3; ++c)
				++count[triangles[t][c]];

		// for each vertex, set the capacity of the vector
		// to the actual size that is required.
		// The vast majority will be zero; many will be six.
		for (int v=0; v<nVertices; ++v)
		{
			vtxT[v].resize(count[v]);
			vtxT[v].clear();
		}

		for (int t = levels[lvl][0]; t < levels[lvl][1]; ++t)
			for (int c = 0; c < 3; ++c)
				vtxT[triangles[t][c]].push_back(t);
	}
	delete[] count;

	// now start working on the triangle neighbors.
	int** neighbors = CPPUtils::new2DArray<int>(nTriangles, 3);

	// Find the 3 neighbors of each triangle.
	// This is an implementation of the "Triangle neighbor identification"
	// algorithm in Ballard, Hipp and Young, 2009,
	// Efficient and Accurate Calculation of Ray Theory Seismic Travel
	// Time through Variable Resolution 3D Earth Models, SRL, 80, 989-999.

	int n, c, vj, vk;
	int* corners;

	bool* marked = new bool[nTriangles];
	CPPUtils::resetArray<bool>(nTriangles, marked, false);

	// loop over all the levels of all tessellations
	for (int tess = 0; tess < nTessellations; ++tess)
	{
		int* tessLevels = tessellations[tess];
		for (int level = tessLevels[0]; level < tessLevels[1]; ++level)
		{
			int* levelTriangles = levels[level];
			vector< vector<int> >& vtxT = vtxTriangles[level];
			for (int t=levelTriangles[0]; t<levelTriangles[1]; ++t)
			{
				corners = triangles[t];
				// t is the index of a triangle and corners are the indexes
				// of the 3 vertices that reside at the corners of triangle
				// t.
				// Loop over corners of triangle t
				for (c = 0; c < 3; ++c)
				{
					vj = corners[(c + 1) % 3];
					vk = corners[(c + 2) % 3];
					// c is the index of a corner of triangle t (c is one of
					// 0,1,2).
					// vj is the index of the vertex (not corner) that is
					// found by
					// moving clockwise around t from corner c.
					// vk is the index of the vertex (not corner) that is
					// found by
					// moving clockwise around t from vj

					// indexes of the triangles of which vj and vk are members
					const vector<int>& tj = vtxT[vj];
					const vector<int>& tk = vtxT[vk];


					// mark all the triangles of which vertex vj is a member
					for (n = 0; n < (int) tj.size(); ++n) marked[tj[n]] = true;

					// loop over all the triangles of which vk is a member.
					// Two of them will be marked. One of the ones that is
					// marked is triangle t. The other one is the triangle
					// that resides on the other side of the edge that
					// connects
					// vertices vj and vk. That second triangle is the
					// neighbor
					// of triangle t.
					for (n = 0; n < (int) tk.size(); ++n)
						if (marked[tk[n]] && (tk[n] != t))
						{
							neighbors[t][c] = tk[n];
							break;
						}

					// unmark all the triangles that were recently marked.
					for (n = 0; n < (int) tj.size(); ++n) marked[tj[n]] = false;
				}
			}
		}
	}
	// done with marked
	delete [] marked;

	// delete edges if they already exist
	for (int t=0; t<(int)edgeList.size(); ++t)
	{
		vector<Edge*>& edges = edgeList[t];
		for (int e=0; e<(int)edges.size(); ++e)
			if (edges[e] != NULL)
				delete edges[e];
	}
	spokeList.clear();
	edgeList.clear();


	// compute the Edges for all the triangles.
	edgeList.resize(nTriangles);
	Edge* edge;
	for (int triangle = 0; triangle < nTriangles; ++triangle)
	{
		vector<Edge*>& tedges = edgeList[triangle];
		tedges.resize(3);

		int* vtxIds = triangles[triangle];
		int* nbrs = neighbors[triangle];

		for (int i=0; i<3; ++i)
		{
			int j=(i+1)%3, k=(j+1)%3;
			edge = new Edge();
			edge->vj = vtxIds[j];
			edge->vk = vtxIds[k];
			edge->tRight = triangle;
			edge->tLeft = nbrs[i];

			if (triangles[nbrs[i]][0] == vtxIds[j]) edge->cornerj = 0;
			else if (triangles[nbrs[i]][1] == vtxIds[j]) edge->cornerj = 1;
			else if (triangles[nbrs[i]][2] == vtxIds[j]) edge->cornerj = 2;
			else edge->cornerj = -1;

			GeoTessUtils::cross(vertices[edge->vk], vertices[edge->vj], edge->normal);
			edge->next = NULL;
			tedges[i] = edge;
		}
	}
	spokeList.resize(nLevels);

	// neighbor info is stored in the Edges.  Temp work space can be deleted.
	CPPUtils::delete2DArray<int>(neighbors);

	// find the descendant of each triangle at the next higher tessellation level.
	if (descendants != NULL) delete[] descendants;
	descendants = new int[nTriangles];

	// initialize all the descendants to -1.
	for (int i = 0; i < nTriangles; ++i)
		descendants[i] = -1;

	double len;
	double *v;
	double x[3];
	int startTriangle, *levelTriangles;

	// loop over all but the last level of each tessellations. The
	// descendants of elements on the last level of each tessellation will remain -1.
	for (int tess = 0; tess < nTessellations; ++tess)
	{
		int* tessLevels = tessellations[tess];
		for (int level = tessLevels[0]; level < tessLevels[1]-1; ++level)
		{
			// set startingTriangle to the first triangle on next level up.
			startTriangle = levels[level+1][0];

			levelTriangles = levels[level];
			for (int t=levelTriangles[0]; t<levelTriangles[1]; ++t)
			{
				corners = triangles[t];

				// set x.vector to a unit vector at center of triangle t
				v = vertices[corners[0]];
				x[0] = v[0];
				x[1] = v[1];
				x[2] = v[2];
				v = vertices[corners[1]];
				x[0] += v[0];
				x[1] += v[1];
				x[2] += v[2];
				v = vertices[corners[2]];
				x[0] += v[0];
				x[1] += v[1];
				x[2] += v[2];
				len = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
				x[0] /= len;
				x[1] /= len;
				x[2] /= len;

				// start from startTriangle, which is on the next higher
				// level from the current triangle, and walk to the
				// triangle that contains x.vector.
				// That is the descendant of the current triangle.
				// Set startTriangle equal to the descendant so we won't
				// have to walk too far when we search for the descendant
				// of the next triangle, which is likely
				// close by (this makes a huge difference).
				descendants[t] = startTriangle = getTriangle(startTriangle, x);
			}
		}
	}
	connectedVertices.resize(nLevels);
}

void GeoTessGrid::computeSpokeLists(const int& level) const
{
	if (spokeList[level].empty())
	{
		Edge *edge, *head, *spoke, *prev, *next;
		// levelSpokes is an nVertices array of spokes (Edge*).  For each vertex, it contains
		// the first spoke (Edge*) that emanates from the vertex.  Each spoke has a pointer to
		// the next spoke in the circular list of spokes that emanate from the same vertex.
		vector<Edge*>& levelSpokes=spokeList[level];
		levelSpokes.resize(nVertices, NULL);

		for (int triangle = levels[level][0]; triangle < levels[level][1]; ++triangle)
		{
			const vector<Edge*>& tedges = edgeList[triangle];
			for (int i=0; i<3; ++i)
			{
				edge = tedges[i];
				spoke = levelSpokes[edge->vj];
				if (spoke == NULL)
					levelSpokes[edge->vj] = edge;
				else
				{
					edge->next = spoke;
					levelSpokes[edge->vj] = edge;
				}
			}
		}

		for (int vertex=0; vertex<nVertices; ++vertex)
		{
			head = levelSpokes[vertex];
			if (head != NULL)
			{
				spoke = head;
				while (spoke->next != NULL)
				{
					prev = spoke;
					next = spoke->next;
					while (next->tLeft != spoke->tRight)
					{
						prev = next;
						next = next->next;
					}
					prev->next = next->next;
					next->next = spoke->next;
					spoke->next = next;
					spoke = next;
				}
				spoke->next = head;

				//				// test!
				//				spoke = levelSpokes[vertex];
				//				for (int i=0; i<20; ++i)
				//				{
				//					cout << spoke->tRight << " " << spoke->tLeft << "  " <<
				//							spoke->next->tRight << " " << spoke->next->tLeft << endl;
				//					if(spoke->next->tLeft != spoke->tRight)
				//						throw GeoTessException("edges are out of order", __FILE__, __LINE__, 2005);
				//					spoke = spoke->next;
				//				}
				//				cout << endl;
			}
		}
	}
}

/**
 * Convert tessellation to a Delaunay tessellation.
 * @return
 *
 * @return number of changes.
 * @throws GeoModelException
 */
int GeoTessGrid::delaunay()
{
	computeCircumCenters();

	int nChanges = 0, tLeft, tRight, iLeft, jLeft, kLeft, iRight, jRight, kRight, nbr, nid;

	int vLeft[3], vRight[3], *idx;

	Edge *erk, *elj, *elk, *erj, *eli, *eri;

	const double* center;

	for (int t=0; t<nTriangles; ++t)
		for (int corner=0; corner<3; ++corner)
		{
			// get the indices of the triangles to the left and right of the common edge.
			tLeft = edgeList[t][corner]->tLeft;
			tRight = edgeList[t][corner]->tRight;

			// retrieve the circumcenter of the triangle on the 'left'
			// side of this edge.
			center = getCircumCenter(tLeft);

			// find the index of tLeft in tRight's array of neighbors.
			// This will correspond to the corner in tRight that is not on edge.
			iRight = getNeighborIndex( tRight, tLeft);

			// if vertex at i2 is inside the circumCircle of tLeft then we
			// need to flip the edge between tLeft and tRight
			if (GeoTessUtils::dot(center, getTriangleVertex(tRight, iRight))-1e-15 > center[3])
			{
				// find the index of tRight in tLeft's array of neighbors.
				// This will correspond to the corner in tLeft that is not on edge.
				iLeft =  getNeighborIndex(tLeft, tRight);

				jLeft = (iLeft+1)%3;
				kLeft = (iLeft+2)%3;
				jRight = (iRight+1)%3;
				kRight = (iRight+2)%3;

				// get copies of the triangles to left and right.
				// These are vertex indices
				vLeft[0] = triangles[tLeft][0];
				vLeft[1] = triangles[tLeft][1];
				vLeft[2] = triangles[tLeft][2];
				vRight[0] = triangles[tRight][0];
				vRight[1] = triangles[tRight][1];
				vRight[2] = triangles[tRight][2];

				// reorganize all the edges.
				eli = edgeList[tLeft][iLeft];
				elj = edgeList[tLeft][kLeft];
				elk = edgeList[tRight][jRight];

				eri = edgeList[tRight][iRight];
				erj = edgeList[tRight][kRight];
				erk = edgeList[tLeft][jLeft];

				eli->vj = vRight[iRight];
				eli->vk = vLeft[iLeft];
				eli->tLeft = tRight;
				eli->tRight = tLeft;

				eri->vj = eli->vk;
				eri->vk = eli->vj;
				eri->tLeft = tLeft;
				eri->tRight = tRight;

				erk->tRight = tRight;
				elk->tRight = tLeft;

				edgeList[tLeft][0] = eli;
				edgeList[tLeft][1] = elj;
				edgeList[tLeft][2] = elk;
				edgeList[tRight][0] = eri;
				edgeList[tRight][1] = erj;
				edgeList[tRight][2] = erk;

				// reorganize the neighbors and their edges
				nbr = elk->tLeft;
				nid = getNeighborIndex(nbr, tRight);
				edgeList[nbr][nid]->tLeft = tLeft;

				nbr = erk->tLeft;
				nid = getNeighborIndex(nbr, tLeft);
				edgeList[nbr][nid]->tLeft = tRight;

				// change the vertex indices of the two triangles.
				idx = triangles[tLeft];
				idx[0] = vLeft[jLeft];
				idx[1] = vRight[iRight];
				idx[2] = vLeft[iLeft];

				GeoTessUtils::circumCenterPlus(vertices[idx[0]], vertices[idx[1]],
						vertices[idx[2]], circumCenters[tLeft]);

				idx = triangles[tRight];
				idx[0] = vRight[jRight];
				idx[1] = vLeft[iLeft];
				idx[2] = vRight[iRight];

				GeoTessUtils::circumCenterPlus(vertices[idx[0]], vertices[idx[1]],
						vertices[idx[2]], circumCenters[tRight]);

				++nChanges;
			}
		}

	//cout << "GeoTessGrid.delaunay().  nChanges=" << nChanges << "  totalChanges=" << totalChanges << endl;

	initialize();

	return nChanges;
}

/**
 * Tests the integrity of the grid. Visits every triangle T, and (1) checks
 * to ensure that every neighbor of T includes T in its list of neighbors,
 * and (2) checks that every neighbor of T shares exactly two nodes with T.
 *
 * @throws GeoTessException
 *             if anything is amiss.
 */
void GeoTessGrid::testGrid()
{
	for (int tessId = 0; tessId < getNTessellations(); ++tessId)
		for (int level = 0; level < getNLevels(tessId); ++level)
		{
			for (int t = 0; t < getNTriangles(tessId, level); ++t)
			{
				// triangle is an index in the triangle array
				int triangle = getTriangle(tessId, level, t);

				for (int triangleSide = 0; triangleSide < 3; triangleSide++)
				{
					// neighbor and triangle share and edge
					int neighbor = getNeighbor(triangle, triangleSide);

					// the index of triangle in neighbors array of neighbors.
					int neighborSide = getNeighborIndex(neighbor, triangle);

					if (neighborSide < 0)
					{
						ostringstream os;
						os << endl << "ERROR in GeoTessGrid::testGrid" << endl
								<< "tessId="<<tessId<<" level="<<level<<" triangle="<<triangle
								<<" side="<<triangleSide<<" neighbor="<<neighbor << endl;
						throw GeoTessException(os, __FILE__, __LINE__, 2005);
					}

					// compare the vertices.
					if (triangles[neighbor][(neighborSide+1)%3] != triangles[triangle][(triangleSide+2)%3])
					{
						ostringstream os;
						os << endl << "ERROR in GeoTessGrid::testGrid" << endl
								<< "tessId="<<tessId<<" level="<<level<<" triangle="<<triangle
								<<" side="<<triangleSide<<" neighbor="<<neighbor << endl
								<< "triangles[neighbor][(neighborSide+1)%3] != triangles[triangle][(triangleSide+2)%3]"
								<< endl;
						throw GeoTessException(os, __FILE__, __LINE__, 2006);
					}

					if (triangles[neighbor][(neighborSide+2)%3] != triangles[triangle][(triangleSide+1)%3])
					{
						ostringstream os;
						os << endl << "ERROR in GeoTessGrid::testGrid" << endl
								<< "tessId="<<tessId<<" level="<<level<<" triangle="<<triangle
								<<" side="<<triangleSide<<" neighbor="<<neighbor << endl
								<< "triangles[neighbor][(neighborSide+2)%3] != triangles[triangle][(triangleSide+1)%3]"
								<< endl;
						throw GeoTessException(os, __FILE__, __LINE__, 2007);
					}

				}

				// if not the top level then check descendants.
				if (level < getNLevels(tessId)-1)
				{
					if (descendants[triangle] <= getLastTriangle(tessId, level))
					{
						ostringstream os;
						os << endl << "ERROR in GeoTessGrid::testGrid" << endl
								<< "tessId="<<tessId<<" level="<<level<<" triangle="<<triangle
								<< "neighbors[triangle][3] <= getLastTriangle(tessId, level)"
								<< endl;
						throw GeoTessException(os, __FILE__, __LINE__, 2008);
					}

					if (descendants[triangle] > getLastTriangle(tessId, level+1))
					{
						ostringstream os;
						os << endl << "ERROR in GeoTessGrid::testGrid" << endl
								<< "tessId="<<tessId<<" level="<<level<<" triangle="<<triangle
								<< "neighbors[triangle][3] > getLastTriangle(tessId, level+1)"
								<< endl;
						throw GeoTessException(os, __FILE__, __LINE__, 2009);
					}
				}
			}

			// get index of first triangle on this level.
			int t = getTriangle(tessId, level, 0);

			// test the first triangle on this level to ensure that the vertices are
			// specified in clockwise order when viewed from outside the sphere.
			if (GeoTessUtils::scalarTripleProduct(getTriangleVertex(t, 0), getTriangleVertex(t, 1),
					getTriangleVertex(t, 2)) > 0.)
			{
				ostringstream os;
				os << endl << "ERROR in GeoTessGrid::testGrid" << endl
						<< "Vertices of this triangle are specified in counter-clockwise order" << endl
						<< "tessId="<<tessId<<" level="<<level<<" triangle=0"<<endl
						<< GeoTessUtils::getLatLonString(getTriangleVertex(t, 0))<<endl
						<< GeoTessUtils::getLatLonString(getTriangleVertex(t, 1))<<endl
						<< GeoTessUtils::getLatLonString(getTriangleVertex(t, 2))<<endl
						<< endl;
				throw GeoTessException(os, __FILE__, __LINE__, 2009);
			}
		}
}

/**
 * Load the 2D grid from an InputStream, which is neither opened nor closed by this method.
 *
 * @param input
 * @throws IOException
 */
GeoTessGrid* GeoTessGrid::loadGridBinary(IFStreamBinary& ifs)
{
	int gridFileFormat;
	string grdSWVersion, fileCreationDate;

	loadGridBinaryFront(ifs, gridFileFormat, grdSWVersion, fileCreationDate,
			gridID, gridInputFile);

	setGridSoftwareVersion(grdSWVersion);
	setGridGenerationDate(fileCreationDate);

	nTessellations = ifs.readInt();
	tessellations = CPPUtils::new2DArray<int>(nTessellations, 2);
	nLevels = ifs.readInt();
	levels = CPPUtils::new2DArray<int>(nLevels, 2);
	nTriangles = ifs.readInt();
	triangles = CPPUtils::new2DArray<int>(nTriangles, 3);
	nVertices = ifs.readInt();
	vertices = CPPUtils::new2DArray<double>(nVertices, 3);

	for (int i = 0; i < nTessellations; ++i)
	{
		int* a = tessellations[i];
		a[0] = ifs.readInt();
		a[1] = ifs.readInt();
	}

	for (int i = 0; i < nLevels; ++i)
	{
		int* a = levels[i];
		a[0] = ifs.readInt();
		a[1] = ifs.readInt();
	}

	for (int i = 0; i < nVertices; ++i)
	{
		double* a = vertices[i];
		a[0] = ifs.readDouble();
		a[1] = ifs.readDouble();
		a[2] = ifs.readDouble();
	}

	for (int i = 0; i < nTriangles; ++i)
	{
		int* a = triangles[i];
		a[0] = ifs.readInt();
		a[1] = ifs.readInt();
		a[2] = ifs.readInt();
	}

	initialize();

	return this;
}

void GeoTessGrid::loadGridBinaryFront(IFStreamBinary& ifs, int& gridFileFormat,
		string& gridSWVersion, string& fileCreationDate,
		string& gridid, const string& grdInptFile)
{
	// get the GeoTess name (GEOTESSGRID) and validate

	string GTName;
	ifs.readCharArray(GTName, 11);
	if (GTName != "GEOTESSGRID")
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessGrid::loadGridBinary" << endl
				<< "  expected file : " << grdInptFile << endl
				<< "  to start with char array \"GEOTESSGRID\" as first entry "
				<< "but found \"";
		for (int i = 0; i < 11; ++i)
			if ((GTName[i] != 127) && (GTName[i] > 31))
				os << GTName[i];
			else
				os << "[" << (int) GTName[i] << "]";
		os << "\" instead ..." << endl;

		throw GeoTessException(os, __FILE__, __LINE__, 2010);
	}

	// read in format ... if it exceeds 65536 then turn on byte reversal,
	// reset read pointer back by sizeof(int) and try again.

	gridFileFormat = ifs.readInt();
	if ((gridFileFormat < 0) || (gridFileFormat > 65536))
	{
		ifs.setByteOrderReverse(!ifs.isByteOrderReversalOn());
		ifs.decrementPos(CPPUtils::SINT);
		gridFileFormat = ifs.readInt();
	}

	if (gridFileFormat != 2)
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessGrid::loadGridBinary" << endl
				<< gridFileFormat << " is not a recognized file format version" << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 2011);
	}

	// read grid population software version and file creation date

	gridSWVersion = "";
	ifs.readString(gridSWVersion);
	fileCreationDate = "";
	ifs.readString(fileCreationDate);

	// read grid data

	ifs.readString(gridid);
}


void GeoTessGrid::writeGrid(const string& fileName)
{
	if (fileName.find(".ascii", fileName.length() - 6) != string::npos)
		writeGridAscii(fileName);
	else
		writeGridBinary(fileName);

	gridOutputFile = fileName;
}

/**
 * Write the 2D grid to a file.
 *
 * @param output
 * @throws IOException
 */
void GeoTessGrid::writeGridBinary(const string& fileName)
{
	IFStreamBinary ofs;
	if (!CPPUtils::isBigEndian()) ofs.byteOrderReverseOn();
	ofs.boundaryAlignmentOff();

	writeGridBinary(ofs);
	ofs.writeToFile(fileName);
}

/**
 * Write the 2D grid to a file.
 *
 * @param output
 * @throws IOException
 */
void GeoTessGrid::writeGridBinary(IFStreamBinary& output)
{
	// write out file type identifier ("GEOTESSGRID"), format version,
	// code version, and data stamp

	output.writeCharArray("GEOTESSGRID", 11);
	output.writeInt(2);
	output.writeString(getGridSoftwareVersion());
	output.writeString(getGridGenerationDate());

	// write grid data

	output.writeString(gridID);

	output.writeInt(nTessellations);
	output.writeInt(nLevels);
	output.writeInt(nTriangles);
	output.writeInt(nVertices);

	for (int i = 0; i < nTessellations; ++i)
	{
		output.writeInt(tessellations[i][0]);
		output.writeInt(tessellations[i][1]);
	}

	for (int i = 0; i < nLevels; ++i)
	{
		output.writeInt(levels[i][0]);
		output.writeInt(levels[i][1]);
	}

	for (int i = 0; i < nVertices; ++i)
	{
		output.writeDouble(vertices[i][0]);
		output.writeDouble(vertices[i][1]);
		output.writeDouble(vertices[i][2]);
	}

	for (int i = 0; i < nTriangles; ++i)
	{
		output.writeInt(triangles[i][0]);
		output.writeInt(triangles[i][1]);
		output.writeInt(triangles[i][2]);
	}
}

/**
 * Write the grid out to an ascii file. File format 2.
 */
void GeoTessGrid::writeGridAscii(const string& fileName)
{
	IFStreamAscii ifs;
	ifs.openForWrite(fileName);
	writeGridAscii(ifs);
	ifs.close();
}

/**
 * Write the grid out to an ascii file. File format 2.
 */
void GeoTessGrid::writeGridAscii(IFStreamAscii& output)
{
	// write out file type identifier ("GEOTESSGRID"), format version,
	// code version, and data stamp

	output.writeStringNL("GEOTESSGRID");
	output.writeStringNL("2");
	output.writeStringNL(getGridSoftwareVersion());
	output.writeStringNL(getGridGenerationDate());

	// write grid data

	output.writeStringNL("#unique Grid ID:");
	output.writeStringNL(gridID);

	output.writeStringNL("#geotess grid java: nTessellations, nLevels, nTriangles, nVertices:");
	output.writeInt(nTessellations); output.writeString(" ");
	output.writeInt(nLevels); output.writeString(" ");
	output.writeInt(nTriangles); output.writeString(" ");
	output.writeIntNL(nVertices);

	output.writeStringNL("#geotess grid tessellations:");
	for (int i = 0; i < nTessellations; ++i)
	{
		output.writeInt(tessellations[i][0]); output.writeString(" ");
		output.writeIntNL(tessellations[i][1]);
	}

	output.writeStringNL("#geotess grid levels:");
	for (int i = 0; i < nLevels; ++i)
	{
		output.writeInt(levels[i][0]); output.writeString(" ");
		output.writeIntNL(levels[i][1]);
	}

	output.writeStringNL("#geotess grid vertices(unit_vectors):");
	for (int i = 0; i < nVertices; ++i)
	{
		output.writeDouble(vertices[i][0]); output.writeString(" ");
		output.writeDouble(vertices[i][1]); output.writeString(" ");
		output.writeDoubleNL(vertices[i][2]);
	}

	output.writeStringNL("#geotess grid triangles:");
	for (int i = 0; i < nTriangles; ++i)
	{
		output.writeInt(triangles[i][0]); output.writeString(" ");
		output.writeInt(triangles[i][1]); output.writeString(" ");
		output.writeIntNL(triangles[i][2]);
	}

	output.flush();
}

} // end namespace geotess
