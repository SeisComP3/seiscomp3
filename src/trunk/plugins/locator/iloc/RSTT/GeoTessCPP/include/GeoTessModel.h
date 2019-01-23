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

#ifndef GEOTESSMODEL_OBJECT_H
#define GEOTESSMODEL_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"
#include "GeoTessUtils.h"
#include "GeoTessInterpolatorType.h"
#include "GeoTessException.h"
#include "GeoTessPointMap.h"
#include "GeoTessProfileSurface.h"
#include "GeoTessProfileEmpty.h"
#include "GeoTessProfileSurfaceEmpty.h"
#include "GeoTessProfileType.h"
#include "EarthShape.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess
{

// **** _FORWARD REFERENCES_ ***************************************************

class GeoTessPosition;
class GeoTessGrid;
class GeoTessMetaData;
class GeoTessOptimizationType;
class GeoTessProfile;
class IFStreamAscii;
class IFStreamBinary;

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Top level class that manages the <i>GeoTessMetaData</i>, <i>GeoTessGrid</i> and <i>GeoTessData</i> that
 * comprise a 3D Earth model.
 *
 * <b>GeoTessModel</b> manages the <i>grid</i> and <i>data</i> that comprise a 3D Earth model. The
 * Earth is assumed to be composed of a number of <i>layers</i> each of which spans the entire
 * geographic extent of the Earth. It is assumed that layer boundaries do not fold back on
 * themselves, i.e., along any radial profile through the model, each layer boundary is intersected
 * exactly one time. Layers may have zero thickness over some or all of their geographic extent.
 * Earth properties stored in the model are assumed to be continuous within a layer, both
 * geographically and radially, but may be discontinuous across layer boundaries.
 *
 * <p>
 * A <b>GeoTessModel</b> is comprised of 3 major components:
 * <ul>
 * <li>The model <i>grid</i> (<i>geometry</i> and <i>topology</i>) is managed by a
 * <b>GeoTessGrid</b> object. The grid is made up of one or more 2D triangular tessellations of a
 * unit sphere.
 *
 * <li>The <i>data</i> are managed by a 2D array of <b>Profile</b> objects. A <b>Profile</b> is
 * essentially a list of radii and <b>Data</b> objects distributed along a radial profile that spans
 * a single layer at a single vertex of the 2D grid. The 2D Profile array has dimensions nVertices
 * by nLayers.
 *
 * <li>Important metadata about the model, such as the names of the major layers, the names of the
 * data attributes stored in the model, etc., are managed by a <b>GeoTessMetaData</b> object.
 * </ul>
 *
 * <p>
 * The term 'vertex' refers to a position in the 2D tessellation. They are 2D positions represented
 * by unit vectors on a unit sphere. The term 'node' refers to a 1D position on a radial profile
 * associated with a vertex and a layer in the model. Node indexes are unique only within a given
 * profile (all profiles have a node with index 0 for example). The term 'point' refers to all the
 * nodes in all the profiles of the model. There is only one 'point' in the model with index 0.
 * PointMap is introduced to manage all these different indexes.
 *
 * @author Sandy Ballard
 *
 */
class GEOTESS_EXP_IMP GeoTessModel
{
	/// @cond PROTECTED  Turn off doxygen documentation until 'endcond' is found

protected:

	/**
	 * Static map used to hold all loaded grids if Grid reuse is desired. Setting the reuse
	 * flag in the MetaData object will turn on grid reuse for the model that defines the
	 * meta data. When new models are created, and if their meta data reuse flag is true,
	 * then the input gridID is used to see if this map contains the desired grid.
	 * If so then it is assigned as the new models grid and its reference count is
	 * incremented. When models are deleted and a contained grids reference count is set
	 * to zero (to be deleted) it is removed from this map before deletion. Using this
	 * map reduces read time if more than one Model is loaded that use the same Grid
	 * definition.
	 */
	static	map<string, GeoTessGrid*> 	reuseGridMap;

	/**
	 * The GeoTessGrid object that supports the 2D components of the model grid.
	 */
	GeoTessGrid*												grid;

	/**
	 * The data stored in the model. An nVertices x nLayers array of Profile objects. Each Profile
	 * consists of an array of radii and the associated Data.
	 */
	GeoTessProfile***													profiles;

	/**
	 * metaData stores basic information about a GeoTessModel including:
	 * <ul>
	 * <li>textual description of the model
	 * <li>the names of all the layers in the model, e.g., ["core", "mantle", "crust"]. Layer names
	 * are specified in order of increasing radius.
	 * <li>dataType; all the data values stored in the model are of this type. Must be one of
	 * DOUBLE, FLOAT, LONG, INT, SHORTINT, BYTE.
	 * <li>number of data attributes
	 * <li>the names of all the data attributes ["P Velocity", "S Velocity", "Density", etc]
	 * <li>the units of the data attributes ["km/sec", "km/sec", "g/cc", etc]
	 * <li>layerTessIds: an integer map from layer index to tessellation index.
	 * </ul>
	 * Each GeoTessModel has a single instance of MetaData that it passes around to wherever the
	 * information is needed.
	 */
	GeoTessMetaData*										metaData;

	/**
	 * A nPoints by 3 array of indexes. For each point in the 3D grid, pointMap stores 3 indexes:
	 * the index of the 2D vertex, the layer index, and the node index within the layer. There is an
	 * entry for each radius, not data object.
	 */
	GeoTessPointMap*														pointMap;

	/**
	 * Deletes the profiles array if it has been allocated
	 */
	void deleteProfiles();

	/**
	 * Load a model (3D grid and data) from an ascii File.
	 * <p>
	 * The format of the file is: <br>
	 * int fileFormatVersion (currently only recognizes 1). <br>
	 * String gridFile: either *, or relative path to gridFile. <br>
	 * int nVertices, nLayers, nAttributes, dataType(DOUBLE or FLOAT). <br>
	 * int[] tessellations = new int[nLayers]; <br>
	 * Profile[nVertices][nLayers]: data
	 */
	void loadModelAscii(const string& inputFile, const string& relGridFilePath);

	/**
	 * Load a model (3D grid and data) from an ascii File.
	 * <p>
	 * The format of the file is: <br>
	 * int fileFormatVersion (currently only recognizes 1). <br>
	 * String gridFile: either *, or relative path to gridFile. <br>
	 * int nVertices, nLayers, nAttributes, dataType(DOUBLE or FLOAT). <br>
	 * int[] tessellations = new int[nLayers]; <br>
	 * Profile[nVertices][nLayers]: data
	 *
	 * @param input ascii stream that provides input
	 * @param inputDirectory the directory where the model file resides
	 * @param relGridFilePath the relative path from the directory where
	 * the model file resides to the directory where the grid file resides.
	 * @throws GeoTessException
	 */
	virtual void loadModelAscii(IFStreamAscii& input, const string& inputDirectory,
			const string& relGridFilePath);

	/**
	 * Load a model (3D grid and data) from a binary File.
	 * <p>
	 * The format of the file is: <br>
	 * int fileFormatVersion (currently only recognizes 1). <br>
	 * String gridFile: either *, or relative path to gridFile. <br>
	 * int nVertices, nLayers, nAttributes, dataType(DOUBLE or FLOAT). <br>
	 * int[] tessellations = new int[nLayers]; <br>
	 * Profile[nVertices][nLayers]: data
	 * @param inputFile the full path name of the file that contains the binary model.
	 * @param relGridFilePath the relative path from the directory that contains the
	 * model to the directory that contains the grid file.
	 */
	void loadModelBinary(const string& inputFile, const string& relGridFilePath);

	/**
	 * Load a model (3D grid and data) from a binary File.
	 * <p>
	 * The format of the file is: <br>
	 * int fileFormatVersion (currently only recognizes 1). <br>
	 * String gridFile: either *, or relative path to gridFile. <br>
	 * int nVertices, nLayers, nAttributes, dataType(DOUBLE or FLOAT). <br>
	 * int[] tessellations = new int[nLayers]; <br>
	 * Profile[nVertices][nLayers]: data
	 * @param input binary stream that provides input
	 * @param inputDirectory the directory where the model file resides
	 * @param relGridFilePath the relative path from the directory where
	 * the model file resides to the directory where the grid file resides.
	 * @throws GeoTessException
	 */
	virtual void loadModelBinary(IFStreamBinary& input, const string& inputDirectory,
			const string& relGridFilePath);

	/**
	 * Write the model currently in memory to an ascii file. A model can be stored with the data and
	 * grid in the same or separate files. This method will write the data from the 3D model to the
	 * specified outputfile. If the supplied gridFileName is the single character "*", then the grid
	 * information is written to the same file as the data. If the gridFileName is anything else, it
	 * is assumed to be the relative path from the data file to an existing file where the grid
	 * information is stored. In the latter case, the grid information is not actually written to
	 * the specified file; all that happens is that the relative path is stored in the data file.
	 */
	void writeModelAscii(const string& outputFile, const string& gridFileName);

	/**
	 * Write the model currently in memory to an ascii file. A model can be stored with the data and
	 * grid in the same or separate files. This method will write the data from the 3D model to the
	 * specified outputfile. If the supplied gridFileName is the single character "*", then the grid
	 * information is written to the same file as the data. If the gridFileName is anything else, it
	 * is assumed to be the relative path from the data file to an existing file where the grid
	 * information is stored. In the latter case, the grid information is not actually written to
	 * the specified file; all that happens is that the relative path is stored in the data file.
	 */
	virtual void writeModelAscii(IFStreamAscii& output, const string& gridFileName);

	/**
	 * Write the model currently in memory to a binary file. A model can be stored with the data and
	 * grid in the same or separate files. This method will write the data from the 3D model to the
	 * specified outputfile. If the supplied gridFileName is the single character "*", then the grid
	 * information is written to the same file as the data. If the gridFileName is anything else, it
	 * is assumed to be the relative path from the data file to an existing file where the grid
	 * information is stored. In the latter case, the grid information is not actually written to
	 * the specified file; all that happens is that the relative path is stored in the data file.
	 */
	void writeModelBinary(const string& outputFile, const string& gridFileName);

	/**
	 * Write the model currently in memory to a binary file. A model can be stored with the data and
	 * grid in the same or separate files. This method will write the data from the 3D model to the
	 * specified outputfile. If the supplied gridFileName is the single character "*", then the grid
	 * information is written to the same file as the data. If the gridFileName is anything else, it
	 * is assumed to be the relative path from the data file to an existing file where the grid
	 * information is stored. In the latter case, the grid information is not actually written to
	 * the specified file; all that happens is that the relative path is stored in the data file.
	 */
	virtual void writeModelBinary(IFStreamBinary& output, const string& gridFileName);

	/**
	 * A templated function on IFStreamAscii, IFStreamBinary, and NcFile. This
	 * function reads any extra data, defines the grid (from reuse, in the current model
	 * file, or from a stand-alone grid file), and builds the point map.
	 *
	 * @param input The input template object from which data is read.
	 * @param inputDirectory The input directory name of the current model file.
	 * @param relGridFilePath The relative grid file path (if not empty).
	 * @param gridFileName The input grid file name.
	 * @param gridID The input grid ID string.
	 * @param funcName The function name (for error output).
	 */
	template<class T>
	void	loadGrid(T& input, const string& inputDirectory,
			const string& relGridFilePath, const string& gridFileName,
			const string& gridID, const string& funcName)
	{
		// process grid

		grid = NULL;
		map<string, GeoTessGrid*>::iterator it = reuseGridMap.find(gridID);
		if (it != reuseGridMap.end())
			grid = it->second;

		if (gridFileName == "*")
		{
			// load the grid from this input file.  The grid has to be read from the
			// file, even a reference was retrieved from the reuseGridMap, so that the
			// file is positioned where classes that extend GeoTessModel can read
			// additional data.
			GeoTessGrid* g = new GeoTessGrid(input);
			if (!grid)
			{
				grid = g;

				grid->setGridInputFile(metaData->getInputModelFile());

				if (metaData->isGridReuseOn())
					reuseGridMap[gridID] = grid;
			}
			else
				delete g;
		}
		else if (!grid)
		{
			// build the name of the grid file using the input directory and
			// the relative path to the grid file. Assume that both
			// inputDirectory and relGridFilePath may be null or empty.
			string gridFil = gridFileName;
			if (relGridFilePath != "")
				gridFil = CPPUtils::insertPathSeparator(relGridFilePath, gridFileName);
			if (inputDirectory != "")
				gridFil = CPPUtils::insertPathSeparator(inputDirectory, gridFil);

			grid = new GeoTessGrid();
			grid->loadGrid(gridFil);
			if (metaData->isGridReuseOn())
				reuseGridMap[gridID] = grid;

			// throw an error if the grid ID's are not equal

			if (grid->getGridID() != gridID)
			{
				ostringstream os;
				os << endl << "ERROR in GeoTessModel::" + funcName << endl
						<< "gridIDs in model file and existingGrid are not equal: "
						<< endl << "  Model File gridID = " << gridID << endl
						<< "  Grid File gridID  = " << grid->getGridID() << endl;
				throw GeoTessException(os, __FILE__, __LINE__, 1002);
			}
		}

		// add a reference to the grid and build the point map

		grid->addReference();
	}

private:

	void constructor(const string& gridFileName, GeoTessGrid* grid, GeoTessMetaData* metaData);

	///@endcond

public:

	/**
	 * Default constructor.
	 *
	 */
	GeoTessModel();

	/**
	 * Construct a new GeoTessModel object and populate it with information from
	 * the specified file.
	 *
	 * @param inputFile
	 *            name of file containing the model.
	 * @param relativeGridPath
	 *            the relative path from the directory where the model is stored
	 *            to the directory where the grid is stored. Often, the model
	 *            and grid are stored together in the same file in which case
	 *            this parameter is ignored. Sometimes, however, the grid is
	 *            stored in a separate file and only the name of the grid file
	 *            (without path information) is stored in the model file. In
	 *            this case, the code needs to know which directory to search
	 *            for the grid file. The default is "" (empty string), which
	 *            will cause the code to search for the grid file in the same
	 *            directory in which the model file resides. Bottom line is that
	 *            the default value is appropriate when the grid is stored in
	 *            the same file as the model, or the model file is in the same
	 *            directory as the model file.
	 */
	GeoTessModel(const string& inputFile, const string& relativeGridPath);

	/**
	 * Construct a new GeoTessModel object and populate it with information from
	 * the specified file.
	 *
	 * <p>relativeGridPath is assumed to be "" (empty string), which is appropriate
	 * when the grid information is stored in the same file as the model or when
	 * the grid is stored in a separate file located in the same directory as the
	 * model file.
	 *
	 * @param modelInputFile
	 *            name of file containing the model.
	 */
	GeoTessModel(const string& modelInputFile);

	/**
	 * Default constructor.
	 *
	 * @param attributeFilter the indexes of available attributes that should
	 *            be loaded into memory.
	 */
	GeoTessModel(vector<int>& attributeFilter);

	/**
	 * Construct a new GeoTessModel object and populate it with information from
	 * the specified file.
	 *
	 * @param inputFile
	 *            name of file containing the model.
	 * @param relativeGridPath
	 *            the relative path from the directory where the model is stored
	 *            to the directory where the grid is stored. Often, the model
	 *            and grid are stored together in the same file in which case
	 *            this parameter is ignored. Sometimes, however, the grid is
	 *            stored in a separate file and only the name of the grid file
	 *            (without path information) is stored in the model file. In
	 *            this case, the code needs to know which directory to search
	 *            for the grid file. The default is "" (empty string), which
	 *            will cause the code to search for the grid file in the same
	 *            directory in which the model file resides. Bottom line is that
	 *            the default value is appropriate when the grid is stored in
	 *            the same file as the model, or the model file is in the same
	 *            directory as the model file.
	 * @param attributeFilter the indexes of available attributes that should
	 *            be loaded into memory.
	 */
	GeoTessModel(const string& inputFile, const string& relativeGridPath,
			vector<int>& attributeFilter);

	/**
	 * Construct a new GeoTessModel object and populate it with information from
	 * the specified file.
	 *
	 * <p>relativeGridPath is assumed to be "" (empty string), which is appropriate
	 * when the grid information is stored in the same file as the model or when
	 * the grid is stored in a separate file located in the same directory as the
	 * model file.
	 *
	 * @param modelInputFile
	 *            name of file containing the model.
	 * @param attributeFilter the indexes of available attributes that should
	 *            be loaded into memory.
	 */
	GeoTessModel(const string& modelInputFile, vector<int>& attributeFilter);

	/**
	 * Parameterized constructor, specifying the grid and metadata for the
	 * model. The grid is constructed and the data structures are initialized
	 * based on information supplied in metadata. The data structures are not
	 * populated with any information however (all Profiles are null). The
	 * application should populate the new model's Profiles after this
	 * constructor completes.
	 *
	 * <p>
	 * Before calling this constructor, the supplied MetaData object must be
	 * populated with required information by calling the following MetaData
	 * methods:
	 * <ul>
	 * <li>setDescription()
	 * <li>setLayerNames()
	 * <li>setAttributes()
	 * <li>setDataType()
	 * <li>setLayerTessIds() (only required if grid has more than one
	 * multi-level tessellation)
	 * </ul>
	 *
	 * @param gridFileName
	 *            name of file from which to load the grid.
	 * @param metaData
	 *            MetaData the new GeoTessModel instantiates a reference to the
	 *            supplied metaData. No copy is made.
	 * @throws GeoTessException
	 *             if metadata is incomplete.
	 */
	GeoTessModel(const string& gridFileName, GeoTessMetaData* metaData);

	/**
	 * Parameterized constructor, specifying the grid and metadata for the
	 * model. The grid is constructed and the data structures are initialized
	 * based on information supplied in metadata. The data structures are not
	 * populated with any information however (all Profiles are null). The
	 * application should populate the new model's Profiles after this
	 * constructor completes.
	 *
	 * <p>
	 * Before calling this constructor, the supplied MetaData object must be
	 * populated with required information by calling the following MetaData
	 * methods:
	 * <ul>
	 * <li>setDescription()
	 * <li>setLayerNames()
	 * <li>setAttributes()
	 * <li>setDataType()
	 * <li>setLayerTessIds() (only required if grid has more than one
	 * multi-level tessellation)
	 * <li>setSoftwareVersion()
	 * <li>setGenerationDate()
	 * </ul>
	 *
	 * @param grid
	 *            a pointer to the GeoTessGrid that will support this
	 *            GeoTessModel.  GeoTessModel assumes ownership of the
	 *            supplied grid object and will delete it when it is
	 *            done with it.
	 * @param metaData
	 *            MetaData the new GeoTessModel instantiates a reference to the
	 *            supplied metaData. No copy is made.
	 * @throws GeoTessException
	 *             if metadata is incomplete.
	 */
	GeoTessModel(GeoTessGrid* grid, GeoTessMetaData* metaData);

	/**
	 * Destructor.
	 */
	virtual ~GeoTessModel();

	/**
	 * Returns the class name.
	 * @return class name
	 */
	static  string class_name() { return "GeoTessModel"; }

	/**
	 * Read model data and grid from a file.
	 * @param inputFile the path to the file that contains the model.
	 * @param relGridFilePath if the grid is stored in a separate file
	 * then relGridFilePath is the relative path from the directory where
	 * the model located to the directory where the grid is located.
	 * The default value for relGridFilePath is "" which indicates
	 * that the grid file resides in the same directory as the model
	 * file.
	 * @return returns a pointer to <i>this</i>
	 */
	GeoTessModel* loadModel(const string& inputFile, const string& relGridFilePath = "");

	/**
	 * Test a file to see if it is a GeoTessModel file.
	 *
	 * @param fileName
	 * @return true if inputFile is a GeoTessModel file.
	 */
	static bool isGeoTessModel(const string& fileName);

	/**
	 * Return the amount of memory currently occupied by this GeoTessModel object
	 * NOT INCLUDING THE GRID. The returned value includes the memory needed for
	 * the GeoTessMetaData, all the Profiles (including all the radii and Data objects),
	 * and memory for the PointMap.
	 *
	 * <p>To retrieve the size of the grid call model.getGrid().getMemory().
	 * Note that multiple GeoTessModels may reference the same GeoTessGrid object so
	 * if you are working with multiple models and they might be sharing references to the same
	 * grids then the best way to find the memory requirements of the set of GeoTessGrid objects
	 * currently in use is  to call the static method GeoTessModel::getReuseGridMapMemory().
	 *
	 * @return memory in bytes.
	 */
	virtual LONG_INT getMemory()
	{
		LONG_INT memory = (LONG_INT)sizeof(GeoTessModel);

		memory += metaData->getMemory();

		if (profiles)
			for (int i=0; i<getNVertices(); ++i)
				for (int j=0; j<getNLayers(); ++j)
					if (profiles[i][j])
						memory += profiles[i][j]->getMemory();
		if (pointMap)
			memory += pointMap->getMemory();
		return memory;
	}

	/**
	 * GeoTessModel will attempt to reuse grids that it has already loaded into
	 * memory when a new model tries to reload the same grid.  This method
	 * returns the amount of memory required by all the grids stored.
	 * @return memory requirements in bytes.
	 */
	static LONG_INT getReuseGridMapMemory()
	{
		// this ignores some memory allocated to support the map.  It is complicated and
		// platform dependent.  Also probably small compared to the size of the Grids.
		LONG_INT memory = sizeof(map<string, GeoTessGrid*>);

		memory += (LONG_INT) (reuseGridMap.size() * (sizeof(string) + sizeof(GeoTessGrid*)));

		for (map<string, GeoTessGrid*>::iterator it = reuseGridMap.begin(); it != reuseGridMap.end(); it++)
		 	memory += (LONG_INT)it->first.length() + it->second->getMemory();

		return memory;
	}

	/**
	 * GeoTessModel will attempt to reuse grids that it has already loaded into
	 * memory when a new model tries to reload the same grid.  This method
	 * returns the size of the map that supports this functionality.
	 * @return size of reuseGridMap.
	 */
	static int getReuseGridMapSize() { return reuseGridMap.size(); }

	/**
	 * GeoTessModel will attempt to reuse grids that it has already loaded into
	 * memory when a new model tries to reload the same grid.  This method
	 * clears the map that supports this functionality.
	 */
	static void clearReuseGrid() { reuseGridMap.clear(); }

	/**
	 *  Retrieve a reference to the ellipsoid that is stored in this GeoTessModel.  This EarthShape
	 *  object can be used to convert between geographic and geocentric latitude, and between
	 *  radius and depth in the Earth.
	 *  <p>
	 *  The following EarthShapes are supported:
	 * <ul>
	 * <li>SPHERE - Geocentric and geographic latitudes are identical and
	 * conversion between depth and radius assume the Earth is a sphere
	 * with constant radius of 6371 km.
	 * <li>GRS80 - Conversion between geographic and geocentric latitudes, and between depth
	 * and radius are performed using the parameters of the GRS80 ellipsoid.
	 * <li>GRS80_RCONST - Conversion between geographic and geocentric latitudes are performed using
	 * the parameters of the GRS80 ellipsoid.  Conversions between depth and radius
	 * assume the Earth is a sphere with radius 6371.
	 * <li>WGS84 - Conversion between geographic and geocentric latitudes, and between depth
	 * and radius are performed using the parameters of the WGS84 ellipsoid.
	 * <li>WGS84_RCONST - Conversion between geographic and geocentric latitudes are performed using
	 * the parameters of the WGS84 ellipsoid.  Conversions between depth and radius
	 * assume the Earth is a sphere with radius 6371.
	 * <li>IERS2003 - Conversion between geographic and geocentric latitudes, and between depth
	 * and radius are performed using the parameters of the IERS2003 ellipsoid.
	 * <li>IERS2003_RCONST - Conversion between geographic and geocentric latitudes are performed using
	 * the parameters of the IERS2003 ellipsoid.  Conversions between depth and radius
	 * assume the Earth is a sphere with radius 6371.
	 * </ul>
	 * @return a reference to the EarthShape currently in use.
	 */
	EarthShape& getEarthShape() { return metaData->getEarthShape(); }

	/**
	 *  Specify the name of the ellipsoid that is to be used to convert between geocentric and
	 *  geographic latitude and between depth and radius.  This ellipsoid will be save in this
	 *  GeoTessModel if it is written to file. The following EarthShapes are supported:
	 * <ul>
	 * <li>SPHERE - Geocentric and geographic latitudes are identical and
	 * conversion between depth and radius assume the Earth is a sphere
	 * with constant radius of 6371 km.
	 * <li>GRS80 - Conversion between geographic and geocentric latitudes, and between depth
	 * and radius are performed using the parameters of the GRS80 ellipsoid.
	 * <li>GRS80_RCONST - Conversion between geographic and geocentric latitudes are performed using
	 * the parameters of the GRS80 ellipsoid.  Conversions between depth and radius
	 * assume the Earth is a sphere with radius 6371.
	 * <li>WGS84 - Conversion between geographic and geocentric latitudes, and between depth
	 * and radius are performed using the parameters of the WGS84 ellipsoid.
	 * <li>WGS84_RCONST - Conversion between geographic and geocentric latitudes are performed using
	 * the parameters of the WGS84 ellipsoid.  Conversions between depth and radius
	 * assume the Earth is a sphere with radius 6371.
	 * <li>IERS2003 - Conversion between geographic and geocentric latitudes, and between depth
	 * and radius are performed using the parameters of the IERS2003 ellipsoid.
	 * <li>IERS2003_RCONST - Conversion between geographic and geocentric latitudes are performed using
	 * the parameters of the IERS2003 ellipsoid.  Conversions between depth and radius
	 * assume the Earth is a sphere with radius 6371.
	 * </ul>
	 * @param earthShapeName the name of the ellipsoid that is to be used.
	 */
	void setEarthShape(const string& earthShapeName) { metaData->setEarthShape(earthShapeName); }

	/**
	 * Return a reference to the grid object.
	 * @return a reference to the grid object that cannot be modified.
	 */
	const GeoTessGrid& getGrid() const { return *grid; }

	/**
	 * Return a reference to the grid object.
	 * @return a reference to the grid object.
	 */
	GeoTessGrid& getGrid(){ return *grid; }

	/**
	 * Return a reference to the GeoTessMetaData object associated with this
	 * model.  The metadata object stores information about the models such
	 * as a description of the model, the layer names, attribute names,
	 * attribute units, the data type, etc.
	 * @return a reference to the meta data object
	 */
	GeoTessMetaData& getMetaData() { return *metaData; }

	/**
	 * Retrieve a pointer to a new GeoTessPosition object that knows how to
	 * interpolate information from the model.  Linear interpolation will
	 * be performed in both the horizontal and radial dimensions
	 * <p>It is the caller's responsibility to delete this object when
	 * it is no longer needed.
	 * @return a GeoTessPosition object.
	 */
	GeoTessPosition* getPosition();

	/**
	 * Retrieve a pointer to a new GeoTessPosition object that knows how to
	 * interpolate information from the model.  If the horizontal InterpolatorType
	 * is LINEAR then the radial InterpolatorType will be LINEAR as well.
	 * If the horizontal InterpolatorType
	 * is NATUAL_NEIGHBOR then the radial InterpolatorType will be CUBIC_SPLINE.
	 * <p>It is the caller's responsibility to delete this object when
	 * it is no longer needed.
	 * @param horizontalType the type of interpolation that is to be used
	 * for interpolation in the geographic dimensions;
	 * either InterpolatorType:LINEAR or InterpolatorType::NATURAL_NEIGHBOR
	 * @return a GeoTessPosition object.
	 */
	GeoTessPosition* getPosition(const GeoTessInterpolatorType& horizontalType);

	/**
	 * Retrieve a pointer to a new GeoTessPosition object that knows how to
	 * interpolate information from the model.
	 * <p>It is the caller's responsibility to delete this object when
	 * it is no longer needed.
	 * @param horizontalType the type of interpolation that is to be used
	 * for interpolation in the geographic dimensions;
	 * either InterpolatorType:LINEAR or InterpolatorType::NATURAL_NEIGHBOR
	 * @param radialType the type of interpolation that is to be used in the
	 * radia dimension;
	 * either InterpolatorType:LINEAR or InterpolatorType::CUBIC_SPLINE
	 * @return a GeoTessPosition object.
	 */
	GeoTessPosition* getPosition(const GeoTessInterpolatorType& horizontalType,
			const GeoTessInterpolatorType& radialType);

	/**
	 * Return number of vertices in the 2D geographic grid.
	 * @return number of vertices in the 2D geographic grid.
	 */
	int getNVertices() const { return grid->getNVertices(); }

	/**
	 * Return the number of layers that comprise the model.
	 * @return the number of layers that comprise the model.
	 */
	int getNLayers() const { return metaData->getNLayers(); }

	/**
	 * Return the number of radii that are specified in the Profile at
	 * vertexId, layerId.
	 * @param vertexId the vertex index
	 * @param layerId the layer index
	 * @return the number of radii that are specified in profile[vertexId][layerId]
	 */
	int getNRadii(int vertexId, int layerId) { return profiles[vertexId][layerId]->getNRadii(); }

	/**
	 * Return the number of Data objects that are specified in the Profile at
	 * vertexId, layerId
	 * @param vertexId the vertex index
	 * @param layerId the layer index
	 * @return the number of Data that are specified in profile[vertexId][layerId]
	 */
	int getNData(int vertexId, int layerId) { return profiles[vertexId][layerId]->getNData(); }

	/**
	 * Return the number of attributes that are associated with each node in the model.
	 * @return the number of attributes that are associated with each node in the model.
	 */
	int getNAttributes() { return metaData->getNAttributes(); }

	/**
	 * Return the radius in km of the node at vertexId, layerId, nodeId.
	 * @param vertexId the vertex index
	 * @param layerId the layer index
	 * @param nodeId the node index
	 * @return the radius in km of the node at vertexId, layerId, nodeId.
	 */
	double getRadius(int vertexId, int layerId, int nodeId)
	{ return profiles[vertexId][layerId]->getRadius(nodeId); }

	/**
	 * Return the depth below surface of current EarthShape in km of the node
	 * at vertexId, layerId, nodeId.
	 * @param vertexId the vertex index
	 * @param layerId the layer index
	 * @param nodeId the node index
	 * @return the depth in km of the node at vertexId, layerId, nodeId.
	 */
	double getDepth(int vertexId, int layerId, int nodeId)
	{ return getEarthShape().getEarthRadius(grid->getVertex(vertexId))
	         - profiles[vertexId][layerId]->getRadius(nodeId); }

	/**
	 * Return the value of the attribute at the specified vertexId, layerId, nodeId,
	 * attributeIndex, cast to a double if necessary.
	 * @param vertexId the vertex index
	 * @param layerId the layer index
	 * @param nodeId the node index
	 * @param attributeIndex the attributeIndex
	 * @return the value of the specifed attribute, cast to double if necessary
	 */
	double getValueDouble(int vertexId, int layerId, int nodeId, int attributeIndex)
	{
		GeoTessData* data = profiles[vertexId][layerId]->getData(nodeId);
		return data == NULL ? NaN_DOUBLE : data->getDouble(attributeIndex);
	}

	/**
	 * Return the value of the attribute at the specified vertexId, layerId, nodeId,
	 * attributeIndex, cast to a float if necessary.
	 * @param vertexId the vertex index
	 * @param layerId the layer index
	 * @param nodeId the node index
	 * @param attributeIndex the attributeIndex
	 * @return the value of the specifed attribute, cast to float if necessary
	 */
	float getValueFloat(int vertexId, int layerId, int nodeId, int attributeIndex)
	{
		GeoTessData* data = profiles[vertexId][layerId]->getData(nodeId);
		return data == NULL ? NaN_FLOAT : data->getFloat(attributeIndex);
	}

	/**
	 * Return the value of the attribute at the specified vertexId, layerId, nodeId,
	 * attributeIndex, cast to a LONG_INT if necessary.
	 * @param vertexId the vertex index
	 * @param layerId the layer index
	 * @param nodeId the node index
	 * @param attributeIndex the attributeIndex
	 * @return the value of the specifed attribute, cast to LONG_INT if necessary
	 */
	LONG_INT getValueLong(int vertexId, int layerId, int nodeId, int attributeIndex)
	{
		GeoTessData* data = profiles[vertexId][layerId]->getData(nodeId);
		return data == NULL ? LONG_MIN : data->getLong(attributeIndex);
	}

	/**
	 * Return the value of the attribute at the specified vertexId, layerId, nodeId,
	 * attributeIndex, cast to a int if necessary.
	 * @param vertexId the vertex index
	 * @param layerId the layer index
	 * @param nodeId the node index
	 * @param attributeIndex the attributeIndex
	 * @return the value of the specifed attribute, cast to int if necessary
	 */
	int getValueInt(int vertexId, int layerId, int nodeId, int attributeIndex)
	{
		GeoTessData* data = profiles[vertexId][layerId]->getData(nodeId);
		return data == NULL ? INT_MIN : data->getInt(attributeIndex);
	}

	/**
	 * Return the value of the attribute at the specified vertexId, layerId, nodeId,
	 * attributeIndex, cast to a short if necessary.
	 * @param vertexId the vertex index
	 * @param layerId the layer index
	 * @param nodeId the node index
	 * @param attributeIndex the attributeIndex
	 * @return the value of the specifed attribute, cast to short if necessary
	 */
	short getValueShort(int vertexId, int layerId, int nodeId, int attributeIndex)
	{
		GeoTessData* data = profiles[vertexId][layerId]->getData(nodeId);
		return data == NULL ? SHRT_MIN : data->getShort(attributeIndex);
	}

	/**
	 * Return the value of the attribute at the specified vertexId, layerId, nodeId,
	 * attributeIndex, cast to a byte if necessary.
	 * @param vertexId the vertex index
	 * @param layerId the layer index
	 * @param nodeId the node index
	 * @param attributeIndex the attributeIndex
	 * @return the value of the specifed attribute, cast to byte if necessary
	 */
	byte getValueByte(int vertexId, int layerId, int nodeId, int attributeIndex)
	{
		GeoTessData* data = profiles[vertexId][layerId]->getData(nodeId);
		return data == NULL ? CHAR_MIN : data->getByte(attributeIndex);
	}

	/**
	 * Modify the attribute value stored at the specified vertex, layer, node, attribute.
	 * @param vertexId the vertex index
	 * @param layerId the layer index
	 * @param nodeId the node index
	 * @param attributeIndex the attributeIndex
	 * @param value the new attribute value (must be of type double, float, LONG_INT,
	 * int, short or byte).
	 */
	template<typename T>
	void setValue(int vertexId, int layerId, int nodeId, int attributeIndex, T value)
	{ profiles[vertexId][layerId]->getData(nodeId)->setValue(attributeIndex, value); }

	/**
	 * Return the radius in km of the node at pointIndex.
	 * @param pointIndex
	 * @return the radius in km of the node at pointIndex.
	 */
	double getRadius(int pointIndex)
	{ return getPointMap()->getPointRadius(pointIndex); }

	/**
	 * Return the depth below surface of the current EarthShape in km of the node
	 * at pointIndex.
	 * @param pointIndex
	 * @return the depth in km of the node at pointIndex.
	 */
	double getDepth(int pointIndex)
	{ return getPointMap()->getPointDepth(pointIndex); }

	/**
	 * Return the value of the attribute at the specified pointIndex,
	 * attributeIndex, cast to a double if necessary.
	 * @param pointIndex
	 * @param attributeIndex the attributeIndex
	 * @return the value of the specifed attribute, cast to double if necessary
	 */
	double getValueDouble(int pointIndex, int attributeIndex)
	{
		GeoTessData* data = getPointMap()->getPointData(pointIndex);
		return data == NULL ? NaN_DOUBLE : data->getDouble(attributeIndex);
	}

	/**
	 * Return the value of the attribute at the specified pointIndex,
	 * attributeIndex, cast to a float if necessary.
	 * @param pointIndex
	 * @param attributeIndex the attributeIndex
	 * @return the value of the specifed attribute, cast to float if necessary
	 */
	float getValueFloat(int pointIndex, int attributeIndex)
	{
		GeoTessData* data = getPointMap()->getPointData(pointIndex);
		return data == NULL ? NaN_FLOAT : data->getFloat(attributeIndex);
	}

	/**
	 * Return the value of the attribute at the specified pointIndex,
	 * attributeIndex, cast to a LONG_INT if necessary.
	 * @param pointIndex
	 * @param attributeIndex the attributeIndex
	 * @return the value of the specifed attribute, cast to LONG_INT if necessary
	 */
	LONG_INT getValueLong(int pointIndex, int attributeIndex)
	{
		GeoTessData* data = getPointMap()->getPointData(pointIndex);
		return data == NULL ? LONG_MIN : data->getLong(attributeIndex);
	}

	/**
	 * Return the value of the attribute at the specified pointIndex,
	 * attributeIndex, cast to a int if necessary.
	 * @param pointIndex
	 * @param attributeIndex the attributeIndex
	 * @return the value of the specifed attribute, cast to int if necessary
	 */
	int getValueInt(int pointIndex, int attributeIndex)
	{
		GeoTessData* data = getPointMap()->getPointData(pointIndex);
		return data == NULL ? INT_MIN : data->getInt(attributeIndex);
	}

	/**
	 * Return the value of the attribute at the specified pointIndex,
	 * attributeIndex, cast to a short if necessary.
	 * @param pointIndex
	 * @param attributeIndex the attributeIndex
	 * @return the value of the specifed attribute, cast to short if necessary
	 */
	short getValueShort(int pointIndex, int attributeIndex)
	{
		GeoTessData* data = getPointMap()->getPointData(pointIndex);
		return data == NULL ? SHRT_MIN : data->getShort(attributeIndex);
	}

	/**
	 * Return the value of the attribute at the specified pointIndex,
	 * attributeIndex, cast to a byte if necessary.
	 * @param pointIndex
	 * @param attributeIndex the attributeIndex
	 * @return the value of the specifed attribute, cast to byte if necessary
	 */
	byte getValueByte(int pointIndex, int attributeIndex)
	{
		GeoTessData* data = getPointMap()->getPointData(pointIndex);
		return data == NULL ? CHAR_MIN : data->getByte(attributeIndex);
	}

	/**
	 * Modify the attribute value stored at the specified vertex, layer, node, attribute.
	 * @param pointIndex
	 * @param attributeIndex the attributeIndex
	 * @param value the new attribute value (must be of type double, float, LONG_INT,
	 * int, short or byte).
	 */
	template<typename T>
	void setValue(int pointIndex, int attributeIndex, T value)
	{
		GeoTessData* data = getPointMap()->getPointData(pointIndex);
		if (data != NULL) data->setValue(attributeIndex, value);
	}

	/**
	 * Retrieve the number of points in the model, including all nodes along all profiles at all
	 * grid vertices.
	 * @return number of points in the model.
	 */
	int getNPoints() { return getPointMap()->size(); }

	/**
	 * Return true if this and the input other model are equal. i.e., their grids have the same
	 * gridIDs, they have the same number of layers, and all their Profiles are equal. For
	 * profiles to be equal, they must be of the same type and size, and all of their radii
	 * must be equal and all of their data must be equal.
	 * @param other the other model to which this model is to be compared for equality.
	 * @return true if this and other are equal.
	 */
	virtual bool operator == (const GeoTessModel& other) const;

	/**
	 * Return true if this and the input other model are not equal. i.e., their grids have different
	 * gridIDs, they have different number of layers, or any of their Profiles are not equal. For
	 * profiles to be equal, they must be of the same type and size, and all of their radii
	 * must be equal and all of their data must be equal.
	 * @param other the other model to which this model is to be compared for equality.
	 * @return true if this and other are not equal.
	 */
	virtual bool operator != (const GeoTessModel& other) const { return !(*this == other); } ;

	bool is2D()
	{
		return getProfile(0, 0)->getType() == GeoTessProfileType::SURFACE
				|| getProfile(0, 0)->getType() == GeoTessProfileType::SURFACE_EMPTY;
	}

	bool is3D()
	{
		return !is2D();
	}

	/**
	 * Retrieve a pointer to the pointMap, which is an nPoints by 3 array of indexes. For each
	 * point in the 3D grid, pointMap stores 3 indexes: the vertex index, the layer index, and the
	 * node index.
	 *
	 * <p>
	 * The term 'vertex' refers to a position in the 2D tessellation. A vertex is a 2D point
	 * represented by unit vectors on a unit sphere. The term 'node' refers to a Data object on a
	 * radial profile associated with a vertex and a layer in the model. Node indexes are unique
	 * only within a given profile (all profiles have a node with index 0 for example). The term
	 * 'point' refers to all the nodes in all the profiles of the model. There is only one 'point'
	 * in the model with index 0. PointMap is introduced to help map back and forth between all
	 * these different indexes.
	 *
	 * <p>Users should not delete this pointer.
	 * @return a reference to the PointMap object that supports this model.
	 */
	GeoTessPointMap* getPointMap()
	{
		if (!pointMap->isPopulated())
			pointMap->setActiveRegion();
		return pointMap;
	}

	/**
	 * Return a reference to the set<int> of the indexes of all the vertices that are
	 * connected together by triangles in the specified layer of the model.
	 *
	 * @param layerIndex
	 * @return a reference to the set<int> of the indexes of all the vertices that are
	 * connected together by triangles in the specified layer of the model.
	 */
	const set<int>& getConnectedVertices(int layerIndex)
	{
		return grid->getVertexIndicesTopLevel(metaData->getTessellation(layerIndex));
	}

	/**
	 * Set the active region such that it encompasses all the nodes in the model.
	 * @return a reference to the updated PointMap that has been configured to
	 * support the specified active region.
	 */
	void setActiveRegion()
	{
		pointMap->setActiveRegion();
	}

	/**
	 * Set the active region to encompass only the nodes contained within the
	 * specified Polygon.
	 * <p>KML and KMZ formats are not supported by the C++ code.
	 * See GeoTessExplorer (Java) which can convert
	 * KML/KMZ files to ascii.
	 * @param polygon the name of a file containing a valid Polygon2D or
	 * Polygon3D object, in ascii format.
	 * @return a reference to the updated PointMap that has been configured to
	 * support the specified active region.
	 * @throws PolygonException
	 */
	void setActiveRegion(const string& polygon)
	{
		pointMap->setActiveRegion(polygon);
	}

	/**
	 * Set the active region to encompass only the nodes contained within the
	 * specified Polygon object.
	 * <p>Polygon implements reference counting.  This instance of GeoTessModel
	 * will increment polygon's reference count.  When this model is done with
	 * the polygon, it will decrement polygon's reference count and delete the
	 * polygon if its reference count is zero.
	 * @param polygon a Polygon object.
	 * @return a reference to the updated PointMap that has been configured to
	 * support the specified active region.
	 */
	void setActiveRegion(GeoTessPolygon* polygon)
	{
		if (polygon == NULL)
			pointMap->setActiveRegion();
		else
			pointMap->setActiveRegion(polygon);
	}

	/**
	 * Retrieve the pointer to the Polygon or Polygon3D object
	 * that supports this PointMap.  May be NULL.
	 *
	 * <p>Polygon implements referenceCounting so if you wish
	 * to retain a copy of this polygon, be sure to
	 * addReference() and delete it when you are done with it.
	 */
	GeoTessPolygon* getPolygon() { return pointMap->getPolygon(); }

	/**
	 * Retrieve the number of points in each layer of the model.
	 * @param activeOnly if true, counts only active nodes otherwise
	 * counts all nodes.
	 * @param layerCount an int array with at least nLayers elements
	 * which will be populated with the number of nodes in each layer.
	 * @return the number of points in each layer of the model.
	 */
	void getLayerCount(bool activeOnly, int* layerCount)
	{
		for (int layer=0; layer<getNLayers(); ++layer)
			layerCount[layer] = 0;

		GeoTessProfile** pp;
		GeoTessProfile* p;
		for (int v=0; v<getNVertices(); ++v)
		{
			pp = profiles[v];
			for (int layer=0; layer<getNLayers(); ++layer)
			{
				if (activeOnly)
				{
					p = pp[layer];
					for (int n=0; n<p->getNData(); ++n)
						if (p->getPointIndex(n) >= 0)
							++layerCount[layer];
				}
				else
					layerCount[layer] += pp[layer]->getNData();
			}
		}
	}

	/**
	 * Get a reference to the Profile object for the specified vertex and layer.
	 * @param vertex index of a vertex in the 2D grid
	 * @param layer index of one of the layers that comprise the model
	 * @return a reference to a Profile object that contains the radii
	 * and Data values stored in profile[vertex][layer].
	 */
	GeoTessProfile* getProfile(int vertex, int layer)
	{
		return profiles[vertex][layer];
	}

	/**
	 * Get a reference to the array of Profile objects for the specified vertex.
	 * @param vertex index of a vertex in the 2D grid
	 * @return a reference to an array of Profile objects that contains the radii
	 * and Data values for all the Profiles at the specified vertex.
	 */
	GeoTessProfile** getProfiles(int vertex)
	{
		return profiles[vertex];
	}

	/**
	 * Get a reference to all Profile objects.
	 * @return  a reference to all Profile objects.
	 */
	GeoTessProfile*** getProfiles() const { return profiles; }

	/**
	 * Replace the Profile object at the specified vertex and layer with a new one.
	 * Profile is checked to ensure that radii are monotonically increasing.
	 * <p>GeoTessModel assumes ownership of the Profile object and will delete
	 * it when it is no longer needed.  Caller should not delete the Profile object.
	 * @param vertex index of a vertex in the 2D grid
	 * @param layer index of one of the layers that comprise the model
	 * @param profile a pointer to a Profile object that contains the radii
	 * and Data values to be stored in profile[vertex][layer].
	 */
	void setProfile(int vertex, int layer, GeoTessProfile* profile);

	/**
	 * Replace the Profile object at the specified vertex and layer with a new one.
	 * All radius and data values are copied out of the supplied vector<>s.
	 * GeoTessModel does not keep any references to the vectors<>
	 * <p>
	 * If vertex is not connected to other vertices by any triangles in the
	 * specified layer, than the values are ignored and a ProfileEmpty object
	 * is instantiated with just the first and last radii provided.
	 * @param vertex index of a vertex in the 2D grid
	 * @param layer index of one of the layers that comprise the model
	 * @param radii the radius values in km
	 * @param values nNodes x nAttributes array of model values.
	 */
	template<typename T>
	void setProfile(int vertex, int layer, vector<float>& radii, vector<vector<T> >& values)
	{
		if (getConnectedVertices(layer).count(vertex) == 1)
			setProfile(vertex, layer, GeoTessProfile::newProfile(radii, values));
		else setProfile(vertex, layer, new GeoTessProfileEmpty(radii[0], radii[radii.size()-1]));
	}

	/**
	 * Replace the Profile object at the specified vertex and layer with a new one.
	 * All radius and data values are copied out of the supplied vector<>s.
	 * GeoTessModel does not keep any references to the vectors<>
	 * If vertex is not connected to other vertices by any triangles in the
	 * specified layer, than the values are ignored and a ProfileEmpty object
	 * is instantiated with just the first and last radii provided.
	 * @param vertex index of a vertex in the 2D grid
	 * @param layer index of one of the layers that comprise the model
	 * @param radii the radius values in km
	 * @param nRadii the number of radius values provided
	 * @param values nNodes x nAttributes array of model values.
	 * @param nNodes
	 * @param nAttributes
	 */
	template<typename T>
	void setProfile(const int& vertex, const int& layer,
			float* radii, const int& nRadii,
			T** values, const int& nNodes, const int& nAttributes)
	{
		if (getConnectedVertices(layer).count(vertex) == 1)
			setProfile(vertex, layer, GeoTessProfile::newProfile(radii, nRadii, values, nNodes, nAttributes));
		else setProfile(vertex, layer, new GeoTessProfileEmpty(radii[0], radii[nRadii-1]));
	}

	/**
	 * Replace the Profile object at the specified vertex and layer with a new
	 * ProfileEmpty object.
	 * @param vertex index of a vertex in the 2D grid
	 * @param layer index of one of the layers that comprise the model
	 * @param radii the radius values in km. The only ones used are radii[0]
	 * and radii[radii.size()-1];
	 */
	void setProfile(int vertex, int layer, vector<float>& radii)
	{ setProfile(vertex, layer, new GeoTessProfileEmpty(radii[0], radii[radii.size()-1])); }

	/**
	 * Replace the Profile object at the specified vertex and layer with a new
	 * ProfileEmpty object.
	 * @param vertex index of a vertex in the 2D grid
	 * @param layer index of one of the layers that comprise the model
	 * @param radii the radius values in km
	 * @param nRadii number of radii provided.  The only ones used are radii[0]
	 * and radii[nRadii-1];
	 */
	template<typename T>
	void setProfile(const int& vertex, const int& layer, float* radii, const int& nRadii)
	{ setProfile(vertex, layer, new GeoTessProfileEmpty(radii[0], radii[nRadii-1])); }

	/**
	 * Replace the Profile object at the specified vertex with a new one
	 * of type ProfileSurface, which supports 2D Earth models.
	 * All data values are copied out of the supplied vector<>.
	 * GeoTessModel does not keep any references to the vectors<>
	 * @param vertex index of a vertex in the 2D grid
	 * @param values array of model values with nAttribute elements.
	 */
	template<typename T>
	void setProfile(const int& vertex, vector<T>& values)
	{ setProfile(vertex, 0, new GeoTessProfileSurface(GeoTessData::getData(values))); }

	/**
	 * Replace the Profile object at the specified vertex with a new one
	 * of type ProfileSurface, which supports 2D Earth models.
	 * All data values are copied out of the supplied vector<>.
	 * GeoTessModel does not keep any references to the array
	 * @param vertex index of a vertex in the 2D grid
	 * @param values array of model values with nAttribute elements.
	 * @param nAttributes length of values array
	 */
	template<typename T>
	void setProfile(const int& vertex, T* values, const int& nAttributes)
	{ setProfile(vertex, 0, new GeoTessProfileSurface(GeoTessData::getData(values, nAttributes))); }

	/**
	 * Replace the Profile object at the specified vertex with a new one
	 * of type ProfileSurfEmpty, which supports 2D Earth models.
	 * @param vertex index of a vertex in the 2D grid
	 */
	void setProfile(const int& vertex)
	{ setProfile(vertex, 0, new GeoTessProfileSurfaceEmpty()); }

	/**
	 * Write the model to file. Grid information will be included in the
	 * specified output file.
	 *
	 * @param outputFile name of the file to receive the model
	 * @throws IOException
	 */
	void	writeModel(const string& outputFile) { writeModel(outputFile, "*"); }

	/**
	 * Write the model to file. The data (radii and attribute values) are
	 * written to outputFile. If gridFileName is '*' or omitted then the grid information
	 * is written to the same file as the data. If gridFileName is something
	 * else, it should be the name of the file that contains the grid
	 * information (just the name; no path information). In the latter case,
	 * the gridFile referenced by gridFileName is not overwritten; all that happens
	 * is that the name of the grid file (with no path information) is stored in the data file.
	 *
	 * @param outputFile
	 *            name of the file to receive the model
	 * @param gridFileName
	 *            name of file to receive the grid (no path info), or "*"
	 */
	void writeModel(const string& outputFile, const string& gridFileName);

	/**
	 * To string method.
	 * @return string with information about this model.
	 */
	string toString();

	/**
	 * Retrieve the number of points in each layer of the model.
	 *
	 * @param activeOnly
	 *            if true, counts only active nodes otherwise counts all nodes.
	 * @param layerCount the number of points in each layer of the model.
	 */
	void getLayerCount(vector<int>& layerCount, const bool& activeOnly)
	{
		layerCount.resize(metaData->getNLayers(), 0);

		GeoTessProfile** pp;
		GeoTessProfile* p;
		for (int vtx=0; vtx<getNVertices(); ++vtx)
		{
			pp = profiles[vtx];
			for (int layer = 0; layer < metaData->getNLayers(); ++layer)
				if (activeOnly)
				{
					p = pp[layer];
					for (int n = 0; n < p->getNData(); ++n)
						if (p->getPointIndex(n) >= 0)
							++layerCount[layer];
				}
				else
					layerCount[layer] += pp[layer]->getNData();
		}
	}

	/**
	 * Retrieve the number of Profiles of each ProfileType in each layer of
	 * the model.  Also returns the total number of profiles of each type
	 * independent of layer.
	 *
	 * @param count is a 2D vector of ints with dimensions (nLayers+1) x nProfileTypes.
	 */
	void profileCount(vector< vector<int> >& count)
	{
		count.clear();

		getPointMap();

		// this is total number of profiles of each type, independent of layer index.
		vector<int> totalCount;
		for (int profileType=0; profileType<GeoTessProfileType::size(); ++profileType)
			totalCount.push_back(0);

		for (int layer = 0; layer < getNLayers(); ++layer)
		{
			vector<int> typeCount;

			for (int profileType=0; profileType<GeoTessProfileType::size(); ++profileType)
				typeCount.push_back(0);

			count.push_back(typeCount);
		}

		for (int layer = 0; layer < getNLayers(); ++layer)
		{
			for (int vertex = 0; vertex < metaData->getNVertices(); ++vertex)
			{
				int pType = profiles[vertex][layer]->getType().ordinal();
				count[layer][pType] += 1;
				totalCount[pType] += 1;
			}
		}

		count.push_back(totalCount);
	}

	/**
	 * Reset all the Data objects in the model.  The number of attributes in the
	 * new Data objects will equal the number of elements in the supplied lists
	 * of attributeNames and attributeUnits (which must be equal in length).
	 * <p>
	 * The DataType of the new Data objects will be the type of the supplied
	 * fillValue.  Data values will be copied from the old to the new Data objects with
	 * casting to the type of the new data objects.  It is the responsibility
	 * of the caller to ensure that casting of data values does not result in
	 * corruption of data values or the loss of important information.
	 * <p>
	 * If the number of attributes in the new Data objects is less than the
	 * number in the old Data objects, old data values are lost.  If the number
	 * of attributes in the new Data objects is greater than the number in the
	 * old Data objects, new values are populated with fillValue.
	 * <p>
	 * This method can only modify the Data objects associated with existing Profiles.
	 * It does not instantiate Profiles, change the ProfileType of existing profiles,
	 * modify the number of nodes in existing Profiles, or modify the radii within
	 * a Profile.  To do any of those things requires replacing the Profile.
	 *
	 * @param attributeNames new names of the attributes.  These names will
	 * replace the old names in the MetaData object.
	 * @param attributeUnits new units of the attributes.  These units will
	 * replace the old units in the MetaData object.  The number of attributeNames
	 * and attributeUntis must be the same.
	 * @param fillValue a value of type double, float, LONG_INT, int,
	 * short or byte.  All data values in this models' Data objects will be of this
	 * type after this method executes.  The supplied value is used to populate data
	 * values that are not copied from the old Data object.
	 * @throws GeoTessException if the model has not been populated with Profiles
	 * or if the number of attributeNames and attributeUnits is not the same.
	 */
	template<typename T>
	void	initializeData(const vector<string>& attributeNames,
			const vector<string>& attributeUnits, T fillValue)
	{
		if (profiles == NULL)
		{
			ostringstream os;
			os << endl << "ERROR in GeoTessModel::initializeData" << endl
					<< "Attempting to initialize the model data before Profiles" << endl
					<< "have been specified (profiles == NULL)" << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 1001);
		}

		const GeoTessDataType& newDataType = GeoTessDataType::getDataType(fillValue);
		int nAttributesNew = attributeNames.size();

		int newDataTypeOrdinal = newDataType.ordinal();
		int oldDataTypeOrdinal = metaData->getDataType().ordinal();

		// if neither the dataType or the number of attributes has changed, there is no need to
		// copy the data.  The only thing that happens is that the attributeNames or attributeUnits might change.
		if (newDataTypeOrdinal != oldDataTypeOrdinal || nAttributesNew != metaData->getNAttributes())
		{
			// initialize an array of the new dataType with number of elements
			// equal to the number of new attributes and populate it with fillValue.
			T* newValues = new T[nAttributesNew];
			for (int i=0; i<nAttributesNew; ++i)
				newValues[i] = fillValue;

			GeoTessProfile* profile;
			vector<GeoTessData*> data;
			for (int v = 0; v < getNVertices(); ++v)
				for (int lid = 0; lid < getNLayers(); ++lid)
				{
					profile = profiles[v][lid];
					data.reserve(profile->getNData());
					data.clear();
					for (int n = 0; n < profile->getNData(); ++n)
					{
						profile->getData(n)->getValues(newValues, nAttributesNew);
						data.push_back(GeoTessData::getData(newValues, nAttributesNew));
					}
					// set new entries into profile and continue
					profile->setData(data);
				}
			delete[] newValues;
		}

		metaData->setDataType(newDataType);

		metaData->setAttributes(attributeNames, attributeUnits);
	}

	/**
	 * Replace all the Data objects in the entire model.  All current Data is lost.
	 * The new Data objects will be of the same type as fillValue.  The length
	 * of the Data arrays will be equal to the number of attributeNames and units
	 * supplied.
	 * <p>
	 * AttributeNames and attributeUnits must contain the same number of entries.
	 *
	 * @param attributeNames a semi-colon delimeted list of attribute names.
	 * @param attributeUnits a semi-colon delimeted list of attribute units.
	 * @param fillValue value to populate the model with
	 */
	template<typename T>
	void	initializeData(const string& attributeNames,
			const string& attributeUnits, T fillValue)
	{
		vector<string> names;
		CPPUtils::tokenizeString(attributeNames, ";", names);
		vector<string> units;
		CPPUtils::tokenizeString(attributeUnits, ";", units);
		initializeData(names, units, fillValue);
	}

	/**
	 * Compute the weights on each model point that results from interpolating positions
	 * along the specified ray path.  This method is only applicable to 2D GeoTessModels.
	 * <p>The following procedure is implemented:
	 * <ol>
	 * <li>divide the great circle path from pointA to pointB into nIntervals
	 * which each are of length less than or equal to pointSpacing.
	 * <li>multiply the length of each interval by the radius of the earth
	 * at the center of the interval, which converts the length of the interval into km.
	 * <li>interpolate the value of the specified attribute at the center of the interval.
	 * <li>sum the length of the interval times the attribute value, along the path.
	 * </ol>
	 * @param pointA the unit vector defining the beginning of the great circle path
	 * @param pointB the unit vector defining the end of the great circle path
	 * @param pointSpacing the maximum spacing between points, in radians.  The actual spacing
	 * will generally be slightly less than the specified value in order for there to be an
	 * integral number of uniform intervals along the great circle path.
	 * @param radius the radius of the great circle path, in km.  If the value is less than or
	 * equal to zero then the radius of the Earth determined by the current EarthShape is used.
	 * See getEarthShape() and setEarathShape() for more information about EarthShapes.
	 * @param horizontalType either InterpolatorType::LINEAR, or InterpolatorType::NATURAL_NEIGHBOR
	 * @param weights (output) map from pointIndex to weight.  The sum of the weights will equal
	 * the length of the ray path in km.
	 */
	void getWeights(const double* pointA, const double* pointB, const double& pointSpacing, const double& radius,
			const GeoTessInterpolatorType& horizontalType, map<int, double>& weights);

	/**
	 * Compute the weights on each model point that results from interpolating positions
	 * along the specified ray path.  This method is only applicable to 2D GeoTessModels.
	 * <p>The following procedure is implemented:
	 * <ol>
	 * <li>divide the great circle path from pointA to pointB into nIntervals
	 * which each are of length less than or equal to pointSpacing.
	 * <li>multiply the length of each interval by the radius of the earth
	 * at the center of the interval, which converts the length of the interval into km.
	 * <li>interpolate the value of the specified attribute at the center of the interval.
	 * <li>sum the length of the interval times the attribute value, along the path.
	 * </ol>
	 * @param greatCircle a GreatCircle object that specifies the ray path.
	 * @param pointSpacing the maximum spacing between points, in radians.  The actual spacing
	 * will generally be slightly less than the specified value in order for there to be an
	 * integral number of uniform intervals along the great circle path.
	 * @param radius the radius of the great circle path, in km.  If the value is less than or
	 * equal to zero then the radius of the Earth determined by the current EarthShape is used.
	 * See getEarthShape() and setEarathShape() for more information about EarthShapes.
	 * @param horizontalType either InterpolatorType::LINEAR, or InterpolatorType::NATURAL_NEIGHBOR
	 * @param weights (output) map from pointIndex to weight.  The sum of the weights will equal
	 * the length of the ray path in km.
	 */
	void getWeights(GeoTessGreatCircle& greatCircle, const double& pointSpacing, const double& radius,
			const GeoTessInterpolatorType& horizontalType, map<int, double>& weights);

	/**
	 * Compute the weights on each model point that results from interpolating positions
	 * along the specified ray path.  The following procedure is implemented:
	 * <ol>
	 * <li>find the midpoint of each increment of the path (line segment between
	 * adjacent positions on the path).
	 * <li>find the interpolation coefficients of all the model points that
	 * are 'touched' by the midpoint of the increment.
	 * <li>find the length of the path increment in km.
	 * <li>find the product of the length of the path increment times each
	 * interpolation coefficient and sum that value for each model point.
	 * </ol>
	 * @param rayPath (input) an ordered list of unit vectors that define a ray path through the model space
	 * @param radii (input) an ordered list of the radii in km of each point that defines the ray path
	 * @param layerIds (input) array of layer indices that specify the index of the layer
	 * in which path increment i resides where i is the path increment between points
	 * i and i+1 that define the ray path.  If layerIds is empty, or layerIds[i] is less than zero, then the
	 * layer in which the path increment resides will be determined from the position and radius of the midpoint.
	 * @param horizontalType (input) either InterpolatorType::LINEAR, or InterpolatorType::NATURAL_NEIGHBOR
	 * @param radialType (input) either InterpolatorType::LINEAR, or InterpolatorType::CUBIC_SPLINE
	 * @param weights (output) map from pointIndex to weight.  The sum of the weights will equal
	 * the length of the ray path in km.
	 */
	void getWeights(const vector<double*>& rayPath,
			const vector<double>& radii,
			const vector<int>& layerIds,
			const GeoTessInterpolatorType& horizontalType,
			const GeoTessInterpolatorType& radialType,
			map<int, double>& weights);

	/**
	 * Compute the weights on each model point that results from interpolating positions
	 * along the specified ray path.  The following procedure is implemented:
	 * <ol>
	 * <li>find the midpoint of each increment of the path (line segment between
	 * adjacent positions on the path).
	 * <li>find the interpolation coefficients of all the model points that
	 * are 'touched' by the midpoint of the increment.
	 * <li>find the length of the path increment in km.
	 * <li>find the product of the length of the path increment times each
	 * interpolation coefficient and sum that value for each model point.
	 * </ol>
	 * @param rayPath (input) an ordered list of unit vectors that define a ray path through the model space
	 * @param radii (input) an ordered list of the radii in km of each point that defines the ray path
	 * @param layerIds (input) array of layer indices that specify the index of the layer
	 * in which path increment i resides where i is the path increment between points
	 * i and i+1 that define the ray path.  If layerIds is empty, or layerIds[i] is less than zero, then the
	 * layer in which the path increment resides will be determined from the position and radius of the midpoint.
	 * @param numPoints the number of points that define the raypath
	 * @param horizontalType (input) either InterpolatorType::LINEAR, or InterpolatorType::NATURAL_NEIGHBOR
	 * @param radialType (input) either InterpolatorType::LINEAR, or InterpolatorType::CUBIC_SPLINE
	 * @param weights (output) map from pointIndex to weight.  The sum of the weights will equal
	 * the length of the ray path in km. Input map will be cleared before population.
	 */
	void getWeights(double** rayPath, double* radii, int* layerIds, const int& numPoints,
			const GeoTessInterpolatorType& horizontalType,
			const GeoTessInterpolatorType& radialType,
			map<int, double>& weights);

	/**
	 * Compute the path integral of the specified attribute along the specified rayPath.
	 * The ray path is comprised of a bunch of pointIndex -> interpolation coefficient
	 * pairs computed by one of the getWeights() functions.
	 *
	 * @param attribute the index of the attribute that is to be integrated.  If a value
	 * greater than or equal to zero is specified then the path integral of the specified
	 * attribute is returned.  Otherwise the function returns the total length of the rayPath in km.
	 * @param weights (input) map from pointIndex to weight.  The sum of the weights will equal
	 * the length of the ray path in km.
	 * @return the path integral.
	 */
	double getPathIntegral(const int& attribute, const map<int, double>& weights);

	/**
	 * Compute the path integral of the specified attribute along the specified rayPath.
	 * The following procedure is implemented:
	 * <ol>
	 * <li>find the midpoint of each increment of the path (line segment between
	 * adjacent positions on the path).
	 * <li>find the straight line distance between the two points, in km.
	 * <li>calculate the interpolated value of the specified attribute at the
	 * center of the path increment.
	 * <li>sum the length of the path increment times the attribute value, along the path.
	 * </ol>
	 *
	 * @param attribute the index of the attribute that is to be integrated.  If a value
	 * less than zero is specified then only the length of the path increments is summed
	 * and the function returns the total length of the rayPath in km.
	 * @param rayPath input array of 3-component unit vectors that define points
	 * along the ray path.
	 * @param radii input array of radius values, in km, that define
	 * the radius of each unit_vector supplied in 'rayPath'.
	 * @param layerIds input array of layer indices that specify the index of the layer
	 * in which path increment i resides where i is the path increment between points
	 * i and i+1 that define the ray path.  If layerIds is null or layerIds[i] is less than zero,
	 * then the layer in which the path increment resides will be determined from the position
	 * and radius of the midpoint.
	 * @param numPoints the number of elements in rayPath, radii, and layerIds
	 * @param horizontalType (input) the type of interpolator to use in the geographic
	 * dimensions, either LINEAR or NATURAL_NEIGHBOR
	 * @param radialType (input) the type of interpolator to use in the radial
	 * dimension, either LINEAR or CUBIC_SPLINE
	 * @param weights (optional) If specified, then weight is a map from an integer point index
	 * to the weight accrued by that point.
	 * @return the value of the path integral
	 */
	double getPathIntegral(const int& attribute,
			double** rayPath, double* radii, int* layerIds, const int& numPoints,
			const GeoTessInterpolatorType& horizontalType, const GeoTessInterpolatorType& radialType,
			map<int, double>* weights = NULL);

	/**
	 * Compute the path integral of the specified attribute along the specified rayPath.
	 * The following procedure is implemented:
	 * <ol>
	 * <li>find the midpoint of each increment of the path (line segment between
	 * adjacent positions on the path).
	 * <li>find the straight line distance between the two points, in km.
	 * <li>calculate the interpolated value of the specified attribute at the
	 * center of the path increment.
	 * <li>sum the length of the path increment times the attribute value, along the path.
	 * </ol>
	 *
	 * @param attribute the index of the attribute that is to be integrated.  If a value
	 * less than zero is specified then only the length of the path increments is summed
	 * and the function returns the total length of the rayPath in km.
	 * @param rayPath input array of 3-component unit vectors that define points
	 * along the ray path.
	 * @param radii input array of radius values, in km, that define
	 * the radius of each unit_vector supplied in 'rayPath'.
	 * @param layerIds input array of layer indices that specify the index of the layer
	 * in which path increment i resides where i is the path increment between points
	 * i and i+1 that define the ray path.  If layerIds is null or layerIds[i] is less than zero,
	 * then the layer in which the path increment resides will be determined from the position
	 * and radius of the midpoint.
	 * @param horizontalType (input) the type of interpolator to use in the geographic
	 * dimensions, either LINEAR or NATURAL_NEIGHBOR
	 * @param radialType (input) the type of interpolator to use in the radial
	 * dimension, either LINEAR or CUBIC_SPLINE
	 * @param weights (optional) If specified, then weight is a map from an integer point index
	 * to the weight accrued by that point.
	 * @return the value of the path integral
	 */
	double getPathIntegral(const int& attribute,
			const vector<double*>& rayPath, const vector<double>& radii, const vector<int>& layerIds,
			const GeoTessInterpolatorType& horizontalType, const GeoTessInterpolatorType& radialType,
			map<int, double>* weights = NULL);

	/**
	 * Compute the path integral of the specified attribute along the specified rayPath
	 * and optionally the weights on each model point that results from interpolating positions
	 * along the specified ray path.  If weights is null on input, no weights are computed.
	 * This method only applies to 2D GeoTessModels.
	 *
	 * <p>The following procedure is implemented:
	 * <ol>
	 * <li>divide the great circle path from firstPoint to lastPoint into nIntervals
	 * which each are of length less than or equal to pointSpacing.
	 * <li>multiply the length of the interval by the radius of the earth
	 * at the center of the interval, which converts the length of the interval into km.
	 * <li>interpolate the value of the specified attribute at the center of the path increment.
	 * <li>sum the length of the path increment times the attribute value, along the path.
	 * <li>find the interpolation coefficients of all the model points that
	 * are 'touched' by the midpoint of the increment.
	 * </ol>
	 *
	 * @param attribute index of the attribute to be integrated.  If a value
	 * less than zero is specified then only the length of the path increments is summed
	 * and the function returns the total length of the rayPath in km.
	 * @param firstPoint unit vector of the first point on the great circle path
	 * @param lastPoint unit vector of the last point on the great circle path
	 * @param pointSpacing maximum point separation in radians.  The actual point spacing
	 * will generally be slightly less than the specified value so that there will be an
	 * integral number of uniformly spaced points along the path.
	 * @param earthRadius the radius of the great circle path, in km.  If the value is less than or
	 * equal to zero then the radius of the Earth determined by the current EarthShape is used.
	 * See getEarthShape() and setEarathShape() for more information about EarthShapes.
	 * @param horizontalType either InterpolatorType.NATURAL_NEIGHBOR or InterpolatorType.LINEAR.
	 * @param weights (optional) If specified, then weight is a map from an integer point index
	 * to the weight accrued by that point.
	 * @return attribute value integrated along the specified great circle path.
	 * @throws GeoTessException if the model is not a 2D model.
	 */
	double getPathIntegral2D(const int& attribute,
			const double* firstPoint, const double* lastPoint, double pointSpacing,
			double earthRadius, const GeoTessInterpolatorType& horizontalType,
			map<int, double>* weights = NULL);

	/**
	 * Compute the path integral of the specified attribute along the specified rayPath
	 * and optionally the weights on each model point that results from interpolating positions
	 * along the specified ray path.  If weights is null on input, no weights are computed.
	 * This method only applies to 2D GeoTessModels.
	 *
	 * <p>The following procedure is implemented:
	 * <ol>
	 * <li>divide the great circle path from firstPoint to lastPoint into nIntervals
	 * which each are of length less than or equal to pointSpacing.
	 * <li>multiply the length of the interval by the radius of the earth
	 * at the center of the interval, which converts the length of the interval into km.
	 * <li>interpolate the value of the specified attribute at the center of the path increment.
	 * <li>sum the length of the path increment times the attribute value, along the path.
	 * <li>find the interpolation coefficients of all the model points that
	 * are 'touched' by the midpoint of the increment.
	 * </ol>
	 *
	 * @param attribute index of the attribute to be integrated.  If a value
	 * less than zero is specified then only the length of the path increments is summed
	 * and the function returns the total length of the rayPath in km.
	 * @param greatCircle a GreatCircle object specifying the path to be integrated.
	 * @param pointSpacing maximum point separation in radians.  The actual point spacing
	 * will generally be slightly less than the specified value so that there will be an
	 * integral number of uniformly spaced points along the path.
	 * @param earthRadius the radius of the great circle path, in km.  If the value is less than or
	 * equal to zero then the radius of the Earth determined by the current EarthShape is used.
	 * See getEarthShape() and setEarathShape() for more information about EarthShapes.
	 * @param horizontalType either InterpolatorType.NATURAL_NEIGHBOR or InterpolatorType.LINEAR.
	 * @param weights (optional) If specified, then weight is a map from an integer point index
	 * to the weight accrued by that point.
	 * @return attribute value integrated along the specified great circle path.
	 * @throws GeoTessException if the model is not a 2D model.
	 */
	double getPathIntegral2D(const int& attribute,
			GeoTessGreatCircle& greatCircle, double pointSpacing,
			double earthRadius, const GeoTessInterpolatorType& horizontalType,
			map<int, double>* weights = NULL);

	/// @cond PROTECTED  Turn off doxygen documentation until 'endcond' is found

	// the following constructors are all deprecated because GeoTessOptimization is no longer used.
	// GeoTess is always optimized for speed.
	GeoTessModel(const GeoTessOptimizationType* optimization);
	GeoTessModel(const string& inputFile, const string& relativeGridPath, const GeoTessOptimizationType* optimization);
	GeoTessModel(const string& modelInputFile, const GeoTessOptimizationType* optimization);
	GeoTessModel(vector<int>& attributeFilter, const GeoTessOptimizationType* optimization);
	GeoTessModel(const string& inputFile, const string& relativeGridPath,
			vector<int>& attributeFilter, const GeoTessOptimizationType* optimization);
 	GeoTessModel(const string& modelInputFile, vector<int>& attributeFilter,
			const GeoTessOptimizationType* optimization);

//	// The following getWeight() and getPathIntegral() functions are deprecated.
//	// They were included in GeoTessModel v2.1.1  but were reconfigured in v2.2.0.
//	// They are included here for backward compatibility, but will not appear in the documentation.
//	void getWeights(const vector<double*>& rayPath, const vector<double>& radii,
//			const GeoTessInterpolatorType& ht, const GeoTessInterpolatorType& rt, map<int, double>& weights)
//	{ vector<int> layerIds; getWeights(rayPath, radii, layerIds, ht, rt, weights);}
//
//	void getWeights(double** rayPath, double* radii, const int& numPoints,
//			const GeoTessInterpolatorType& ht,const GeoTessInterpolatorType& rt,map<int, double>& weights)
//	{ getWeights(rayPath, radii, NULL, numPoints, ht, rt, weights);}
//
//	double getPathIntegral(const int& attribute, const bool& reciprocal, double** rayPath, double* radii, const int& numPoints,
//			const GeoTessInterpolatorType& ht, const GeoTessInterpolatorType& rt)
//	{ return getPathIntegral(attribute, reciprocal, rayPath, radii, NULL, numPoints, ht, rt); }
//
//	double getPathIntegral(const int& attribute, const bool& reciprocal, double** rayPath, double* radii, const int& numPoints,
//			const GeoTessInterpolatorType& ht, const GeoTessInterpolatorType& rt, map<int, double>& weights)
//	{ return getPathIntegral(attribute, reciprocal, rayPath, radii, NULL, numPoints, ht, rt, &weights); }
//
//	double getPathIntegral2D(const int& attribute, const bool& reciprocal,
//			const double* firstPoint, const double* lastPoint, const double& pointSpacing,
//			const double& earthRadius,
//			const GeoTessInterpolatorType& horizontalType, map<int, double>& weights)
//	{ return getPathIntegral2D(attribute, reciprocal, firstPoint, lastPoint, pointSpacing, earthRadius, horizontalType, &weights); }


	/**
	 * Returns true if the input format version is supported.
	 */
	bool isSupportedFormatVersion(int frmtVrsn) { return (frmtVrsn == 1) ? true : false; }

	/**
	 * Test the array of profiles at each vertex to ensure that the top of one layer and the bottom
	 * of the layer above it have the same radii, within 0.01 km. If they do not, throw an
	 * exception.
	 */
	bool testLayerRadii();


	///@endcond

};
// end class GeoTessModel

}// end namespace geotess

#endif  // GEOTESSMODEL_OBJECT_H
