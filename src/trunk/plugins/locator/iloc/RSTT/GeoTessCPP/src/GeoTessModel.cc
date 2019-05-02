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

#include <sstream>
#include <cmath>
#include <iomanip>

// **** _LOCAL INCLUDES_ *******************************************************

#include "GeoTessModel.h"
#include "GeoTessPosition.h"
#include "GeoTessPositionLinear.h"
#include "GeoTessPositionNaturalNeighbor.h"
#include "GeoTessException.h"
#include "IFStreamAscii.h"
#include "IFStreamBinary.h"
#include "GeoTessGrid.h"
#include "GeoTessMetaData.h"
#include "GeoTessProfile.h"
#include "GeoTessProfileEmpty.h"
#include "CpuTimer.h"
#include "EarthShape.h"

#include "GeoTessPosition.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess
{

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

map<string, GeoTessGrid*> GeoTessModel::reuseGridMap;

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

GeoTessModel::GeoTessModel()
: grid(NULL), profiles(NULL), metaData(NULL), pointMap(NULL)
{
	metaData = new GeoTessMetaData();
	metaData->addReference();
}

GeoTessModel::GeoTessModel(const string& inputFile)
: grid(NULL), profiles(NULL), metaData(NULL), pointMap(NULL)
{
	metaData = new GeoTessMetaData();
	metaData->addReference();

	loadModel(inputFile, ".");
}

GeoTessModel::GeoTessModel(const string& inputFile, const string& relativeGridPath)
: grid(NULL), profiles(NULL), metaData(NULL), pointMap(NULL)
{
	metaData = new GeoTessMetaData();
	metaData->addReference();

	loadModel(inputFile, relativeGridPath);
}

GeoTessModel::GeoTessModel(vector<int>& attributeFilter)
: grid(NULL), profiles(NULL), metaData(NULL), pointMap(NULL)
{
	metaData = new GeoTessMetaData();
	metaData->addReference();
	metaData->setAttributeFilter(attributeFilter);
}

GeoTessModel::GeoTessModel(const string& inputFile, vector<int>& attributeFilter)
: grid(NULL), profiles(NULL), metaData(NULL), pointMap(NULL)
{
	metaData = new GeoTessMetaData();
	metaData->addReference();

	metaData->setAttributeFilter(attributeFilter);

	loadModel(inputFile, ".");
}

GeoTessModel::GeoTessModel(const string& inputFile, const string& relativeGridPath,
		vector<int>& attributeFilter)
: grid(NULL), profiles(NULL), metaData(NULL), pointMap(NULL)
{
	metaData = new GeoTessMetaData();
	metaData->addReference();
	metaData->setAttributeFilter(attributeFilter);

	loadModel(inputFile, relativeGridPath);
}


// DEPRECATED because GeoTessOptimizationType is always SPEED.
GeoTessModel::GeoTessModel(const GeoTessOptimizationType* optimization)
: grid(NULL), profiles(NULL), metaData(NULL), pointMap(NULL)
{
	metaData = new GeoTessMetaData();
	metaData->addReference();
}

// DEPRECATED because GeoTessOptimizationType is always SPEED.
GeoTessModel::GeoTessModel(const string& inputFile, const GeoTessOptimizationType* optimization)
: grid(NULL), profiles(NULL), metaData(NULL), pointMap(NULL)
{
	metaData = new GeoTessMetaData();
	metaData->addReference();

	loadModel(inputFile, ".");
}

// DEPRECATED because GeoTessOptimizationType is always SPEED.
GeoTessModel::GeoTessModel(const string& inputFile, const string& relativeGridPath,
		const GeoTessOptimizationType* optimization)
: grid(NULL), profiles(NULL), metaData(NULL), pointMap(NULL)
{
	metaData = new GeoTessMetaData();
	metaData->addReference();

	loadModel(inputFile, relativeGridPath);
}

// DEPRECATED because GeoTessOptimizationType is always SPEED.
GeoTessModel::GeoTessModel(vector<int>& attributeFilter, const GeoTessOptimizationType* optimization)
: grid(NULL), profiles(NULL), metaData(NULL), pointMap(NULL)
{
	metaData = new GeoTessMetaData();
	metaData->addReference();
	metaData->setAttributeFilter(attributeFilter);
}

// DEPRECATED because GeoTessOptimizationType is always SPEED.
GeoTessModel::GeoTessModel(const string& inputFile, vector<int>& attributeFilter, const GeoTessOptimizationType* optimization)
: grid(NULL), profiles(NULL), metaData(NULL), pointMap(NULL)
{
	metaData = new GeoTessMetaData();
	metaData->addReference();

	metaData->setAttributeFilter(attributeFilter);

	loadModel(inputFile, ".");
}

// DEPRECATED because GeoTessOptimizationType is always SPEED.
GeoTessModel::GeoTessModel(const string& inputFile, const string& relativeGridPath,
		vector<int>& attributeFilter, const GeoTessOptimizationType* optimization)
: grid(NULL), profiles(NULL), metaData(NULL), pointMap(NULL)
{
	metaData = new GeoTessMetaData();
	metaData->addReference();
	metaData->setAttributeFilter(attributeFilter);

	loadModel(inputFile, relativeGridPath);
}

GeoTessModel::GeoTessModel(const string& gridFileName, GeoTessMetaData* md)
		: grid(NULL), profiles(NULL), metaData(NULL), pointMap(NULL)
{
	constructor(gridFileName, NULL, md);
}

GeoTessModel::GeoTessModel(GeoTessGrid* grd, GeoTessMetaData* md)
		: grid(NULL), profiles(NULL), metaData(NULL), pointMap(NULL)
{
	constructor("", grd, md);
}

// call this private "constructor" with either:
// 1) gridFileName.length()  > 0 and grd == NULL, or
// 2) gridFileName.length() == 0 and grd != null
//
// the two real constructors both call this so that lots
// of initialization will happen identically regardless of
// which real constructor was called.
void GeoTessModel::constructor(const string& gridFileName, GeoTessGrid* grd, GeoTessMetaData* md)
{
	// check meta data and add a reference

	metaData = md;
	metaData->checkComplete();
	metaData->addReference();

	// create / assign grid ... if reuseGrids is true then check map for existence of grid
	// add a reference when done

	if (gridFileName.length() > 0)
	{
		// called with gridFile name, not an actual grid object
		if (metaData->isGridReuseOn())
		{
			// see if grid exists in reuse map

			string gridID = GeoTessGrid::getGridID(gridFileName);
			map<string, GeoTessGrid*>::iterator it = reuseGridMap.find(gridID);
			if (it == reuseGridMap.end())
			{
				// map does not contain grid ... create grid and add to map

				grid = new GeoTessGrid();
				grid->loadGrid(gridFileName);
				reuseGridMap[gridID] = grid;
			}
			else
				// map contains grid ... assign it from map
				grid = it->second;
		}
		else // reuse grids flag is not on ... create the grid and assign
		{
			grid = new GeoTessGrid();
			grid->loadGrid(gridFileName);
		}
	}
	else
	{
		// called with grid object, not a file name.
		if (metaData->isGridReuseOn())
		{
			// see if grid exists in reuse map

			map<string, GeoTessGrid*>::iterator it = reuseGridMap.find(grd->getGridID());
			if (it == reuseGridMap.end())
			{
				// map does not contain grid ... set reference
				grid = grd;
			}
			else
			{
				// map contains grid ... assign it from map
				grid = it->second;
			}
		}
		else // reuse grids flag is not on ... create the grid and assign
		{
			grid = grd;
		}

	}

	grid->addReference();

	metaData->setNVertices(grid->getNVertices());

	metaData->setInputGridFile(grid->getGridInputFile());


	// create profile array and exit

	profiles = CPPUtils::new2DArray<GeoTessProfile*>(grid->getNVertices(),
			metaData->getNLayers());
	for (int i = 0; i < grid->getNVertices(); ++i)
		CPPUtils::resetArray<GeoTessProfile*>(metaData->getNLayers(), profiles[i],
				(GeoTessProfile*) NULL);

	pointMap = new GeoTessPointMap(*this);

}


/**
 * Destructor.
 */
GeoTessModel::~GeoTessModel()
{
	if (pointMap != NULL)
		delete pointMap;

	deleteProfiles();

	// delete the grid if it is not referenced

	if (grid != NULL)
	{
		grid->removeReference();
		if (grid->isNotReferenced())
		{
			// see if the grid is contained in the reuse map ... if it is identically
			// (i.e. the grid id matches and the pointer match) then remove the grid
			// from the map. It is possible that the grid id matches but that this was
			// a different instance of grid created before the grid reuse flag
			// (reuseGrids) was turned on or by using the constructor where a grid
			// instance is passed in but it is different than the one stored in the
			// reuse map (different instance but same grid). If that is true then
			// simply delete this grid.

			map<string, GeoTessGrid*>::iterator it = reuseGridMap.find(
					grid->getGridID());
			if ((it != reuseGridMap.end()) && (it->second == grid))
				reuseGridMap.erase(it);

			// delete the grid

			delete grid;
			grid = NULL;
		}
	}

	// delete the metadata if it is no longer referenced

	if (metaData != NULL)
	{
		metaData->removeReference();
		if (metaData->isNotReferenced())
		{
			delete metaData;
			metaData = NULL;
		}
	}

}

/**
 * Test a file to see if it is a GeoTessModel file.
 *
 * @param inputFile
 * @return true if inputFile is a GeoTessModel file.
 */
bool GeoTessModel::isGeoTessModel(const string& fileName)
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
		ifs.readCharArray(line, 12);
	}

	return (line == "GEOTESSMODEL");
}

/**
 * Deletes the profiles array if it has been allocated
 */
void GeoTessModel::deleteProfiles()
{
	if (profiles != NULL)
	{
		for (int i = 0; i < grid->getNVertices(); ++i)
			for (int j = 0; j < metaData->getNLayers(); ++j)
				delete profiles[i][j];

		CPPUtils::delete2DArray<GeoTessProfile*>(profiles);
		profiles = NULL;
	}
}

/**
 * Retrieve a GeoTessPosition object configured to interpolate data from the
 * input model using either LINEAR or NATURAL_NEIGHBOR interpolation.
 */
GeoTessPosition* GeoTessModel::getPosition()
{
	return getPosition(GeoTessInterpolatorType::LINEAR, GeoTessInterpolatorType::LINEAR);
}

/**
 * Retrieve a GeoTessPosition object configured to interpolate data from the
 * input model using either LINEAR or NATURAL_NEIGHBOR interpolation.
 */
GeoTessPosition* GeoTessModel::getPosition(const GeoTessInterpolatorType& horizontalType)
{
	return getPosition(horizontalType,
			horizontalType == GeoTessInterpolatorType::LINEAR ? GeoTessInterpolatorType::LINEAR
					: GeoTessInterpolatorType::CUBIC_SPLINE);
}

/**
 * Retrieve a GeoTessPosition object configured to interpolate data from the
 * input model using either LINEAR or NATURAL_NEIGHBOR interpolation.
 */
GeoTessPosition* GeoTessModel::getPosition(const GeoTessInterpolatorType& horizontalType,
		const GeoTessInterpolatorType& radialType)
{
	switch (horizontalType.ordinal())
	{
	case 0: // LINEAR
		return new GeoTessPositionLinear(this, radialType);
	case 1: // NATURAL_NEIGHBOR
		return new GeoTessPositionNaturalNeighbor(this, radialType);
	default:
		ostringstream os;
		os << endl << "ERROR in Interpolator::getInterpolator" << endl
				<< "Unsupported InterpolatorType " << horizontalType.name() << endl
				<< "Must specify either LINEAR or NATURAL_NEIGHBOR." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 1003);
	}
}

/**
 * Return true if this and the input other model are equal. i.e., their grids have the same
 * gridIDs, they have the same number of layers, and all their Profiles are equal. For
 * profiles to be equal, they must be of the same type and size, and all of their radii
 * must be equal and all of their data must be equal.
 */
bool GeoTessModel::operator ==(const GeoTessModel& m) const
{
	if (!(*grid == *(m.grid)))
		return false;
	for (int i = 0; i < grid->getNVertices(); ++i)
		for (int j = 0; j < metaData->getNLayers(); ++j)
			if (!(*profiles[i][j] == *m.profiles[i][j]))
				return false;

	return true;
}

/**
 * Test the array of profiles at each vertex to ensure that the top of one layer and the bottom
 * of the layer above it have the same radii, within 0.01 km. If they do not, throw an
 * exception.
 *
 * @param profiles
 * @throws GeoTessException
 */
bool GeoTessModel::testLayerRadii()
{
	for (int layer=0; layer < metaData->getNLayers(); ++layer)
		for (int vertex=0; vertex < grid->getNVertices(); ++vertex)
			if (!profiles[vertex][layer])
			{
				ostringstream os;
				os << endl << "ERROR in GeoTessModel::testLayerRadii" << endl
						<< "Profile at vertex " << vertex << " layer " << layer << " is NULL." << endl
						<< "Be sure to call one of the GeoTessModel::setProfile() methods for each vertex and layer in the model." << endl;
				throw GeoTessException(os, __FILE__, __LINE__, 1004);
			}

	bool isSurface = profiles[0][0]->getType() == GeoTessProfileType::SURFACE
			|| profiles[0][0]->getType() == GeoTessProfileType::SURFACE_EMPTY;

	if (isSurface && getNLayers() != 1)
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessModel::testLayerRadii" << endl
				<< "Model comprised of profiles of type ProfileSurface must have exactly 1 layer." << endl
				<< "This model contains layers " << metaData->getLayerNamesString() << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 1004);
	}

	for (int i = 0; i < grid->getNVertices(); ++i)
	{
		GeoTessProfile** p = profiles[i];

		for (int layer = 0; layer < metaData->getNLayers(); ++layer)
			if ((p[0]->getType() == GeoTessProfileType::SURFACE ||
					p[0]->getType() == GeoTessProfileType::SURFACE_EMPTY) != isSurface)
		{
			ostringstream os;
			os << endl << "ERROR in GeoTessModel::testLayerRadii" << endl
					<< "Model may not contain a mix of ProfileSurface profiles and profiles of other types." << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 1004);
		}

		for (int j = 1; j < metaData->getNLayers(); ++j)
		{
			if (abs(p[j]->getRadiusBottom() - p[j - 1]->getRadiusTop()) > 0.01)
			{
				ostringstream os;
				os << endl << "ERROR in GeoTessModel::testLayerRadii" << endl
						<< "At vertex " << i
						<< " the radius at the top of layer " << (j - 1)
						<< " is "
						<< CPPUtils::ftos(p[j - 1]->getRadiusTop(), "%1.3f")
						<< " and the radius at the bottom of layer " << j
						<< " is "
						<< CPPUtils::ftos(p[j]->getRadiusBottom(), "%1.3f")
						<< ".  They differ by "
						<< CPPUtils::ftos(
								p[j]->getRadiusBottom()
										- p[j - 1]->getRadiusTop(), "%1.3f")
						<< "." << endl;
				throw GeoTessException(os, __FILE__, __LINE__, 1004);
			}
		}

		// check to ensure that radiusBottom <= radiusTop
		for (int j = 0; j < metaData->getNLayers(); ++j)
			if (p[j]->getNRadii() > 1
					&& p[j]->getRadiusBottom() > p[j]->getRadiusTop())
			{
				ostringstream os;
				os << endl << "ERROR in GeoTessModel::testLayerRadii" << endl
						<< "radiusBottom > radiusTop" << endl
						<< "radiusTop    = " << p[j]->getRadiusTop() << endl
						<< "radiusBottom = " << p[j]->getRadiusTop() << endl;
				throw GeoTessException(os, __FILE__, __LINE__, 1005);
			}
	}
	return true;
}

/**
 * Replace the Profile object at the specified vertex and layer with a new
 * one.
 *
 * @param vertex
 * @param layer
 * @param profile
 * @throws GeoTessException
 */
void GeoTessModel::setProfile(int vertex, int layer, GeoTessProfile* profile)
{
	// ensure that radiusBottom <= radiusTop

	if (profile->getType() != GeoTessProfileType::SURFACE
			&& (profile->getRadiusBottom() > profile->getRadiusTop()))
	{
		ostringstream os;
		os << fixed << setprecision(6);
		os << endl << "ERROR in GeoTessModel::setProfile()" << endl
				<< "radiusBottom > radiusTop" << endl
				<< "radiusBottom = " << setw(11) << profile->getRadiusBottom() << endl
				<< "radiusTop    = " << setw(11) << profile->getRadiusTop() << endl
				<< "vertex = " << vertex
				<< "  layer = " << layer << endl << "type = "
				<< profile->getType().toString() << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 1006);
	}

	// if type is NPOINT, ensure that nRadii >= 2 and nRadii == nData

	if (profile->getType() == GeoTessProfileType::NPOINT
			&& ((profile->getNRadii() < 2)
					|| (profile->getNRadii() != profile->getNData())))
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessModel::setProfile()" << endl
				<< "Profile type is NPOINT, nRadii = " << profile->getNRadii()
				<< " and nData = " << profile->getNData() << endl
				<< "nRadii must equal nData, and both must be >= 2 for type NPOINT"
				<< endl << "vertex = " << vertex << "  layer = " << layer
				<< endl << "type = " << profile->getType().toString() << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 1007);
	}

	for (int i=0; i<profile->getNData(); ++i)
		if (profile->getData(i)->getDataType() != metaData->getDataType())
		{
			ostringstream os;
			os << endl << "ERROR in GeoTessModel::setProfile()" << endl
					<< "The model has been set to accept data of type "
					<< metaData->getDataType().name() << endl
					<< "but a profile has been provided that contains Data of type "
					<< profile->getData(i)->getDataType().name() << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 1008);
		}

	if (profiles[vertex][layer] != NULL)
		delete profiles[vertex][layer];
	profiles[vertex][layer] = profile;
}

/**
 * To string method.
 */
string GeoTessModel::toString()
{
	ostringstream os;

	if (grid)
	{
		os << endl<< "********************************************************************************"<< endl;

		os << metaData->toString(class_name(), getMemory()) << endl;

		if (profiles != NULL && profiles[0][0] != NULL)
		{
			vector< vector<int> > pCount;
			profileCount(pCount);

			vector<int> layerCount;
			getLayerCount(layerCount, false);



			os << "Layer  connected   number    profile   profile    profile  profile  profile  profile" << endl;
			os << "Index  vertices   of points  npoints   constant    thin     empty   surface surfemtpy" << endl;
			os << "-----  --------   ---------  -------   --------   ------   -------  -------  -------" << endl;
			for (int i = getNLayers() - 1; i >= 0; --i)
			{
				const set<int>& connectedVertices = grid->getVertexIndicesTopLevel(metaData->getTessellation(i));
				os << setw(4) << i
						<< setw(9) << connectedVertices.size()
						<< " " << setw(12) << layerCount[i] <<
						" " << setw(9) << pCount[i][3]  <<
						" " << setw(10) << pCount[i][2] <<
						" " << setw(8) << pCount[i][1] <<
						" " << setw(9) << pCount[i][0] <<
						" " << setw(7) << pCount[i][4] <<
						" " << setw(7) << pCount[i][5] <<
						endl;
			}

			os << "-----  --------   ---------  -------   --------   ------   -------  -------  -------" << endl;
			os << "Total "
					<< setw(7) << grid->getNVertices()
					<< " " << setw(12) << getPointMap()->size()
					<< " " << setw(9) << pCount[getNLayers()][3] <<
					" " << setw(10) << pCount[getNLayers()][2] <<
					" " << setw(8) << pCount[getNLayers()][1] <<
					" " << setw(9) << pCount[getNLayers()][0] <<
					" " << setw(7) << pCount[getNLayers()][4] <<
					" " << setw(7) << pCount[getNLayers()][5] <<
					endl;

			os << endl;
		}

		os << grid->toString() << endl;

		os << "********************************************************************************" << endl;
	}
	else
		os
		<< "No model information is available \nbecause model has not been loaded from file.\n";

	return os.str();
}

/**
 * Read model data and grid from a file.
 *
 * @param inputFile
 * @return a reference to this.
 * @throws GeoTessException
 */
GeoTessModel* GeoTessModel::loadModel(const string& inputFile,
		const string& relGridFilePath)
{
	CpuTimer timr;
	metaData->setInputModelFile(inputFile);

	if (inputFile.find(".ascii", inputFile.length() - 6) != string::npos)
		loadModelAscii(inputFile, relGridFilePath);
	else
		loadModelBinary(inputFile, relGridFilePath);

	metaData->setLoadTimeModel(timr.realTime() * 1e-3);

	pointMap = new GeoTessPointMap(*this);

	return this;
}

/**
 * Load a model (3D grid and data) from a binary File.
 * <p>
 * The format of the file is: <br>
 * int fileFormatVersion (currently only recognizes 1). <br>
 * String gridFile: either *, or relative path to gridFile. <br>
 * int nVertices, nLayers, nAttributes, dataType(DOUBLE or FLOAT). <br>
 * int[] tessellations = new int[nLayers]; <br>
 * Profile[nVertices][nLayers]: data
 *
 * @param model a reference to the Model3D into which the data is to be stored.
 * @param inputFile
 * @throws GeoTessException
 */
void GeoTessModel::loadModelBinary(const string& inputFile,
		const string& relGridFilePath)
{
	IFStreamBinary ifs(inputFile);
	ifs.setBoundaryAlignment(false);
	ifs.resetPos();

	size_t fp = inputFile.find_last_of(CPPUtils::FILE_SEP);
	string inputDirectory = "";
	if (fp != string::npos)
		inputDirectory = inputFile.substr(0, fp);
	loadModelBinary(ifs, inputDirectory, relGridFilePath);
}

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
 * @param model a reference to the Model3D into which the data is to be stored.
 * @param inputFile
 * @throws GeoTessException
 */
void GeoTessModel::loadModelAscii(const string& inputFile,
		const string& relGridFilePath)
{
	IFStreamAscii input;
	input.openForRead(inputFile);

	size_t fp = inputFile.find_last_of(CPPUtils::FILE_SEP);
	string inputDirectory = "";
	if (fp != string::npos)
		inputDirectory = inputFile.substr(0, fp);
	loadModelAscii(input, inputDirectory, relGridFilePath);

	input.close();
}

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
 * @param model a reference to the Model3D into which the data is to be stored.
 * @param input
 * @throws GeoTessException
 */
void GeoTessModel::loadModelAscii(IFStreamAscii& input,
		const string& inputDirectory, const string& relGridFilePath)
{
	metaData->loadMetaData(input);

	// loop over all the vertices of the 2D grid and load the data

	deleteProfiles();
	profiles = CPPUtils::new2DArray<GeoTessProfile*>(metaData->getNVertices(),
			metaData->getNLayers());
	for (int i = 0; i < metaData->getNVertices(); ++i)
		for (int j = 0; j < metaData->getNLayers(); ++j)
			profiles[i][j] = GeoTessProfile::newProfile(input, *metaData);

	// read the name of the gridFile

	string gridFileName;
	input.readLine(gridFileName);
	metaData->setInputGridFile(gridFileName);

	// read the gridID from the model

	string gridID;
	input.readLine(gridID);

	// finish with extra data and grid assignment ... and build point map

	loadGrid<IFStreamAscii>(input, inputDirectory, relGridFilePath,
			gridFileName, gridID, "loadModelAscii");
}

/**
 * Load a model (3D grid and data) from a binary File.
 * <p>
 * The format of the file is: <br>
 * int fileFormatVersion (currently only recognizes 1). <br>
 * String gridFile: either *, or relative path to gridFile. <br>
 * int nVertices, nLayers, nAttributes, dataType(DOUBLE or FLOAT). <br>
 * int[] tessellations = new int[nLayers]; <br>
 * Profile[nVertices][nLayers]: data
 *
 * @param model a reference to the Model3D into which the data is to be stored.
 * @param input
 * @throws GeoTessException
 */
void GeoTessModel::loadModelBinary(IFStreamBinary& input,
		const string& inputDirectory, const string& relGridFilePath)
{
	metaData->loadMetaData(input);

	// loop over all the vertices of the 2D grid and load the data

	deleteProfiles();
	profiles = CPPUtils::new2DArray<GeoTessProfile*>(metaData->getNVertices(),
			metaData->getNLayers());
	for (int i = 0; i < metaData->getNVertices(); ++i)
		for (int j = 0; j < metaData->getNLayers(); ++j)
			profiles[i][j] = GeoTessProfile::newProfile(input, *metaData);

	// read the name of the gridFile

	string inputGridFile;
	input.readString(inputGridFile);
	metaData->setInputGridFile(inputGridFile);

	// read the gridID from the model file.

	string gridID;
	input.readString(gridID);

	// finish with extra data and grid assignment ... and build point map

	loadGrid<IFStreamBinary>(input, inputDirectory, relGridFilePath,
			inputGridFile, gridID, "loadModelBinary");
}

/**
 * Write the model to file. The data (radii and attribute values) are written
 * to the outputFile. If grdFileName is '*' then the grid information is
 * written to the same file as the data. If gridFileName is something else, it
 * should be the relative path from outputFIle tot he file that contains the
 * grid information. In the latter case, the gridFile referenced by gridFileName
 * is not overwritten; all that happens is that the relative path to grid file
 * is stored in the data file.
 *
 * @param outputFile the File to receive model data.
 * @param gridFileName name of the file to receive model grid.
 * @throws GeoTessException
 */
void GeoTessModel::writeModel(const string& outputFile,
		const string& gridFileName)
{
	testLayerRadii();

	CpuTimer timr;

	string gfName = gridFileName;

	if (gfName == "null" || gfName == "NULL")
		gfName = "*";


	if (CPPUtils::trim(gfName).length() == 0)
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessModel::writeModel" << endl
				<< "Cannot write the model to an empty string file name ..."
				<< endl
				<< "Must specify the name of an existing geotess grid file (no path), or '*'."
				<< endl
				<< "If '*' is specified, then grid info is written to the same file as the model data."
				<< endl;
		throw GeoTessException(os, __FILE__, __LINE__, 1009);
	}

	if (outputFile.find(".ascii", outputFile.length() - 6) != string::npos)
		writeModelAscii(outputFile, gfName);
	else
		writeModelBinary(outputFile, gfName);

	metaData->setWriteTimeModel(timr.realTime() * 1e-3);
	metaData->setOutputModelFile(outputFile);
}

/**
 * Write the model currently in memory to a binary file. A model can be stored with the data and
 * grid in the same or separate files. This method will write the data from the 3D model to the
 * specified outputfile. If the supplied gridFileName is the single character "*", then the grid
 * information is written to the same file as the data. If the gridFileName is anything else, it
 * is assumed to be the relative path from the data file to an existing file where the grid
 * information is stored. In the latter case, the grid information is not actually written to
 * the specified file; all that happens is that the relative path is stored in the data file.
 *
 * @param model the model containing the data and grid that is to be written to the outputFile.
 * @param outputFile the name of the file to which the data should be written.
 * @param gridFileName either "*" or the relative path from the new data file to the file that
 *            contains the grid definition.
 */
void GeoTessModel::writeModelBinary(const string& outputFile,
		const string& gridFileName)
{
	IFStreamBinary ofs;
	if (!CPPUtils::isBigEndian())
		ofs.byteOrderReverseOn();
	ofs.boundaryAlignmentOff();

	writeModelBinary(ofs, gridFileName);
	ofs.writeToFile(outputFile);
}

/**
 * Write the model currently in memory to a binary file. A model can be stored with the data and
 * grid in the same or separate files. This method will write the data from the 3D model to the
 * specified outputfile. If the supplied gridFileName is the single character "*", then the grid
 * information is written to the same file as the data. If the gridFileName is anything else, it
 * is assumed to be the relative path from the data file to an existing file where the grid
 * information is stored. In the latter case, the grid information is not actually written to
 * the specified file; all that happens is that the relative path is stored in the data file.
 *
 * @param model the model containing the data and grid that is to be written to the outputFile.
 * @param output the OutputStream to which the data should be written.
 * @param gridFileName either "*" or the relative path from the new data file to the file that
 *            contains the grid definition.
 */
void GeoTessModel::writeModelBinary(IFStreamBinary& output,
		const string& gridFileName)
{
	// write out file type identifier ("GEOTESSMODEL"), format version,
	// code version, and data stamp

	output.writeCharArray("GEOTESSMODEL", 12);

	int format = getEarthShape().getShapeName() == "WGS84" ? 1 : 2;

	output.writeInt(format);

	output.writeString(metaData->getModelSoftwareVersion());
	output.writeString(metaData->getModelGenerationDate());

	if (format > 1)
		output.writeString(getEarthShape().getShapeName());

	output.writeString(metaData->getDescription());

	output.writeString(metaData->getAttributeNamesString());
	output.writeString(metaData->getAttributeUnitsString());
	output.writeString(metaData->getLayerNamesString());

	output.writeString(metaData->getDataType().toString());
	output.writeInt(grid->getNVertices());

	// tessellation ids
	for (int i = 0; i < metaData->getNLayers(); ++i)
		output.writeInt(metaData->getTessellation(i));

	for (int i = 0; i < grid->getNVertices(); ++i)
		for (int j = 0; j < metaData->getNLayers(); ++j)
			profiles[i][j]->write(output);

	output.writeString(gridFileName);
	output.writeString(grid->getGridID());

	if (gridFileName == "*")
		grid->writeGridBinary(output);
}

/**
 * Write the model currently in memory to an ascii file. A model can be stored with the data and
 * grid in the same or separate files. This method will write the data from the 3D model to the
 * specified outputfile. If the supplied gridFileName is the single character "*", then the grid
 * information is written to the same file as the data. If the gridFileName is anything else, it
 * is assumed to be the relative path from the data file to an existing file where the grid
 * information is stored. In the latter case, the grid information is not actually written to
 * the specified file; all that happens is that the relative path is stored in the data file.
 *
 * @param model the model containing the data and grid that is to be written to the outputFile.
 * @param outputFile the name of the file to which the data should be written.
 * @param gridFileName either "*" or the relative path from the new data file to the file that
 *            contains the grid definition.
 */
void GeoTessModel::writeModelAscii(const string& outputFile,
		const string& gridFileName)
{
	IFStreamAscii ofs;
	ofs.openForWrite(outputFile);
	writeModelAscii(ofs, gridFileName);
	ofs.close();
}

/**
 * Write the model currently in memory to an ascii file. A model can be stored with the data and
 * grid in the same or separate files. This method will write the data from the 3D model to the
 * specified outputfile. If the supplied gridFileName is the single character "*", then the grid
 * information is written to the same file as the data. If the gridFileName is anything else, it
 * is assumed to be the relative path from the data file to an existing file where the grid
 * information is stored. In the latter case, the grid information is not actually written to
 * the specified file; all that happens is that the relative path is stored in the data file.
 *
 * @param model the model containing the data and grid that is to be written to the outputFile.
 * @param output the name of the file to which the data should be written.
 * @param gridFileName either "*" or the relative path from the new data file to the file that
 *            contains the grid definition.
 */
void GeoTessModel::writeModelAscii(IFStreamAscii& output,
		const string& gridFileName)
{
	// write out file type identifier ("GEOTESSMODEL"), format version,
	// code version, and data stamp

	output.writeStringNL("GEOTESSMODEL");

	string format = getEarthShape().getShapeName() == "WGS84" ? "1" : "2";

	output.writeStringNL(format);

	output.writeStringNL(metaData->getModelSoftwareVersion());
	output.writeStringNL(metaData->getModelGenerationDate());

	if (format != "1")
		output.writeStringNL(getEarthShape().getShapeName());

	output.writeStringNL("<model_description>");
	output.writeStringNL(metaData->getDescription());
	output.writeStringNL("</model_description>");

	output.writeStringNL("attributes: " + metaData->getAttributeNamesString());
	output.writeStringNL("units: " + metaData->getAttributeUnitsString());
	output.writeStringNL("layers: " + metaData->getLayerNamesString());
	output.writeStringNL(metaData->getDataType().toString());

	output.writeIntNL(grid->getNVertices());

	for (int i = 0; i < metaData->getNLayers(); ++i)
	{
		output.writeString(" ");
		output.writeInt(metaData->getTessellation(i));
	}
	output.writeNL();

	for (int n = 0; n < grid->getNVertices(); ++n)
	{
		GeoTessProfile** prfs = profiles[n];
		for (int l = 0; l < metaData->getNLayers(); ++l)
			prfs[l]->write(output);
	}

	// write grid file name and grid id

	output.writeStringNL(gridFileName);
	output.writeStringNL(grid->getGridID());

	// write grid file if requested

	if (gridFileName == "*")
		grid->writeGridAscii(output);
}

void GeoTessModel::getWeights(GeoTessGreatCircle& greatCircle, const double& pointSpacing, const double& radius,
		const GeoTessInterpolatorType& horizontalType, map<int, double>& weights)
{
	weights.clear();

	if (!is2D())
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessModel::getWeights" << endl
				<< "Can only apply this method to 2D models." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 1003);

	}

	int nIntervals = (int) ceil(greatCircle.getDistance()/pointSpacing);

	if (nIntervals == 0) return;

	double u[3], r, delta = greatCircle.getDistance()/nIntervals;

	GeoTessPosition* pos = getPosition(horizontalType, GeoTessInterpolatorType::LINEAR);

	for (int i = 0; i < nIntervals; ++i)
	{
		greatCircle.getPoint((i+0.5)*delta, u);
		r = radius > 0. ? radius : getEarthShape().getEarthRadius(u);
		pos->set(0, u, r);
		pos->getWeights(weights, delta*r);
	}

	delete pos;
}

void GeoTessModel::getWeights(const double* pointA, const double* pointB, const double& pointSpacing, const double& radius,
		const GeoTessInterpolatorType& horizontalType, map<int, double>& weights)
{
	GeoTessGreatCircle greatCircle(pointA, pointB);
	getWeights(greatCircle, pointSpacing, radius, horizontalType,weights);
}

//void GeoTessModel::getWeights(const double* pointA, const double* pointB, const double& pointSpacing,
//		const int& layerIndex, const double& fractionalRadius,
//		const GeoTessInterpolatorType& horizontalType,
//		const GeoTessInterpolatorType& radialType,
//		map<int, double>& weights)
//{
//	weights.clear();
//
//	GeoTessGreatCircle greatCircle(pointA, pointB);
//
//	int nIntervals = (int) ceil(greatCircle.getDistance()/pointSpacing);
//
//	if (nIntervals == 0) return;
//
//	double delta = greatCircle.getDistance()/nIntervals;
//
//	GeoTessPosition* pos = getPosition(horizontalType, radialType);
//
//	bool top = fractionalRadius == 1.;
//	bool bottom = fractionalRadius == 0.;
//
//	double r1, r2, u1[3], u2[3], u[3];
//	u2[0] = pointA[0];
//	u2[1] = pointA[1];
//	u2[2] = pointA[2];
//
//	pos->set(layerIndex, u2, 6371.);
//	if (bottom)
//		r2 = pos->getRadiusBottom(layerIndex);
//	else if (top)
//		r2 = pos->getRadiusTop(layerIndex);
//	else
//		r2 = pos->getRadiusBottom(layerIndex)+pos->getLayerThickness(layerIndex)*fractionalRadius;
//
//	for (int i = 1; i <= nIntervals; ++i)
//	{
//		u1[0] = u2[0];
//		u1[1] = u2[1];
//		u1[2] = u2[2];
//		r1 = r2;
//
//		greatCircle.getPoint(i*delta, u2);
//		pos->set(layerIndex, u2, 6371.);
//		if (bottom)
//			r2 = pos->getRadiusBottom(layerIndex);
//		else if (top)
//			r2 = pos->getRadiusTop(layerIndex);
//		else
//			r2 = pos->getRadiusBottom(layerIndex)+pos->getLayerThickness(layerIndex)*fractionalRadius;
//
//		greatCircle.getPoint((i-0.5)*delta, u);
//		pos->set(layerIndex, u, (r1+r2)/2);
//
//		pos->getWeights(weights, GeoTessUtils::getDistance3D(u1, r1, u2, r2));
//	}
//
//	delete pos;
//}

void GeoTessModel::getWeights(const vector<double*>& rayPath,
		const vector<double>& radii,
		const vector<int>& layerIds,
		const GeoTessInterpolatorType& horizontalType,
		const GeoTessInterpolatorType& radialType,
		map<int, double>& weights)
{
	// ensure that pointMap has been instantiated.
	getPointMap();

	weights.clear();
	GeoTessPosition* pos = getPosition(horizontalType, radialType);

	double center[3];
	double *vi, ri, *vj=rayPath[0], rj=radii[0];

	for (int i = 1; i < (int)rayPath.size(); ++i)
	{
		vi = vj;
		vj = rayPath[i];
		ri = rj;
		rj = radii[i];
		center[0] = vi[0] + vj[0];
		center[1] = vi[1] + vj[1];
		center[2] = vi[2] + vj[2];
		GeoTessUtils::normalize(center);
		pos->set(layerIds.empty() ? -1 : layerIds[i-1], center, (ri + rj) / 2.0);
		pos->getWeights(weights, GeoTessUtils::getDistance3D(vi, ri, vj, rj));
	}
	delete pos;
}

void GeoTessModel::getWeights(double** rayPath, double* radii, int* layerIds,
		const int& size,
		const GeoTessInterpolatorType& horizontalType,
		const GeoTessInterpolatorType& radialType,
		map<int, double>& weights)
{
	// ensure that pointMap has been instantiated.
	getPointMap();

	weights.clear();
	GeoTessPosition* pos = getPosition(horizontalType, radialType);

	double center[3];
	double *vi, ri, *vj=rayPath[0], rj=radii[0];

	for (int i = 1; i < size; ++i)
	{
		vi = vj;
		vj = rayPath[i];
		ri = rj;
		rj = radii[i];
		center[0] = vi[0] + vj[0];
		center[1] = vi[1] + vj[1];
		center[2] = vi[2] + vj[2];
		GeoTessUtils::normalize(center);
		pos->set(layerIds == NULL ? -1 : layerIds[i-1], center, (ri + rj) / 2.0);
		pos->getWeights(weights, GeoTessUtils::getDistance3D(vi, ri, vj, rj));
	}
	delete pos;
}

double GeoTessModel::getPathIntegral(const int& attribute, const map<int, double>& weights)
{
	getPointMap();
	double sum = 0;
	if (attribute < 0)
		for (map<int, double>::const_iterator it = weights.begin(); it != weights.end(); ++it)
			sum += (*it).second;
	else
		for (map<int, double>::const_iterator it = weights.begin(); it != weights.end(); ++it)
			if ((*it).first >= 0)
				sum += (*it).second * pointMap->getPointValue((*it).first, attribute);
	return sum;
}

double GeoTessModel::getPathIntegral(const int& attribute,
		double** rayPath, double* radii, int* layerIds, const int& size,
		const GeoTessInterpolatorType& horizontalType,
		const GeoTessInterpolatorType& radialType,
		map<int, double>* weights)
{
	if (weights) (*weights).clear();

	double integral = 0.;

	if (attribute < 0 && weights == NULL)
		for (int i = 1; i < size; ++i)
			integral += GeoTessUtils::getDistance3D(rayPath[i-1], radii[i-1], rayPath[i], radii[i]);
	else
	{
		GeoTessPosition* pos = getPosition(horizontalType, radialType);
		double center[3], dkm, *vi, ri, *vj=rayPath[0], rj=radii[0];
		for (int i = 1; i < size; ++i)
		{
			vi = vj;
			vj = rayPath[i];
			ri = rj;
			rj = radii[i];
			dkm = GeoTessUtils::getDistance3D(vi, ri, vj, rj);
			center[0] = vi[0] + vj[0];
			center[1] = vi[1] + vj[1];
			center[2] = vi[2] + vj[2];
			GeoTessUtils::normalize(center);
			pos->set(layerIds == NULL ? -1 : layerIds[i], center, (ri + rj) / 2.0);

			integral += attribute < 0 ? dkm : dkm * pos->getValue(attribute);

			if (weights) pos->getWeights(*weights, dkm);
		}
		delete pos;
	}
	return integral;
}

double GeoTessModel::getPathIntegral(const int& attribute,
		const vector<double*>& rayPath, const vector<double>& radii, const vector<int>& layerIds,
		const GeoTessInterpolatorType& horizontalType, const GeoTessInterpolatorType& radialType,
		map<int, double>* weights)
{
	if (weights) (*weights).clear();

	double integral = 0.;

	if (attribute < 0 && weights == NULL)
		for (int i = 1; i < (int)rayPath.size(); ++i)
			integral += GeoTessUtils::getDistance3D(rayPath[i-1], radii[i-1], rayPath[i], radii[i]);
	else
	{
		GeoTessPosition* pos = getPosition(horizontalType, radialType);
		double center[3], dkm, ri, rj=radii[0], *vi, *vj=rayPath[0];
		for (int i = 1; i < (int)rayPath.size(); ++i)
		{
			vi = vj;
			vj = rayPath[i];
			ri = rj;
			rj = radii[i];
			dkm = GeoTessUtils::getDistance3D(vi, ri, vj, rj);
			center[0] = vi[0] + vj[0];
			center[1] = vi[1] + vj[1];
			center[2] = vi[2] + vj[2];
			GeoTessUtils::normalize(center);
			pos->set(layerIds.empty() ? -1 : layerIds[i], center, (ri + rj) / 2.0);

			integral += attribute < 0 ? dkm : dkm * pos->getValue(attribute);

			if (weights) pos->getWeights(*weights, dkm);
		}
		delete pos;
	}
	return integral;
}

double GeoTessModel::getPathIntegral2D(const int& attribute,
		GeoTessGreatCircle& greatCircle, double pointSpacing,
		double earthRadius, const GeoTessInterpolatorType& horizontalType,
		map<int, double>* weights)
{
	if (weights) (*weights).clear();

	if (!is2D())
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessModel::getPathIntegral2D" << endl
				<< "Can only apply this method to 2D models." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 1003);

	}

	int nIntervals = (int) ceil(greatCircle.getDistance()/pointSpacing);

	if (nIntervals == 0) return 0.;

	double u[3], r, integral=0., delta = greatCircle.getDistance()/nIntervals;

	if (attribute < 0 && weights == NULL)
		for (int i = 0; i < nIntervals; ++i)
		{
			greatCircle.getPoint((i+0.5)*delta, u);
			r = earthRadius > 0. ? earthRadius : getEarthShape().getEarthRadius(u);
			integral += delta*r;
		}
	else
	{
		GeoTessPosition* pos = getPosition(horizontalType, GeoTessInterpolatorType::LINEAR);
		for (int i = 0; i < nIntervals; ++i)
		{
			greatCircle.getPoint((i+0.5)*delta, u);
			r = earthRadius > 0. ? earthRadius : getEarthShape().getEarthRadius(u);
			pos->set(0, u, r);
			integral += attribute < 0 ? delta*r : delta*r * pos->getValue(attribute);

			if (weights) pos->getWeights(*weights, delta*r);
		}
		delete pos;
	}
	return integral;
}

double GeoTessModel::getPathIntegral2D(const int& attribute,
		const double* firstPoint, const double* lastPoint, double pointSpacing,
		double earthRadius, const GeoTessInterpolatorType& horizontalType,
		map<int, double>* weights)
{
	GeoTessGreatCircle greatCircle(firstPoint, lastPoint);
	return getPathIntegral2D(attribute, greatCircle, pointSpacing, earthRadius,
			horizontalType, weights);
}

} // end namespace geotess
