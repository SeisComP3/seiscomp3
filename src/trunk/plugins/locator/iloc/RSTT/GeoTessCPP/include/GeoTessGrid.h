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

#ifndef GEOTESSGRID_OBJECT_H
#define GEOTESSGRID_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <set>
#include <list>
#include <set>
#include <sstream>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"
#include "GeoTessUtils.h"
#include "ArrayReuse.h"
#include "GeoTessException.h"
#include "GeoTessOptimizationType.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess
{

// **** _FORWARD REFERENCES_ ***************************************************

class IFStreamAscii;
class IFStreamBinary;

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Stores information about the connection between two adjacent vertices which
 * separates two neighboring triangles.
 *
 * An Edge stores information about the connection between two adjacent vertices which
 * separates two neighboring triangles.  These Edge objects are used in two contexts:
 * (1) every triangle in the grid has an array of three Edges, stored in variable
 * edgeList.  (2) at every level of the grid, each vertex has a circular linked
 * list of Edges which define the Edges emanating from the vertex, in clockwise order.
 * These Edges are stored in variable spokeLists.  These two structures store
 * references to the same set of Edge objects.
 *
 * edgeList - For a triangle formed by vertices i, j and k, edge[i] is the
 * edge opposite vertex i. Put another way, edge[i] is the edge that does not contain
 * vertex i.  edge[i]->vj and edge[i]->vk are the indices of the other two vertices of
 * the triangle accessed, in clockwise order.
 * edge->tLeft is the index of the triangle on the left side of the edge from
 * vj to vk (the triangle that does not contain vertex i). edge->tRight is the
 * index of the triangle on the right side of the edge from vj to vk (the
 * triangle that contains vertex i).  edge(i)->normal is the unit vector normal
 * to edge from vj to vk, pointing toward vertex i (edge->vk cross edge->vj),
 * edge->normal is NOT normalized to unit length.
 *
 * <p>Note that edgeList and spokeList contain pointers to the same instantiations
 * of Edge objects.  edgeList does not use the "Edge* next" field in the Edge objects
 * but spokeList relies on those fields.  Hence the 'next' pointers in Edge objects
 * should not be manipulated via edgeList.
 */
struct Edge
{
	/**
	 * vertex index j
	 */
	int vj;

	/**
	 * vertex index k
	 */
	int vk;

	int cornerj;

	/**
	 * lndex of triangle to the left of edge from vj to vk
	 */
	int tLeft;

	/**
	 * index of triangle to the right of edge from vj to vk
	 */
	int tRight;

	/**
	 * vertex k cross vertex j, not normalized to unit length
	 */
	double normal[3]; //

	/**
	 * pointer to next edge in circular list of edges emanating from vertex vj.
	 * Used by spokeList but not by edgeList.
	 */
	Edge* next; //
};

/**
 * \brief Manages the geometry and topology of one or more multi-level triangular
 * tessellations of a unit sphere. Has many functions to retrieve information about
 * the grid but knows nothing about Data.
 *
 * Manages the geometry and topology of one or more multi-level triangular
 * tessellations of a unit sphere. It knows:
 * <ul>
 * <li>the positions of all the vertices,
 * <li>the connectivity information that defines how vertices are connected to form triangles,
 * <li>for each triangle it knows the indexes of the 3 neighboring triangles,
 * <li>for each triangle it knows the index of the triangle which is a descendant at the next higher
 * tessellation level, if there is one.
 * <li>information about which triangles reside on which tessellation level
 * </ul>
 * <p>
 * GeoTessGrid is thread-safe in that its internal state is not modified after its data has been
 * loaded into memory. The design intention is that single instances of a GeoTessGrid object and
 * GeoTessData object can be shared among all the threads in a multi-threaded application and each
 * thread will have it's own instance of a GeoTessPosition object that references the common
 * GeoTessGrid + GeoTessData combination.
 *
 * <p>
 * References Ballard, S., J. R. Hipp and C. J. Young, 2009, Efficient and Accurate Calculation of
 * Ray Theory Seismic Travel Time Through Variable Resolution 3D Earth Models, Seismological
 * Research Letters, v.80, n. 6 p. 989-999.
 *
 * @author Sandy Ballard
 */
class GEOTESS_EXP_IMP GeoTessGrid
{
protected:

	/**
	 * An nVertices x 3 array of tessellation vertices. Each vertex is
	 * represented by a 3-component unit vector with it's origin at the center
	 * of the earth. The x-component points toward lat,lon = 0, 0. The
	 * y-component points toward lat,lon = 0, 90. The z-component points toward
	 * north pole.
	 */
	double** vertices;

	/**
	 * Number of vertices.
	 */
	int nVertices;

	/**
	 * An nTriangles x 3 array of integers. Each triangle is represented by the
	 * indexes of the 3 vertices that form the corners of the triangle, listed
	 * in clockwise order when viewed from outside the tessellation.
	 */
	int** triangles;

	/**
	 * Number of triangles.
	 */
	int nTriangles;

	/**
	 * An n x 2 array where n is the number of tessellation levels in all the
	 * tessellations that constitute this model. For each tessellation level,
	 * the first element of the int[2] specifies the index of the first
	 * triangle in the tess level and the second element specifies the index
	 * of the last triangle on the tess level + 1.
	 * <p>
	 * A level is a single-level tessellation of a unit sphere, which is to say
	 * that it is a set of triangles that completely spans the surface of a unit
	 * sphere without gaps or overlaps.
	 */
	int** levels;

	/**
	 * Number of levels.
	 */
	int nLevels;

	/**
	 * tessellations is a n x 2 int array where n is the number of tessellations
	 * in the topology of the model. Each element specifies [0] the index of
	 * the first level and [1] last level + 1 that make up the tessellation.
	 * <p>
	 * A tessellation is a multi-level tessellation of a unit sphere, which is
	 * to say that it is a hierarchical set of single-level tessellations (see
	 * 'levels' above).
	 * <p>
	 * To loop over the triangles of tessellation tessid, level levelid: <br>
	 * for (int i=getFirtTriangle(tessid, levelid); i < getLastTriangle(tessid,
	 * levelid); ++i) <br> { // do something with triangles[i] }
	 */
	int** tessellations;

	/**
	 * Number of tessellations.
	 */
	int nTessellations;

	/**
	 * Identify the neighbors and descendants of each triangle. This method is called during
	 * construction of a GeoTessGrid object (i.e., when it is loaded from a file). Applications
	 * should not call this method.
	 */
	void initialize();

	/**
	 * Standard constructor. Builds this grid from the input file name.
	 */
	GeoTessGrid(const string& gid) :
		vertices(NULL), nVertices(0),
		triangles(NULL), nTriangles(0),
		levels(NULL), nLevels(0),
		tessellations(NULL), nTessellations(0),
		descendants(NULL), gridID(gid),
		gridInputFile("null"), gridOutputFile("null"), gridSoftwareVersion(""),
		gridGenerationDate(""), refCount(0)
	{
	}

private:

	/**
	 * For every triangle, its descendant is the index of a triangle on the next
	 * higher tessellation level. In the normal course of events, a triangle on
	 * tessellation level i will be subdivided into 4 triangles on tessellation level i+1.
	 * A triangle's descendant will be the central one of those 4 triangles.
	 * For triangles on the top level of a given tessellation, their descendants are -1.
	 */
	int* descendants;

	/**
	 * An nTriangles by 3 array of Edge objects. For a triangle formed by vertices
	 * i, j and k, edge[i] is the edge opposite vertex i. Put another way, edge i
	 * is the edge that does not contain vertex i.  For edge(i), edge->vj and
	 * edge->vk are the other two vertices of the triangle accessed clockwise order.
	 * edge->tLeft is the index of the triangle on the left side of the edge from
	 * vj to vk (the triangle that does not contain vertex i). edge->tRight is the
	 * index of the triangle on the right side of the edge from vj to vk (the
	 * triangle that contain vertex i).  edge(i)->normal is the unit vector normal
	 * to edge from vj to vk.  It was computed as edge->vk cross edge->vj, NOT normalized
	 * to unit length.
	 *
	 * <p>Note that edgeList and spokeList contain pointers to the same instantiations
	 * of Edge objects.  edgeList does not use the "Edge* next" field in the Edge objects
	 * but spokeList relies on those fields.  Hence the 'next' pointers in Edge objects
	 * should not be manipulated via edgeList.
	 */
	vector<vector<Edge*> > edgeList;

	/**
	 * An nLevels x nVertices array of Edge objects that define spokes emanating from each
	 * vertex in clockwise order. spokeList[level][vertex] returns a pointer to single Edge
	 * object which is the entry point into a circular list of Edge objects.  Given an Edge
	 * object, subsequent edges are accessed with edge->next.  This is a circular list, so
	 * you can't simply follow next forever (infinite loop).  You have to keep track of the
	 * first edge (head) and loop until edge == head.
	 *
	 * Each Spoke stores the index of the vertex, the index of the neighboring vertex,
	 * the triangle to the right of the spoke and the triangle to the left of the spoke.
	 * Following edge->next iterates over the spokes in clockwise order.
	 */
	mutable vector<vector<Edge*> > spokeList;

	/**
	 * An nTriangles x 3 array that stores the circumCenters of each triangle
	 * in the grid.  Lazy evaluation is used.  circumCenters are only needed by
	 * GeoTessPositionNaturalNeighbor and computeCircumCenters() is called in its
	 * constructor.
	 */
	mutable vector<double*> circumCenters;

	/**
	 * A String ID that uniquely identifies this GeoTessGrid. It must be true
	 * that two GeoTessGrid objects that have different geometry or topology
	 * also have different uniqueID values. An MD5 hash of the primary data
	 * structures (tessellations, levels, triangles and vertices) would be an
	 * excellent choice for the uniqueId, but the uniqueId can be any String
	 * that uniquely identifies the grid.
	 */
	string gridID;

	/**
	 * The name of the file from which the grid was loaded.
	 */
	string gridInputFile;

	/**
	 * The name of the file from which the grid was loaded.
	 */
	string gridOutputFile;

	/**
	 * Name and version number of the software that generated this grid.
	 */
	string gridSoftwareVersion;

	/**
	 * The date when this grid was generated. Not necessarily the same
	 * as the date that the grid file was copied or translated.
	 */
	string gridGenerationDate;

	/**
	 * Reference count.
	 */
	int refCount;

	/**
	 * An nLevels x nVertices x n array that stores the indices of the n
	 * triangles of which each vertex is a member.
	 */
	vector< vector< vector<int> > > vtxTriangles;

	/**
	 * An nLevels x n array that stores the indexes of the n vertices
	 * that are connected together by triangles on the corresponding
	 * level.  Lazy evaluation is used to construct this array of sets.
	 */
	mutable vector< set<int> > connectedVertices;

	/**
	 * Builds this grid from the contents of the input ascii file stream.
	 */
	GeoTessGrid* loadGridAscii(const string& inputFile);

	/**
	 * Read grid from an input stream in ascii format. Stream is not closed by this method.
	 * @param input stream from which to read grid
	 */
	GeoTessGrid* loadGridAscii(IFStreamAscii& input);

	/**
	 * Load the 2D grid from a File.
	 */
	GeoTessGrid* loadGridBinary(const string& inputFile);

	/**
	 * Builds this grid from the contents of the input binary file stream.
	 */
	virtual GeoTessGrid* loadGridBinary(IFStreamBinary& ifs);

	static void loadGridAsciiFront(IFStreamAscii& input, int& gridFileFormat,
			string& gridSWVersion, string& fileCreationDate,
			string& gridid, const string& grdInptFile);

	static void loadGridBinaryFront(IFStreamBinary& ifs, int& gridFileFormat,
			string& gridSWVersion, string& fileCreationDate,
			string& gridid, const string& grdInptFile);

	/**
	 * Retrieve a set containing the indices of all the vertices that are connected
	 * together by triangles on the specified level.  The index of the level is relative
	 * to all the levels in all tessellations.
	 * <p>
	 * Lazy evaluation is used to generate and store these sets.
	 *
	 * For parallel application where thread safety of the GeoTessGrid object
	 * is important, this method will need to be synchronized.
	 *
	 * @param level index of the level is relative to all the levels in all tessellations.
	 * @return a set containing the indices of all the vertices that are connected
	 * together by triangles on the specified level.
	 */
	const set<int>&	getVertexIndices(const int& level);

	void computeSpokeLists(const int& level) const;

public:

	/**
	 * Default constructor.
	 * All grid information initialized to NULL.  Applications need to call
	 *
	 */
	GeoTessGrid() :
		vertices(NULL), nVertices(0),
		triangles(NULL), nTriangles(0),
		levels(NULL), nLevels(0),
		tessellations(NULL), nTessellations(0),
		descendants(NULL), gridID(""),
		gridInputFile("null"), gridOutputFile("null"), gridSoftwareVersion(""),
		gridGenerationDate(""), refCount(0)
	{ }

	/**
	 * Standard constructor. Builds this grid from an ascii file.
	 */
	GeoTessGrid(IFStreamAscii& input) :
		vertices(NULL), triangles(NULL), levels(NULL), tessellations(NULL),
		descendants(NULL), gridID(""),
		gridInputFile("null"), gridOutputFile("null"), gridSoftwareVersion(""),
		gridGenerationDate(""), refCount(0)
	{ loadGridAscii(input); }

	/**
	 * Standard constructor. Builds this grid from a binary file.
	 */
	GeoTessGrid(IFStreamBinary& input) :
		vertices(NULL), triangles(NULL), levels(NULL), tessellations(NULL),
		descendants(NULL), gridID(""),
		gridInputFile("null"), gridOutputFile("null"), gridSoftwareVersion(""),
		gridGenerationDate(""), refCount(0)
	{ loadGridBinary(input); }

	/**
	 * Copy constructor.  Makes a deep copy
	 * @param other the other GeoTessGrid whose values are to be duplicated here.
	 */
	GeoTessGrid(GeoTessGrid &other);

	/**
	 * Equal operator. Makes a deep copy.
	 * @param other the other GeoTessGrid whose values are to be duplicated here.
	 * @return reference to a deep copy of this grid
	 */
	GeoTessGrid& operator= (const GeoTessGrid& other);

	/**
	 * Retrieve the amount of memory required by this GeoTessGrid object in bytes.
	 * @return the amount of memory required by this GeoTessGrid object in bytes.
	 */
	LONG_INT getMemory()
	{
		LONG_INT memory = (LONG_INT)sizeof(GeoTessGrid);

		// double** vertices
		memory += nVertices * (LONG_INT)sizeof(double*) + nVertices * 3 * (LONG_INT)sizeof(double);

		// int** triangles
		memory += nTriangles * (LONG_INT)sizeof(int*) + nTriangles * 3 * (LONG_INT)sizeof(int);

		// int** levels
		memory += nLevels * (LONG_INT)sizeof(int*) + nLevels * 2 * (LONG_INT)sizeof(int);

		// int** tessellations
		memory += nTessellations * (LONG_INT)sizeof(int*) + nTessellations * 2 * (LONG_INT)sizeof(int);

		// int* descendants
		memory += nTriangles * (LONG_INT)sizeof(int);

		// vector<vector<Edge*> > edgeList : nTriangles x 3 array of Edges
		memory += (LONG_INT) (edgeList.capacity() * sizeof(vector<Edge*>)
				+ edgeList.size() * 3 * (sizeof(Edge*) + sizeof(Edge)));

		// vector<vector<Edge*> > spokeList;
		memory += (LONG_INT) (spokeList.capacity() * sizeof(vector<Edge*>)
				+ spokeList.size() * 3 * sizeof(Edge*));

		// vector<double*> circumCenters is nTriangles x 3 array of doubles
		memory += (LONG_INT) (circumCenters.capacity() * sizeof(double*));
		for (int i=0; i<(int)circumCenters.size(); ++i)
			if (circumCenters[i]) memory += 3 * (LONG_INT)sizeof(double);

		// An nLevels x nVertices x n array that stores the indices of the n
		// triangles of which each vertex is a member.
		// vector< vector< vector<int> > > vtxTriangles;
		memory += (LONG_INT) (vtxTriangles.capacity() * sizeof(vector< vector<int> >));
		for (int i=0; i<(int)vtxTriangles.size(); ++i)
		{
			memory += (LONG_INT) (vtxTriangles[i].capacity() * sizeof(vector<int>));
			for (int j=0; j<(int)vtxTriangles[i].size(); ++j)
				memory += (LONG_INT) (vtxTriangles[i][j].capacity() * sizeof(int));
		}

		// An nLevels x n array that stores the indexes of the n vertices
		// that are connected together by triangles on the corresponding
		// level.  Lazy evaluation is used to construct this array of sets.
		// vector< set<int> > connectedVertices;
		memory += (LONG_INT) (connectedVertices.capacity() * sizeof(set<int>));
		for (int i=0; i<(int)connectedVertices.size(); ++i)
			memory += (LONG_INT) (connectedVertices[i].size() * sizeof(int));

		// add memory requirements for all the string variables.
		memory += (LONG_INT) (gridID.length() + gridInputFile.length() + gridOutputFile.length()
				+ gridSoftwareVersion.length() + gridGenerationDate.length());

		return memory;
	}

	/**
	 * Load GeoTessGrid object from a File.
	 * @param inputFile name of file from which to load grid.
	 * @return pointer to a Grid object
	 */
	GeoTessGrid* loadGrid(const string& inputFile);

	/**
	 * Open the specified file using the appropriate format, and read only enough of the file to
	 * retrieve the gridID.
	 * @param fileName name of file
	 * @return the grid id of the grid stored in the specified file.
	 */
	static string getGridID(const string& fileName);

	/**
	 * Test a file to see if it is a GeoTessGrid file.
	 *
	 * @param inputFile
	 * @return true if inputFile is a GeoTessGrid file.
	 */
	static bool isGeoTessGrid(const string& inputFile);

	/**
	 * Return true if the input Grid object (g) gridID equals this Grid objects gridID.
	 * @param g the other grid object to which this grid is to be compared.
	 * @return true if this grid and other grid have same gridIDs.
	 */
	bool					operator == (const GeoTessGrid& g) const
																			{	return (gridID == g.gridID); }

	/**
	 * Return true if the input Grid object (g) gridID is not equal to this Grid objects gridID.
	 * @param g the other grid object to which this grid is to be compared.
	 * @return true if this grid and other grid have different gridIDs.
	 */
	bool					operator != (const GeoTessGrid& g) const
																			{	return !(*this == g); }

	/**
	 * A String ID that uniquely identifies this GeoTessGrid. It must be true
	 * that two GeoTessGrid objects that have different geometry or topology
	 * also have different uniqueID values. An MD5 hash of the primary data
	 * structures (tessellations, levels, triangles and vertices) would be an
	 * excellent choice for the uniqueId, but the uniqueId can be any String
	 * that uniquely identifies the grid.
	 *
	 * @return String gridID
	 */
	const string& getGridID() const
	{
		return gridID;
	}

	/**
	 * Set the name and version number of the software that generated
	 * the contents of this grid.
	 * @param swVersion the name and version number of the software that generated
	 * the contents of this grid.
	 */
	void setGridSoftwareVersion(const string& swVersion) {gridSoftwareVersion = swVersion; };

	/**
	 * Get the name and version number of the software that generated
	 * the contents of this grid.
	 * @return the name and version number of the software that generated
	 * the contents of this grid.
	 */
	const string& getGridSoftwareVersion() const {return gridSoftwareVersion; };

	/**
	 * Set the date when this grid was generated.
	 * This is not necessarily the same as the date when the file was
	 * copied or translated.
	 * @param gridDate
	 */
	void setGridGenerationDate(const string& gridDate) {gridGenerationDate = gridDate; };

	/**
	 * Retrieve the date when the contents of this grid was generated.
	 * This is not necessarily the same as the date when the file was
	 * copied or translated.
	 * @return the date when the contents of this grid was generated.
	 */
	const string& getGridGenerationDate() const {return gridGenerationDate; };

	/**
	 * Set the grid input file name.
	 * @param gridFile name of grid file.
	 */
	void setGridInputFile(const string& gridFile) { gridInputFile = gridFile; }

	/**
	 * Retrieve the name of the file from which the grid was loaded. This will
	 * be the name of a GeoTessModel file if the grid was stored in the same
	 * file as the model.
	 *
	 * @return the name of the file from which the grid was loaded.
	 */
	const string& getGridInputFile() const {return gridInputFile; }

	/**
	 * Retrieve the name of the file to which this grid was most recently
	 * written, or the string "null" if it has not been written.
	 *
	 * @return the name of the file to which this grid was most recently
	 *         written, or the string "null" if it has not been written.
	 */
	const string& getGridOutputFile() const { return gridOutputFile; }

	/**
	 * Retrieve the unit vector that corresponds to the specified vertex.
	 * @param vertex the index of the vertex
	 * @return unit vector of vertex
	 */
	const double* getVertex(int vertex) const
	{
		return vertices[vertex];
	}

	/**
	 * Retrieve the index of the vertex that is closest to the supplied
	 * unit vector.  Only vertices connected at the specified tessellation
	 * index are searched.
	 * @param unit_vector a unit vector
	 * @param tessId tessellation to search for the specified unit vector.
	 * @return index of closest vertex.
	 */
	int findClosestVertex(double* unit_vector, int tessId)
	{
		int* t = triangles[getTriangle(getFirstTriangle(tessId, 0), unit_vector)];

		int index = 0;
		double dot = GeoTessUtils::dot(unit_vector, vertices[t[0]]);

		double doti = GeoTessUtils::dot(unit_vector, vertices[t[1]]);
		if (doti > dot)
		{
			index = 1;
			dot = doti;
		}

		doti = GeoTessUtils::dot(unit_vector, vertices[t[2]]);
		if (doti > dot)
		{
			index = 2;
			dot = doti;
		}

		return t[index];
	}

	/**
	 * Get the index of the vertex that occupies the specified position in the
	 * hierarchy.
	 *
	 * @param triangle the i'th triangle in the grid
	 * @param corner the i'th corner of the specified triangle
	 * @return index of a vertex
	 */
	int getVertexIndex(int triangle, int corner) const
	{ return triangles[triangle][corner]; }

	/**
	 * Get the index of the vertex that occupies the specified position in the
	 * hierarchy.
	 *
	 * @param tessId
	 *            tessellation index
	 * @param level
	 *            index of a level relative to the first level of the specified
	 *            tessellation
	 * @param triangle
	 *            the i'th triangle in the specified tessellation/level
	 * @param corner
	 *            the i'th corner of the specified tessellation/level/triangle
	 * @return index of a vertex
	 */
	int getVertexIndex(int tessId, int level, int triangle, int corner) const
	{
		return triangles[levels[tessellations[tessId][0] + level][0] + triangle][corner];
	}

	/**
	 * Retrieve the index of the vertex that is colocated with the supplied
	 * unit vector.
	 * @param u a unit vector
	 * @return index of colocated vertex, or -1 if there is no such vertex.
	 */
	int getVertexIndex(const double* u)
	{
		for (int i=nTessellations-1; i>=0; --i)
		{
			int vid = getVertexIndex(u, i);
			if (vid >= 0)
				return vid;
		}
		return -1;
	}

	/**
	 * Retrieve the index of the vertex that is colocated with the supplied
	 * unit vector.  Only vertices connected at the specified tessellation
	 * index are searched.
	 * @param u a unit vector
	 * @param tessId tessellation to search for the specified unit vector.
	 * @return index of colocated vertex, or -1 if there is no such vertex.
	 */
	int getVertexIndex(const double* u, int tessId)
	{
		int* t = triangles[getTriangle(getFirstTriangle(tessId, 0), u)];

		if (GeoTessUtils::dot(u, vertices[t[0]]) > cos(1e-7))
			return t[0];
		if (GeoTessUtils::dot(u, vertices[t[1]]) > cos(1e-7))
			return t[1];
		if (GeoTessUtils::dot(u, vertices[t[2]]) > cos(1e-7))
			return t[2];
		return -1;
	}

	/**
	 * Get the unit vector of the vertex that occupies the specified position in
	 * the hierarchy.
	 *
	 * @param tessId tessellation index
	 * @param level index of a level relative to the first level of the specified
	 *            tessellation
	 * @param triangle the i'th triangle in the specified tessellation/level
	 * @param corner the i'th corner of the specified tessellation/level/triangle
	 * @return unit vector of a vertex
	 */
	double* getVertex(int tessId, int level, int triangle, int corner)
	{
		return vertices[triangles[levels[tessellations[tessId][0] + level][0] + triangle][corner]];
	}

	/**
	 * Retrieve a reference to all of the vertices. Vertices consists of an
	 * nVertices x 3 array of doubles. The double[3] array associated with each
	 * vertex is the 3 component unit vector that defines the position of the
	 * vertex.
	 * <p>
	 * Users should not modify the contents of the array.
	 * @return nVertices x 3 array of unit vectors.
	 */
	double const* const* getVertices() const { return vertices; }

	/**
	 * Retrieve a set containing the unit vectors of all the vertices that are
	 * connected together by triangles on the top level of the specified tessellation.
	 *
	 * @param tessellation
	 * @param vectors (output) a set containing the unit vectors of all the vertices that are
	 * connected together by triangles on the top level of the specified tessellation.
	 */
	void getVerticesTopLevel(const int& tessellation, set<const double*>& vectors)
	{
		getVertices(tessellation, getNLevels(tessellation)-1, vectors);
	}

	/**
	 * Return a set containing the unit vectors of all the vertices that are
	 * connected together by triangles on the specified tessellation and level.
	 *
	 * @param tessellation
	 * @param level the level relative to the first level of the specified tessellation
	 * @param vectors (output) a set containing the unit vectors of all the vertices that are
	 * connected together by triangles on the specified tessellation and level.
	 */
	void getVertices(const int& tessellation, const int& level, set<const double*>& vectors);

	/**
	 * Retrieve a set containing the indices of all the vertices that are connected
	 * together by triangles on the specified level.  The index of the level is relative
	 * to the first level of the specified tessellation.
	 * <p>
	 * Lazy evaluation is used to generate and store these sets.
	 *
	 * @param tessId index of the tessellation.
	 * @param level index of the level relative to the first level of the specified tessellation.
	 * @return a set containing the indices of all the vertices that are connected
	 * together by triangles on the specified level.
	 */
	const set<int>&	getVertexIndices(const int& tessId, const int& level)
									{ return getVertexIndices(getLevel(tessId, level)); }

	/**
	 * Retrieve a set containing the indices of all the vertices that are connected
	 * together by triangles on the top level of the specified tessellation.
	 *
	 * <p>
	 * Lazy evaluation is used to generate and store these sets.
	 *
	 * @param tessId index of the tessellation.
	 * @return a set containing the indices of all the vertices that are connected
	 * together by triangles on the top level of the specified tessellation.
	 */
	const set<int>&	getVertexIndicesTopLevel(const int& tessId)
									{ return getVertexIndices(tessellations[tessId][1] - 1); }

	/**
	 * Returns the number of vertices in the vectices array.
	 * @return number of vertices
	 */
	int getNVertices() const { return nVertices; }

	/**
	 * Returns the number of tessellations in the tessellations array.
	 * @return the number of multi-level tesseallations.
	 */
	int getNTessellations() const { return nTessellations; }

	/**
	 * Retrieve number of tessellation levels that define the specified
	 * multi-level tessellation of the model.
	 *
	 * @param tessellation
	 * @return number of levels
	 */
	int getNLevels(int tessellation) const
	{ return tessellations[tessellation][1] - tessellations[tessellation][0]; }

	/**
	 * Retrieve the index of one of the levels on the specified tessellation
	 *
	 * @param tessellation
	 * @param i the index of the desired level relative to
	 *            the first level on tessellation.
	 * @return the index of the level relative to all tessellation levels.
	 */
	int getLevel(int tessellation, int i) const
	{ return tessellations[tessellation][0] + i; }

	/**
	 * Retrieve the index of the last level on the specified tessellation, relative to all
	 * levels in all tessellations.
	 *
	 * <p>Levels for all tessellations are stored internally in a single array of level indices.
	 * In some instances, the index of a level relative to all the levels in all tessellations
	 * is needed. Use this method, getLastLevel(tessId), to retrieve this index.
	 * In other instances, the index of a level relative to the first level of
	 * a specified tessellation is needed.  Use method getTopLevel(tessid) to retrieve that
	 * index.
	 *
	 * @param tessellation
	 * @return the index of the last level on the specified tessellation
	 * relative to all levels of all tessellations.
	 */
	int getLastLevel(int tessellation) const
	{ return tessellations[tessellation][1] - 1; }

	/**
	 * Retrieve the index of the last level on the specified tessellation, relative to first
	 * level of the specified tessellation.
	 *
	 * <p>Levels for all tessellations are stored internally in a single array of level indices.
	 * In some instances, the index of a level relative to the first level of
	 * a specified tessellation is needed.  Use this method, getTopLevel(tessId), to retrieve this index.
	 * In other instances, the index of a level relative to all the levels in all tessellations
	 * is needed.  Use method getLastLevel(tessid) to retrieve that index.
	 *
	 * @param tessellation
	 * @return the index of the last level on the specified tessellation
	 * relative to first level of the tessellation.
	 */
	int getTopLevel(int tessellation) const
	{ return tessellations[tessellation][1] - tessellations[tessellation][0] - 1; }

	/**
	 * Retrieve the number of triangles that define the specified level of the
	 * specified multi-level tessellation of the model.
	 *
	 * @param tessellation
	 * @param level index of a level relative to the first level of the specified
	 *            tessellation
	 * @return number of triangles on specified tessellation and level.
	 */
	int getNTriangles(int tessellation, int level) const
	{
		return levels[tessellations[tessellation][0] + level][1]
		                                                      - levels[tessellations[tessellation][0] + level][0];
	}

	/**
	 * Retrieve the index of the i'th triangle on the specified level of the
	 * specified tessellation of the model.
	 *
	 * @param tessellation
	 * @param level index of a level relative to the first level of the specified
	 *            tessellation
	 * @param i
	 * @return a triangle index
	 *
	 */
	int getTriangle(int tessellation, int level, int i) const
	{ return levels[tessellations[tessellation][0] + level][0] + i; }

	/**
	 * Retrieve the index of the first triangle on the specified level of the specified tessellation
	 * of the model.
	 *
	 * @param tessellation
	 * @param level index of a level relative to the first level of the specified
	 *            tessellation
	 * @return a triangle index
	 */
	int getFirstTriangle(int tessellation, int level) const
	{ return levels[tessellations[tessellation][0] + level][0]; }

	/**
	 * Retrieve the index of the last triangle on the specified level of the specified tessellation
	 * of the model.
	 *
	 * @param tessellation
	 * @param level index of a level relative to the first level of the specified
	 *            tessellation
	 * @return a triangle index
	 */
	int getLastTriangle(int tessellation, int level) const
	{ return levels[tessellations[tessellation][0] + level][1]-1; }

	/**
	 * Retrieve a reference to the nTriangles x 3 array of int that specifies
	 * the indexes of the 3 vertices that define each triangle of the
	 * tessellation.
	 * <p>
	 * Users should not modify the contents of the array.
	 *
	 * @return a reference to the triangles.
	 */
	int const* const* getTriangles() const { return triangles; }

	/**
	 * Retrieve an int[3] array containing the indexes of the vertices that form
	 * the corners of the triangle with index triangleIndex.
	 * <p>
	 * Users should not modify the contents of the array.
	 *
	 * @param triangleIndex
	 *            triangleIndex
	 * @return an int[3] array containing the indexes of the vertices that form
	 *         the corners of the specified triangle.
	 */
	const int* getTriangleVertexIndexes(int triangleIndex) const
	{ return triangles[triangleIndex]; }

	/**
	 * Retrieve the index of the i'th vertex (0..2) that represents one of the
	 * corners of the specified triangle.
	 *
	 * @param triangleIndex
	 *            triangleIndex
	 * @param cornerIndex
	 *            0..2
	 * @return the index of the vertex at the specified corner of the specified
	 *         triangle
	 */
	int getTriangleVertexIndex(int triangleIndex, int cornerIndex) const
	{ return triangles[triangleIndex][cornerIndex]; }

	/**
	 * Retrieve the unit vector of the vertex located at one of the corners of
	 * the specified triangle.
	 *
	 * @param triangleIndex
	 *            triangleIndex
	 * @param cornerIndex
	 *            0..2
	 * @return the unit vector of the vertex at the specified corner of the
	 *         specified triangle
	 */
	const double* getTriangleVertex(int triangleIndex, int cornerIndex) const
	{ return vertices[triangles[triangleIndex][cornerIndex]]; }


	/**
	 * Get the 3 vertices that form the corners of the specified triangle, in clockwise order.
	 *
	 * @param triangle index of the desired triangle
	 * @param triVrt the 3 vertices (unit vectors).
	 * @return 3 x 3 array of doubles with the unit vectors that define the 3 corners of the
	 *         specified triangle.
	 */
	void getTriangleVertices(int triangle, double** triVrt)
	{
		int* corners = triangles[triangle];
		triVrt[0][0] = vertices[corners[0]][0];
		triVrt[0][1] = vertices[corners[0]][1];
		triVrt[0][2] = vertices[corners[0]][2];
		triVrt[1][0] = vertices[corners[1]][0];
		triVrt[1][1] = vertices[corners[1]][1];
		triVrt[1][2] = vertices[corners[1]][2];
		triVrt[2][0] = vertices[corners[2]][0];
		triVrt[2][1] = vertices[corners[2]][1];
		triVrt[2][2] = vertices[corners[2]][2];
	}

	/**
	 * Compute the circumcenters of all triangles if they have not already
	 * been computed.
	 */
	const void computeCircumCenters()
	{
		if (circumCenters.empty()) circumCenters.resize(nTriangles, NULL);

		for (int triangle=0; triangle<nTriangles; ++triangle)
			if (circumCenters[triangle] == NULL)
			{
				int* corners = triangles[triangle];
				circumCenters[triangle] = GeoTessUtils::circumCenterPlus(vertices[corners[0]],
						vertices[corners[1]], vertices[corners[2]]);
			}
	}

	/**
	 * Compute the circumcenters of all triangles on the specified level if they have not already
	 * been computed.
	 */
	const void computeCircumCenters(const int& level)
	{
		if (circumCenters.empty()) circumCenters.resize(nTriangles);

		if (circumCenters[levels[level][0]] == NULL)
		{
			for (int triangle=levels[level][0]; triangle<levels[level][1]; ++triangle)
			{
				int* corners = triangles[triangle];
				circumCenters[triangle] =  GeoTessUtils::circumCenterPlus(vertices[corners[0]],
						vertices[corners[1]], vertices[corners[2]]);
			}
		}
	}

	/**
	 * Retrieve the circumCenter of the specified triangle. The circumCenter of
	 * a triangle is the center of the circle that has all three corners of the
	 * triangle on its circumference.
	 *
	 * <p>The fourth element of returned circumcenter is the dot product of the new
	 * circumcenter with one of the vertices.  In other words, cc[3] = cos(ccRadius).
	 *
	 * <p>This method will fail if method computeCircumCenters() has not already
	 * been called.  computeCircumCenters() is called from the GeoTessPositionNaturalNeighbor
	 * constructor.
	 *
	 * @param triangle Triangle for which the circumcenter will be returned.
	 * @return unit vector that defines circumCenter. Fourth element is cos(ccRadius)
	 */
	const double* getCircumCenter(const int& triangle) const { return circumCenters[triangle]; }

	/**
	 * Copy the circumCenter of the specified triangle. The circumCenter of
	 * a triangle is the center of the circle that has all three corners of the
	 * triangle on its circumference.
	 *
	 * <p>The fourth element of returned circumcenter is the dot product of the new
	 * circumcenter with one of the vertices.  In other words, cc[3] = cos(ccRadius).
	 *
	 * <p>This method will fail if method computeCircumCenters() has not already
	 * been called.  computeCircumCenters() is called from the GeoTessPositionNaturalNeighbor
	 * constructor.
	 *
	 * @param triangle Triangle for which the circumcenter will be returned.
	 * @param cc the 4-element array into which the circumCenter will be copied.
	 * The first three elements will get the unit vector that defines circumCenter.
	 * Fourth element will get cos(ccRadius)
	 */
	void getCircumCenter(const int& triangle, double* cc) const
	{ cc[0]=circumCenters[triangle][0]; cc[1]=circumCenters[triangle][1]; cc[2]=circumCenters[triangle][2]; }

	/**
	 * Retrieve the index of one of the triangles that is a neighbor of the specified triangle. A
	 * triangle has at least 3 neighbors and usually has 4. For triangle T, neighbors 0, 1, and 2
	 * reside on the same tessellation level as T and refer to the triangles that share an edge with
	 * T. If T has a fourth neighbor it is a descendent of T and resides on the next higher
	 * tessellation level relative to T. In other words, neighbor(3) is one of the triangles into
	 * which T was subdivided when the tessellation was constructed. If T does not have a
	 * descendant, then getNeighbor(3) will return -1. getNeighbor(i) will always return a valid
	 * triangle index for i=[0,1,2] but may or may not return a valid triangle index for i=3.
	 *
	 * @param triangleIndex index of the triangle whose neighbor is desired.
	 * @param neighborIndex (0..3)
	 * @return int index of the triangle that is a neighbor of triangle.
	 */
	int getNeighbor(const int& triangleIndex, const int& neighborIndex) const
	{ return neighborIndex == 3 ? descendants[triangleIndex] : edgeList[triangleIndex][neighborIndex]->tLeft; }

	/**
	 * Retrieve the index of the triangle that is the i'th neighbor of the
	 * specified triangle. A triangle has at least 3 neighbors and usually has
	 * 4. For triangle T, neighbors 0, 1, and 2 reside on the same tessellation
	 * level as T and refer to the triangles that share an edge with T. If T has
	 * a fourth neighbor it is a descendent of T and resides on the next higher
	 * tessellation level relative to T. In other words, neighbor(3) is one of
	 * the triangles into which T was subdivided when the tessellation was
	 * constructed. If T does not have a descendant, then getNeighbor(3) will
	 * return -1. getNeighbor(i) will always return a valid triangle index for
	 * i=[0,1,2] but may or may not return a valid triangle index for i=3.
	 *
	 * @param tessellation
	 *            tessellation index
	 * @param level
	 *            index of a level relative to the first level of the specified
	 *            tessellation
	 * @param triangle
	 *            the i'th triangle in the specified tessellation/level
	 * @param side
	 *            the index of the triangle side (0..2)
	 * @return the index of the triangle that is the i'th neighbor of the
	 *         specified triangle.
	 */
	int getNeighbor(const int& tessellation, const int& level, const int& triangle, const int& side)
	{ return getNeighbor(getTriangle(tessellation, level, triangle), side); }

	/**
	 * Retrieve the indexes of the triangles that are neighbors of the specified triangle. A
	 * triangle has at least 3 neighbors and usually has 4. For triangle T, neighbors 0, 1, and 2
	 * reside on the same tessellation level as T and refer to the triangles that share an edge with
	 * T. If T has a fourth neighbor it is a descendent of T and resides on the next higher
	 * tessellation level relative to T. In other words, neighbor(3) is one of the triangles into
	 * which T was subdivided when the tessellation was constructed. If T does not have a
	 * descendant, then getNeighbor(3) will return -1. getNeighbor(i) will always return a valid
	 * triangle index for i=[0,1,2] but may or may not return a valid triangle index for i=3.
	 *
	 * @param triangleIndex index of the triangle whose neighbors are desired.
	 * @param neighbors an array that will be cleared and populated with the three triangle neighbors
	 * and the descendant of the supplied triangle.
	 */
	void getNeighbors(int triangleIndex, vector<int>& neighbors)
	{
		neighbors.clear();
		vector<Edge*> edges = edgeList[triangleIndex];
		neighbors.push_back(edges[0]->tLeft);
		neighbors.push_back(edges[1]->tLeft);
		neighbors.push_back(edges[2]->tLeft);
		neighbors.push_back(descendants[triangleIndex]);
	}

	/**
	 * Retrieve the indexes of the triangles that are neighbors of the specified
	 * triangle. A triangle has at least 3 neighbors and usually has 4. For
	 * triangle T, neighbors 0, 1, and 2 reside on the same tessellation level
	 * as T and refer to the triangles that share an edge with T. If T has a
	 * fourth neighbor it is a descendent of T and resides on the next higher
	 * tessellation level relative to T. In other words, neighbor(3) is one of
	 * the triangles into which T was subdivided when the tessellation was
	 * constructed. If T does not have a descendant, then getNeighbor(3) will
	 * return -1. getNeighbor(i) will always return a valid triangle index for
	 * i=[0,1,2] but may or may not return a valid triangle index for i=3.
	 *
	 * @param tessellation
	 *            tessellation index
	 * @param level
	 *            index of a level relative to the first level of the specified
	 *            tessellation
	 * @param triangle
	 *            the i'th triangle in the specified tessellation/level
	 * @param neighbors an array that will be cleared and populated with the three triangle neighbors
	 * and the descendant of the supplied triangle.
	 */
	void getNeighbors(const int& tessellation, const int& level, const int& triangle, vector<int>& neighbors)
	{ getNeighbors(getTriangle(tessellation, level, triangle), neighbors); }

	/**
	 * If triangle with index tid has a neighbor with index nid, then return the
	 * index of neighbor in triangle's neighbor array.
	 * <p>
	 * In other words, if triangle nid is a neighbor of triangle tid, i.e.,
	 * neighbors[tid][i] == nid, then this method returns i.
	 *
	 * @param tid
	 *            the index of a triangle
	 * @param nid
	 *            the index of another triangle
	 * @return the index of neighbor in triangle's array of neighbors.
	 */
	int getNeighborIndex(const int& tid, const int& nid)
	{
		if (edgeList[tid][0]->tLeft == nid)
			return 0;
		if (edgeList[tid][1]->tLeft == nid)
			return 1;
		if (edgeList[tid][2]->tLeft == nid)
			return 2;
		return -1;
	}

	int const* getDescendants() const { return descendants; }

	int getDescendant(const int& triangle) const { return descendants[triangle]; }

	int getDescendant(const int& tessId, const int& level, const int& triangle) const
	{ return descendants[getTriangle(tessId, level, triangle)]; }

	/**
	 * @return summary information about this GeoTessGrid object.
	 */
	string toString();

	/**
	 * Perform walking triangle search to find the index of the triangle that contains position
	 * defined by vector and which has no descendant.
	 * @param triangleIndex the triangle from which to start the search
	 * @param vector the unit vector of the point that is to be searched for.
	 * @return the index of the triangle that contains vector
	 */
	int getTriangle(int triangleIndex, const double* vector);

	/**
	 * Retrieve a list of the triangles a particular vertex is a member of, considering only
	 * triangles in the specified tessellation/level.
	 *
	 * @param level index of a level relative to all levels in the grid.
	 * @return vector of vertex indexes
	 */
	const vector<vector<int> >& getVertexTriangles(const int& level) const { return vtxTriangles[level]; }

	/**
	 * Retrieve a list of the triangles a particular vertex is a member of, considering only
	 * triangles in the specified tessellation/level.
	 * <p>
	 * Lazy evaluation is used here. The list of indexes is initially empty and is computed and
	 * stored on demand. Once computed the indexes remain in memory for the next time they might be
	 * called.
	 * @param tessId the tessellation index
	 * @param level index of a level relative to the first level of the specified tessellation
	 * @param vertex the index of the vertex
	 * @return vector of vertex indexes
	 */
	const vector<int>& getVertexTriangles(const int& tessId, const int& level, const int& vertex) const
							{ return vtxTriangles[tessellations[tessId][0] + level][vertex]; }

	/**
	 * Retrieve a list of the triangles a particular vertex is a member of,
	 * considering only triangles in the top level of the specified tessellation.
	 * <p>
	 * Lazy evaluation is used here. The list of indexes is initially empty and
	 * is computed and stored on demand. Once computed the indexes remain in
	 * memory for the next time they might be called.
	 *
	 * @param tessId
	 *            tessellation index
	 * @param vertex
	 * @return list of triangle indeces
	 */
	const vector<int>& getVertexTriangles(int tessId, int vertex) const
							{ return getVertexTriangles(tessId, getNLevels(tessId)-1, vertex); }

	/**
	 * GeoTessGrid maintains a list of the triangles that each vertex is a member of.
	 * there is a separate list for every tessellation level in the grid.  Lazy
	 * evaluation is used to build this list.
	 * <p>This method clears the list for all tessellation levels.
	 */
	//void clearVertexTriangles();

	/**
	 * Retrieve a list of the indexes of all the vertexes that are connected to the specified vertex
	 * by a single edge, considering only triangles in the specified tessellation and level. The
	 * vertices will be arranged in clockwise order when viewed from outside the unit sphere.
	 * @param tessId the tessellation index
	 * @param level index of a level relative to the first level of the specified tessellation
	 * @param vertex index of central vertex
	 * @param v vector of vertex indexes
	 */
	void getVertexNeighborsOrdered(const int& tessId, const int& level, const int& vertex, vector<int>& v);

	/**
	 * Retrieve a list of the indexes of all the vertexes that are connected to the specified vertex
	 * by a single edge, considering only triangles in the specified tessellation and level.
	 * <p>
	 * Equivalent to calling this function with extra parameter "order" = 1.
	 *
	 * @param tessId tessellation index
	 * @param level index of a level relative to the first level of the specified tessellation
	 * @param vertex index of a vertex
	 * @param nbrs indexes of vertices
	 */
	void getVertexNeighbors(const int& tessId, const int& level, const int& vertex, set<int>& nbrs)
	{ getVertexNeighbors(tessId, level, vertex, 1, nbrs); }

	/**
	 * Retrieve a list of the indexes of all the vertexes that are within a neighborhood of the
	 * specified vertex. The neighborhood is defined by the argument "order". If order is 1, then
	 * all the vertices that are connected by a single edge to vertex are included. If order is 2,
	 * then take the order 1 neighborhood and add all the vertices that are connected to any vertex
	 * in the order-1 neighborhood by a single edge. Keep doing that to as high order as desired.
	 * Only triangles in the specified tessellation and level are considered.
	 *
	 * <p>Supplied set nbrs is cleared at the beginning of this method and it will not contain
	 * starting vertex upon return.
	 *
	 * @param tessId tessellation index
	 * @param level index of a level relative to the first level of the specified tessellation
	 * @param vertex
	 * @param order
	 * @param nbrs list of vertex indices
	 */
	void getVertexNeighbors(const int& tessId, const int& level, const int& vertex,
			const int& order, set<int>& nbrs);

	// All methods below this point are public but are not documented in the doxygen documentation.
	// These are methods that typical applications will never need to call.  They have to be
	// public because other classes in the GeoTess namespace need to access them.
	//
	/// @cond PROTECTED

	/**
	 * Deprecated because GeoTessOptimizationType is no longer used.
	 * GeoTess is always optimized for speed.
	 * @param opttype
	 */
	GeoTessGrid(const GeoTessOptimizationType* opttype) :
		vertices(NULL), nVertices(0),
		triangles(NULL), nTriangles(0),
		levels(NULL), nLevels(0),
		tessellations(NULL), nTessellations(0),
		descendants(NULL), gridID(""),
		gridInputFile("null"), gridOutputFile("null"), gridSoftwareVersion(""),
		gridGenerationDate(""), refCount(0)
	{ }

	/**
	 * Deprecated because GeoTessOptimizationType is no longer used.
	 * GeoTess is always optimized for speed.
	 */
	GeoTessGrid(IFStreamAscii& input, const GeoTessOptimizationType* opttype) :
		vertices(NULL), triangles(NULL), levels(NULL), tessellations(NULL),
		descendants(NULL), gridID(""),
		gridInputFile("null"), gridOutputFile("null"), gridSoftwareVersion(""),
		gridGenerationDate(""), refCount(0)
	{ loadGridAscii(input); }

	/**
	 * Deprecated because GeoTessOptimizationType is no longer used.
	 * GeoTess is always optimized for speed.
	 */
	GeoTessGrid(IFStreamBinary& input, const GeoTessOptimizationType* opttype) :
		vertices(NULL), triangles(NULL), levels(NULL), tessellations(NULL),
		descendants(NULL), gridID(""),
		gridInputFile("null"), gridOutputFile("null"), gridSoftwareVersion(""),
		gridGenerationDate(""), refCount(0)
	{ loadGridBinary(input); }

	/**
	 * Deprecated because GeoTessOptimizationType is no longer used.
	 * GeoTess is always optimized for speed.
	 */
	GeoTessGrid(double** _vertices, int& _nVertices, 
			int** _triangles, int& _nTriangles,
			int** _levels, int& _nLevels,
			int** _tessellations, int& _nTessellations,
			string& _gridID,
			string& _gridInputFile,
			string& _gridOutputFile,
			string& _gridSoftwareVersion,
			string& _gridGenerationDate) :
				vertices(_vertices), nVertices(_nVertices),
				triangles(_triangles), nTriangles(_nTriangles),
				levels(_levels), nLevels(_nLevels),
				tessellations(_tessellations), nTessellations(_nTessellations),
				descendants(NULL),
				gridID(_gridID),
				gridInputFile(_gridInputFile),
				gridOutputFile(_gridOutputFile),
				gridSoftwareVersion(_gridSoftwareVersion),
				gridGenerationDate(_gridGenerationDate),
				refCount(0)
	{ initialize(); }

	/**
	 * Destructor.
	 */
	virtual ~GeoTessGrid();

	/**
	 * Retrieve reference count;
	 */
	int getReferenceCount() { return refCount; }

	/**
	 * Add reference count;
	 */
	void addReference() { ++refCount; }

	/**
	 * Remove reference count;
	 */
	void removeReference()
	{
		if (isNotReferenced())
		{
			ostringstream os;
			os << endl << "ERROR in GeoTessGrid::removeReference" << endl
					<< "Reference count (" << refCount << ") is already zero." << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 2001);
		}

		--refCount;
	}

	/**
	 * Returns true if reference count is zero.
	 */
	bool isNotReferenced() { return (refCount == 0); }

	/**
	 * Write the grid out to a file.  If the extension is 'ascii'
	 * the grid is written to an ascii file. Otherwise it
	 * is written to a binary file.
	 */
	void writeGrid(const string& fileName);

	/**
	 * Write the grid out to an ascii file.
	 */
	void writeGridAscii(const string& fileName);

	/**
	 * Write the grid out to an ascii file.
	 */
	void writeGridAscii(IFStreamAscii& output);

	/**
	 * Write the grid out to an binary file.
	 */
	void writeGridBinary(const string& fileName);

	/**
	 * Write the 2D grid to a binary file.
	 */
	void writeGridBinary(IFStreamBinary& output);

	/**
	 * Returns true if the input format version is supported.
	 * @param a format version number.
	 * @return true if the specified format version is supported
	 */
	bool isSupportedFormatVersion(int frmtVrsn)
	{ return (frmtVrsn == 2); }

	// deprecated. GeoTessOptimizationType is no longer used.  Always optimized for speed.
	const GeoTessOptimizationType& getOptimizationType() const { return GeoTessOptimizationType::SPEED; }

	/// @endcond

	/**
	 * Tests the integrity of the grid. Visits every triangle T, and (1) checks
	 * to ensure that every neighbor of T includes T in its list of neighbors,
	 * and (2) checks that every neighbor of T shares exactly two nodes with T.
	 *
	 * @throws GeoTessException
	 *             if anything is amiss.
	 */
	void testGrid();

	/**
	 * Returns the number of tessellation levels defined for this grid.
	 *
	 * @return The number of tessellation levels defined for this grid.
	 */
	int getNLevels() const { return nLevels; }

	/**
	 * Retrieve a reference to all of the tessellation levels. Levels consists of
	 * an nLevels x 2 array of ints. The int[2] array associated with each
	 * level is the first ([0]) and last ([1]) + 1 index of the triangles on the
	 * level.
	 * <p>
	 * Users should not modify the contents of the array.
	 *
	 * @return nLevels x 2 array of unit triangle start and end indices for
	 *         each tessellation.
	 */
	int const* const* getLevels() const { return levels; }

	/**
	 * Retrieve a reference to all of the tessellations of this grid.
	 * Tessellations is a n x 2 int array where n is the number of tessellations
	 * in the topology of the model. Each element specifies [0] the index of
	 * the first level and [1] last level + 1 that make up the tessellation.
	 * <p>
	 * A tessellation is a multi-level tessellation of a unit sphere, which is
	 * to say that it is a hierarchical set of single-level tessellations (see
	 * 'levels' above).
	 * <p>
	 * Users should not modify the contents of the array.
	 * @return nLevels x 2 array of unit triangle start and end indices for
	 *         each tessellation.
	 */
	int const* const* getTessellations() const { return tessellations; }

	/**
	 * Retrieve the total number of triangles including those on all levels of
	 * all tessellations.
	 *
	 * @return the total number of triangles including those on all levels of
	 *         all tessellations.
	 */
	int getNTriangles() const { return nTriangles; }

	vector<Edge*>& getSpokeList(const int& level) const
	{ computeSpokeLists(level); return spokeList[level]; }

	const vector<vector<Edge*> >& getEdgeList() const { return edgeList; }

	const vector<Edge*>& getEdgeList(const int& triangle) const { return edgeList[triangle]; }

	/**
	 * Compute the unit vector that resides at the center of the specified triangle.
	 * @param triangle the index of the triangle
	 * @param center unit vector that is populated with the location of the center of the
	 * specified triangle;
	 */
	void getCenter(const int& triangle, double* center)
	{
		vector<double*> corners;
		corners.push_back(vertices[triangles[triangle][0]]);
		corners.push_back(vertices[triangles[triangle][1]]);
		corners.push_back(vertices[triangles[triangle][2]]);
		GeoTessUtils::center(corners, center);
	}

	/**
	 * Convert tessellation to a Delaunay tessellation.
	 *
	 * @return number of changes.
	 */
	int delaunay();

};
// end class GeoTessGrid

}// end namespace geotess

#endif  // GEOTESSGRID_OBJECT_H
