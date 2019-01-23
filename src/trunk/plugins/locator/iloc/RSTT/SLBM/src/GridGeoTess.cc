/*
 * GridGeoTess.cc
 *
 *  Created on: Sep 25, 2012
 *      Author: sballar
 */

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

#include "GridGeoTess.h"
#include "GridProfileGeoTess.h"
#include "GeoTessGrid.h"
#include "CPPUtils.h"
#include "GeoTessProfile.h"


namespace slbm {

GridGeoTess::GridGeoTess() : Grid(), model(NULL), position(NULL)
{
}  // END GridGeoTess Default Constructor


GridGeoTess::~GridGeoTess()
{
	clear();
}

void GridGeoTess::clear()
{
	Grid::clear();

	if (model != NULL)
	{
		delete model;
		model = NULL;
	}

	if (position != NULL)
	{
		delete position;
		position = NULL;
	}

}

size_t GridGeoTess::memSize()
{
	return sizeof(*model);
}

void GridGeoTess::loadFromFile(const string& filename)
{
	clear();

	modelPath = filename;

	model = new GeoTessModelSLBM(modelPath, uncertainty);
	position = model->getPosition(GeoTessInterpolatorType::NATURAL_NEIGHBOR);

	profiles.resize(model->getNVertices());
	Location location;
	for (int nodeId=0; nodeId<model->getNVertices(); ++nodeId)
	{
		location.setLocation(model->getGrid().getVertex(nodeId), 0.);
		profiles[nodeId] = new GridProfileGeoTess(*this, nodeId, location);
	}
}

void GridGeoTess::loadFromDirectory(const string& dirname)
{
	clear();

	modelPath = dirname;

	geotess::CPPUtils::addPathSeparator(modelPath);

	string path = geotess::CPPUtils::insertPathSeparator(modelPath, "geotessmodel");

	model = new GeoTessModelSLBM(path, "../tess", uncertainty);

	// now load the uncertainties from files in modelPath.
	// 4 phases (Pn, Sn, Pg, Lg) and 3 attributes (TT, SH, AZ)
	// azimuth uncertainty will always be null.
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
		os << endl << "ERROR in GridGeoTess::loadFromDirectory()" << endl
				<<"Uncertainty file does not exist:"<< endl
				<< fname << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),104);
	}


	position = model->getPosition(GeoTessInterpolatorType::NATURAL_NEIGHBOR);

	profiles.resize(model->getNVertices());
	Location location;
	for (int nodeId=0; nodeId<model->getNVertices(); ++nodeId)
	{
		location.setLocation(model->getGrid().getVertex(nodeId), 0.);
		profiles[nodeId] = new GridProfileGeoTess(*this, nodeId, location);
	}
}

void GridGeoTess::loadFromDataBuffer(util::DataBuffer& input)
{
	//clear();

	string s;

	// get the GeoTess name (GEOTESSMODEL) and validate

	input.readString(s, 12);
	if (s != "GEOTESSMODEL")
	{
		ostringstream os;
		os << endl << "GridGeoTess::loadFromDataBuffer(util::DataBuffer& input)" << endl
				<< "  expected char array \"GEOTESSMODEL\" as first entry of file "
				<< "but found \"";
		for (int i = 0; i < 12; ++i)
			if ((s[i] != 127) && (s[i] > 31))
				os << s[i];
			else
				os << "[" << (int) s[i] << "]";
		os << "\" instead ..." << endl;

		throw GeoTessException(os, __FILE__, __LINE__, 6021);
	}

	// get the fileFormatVersion. Only recognized value right now is 1.

	int fileFormatVersion = input.readInt32();
	//	if ((fileFormatVersion < 0) || (fileFormatVersion > 65536))
	//	{
	//		input.setByteOrderReverse(!input.isByteOrderReversalOn());
	//		input.decrementPos(CPPUtils::SINT);
	//		fileFormatVersion = input.readInt();
	//	}

	if (fileFormatVersion != 1)
	{
		ostringstream os;
		os << endl << "GridGeoTess::loadFromDataBuffer(util::DataBuffer& input)" << endl
				<< fileFormatVersion << " is not a recognized file format version." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6022);
	}

	GeoTessMetaData* md = new GeoTessMetaData();

	s = input.readString();
	md->setOptimizationType(s);

	// read model population software version and file creation date
	s = input.readString();
	md->setModelSoftwareVersion(s);

	s = input.readString();
	md->setModelGenerationDate(s);

	s = input.readString();
	md->setDescription(s);

	s = input.readString();
	md->setAttributes(s, input.readString());

	s = input.readString();
	md->setLayerNames(s);

	s = input.readString();
	md->setDataType(s);

	md->setNVertices(input.readInt32());

	// an array of length nLayers where each element is the
	// index of the tessellation that supports that layer.
	vector<int> tessIds;
	for (int i = 0; i < md->getNLayers(); ++i)
		tessIds.push_back(input.readInt32());
	md->setLayerTessIds(tessIds);

	// done populating GeoTessMetaData.

	GeoTessProfile*** gtProfiles = CPPUtils::new2DArray<GeoTessProfile*>(md->getNVertices(), md->getNLayers());

	vector<float> radii;
	int nAttributes = md->getNAttributes();
	float* velocities = new float[nAttributes];
	vector<GeoTessData*> data;

	for (int v=0; v<md->getNVertices(); ++v)
	{
		for (int layer=0; layer<md->getNLayers(); ++layer)
		{
			// read ProfileType and return new Profile object.
			int profileType = input.readByte();
			switch (profileType)
			{
			case 0:
			{
				// EMPTY layer defined by two radii and no data
				radii.push_back(input.readFloat());
				radii.push_back(input.readFloat());
				break;
			}
			case 1:
			{
				// THIN layer defined by one radius and one data
				radii.push_back(input.readFloat());
				for (int a=0; a<nAttributes; ++a)
					velocities[a] = input.readFloat();
				data.push_back(GeoTessData::getData(velocities, nAttributes));
				break;
			}
			case 2:
			{
				// CONSTANT layer defined by two radii and one data object
				radii.push_back(input.readFloat());
				radii.push_back(input.readFloat());
				for (int a=0; a<nAttributes; ++a)
					velocities[a] = input.readFloat();
				data.push_back(GeoTessData::getData(velocities, nAttributes));
				break;
			}
			case 3:
			{
				// NPOINT layer with 2 or more radii and one data object for each
				// radius
				for (int n=0; n<input.readInt32(); ++n)
				{
					radii.push_back(input.readFloat());
					for (int a=0; a<nAttributes; ++a)
						velocities[a] = input.readFloat();
					data.push_back(GeoTessData::getData(velocities, nAttributes));
				}
				break;
			}
			case 4:
			{
				// SURFACE layer with 0 radii and one data object
				for (int a=0; a<nAttributes; ++a)
					velocities[a] = input.readFloat();
				data.push_back(GeoTessData::getData(velocities, nAttributes));
				break;
			}
			case 5:
			{
				// SURFACE_EMPTY layer with 0 radii and 0 data objects
				break;
			}
			default:
			{
				ostringstream os;
				os << endl << "ERROR in GridGeoTess::loadFromDataBuffer(util::DataBuffer& input)" << endl
						<< "Unrecognized ProfileType " << profileType << endl;
				throw GeoTessException(os, __FILE__, __LINE__, 4003);
				break;
			}
			}
			gtProfiles[v][layer] = GeoTessProfile::newProfile(radii, data);
			radii.clear();
			data.clear();
		}

	}

	delete[] velocities;

	s = input.readString();
	if (s != "*")
	{
		ostringstream os;
		os << endl << "GridGeoTess::loadFromDataBuffer(util::DataBuffer& input)" << endl
				<< "Expected '*' but found " << s << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6022);
	}


	string gridId = input.readString();

	input.readString(s, 11);
	if (s != "GEOTESSGRID")
	{
		ostringstream os;
		os << endl << "GridGeoTess::loadFromDataBuffer(util::DataBuffer& input)" << endl
				<< "Expected buffer to start with 'GEOTESSGRID' but found " << s << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6022);
	}

	fileFormatVersion = input.readInt32();
	if (fileFormatVersion != 2)
	{
		ostringstream os;
		os << endl << "GridGeoTess::loadFromDataBuffer(util::DataBuffer& input)" << endl
				<< fileFormatVersion << " is not a recognized GeoTessGrid file format version." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6022);
	}


	string gridInputFile = input.readString();
	string gridOutputFile = input.readString();
	string gridSoftwareVersion = input.readString();
	string gridGenerationDate = input.readString();
	s = input.readString(); // repeat of gridId

	int nTessellations = input.readInt32();
	int nLevels = input.readInt32();
	int nTriangles = input.readInt32();
	int nVertices = input.readInt32();

	int** tessellations = CPPUtils::new2DArray<int>(nTessellations, 2);
	input.readIntArray(tessellations[0], nTessellations*2);

	int** levels = CPPUtils::new2DArray<int>(nLevels, 2);
	input.readIntArray(levels[0], nLevels*2);

	double** vertices = CPPUtils::new2DArray<double>(nVertices, 3);
	input.readDoubleArray(vertices[0], nVertices*3);

	int** triangles = CPPUtils::new2DArray<int>(nTriangles, 3);
	input.readIntArray(triangles[0], nTriangles*3);

	GeoTessGrid* grid = new GeoTessGrid(
			vertices, nVertices,
			triangles, nTriangles,
			levels, nLevels,
			tessellations, nTessellations,
			gridId,
			gridInputFile,
			gridOutputFile,
			gridSoftwareVersion,
			gridGenerationDate);


	s = input.readString();
	if (s != "SLBM")
	{
		ostringstream os;
		os << endl << "ERROR in GridGeoTess::loadFromDataBuffer(util::DataBuffer& input)" << endl <<
				"Expecting string SLBM but found " << s << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),105);
	}

	fileFormatVersion = input.readInt32();
	if (fileFormatVersion != 1)
	{
		ostringstream os;
		os << endl << "ERROR in GridGeoTess::loadFromDataBuffer(util::DataBuffer& input)" << endl <<
				"Expecting fileFormatVersion = 1 but found " << fileFormatVersion << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),105);
	}

	double averageMantleVelocity[2];
	averageMantleVelocity[0] = (double)input.readFloat();
	averageMantleVelocity[1] = (double)input.readFloat();

	// now load the uncertainties from buffer.
	int np = input.readInt32();
	int na = input.readInt32();
	if (na > 0 && np > 0)
	{
		if (na != 3 || np != 4)
		{
			ostringstream os;
			os << endl << "ERROR in GridGeoTess::loadFromDataBuffer(util::DataBuffer& input)" << endl <<
					"Expecting uncertainty information for 3 attributes and 4 phases." << endl
					<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
			throw SLBMException(os.str(),105);
		}
		for (int i=0; i<np; i++)
			for (int j=0; j<na; ++j)
			{
				int iphase = Uncertainty::getPhase(input.readString());
				int iattribute = Uncertainty::getAttribute(input.readString());
				if (uncertainty[iphase][iattribute] != NULL)
					delete uncertainty[iphase][iattribute];
				uncertainty[iphase][iattribute] = Uncertainty::getUncertainty(input, iphase, iattribute);
			}

	}

	model = new GeoTessModelSLBM(grid, md, uncertainty, averageMantleVelocity);

	// it is possible that some other grid object with the same gridId has already been loaded by GeoTessModel
	// and is already stored in reuseGrid.  If true then the current grid was not adopted by the new
	// GeoTessModel and this new grid should be deleted.
	if (grid->isNotReferenced()) delete grid;

	for (int v=0; v<md->getNVertices(); ++v)
		for (int layer=0; layer<md->getNLayers(); ++layer)
			model->setProfile(v,layer, gtProfiles[v][layer]);

	CPPUtils::delete2DArray(gtProfiles);

	position = model->getPosition(GeoTessInterpolatorType::NATURAL_NEIGHBOR);

	profiles.resize(model->getNVertices());
	Location location;
	for (int nodeId=0; nodeId<model->getNVertices(); ++nodeId)
	{
		location.setLocation(model->getGrid().getVertex(nodeId), 0.);
		profiles[nodeId] = new GridProfileGeoTess(*this, nodeId, location);
	}
}

void GridGeoTess::saveVelocityModel(util::DataBuffer& buffer)
{
	model->writeModelDataBuffer(buffer);
}

void GridGeoTess::saveVelocityModel(const string& destination, const int& format)
{
	if (destination == model->getMetaData().getInputModelFile())
	{
		ostringstream os;
		os << endl << "ERROR in GridSLBM::saveVelocityModel" << endl
				<<"Output file name cannot equal input file name."<< endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),102);
	}

	if (format == 4)
	{
		model->setIOUncertainty(true);
		model->writeModel(destination, "*");
	}
	else if (format == 3)
	{
		if (!is_directory(destination))
			mkdir(destination.c_str() PGL_MKDIR_OPTIONS);

		string filename = CPPUtils::insertPathSeparator(destination, "geotessmodel");
		string tessDir = CPPUtils::insertPathSeparator(destination, "../tess");

		if (!is_directory(tessDir))
			mkdir(tessDir.c_str() PGL_MKDIR_OPTIONS);

		string gridfile = CPPUtils::insertPathSeparator(tessDir,model->getGrid().getGridID());

		// see if the grid file exists.  if not, write it.
		ifstream fin;
		fin.open(gridfile.c_str());
		if (!fin.fail())
			fin.close();
		else
		{
			model->getGrid().writeGrid(gridfile);
		}


		model->setIOUncertainty(false);
		model->writeModel(filename, model->getGrid().getGridID());

		for (int i=0; i<(int)uncertainty.size(); ++i)
			for (int j=0; j<(int)uncertainty[i].size(); ++j)
				if (uncertainty[i][j])
					uncertainty[i][j]->writeFile(destination);
	}
	else if (format == 2)
	{
		if (!is_directory(destination))
			mkdir(destination.c_str() PGL_MKDIR_OPTIONS);

		util::DataBuffer buffer;

		//***NOTE reverse byte order on little endian machines
		if(!util::MD50::isBigEndian())
			buffer.setByteOrderReverse(true);

		//******************************************************
		//write Tessellation file
		//******************************************************
		GeoTessGrid& grid = model->getGrid();

		int tessId = grid.getNTessellations()-1; // this will equal 0
		int level = grid.getNLevels(tessId)-1; // will equal 6 for 1 degree tessellation

		int nTriangles = model->getGrid().getNTriangles(tessId, level);

		buffer.reserve(profiles.size()*2*sizeof(double)
		+nTriangles*3*sizeof(int) + 1000);

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

		buffer.writeInt32(nTriangles);

		for (int i=grid.getFirstTriangle(tessId, level); i<=grid.getLastTriangle(tessId, level); i++)
			for (int j=0; j<3; j++)
				buffer.writeInt32(grid.getTriangleVertexIndex(i, j));

		buffer.writeInt32(0); // no additional metadata

		string gridId = buffer.generateDataBufMD5HashKey();

		string tessDir = CPPUtils::insertPathSeparator(destination, "..");
		tessDir = CPPUtils::insertPathSeparator(tessDir, "tess");

		if (!is_directory(tessDir))
			mkdir(tessDir.c_str() PGL_MKDIR_OPTIONS);

		string filename = CPPUtils::insertPathSeparator(tessDir, gridId);

		writeBufferToFile(buffer, filename);

		buffer.clear();

		//******************************************************
		//write node adjacency file
		//******************************************************
		buffer.reserve(profiles.size()*7*sizeof(int));

		buffer.writeString(gridId);

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

		filename = filename + "_adjacency";

		writeBufferToFile(buffer, filename);

		buffer.clear();

		// ******************************************************
		// write GeoStack file
		// ******************************************************
		buffer.reserve(profiles.size()*3*NLAYERS*4 + 1000);

		buffer.writeInt32(profiles.size());

		buffer.writeFloat((float)model->getAverageMantleVelocity(PWAVE));
		buffer.writeFloat((float)model->getAverageMantleVelocity(SWAVE));

		double z;

		for (int i=0; i<(int)profiles.size(); i++)
		{
			for (int k=1; k<NLAYERS; k++)
			{
				if (k != 6)
				{
					// write out the depth below surface of solid earth, which
					// is stored in layer index 1. (index 0 is the top of water).
					z = profiles[i]->getInterfaceRadius(1)-profiles[i]->getInterfaceRadius(k);
					buffer.writeFloat((float)z);
				}
				buffer.writeFloat((float)profiles[i]->getVelocity(PWAVE, k));
				buffer.writeFloat((float)profiles[i]->getVelocity(SWAVE, k));
			}
			buffer.writeFloat((float)profiles[i]->getMantleGradient(PWAVE));
			buffer.writeFloat((float)profiles[i]->getMantleGradient(SWAVE));
		}

		filename = CPPUtils::insertPathSeparator(destination, "geostacks");

		writeBufferToFile(buffer, filename);

		buffer.clear();

		// ******************************************************
		// write Connectivity file
		// ******************************************************
		buffer.reserve(profiles.size()*12 + 1000);

		buffer.writeString(gridId);

		buffer.writeInt32(profiles.size());

		for (int i=0; i<(int)profiles.size(); i++)
		{
			buffer.writeInt32(i);
			// write out the elevation in km of the surface of the solid earth.
			// Elevation is relative to ellipsoid.
			buffer.writeFloat((float)(-profiles[i]->getInterfaceDepth(1)));
			buffer.writeFloat((float)profiles[i]->getWaterThick());
		}

		filename = CPPUtils::insertPathSeparator(destination, "connectivity");

		writeBufferToFile(buffer, filename);

		buffer.clear();

		for (int i=0; i<(int)uncertainty.size(); ++i)
			for (int j=0; j<(int)uncertainty[i].size(); ++j)
				if (uncertainty[i][j])
					uncertainty[i][j]->writeFile(destination);

	}
}

string GridGeoTess::toString()
{
	ostringstream os;
	os << model->toString() << endl;
	for (int i=0; i<(int)uncertainty.size(); ++i)
		for (int j=0; j<(int)uncertainty[i].size(); ++j)
			if (uncertainty[i][j] != NULL)
				os << uncertainty[i][j]->toStringTable();
	return os.str();
}

bool GridGeoTess::findProfile(Location& location,
		vector<GridProfile*>& neighbors, vector<int>& nodeIds,
		vector<double>& coefficients)
{
	position->set(location.getUnitVector(), location.getRadius());

	int n = position->getNVertices();

	neighbors.resize(n);
	nodeIds.resize(n);
	coefficients.resize(n);
	for (int i=0; i<n; ++i)
	{
		nodeIds[i] = position->getVertex(i);
		neighbors[i] = profiles[nodeIds[i]];
		coefficients[i] = position->getHorizontalCoefficient(i);
	}

	return true;
}

} /* namespace slbm */
