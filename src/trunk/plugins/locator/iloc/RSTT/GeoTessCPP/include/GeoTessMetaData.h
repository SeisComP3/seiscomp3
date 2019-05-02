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

#ifndef GEOTESSMETADATA_OBJECT_H
#define GEOTESSMETADATA_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <set>
#include <list>
#include <sstream>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"
#include "GeoTessUtils.h"
#include "GeoTessDataType.h"
#include "GeoTessOptimizationType.h"
#include "GeoTessException.h"
#include "IFStreamBinary.h"
#include "IFStreamAscii.h"
#include "EarthShape.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess
{

// **** _FORWARD REFERENCES_ ***************************************************

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Basic metadata information about a GeoTessModel.
 *
 * GeoTessMetaData stores basic information about a GeoTessModel, including:
 * <ul>
 * <li> Description of the model.
 * <li> The names of the layers that comprise the model.
 * <li> Map from layer index to grid tessellation index.
 * <li> The names of the attributes supported by the model.
 * <li> The units of the attributes
 * <li> The DataType of the attributes (DOUBLE, FLOAT, LONG_INT, INT, SHORT, BYTE).
 * <li> The name and version number of the software that generated the model.
 * <li> The date that the model was generated.
 * <li> The computer platform upon which the code was compiled.
 * <li> The name of the file from which the model was loaded.
 * <li> The amount of time required to load the model from file.
 * </ul>
 * Each GeoTessModel has a single instance of MetaData that it passes around to
 * wherever the information is needed.
 *
 * @author Sandy Ballard
 *
 */
class GEOTESS_EXP_IMP GeoTessMetaData
{
private:
	
	EarthShape earthShape;

	/**
	 * A description of the contents of the model.
	 */
	string description;

	/**
	 * The number of layers in the model.
	 */
	int nLayers;

	/**
	 * The number of vertices in the 2D grid.
	 */
	int nVertices;

	/**
	 * The names of each layer in the model, in order from the deepest to shallowest layer
	 * (increasing radius).
	 */
	string* layerNames;

	/**
	 * An array of length nLayers where each element is the index of the tessellation that supports
	 * the corresponding layer. Tessellation indexes are managed by the grid object.
	 */
	int* layerTessIds;

	/**
	 * One of DOUBLE, FLOAT, LONG, INT, SHORTINT, BYTE; All of the data stored in this model is of
	 * this type.
	 */
	const GeoTessDataType* dataType;

	/**
	 * It is assumed that every Data object attached to a node has the same size, i.e., number of
	 * attributes. This value is stored and retrieved from the model files. Must equal the length of
	 * the attributeNames and attributeUnits arrays.
	 */
	int nAttributes;

	/**
	 * The names of the attributes stored in the model. The lengh of this array must equal
	 * nAttributes.
	 */
	string* attributeNames;

	/**
	 * The units that correspond to the attributes strored in the model. The length of this array
	 * must equal nAttributes.
	 */
	string* attributeUnits;

	/**
	 * If false, all the attributes in the model file are loaded into memory.  If true, then
	 * attributeFilter contains the map from attribute index in file to attributeIndex in
	 * memory.  This is only used during the process of loading a model from file.
	 */
	bool boolAttributeFilter;

	/**
	 * AttributeFilter is used to limit the number of attribute read from the model input file.
	 * It is an array of length equal to the number of attributes in the model file.  Elements
	 * that contain a value < 0 are not loaded into memory.  Element values >= 0 are the
	 * index of the attributes array in memory into which the model values will be loaded.
	 * This variable is populated from values in inputFilter, which is the entity actually
	 * specified by calling application.
	 */
	vector<int> attributeFilter;

	/**
	 * Array with of length equal to the number of attributes that will be loaded into memory.
	 * The elements are the indexes of the attributes in the model file that are to be loaded
	 * into memory.
	 */
	vector<int> inputFilter;

	/**
	 * If attribute filtering occurred during loading of the model from file, this string
	 * records some history about the mapping.  Used only in the metadata toString() function.
	 */
	string attributeFilterString;

	/**
	 * Name of the file from which model was loaded, or "none".
	 */
	string inputModelFile;

	/**
	 * Name of the file from with 2D grid was loaded, or "none".
	 */
	string inputGridFile;

	/**
	 * Time, in seconds, required to read the model, or -1.
	 */
	double loadTimeModel;

	/**
	 * Name of file to which the model was written, or "none".
	 */
	string outputModelFile;

	/**
	 * Name of file to which the 2D grid was written, or "none".
	 */
	string outputGridFile;

	/**
	 * Time in second required to write the model to a file, or -1 if model has not been written to
	 * a file.
	 */
	double writeTimeModel;

	/**
	 * Reference count.
	 */
	int refCount;

	/**
	 * If true grid reuse is turned on for the model using this meta data
	 */
	bool reuseGrids;

	/**
	 * Name and version number of the software that generated this model.
	 */
	string modelSoftwareVersion;

	/**
	 * The date when this model was generated. Not necessarily the same
	 * as the date that the model file was copied or translated.
	 */
	string modelGenerationDate;

public:

	/**
	 * Default constructor.
	 *
	 * <p>During construction of a GeoTessModel object,
	 * the following methods should be called to make the MetaData object
	 * complete.
	 * <ul>
	 * <li>setDescription()
	 * <li>setLayerNames()
	 * <li>setAttributes()
	 * <li>setDataType()
	 * <li>setLayerTessIds() (only required if grid has more than one
	 * multi-level tessellation)
	 * <li>setModelSoftwareVersion()
	 * <li>setModelGenerationDate()
	 * </ul>
	 */
	GeoTessMetaData()
			: earthShape(), description(""), nLayers(0), nVertices(0), layerNames(NULL), layerTessIds(
					NULL), dataType(&GeoTessDataType::NONE), nAttributes(-1), attributeNames(
					NULL), attributeUnits(NULL), boolAttributeFilter(false),
					inputModelFile("none"), inputGridFile("none"), loadTimeModel(-1.0),
					outputModelFile("none"), outputGridFile("none"), writeTimeModel(-1.0),
					refCount(0), reuseGrids(true), modelSoftwareVersion(""), modelGenerationDate("")
	{ }

	/**
	 * Load just the metaData object from a GeoTessModel file.
	 * @param fileName name of GeoTessModel from which to read meta data
	 */
	GeoTessMetaData(const string &fileName);

	/**
	 * Copy constructor.
	 * @param md reference to meta data object to copy.
	 */
	GeoTessMetaData(const GeoTessMetaData& md);

	/**
	 * Overloaded assignment operator
	 * @param other reference to meta data object to copy.
	 * @return reference to copy of other
	 */
	GeoTessMetaData& operator=(const GeoTessMetaData& other);

	/**
	 * Overloaded equality operator
	 * @param other reference to the other meta data object to which
	 * this meta data object is to be compared
	 * @return true if this and other are equal.
	 */
	bool operator==(const GeoTessMetaData& other);

	/**
	 * Overloaded inequality operator
	 * @param other reference to the other meta data object to which
	 * this meta data object is to be compared
	 * @return true if this and other are not equal.
	 */
	bool operator!=(const GeoTessMetaData& other)
	{
		return !(*this == other);
	}

	/**
	 * Returns the class name.
	 * @return class name
	 */
	static  string class_name() { return "GeoTessMetaData"; }

	/**
	 * Destructor.
	 */
	virtual ~GeoTessMetaData();

	LONG_INT getMemory()
	{
		LONG_INT memory = (LONG_INT) sizeof(GeoTessMetaData);

		// EarthShape earthShape;
		memory += (LONG_INT) sizeof(EarthShape);

		memory += (LONG_INT) (description.length() + attributeFilterString.length()
				+ inputModelFile.length() + inputGridFile.length() + outputModelFile.length()
				+ outputGridFile.length() + modelSoftwareVersion.length() + modelGenerationDate.length());

		// string* layerNames;
		memory += nLayers * (LONG_INT) sizeof(string);
		for (int i=0; i<nLayers; ++i)
			memory += (LONG_INT) layerNames[i].length();

		// int* layerTessIds;
		memory += nLayers * (LONG_INT)sizeof(int);

		// GeoTessDataType* dataType;
		memory += nLayers * (int)sizeof(GeoTessDataType);

		// string* attributeNames;
		// string* attributeUnits;
		if (nAttributes > 0)
		{
			memory += 2 * nAttributes * (LONG_INT) sizeof(string);
			for (int i=0; i<nAttributes; ++i)
			{
				memory += (LONG_INT) attributeNames[i].length();
				memory += (LONG_INT) attributeUnits[i].length();
			}
		}

		// vector<int> attributeFilter;
		memory += (LONG_INT) (attributeFilter.capacity() * sizeof(int));

		// vector<int> inputFilter;
		memory += (LONG_INT) (inputFilter.capacity() * sizeof(int));

		return memory;
	}

	/**
	 * Retrieve a deep copy of this GeoTessMetaData object.
	 * @return a pointer to a deep copy of this GeoTessMetaData object.
	 */
	GeoTessMetaData* copy()
	{
		GeoTessMetaData* cpy = new GeoTessMetaData(*this);
		return cpy;
	}

	/**
	 * Returns true if grid reuse is on.
	 * @return true if grid reuse is on.
	 */
	bool isGridReuseOn()
	{
		return reuseGrids;
	}

	/**
	 * Set grid reuse on or off.
	 *
	 * @param rg true turns grid reuse on.
	 */
	void setReuseGrids(bool rg)
	{
		reuseGrids = rg;
	}

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
	EarthShape& getEarthShape() { return earthShape; }

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
	void setEarthShape(const string& earthShapeName) { earthShape.setEarthShape(earthShapeName); }

	/**
	 * Retrieve the name of the file from which the model was loaded, or "none".
	 *
	 * @return the name of the file from which the model was loaded, or "none".
	 */
	const string& getInputModelFile() const
	{
		return inputModelFile;
	}
	;

	/**
	 * Retrieve the name of the file from which the grid was loaded, or "none".
	 *
	 * @return the name of the file from which the grid was loaded, or "none".
	 */
	const string& getInputGridFile() const
	{
		return inputGridFile;
	}

	/**
	 * Retrieve the amount of time, in seconds, required to load the model, or -1.
	 *
	 * @return the amount of time, in seconds, required to load the model, or -1.
	 */
	double getLoadTimeModel() const
	{
		return loadTimeModel;
	}

	/**
	 * Retrieve the name of the file to which the model was most recently written, or "none".
	 *
	 * @return the name of the file to which the model was most recently written, or "none".
	 */
	const string& getOutputModelFile() const
	{
		return outputModelFile;
	}
	;

	/**
	 * Retrieve the name of the file to which the grid was most recently written, or "none".
	 *
	 * @return the name of the file to which the grid was most recently written, or "none".
	 */
	const string& getOutputGridFile() const
	{
		return outputGridFile;
	}
	;

	/**
	 * Retrieve the amount of time, in seconds, required to write the model to file, or -1.
	 *
	 * @return the amount of time, in seconds, required to write the model to file, or -1.
	 */
	double getWriteTimeModel() const
	{
		return writeTimeModel;
	}
	;

	/**
	 * Retrieve the description of the model.
	 *
	 * @return the description of the model.
	 */
	const string& getDescription() const
	{
		return description;
	}
	;

	/**
	 * Set the description of the model.
	 *
	 * @param dscr  the description of the model.
	 */
	void setDescription(const string& dscr)
	{
		description = dscr;
		description = CPPUtils::stringReplaceAll("\r\n", "\n", description);
		description = CPPUtils::stringReplaceAll("\r", "\n", description);
		//description = CPPUtils::stringReplaceAll("\n", CPPUtils::NEWLINE, description);

		// make sure that description ends with a newline by removing
		// newline from the end if it exists and then adding it on.
		CPPUtils::removeEOL(description);
		description += CPPUtils::NEWLINE;
	}

	/**
	 * Specify the names of all the layers that comprise the model.  This will determine
	 * the value of nLayers as well. The input lyrNms is a semicolon concatenation of
	 * all layer names (i.e. LAYERNAME1; LAYERNAME2; ...).
	 * @param lyrNms single string containing all the layer names separated by semi-colons
	 */
	void setLayerNames(const string& lyrNms)
	{
		vector<string> layrNames;
		CPPUtils::tokenizeString(lyrNms, ";", layrNames);
		setLayerNames(layrNames);
	}

	/**
	 * Specify the names of all the layers that comprise the model.  This will determine
	 * the value of nLayers as well.
	 * @param layrNms the names of the layers that comprise the model.
	 */
	void setLayerNames(vector<string>& layrNms);

	/**
	 * Retrieve the number of vertices in the 2D grid.
	 *
	 * @return number of layers represented in the model.
	 */
	int getNVertices() const
	{
		return nVertices;
	}
	;

	/**
	 * Retrieve the number of layers represented in the model.
	 *
	 * @return number of layers represented in the model.
	 */
	int getNLayers() const
	{
		return nLayers;
	}
	;

	/**
	 * Retrieve the index of the layer that has the specified name, or -1.
	 * @param layerName the name of the layer whose index is sought.
	 * @return the index of the layer that has the specified name, or -1.
	 */
	int getLayerIndex(const string& layerName) const;

	/**
	 * Retrieve the names of the layers supported by the model.
	 * @param layers a vector of strings that will be cleared and populated with the
	 * layer names
	 */
	void getLayerNames(vector<string>& layers)
	{
		layers.clear();
		for (int i=0; i<nLayers; ++i)
			layers.push_back(layerNames[i]);
	}

	/**
	 * Retrieve the names of the layers supported by the model.
	 * There will be nLayers elements in the returned string array.
	 * @return a reference to the array of layer names
	 */
	const string* const getLayerNames()
	{
		return layerNames;
	}

	/**
	 * Retrieve the name of one of the layers supported by the model.
	 * @param layerIndex the index of the layer
	 * @return the name of the layer
	 */
	string getLayerName(const int& layerIndex)
	{
		if (layerIndex < 0 || layerIndex >= nLayers)
		{
			ostringstream os;
			os << endl << "ERROR in GeoTessMetaData::getLayerName(int layerIndex)" << endl
					<< "attributeIndex (" << layerIndex << ") is out of range (0-" << nLayers-1 << ")"
					<< endl;
			throw GeoTessException(os, __FILE__, __LINE__, 6001);
		}

		return layerNames[layerIndex];
	}

	/**
	 * Retrieve the names of all the layers assembled into a single,
	 * semi-colon separated string.
	 * @return the names of all the layers assembled into a single,
	 * semi-colon separated string.
	 */
	string getLayerNamesString();

	/**
	 * LayerTessIds is a map from a layer index to a tessellation index.  There is an element
	 * for each layer.  This method can only be called after the names of the layers have
	 * been specified with call to GeoTessMetaData::setLayerNames().
	 * <p>
	 * This method makes a copy of the supplied interger array.
	 * @param layrTsIds an int[] of length equal to the number of layers in the model.
	 */
	void setLayerTessIds(int layrTsIds[])
	{
		if (nLayers <= 0)
		{
			ostringstream os;
			os	 << "Cannot call GeoTessMetaData::setLayerTessIds() "
				<< "before calling GeoTessMetaData::setLayerNames()" << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 6002);
		}

		if (layerTessIds != NULL)
			delete[] layerTessIds;
		layerTessIds = new int[nLayers];
		for (int i=0; i<nLayers; ++i)
			layerTessIds[i] = layrTsIds[i];
	}

	/**
	 * LayerTessIds is a map from a layer index to a tessellation index.  There is an element
	 * for each layer.
	 * @param layrTsIds an vector<int> of length equal to the number of layers in the model.
	 */
	void setLayerTessIds(vector<int>& layrTsIds);

	/**
	 * Retrieve a reference to layerTessIds; an int[] with an entry for each layer specifying the
	 * index of the tessellation that supports that layer.
	 *
	 * @return a reference to layerTessIds
	 */
	const int* getLayerTessIds() const
	{
		return layerTessIds;
	}
	;

	/**
	 * Retrieve the index of the tessellation that supports the specified layer.
	 *
	 * @param layer
	 * @return the index of the tessellation that supports the specified layer.
	 */
	int getTessellation(int layer) const
	{
		return layerTessIds[layer];
	}

	/**
	 * Retrieve a list of all the layer indexes that are associated
	 * with a specific tessellation index.
	 * @param tessId tessellation index
	 * @param layers (output) a list of all the layer indexes that are associated
	 * with a specific tessellation index.
	 */
	void getLayers(const int& tessId, vector<int>& layers)
	{
		layers.clear();
		for (int i=0; i<nLayers; ++i)
			if (layerTessIds[i] == tessId)
				layers.push_back(i);
	}

	/**
	 * Retrieve the index of the first layer associated with the
	 * specified tessellation.
	 * @param tessId tessellation index
	 * @return the index of the first layer associated with the
	 * specified tessellation.
	 */
	int getFirstLayer(const int& tessId)
	{
		for (int i=0; i<nLayers; ++i)
			if (layerTessIds[i] == tessId)
				return i;
		return -1;
	}

	/**
	 * Retrieve the index of the last layer associated with the
	 * specified tessellation.
	 * @param tessId tessellation index
	 * @return the index of the last layer associated with the
	 * specified tessellation.
	 */
	int getLastLayer(const int& tessId)
	{
		for (int i=nLayers-1; i >= 0; --i)
			if (layerTessIds[i] == tessId)
				return i;
		return -1;
	}

	/**
	 * Return the type of all the data stored in the model; Will be one of DOUBLE, FLOAT, LONG, INT,
	 * SHORTINT, BYTE.
	 *
	 * @return the dataType
	 */
	const GeoTessDataType& getDataType() const
	{
		return *dataType;
	}

	/**
	 * Specify the type of the data that is stored in the model.
	 *
	 * @param dt the dataType to set
	 */
	void setDataType(const GeoTessDataType& dt);

	/**
	 * Specify the type of the data that is stored in the model; Must be one of DOUBLE, FLOAT, LONG,
	 * INT, SHORTINT, BYTE.
	 *
	 * @param dt the dataType to set
	 */
	void setDataType(const string& dt);

	/**
	 * Retrieve the names of the attributes supported by the model.
	 * @param attributes a vector of strings that will be cleared and
	 * populated with the names of the attributes
	 */
	void getAttributeNames(vector<string>& attributes)
	{
		attributes.clear();
		for (int i = 0; i < nAttributes; ++i)
			attributes.push_back(attributeNames[i]);
	}

	/**
	 * Retrieve the units of the attributes supported by the model.
	 * @param units a vector of strings that will be cleared and
	 * populated with the units of the attributes
	 */
	void getAttributeUnits(vector<string>& units)
	{
		units.clear();
		for (int i = 0; i < nAttributes; ++i)
			units.push_back(attributeUnits[i]);
	}

	/**
	 * Retrieve the names of the attributes supported by the model.
	 * There will be nAttributes elements in the returned string array.
	 * @return the names of the attributes supported by the model.
	 */
	const string* const getAttributeNames()
	{
		return attributeNames;
	}

	/**
	 * Retrieve the units of the attributes supported by the model.
	 * There will be nAttributes elements in the returned string array.
	 * @return the units of the attributes supported by the model.
	 */
	const string* const getAttributeUnits()
	{
		return attributeUnits;
	}

	/**
	 * Specify the names of all the layers that comprise the model.  This will determine
	 * the value of nLayers as well. The input lyrNms is a semicolon concatenation of
	 * all layer names (i.e. LAYERNAME1; LAYERNAME2; ...).
	 * @param nms the names of the attributes
	 * @param unts the units of the attributes
	 */
	void setAttributes(const string& nms, const string& unts)
	{
		vector<string> names, units;
		CPPUtils::tokenizeString(nms, ";", names);
		CPPUtils::tokenizeString(unts, ";", units);
		if (names.size() != units.size())
		{
			ostringstream os;
			os	 << "Error in GeoTessMetaData::setAttributes(const string& nms, const string& unts)" << endl
					<< "Attribute names size (" << names.size()
					<< ") is not equal to units size (" << units.size() << ")" << endl;
			names.clear(); units.clear();
			throw GeoTessException(os, __FILE__, __LINE__, 6009);
		}
		setAttributes(names, units);
	}

	/**
	 * Specify the names and units of the attributes that comprise the model.  The number
	 * names and units must be equal.
	 * @param names names of the attributes.
	 * @param units units of the attributes.
	 */
	void setAttributes(const vector<string>& names, const vector<string>& units);

	/**
	 * Retrieve the number of attributes supported by the model.
	 *
	 * @return the number of attributes supported by the model.
	 */
	int getNAttributes() const
	{
		return nAttributes;
	}

	/**
	 * Retrieve the name of the i'th attribute supported by the model.
	 *
	 * @param attributeIndex
	 * @return the name of the i'th attribute supported by the model.
	 */
	const string& getAttributeName(int attributeIndex) const
	{
		if (attributeIndex < 0 || attributeIndex >= nAttributes)
		{
			ostringstream os;
			os << endl << "ERROR in GeoTessMetaData::getAttributeName(int attributeIndex)" << endl
					<< "attributeIndex (" << attributeIndex << ") is out of range (0-" << nAttributes-1 << ")"
					<< endl;
			throw GeoTessException(os, __FILE__, __LINE__, 6003);
		}

		return attributeNames[attributeIndex];
	}

	/**
	 * Retrieve the index of the attribute that has the specified name.
	 * @param name
	 * @return the index of the attribute that has the specified name.
	 */
	int getAttributeIndex(string name);

	/**
	 * Retrieve the names of all the attributes assembled into a single,
	 * semi-colon separated string.
	 * @return the names of all the attributes assembled into a single,
	 * semi-colon separated string.
	 */
	string getAttributeNamesString() const;

	/**
	 * Retrieve the units of all the attributes assembled into a single,
	 * semi-colon separated string.
	 * @return the units of all the attributes assembled into a single,
	 * semi-colon separated string.
	 */
	string getAttributeUnitsString() const;

	/**
	 * Retrieve the units of the i'th attribute supported by the model.
	 *
	 * @param attributeIndex
	 * @return the units of the i'th attribute supported by the model.
	 */
	const string& getAttributeUnit(int attributeIndex) const
	{
		if (attributeIndex < 0 || attributeIndex >= nAttributes)
		{
			ostringstream os;
			os << endl << "ERROR in GeoTessMetaData::getAttributeUnit(int attributeIndex)" << endl
					<< "attributeIndex (" << attributeIndex << ") is out of range (0-" << nAttributes-1 << ")"
					<< endl;
			throw GeoTessException(os, __FILE__, __LINE__, 6004);
		}
		return attributeUnits[attributeIndex];
	}

	/**
	 * Retrieve the units of the i'th attribute supported by the model.
	 *
	 * @param attributeIndex
	 * @return the units of the i'th attribute supported by the model.
	 */
	string getAttributeString(int attributeIndex) const
	{
		if (attributeIndex < 0 || attributeIndex >= nAttributes)
		{
			ostringstream os;
			os << endl << "ERROR in GeoTessMetaData::getAttributeUnit(int attributeIndex)" << endl
					<< "attributeIndex (" << attributeIndex << ") is out of range (0-" << nAttributes-1 << ")"
					<< endl;
			throw GeoTessException(os, __FILE__, __LINE__, 6004);
		}
		return attributeNames[attributeIndex] + " (" + attributeUnits[attributeIndex]+")";
	}

	/**
	 * toString function.
	 * @return string representation of this meta data object
	 */
	string toString(const string& className, LONG_INT memory) const;

	/**
	 * toString function.
	 * @return string representation of this meta data object
	 */
	string toString() const;

	/**
	 * Get the name and version of the software that generated
	 * the content of this model.
	 * @return the name and version of the software that generated
	 * this model.
	 */
	const string& getModelSoftwareVersion() { return modelSoftwareVersion; }

	/**
	 * Retrieve the date when the content of this model was generated.
	 * This is not necessarily the same as the date when the file was
	 * copied or translated.
	 * @return the date when the content of this model was generated.
	 * This is not necessarily the same as the date when the file was
	 * copied or translated.
	 */
	const string& getModelGenerationDate()
	{ return modelGenerationDate; }

	/**
	 * Set the name and version number of the software that generated
	 * the contents of this model.
	 * @param swVersion
	 */
	void setModelSoftwareVersion(const string& swVersion)
	{ modelSoftwareVersion = swVersion; }

	/**
	 * Set the date when this model was generated.
	 * This is not necessarily the same as the date when the file was
	 * copied or translated.
	 * @param genDate
	 */
	void setModelGenerationDate(const string& genDate)
	{ modelGenerationDate = genDate; }

	// All methods below this point are public but are not documented in the doxygen documentation.
	// These are methods that typical applications will never need to call.  They have to be
	// public because other classes in the GeoTess namespace need to access them.
	//
	/// @cond PROTECTED

	/**
	 * If false, all the attributes in the model file were loaded into memory.  If true, then
	 * attributeFilter contains the map from attribute index in file to attributeIndex in
	 * memory.  This is only used during the process of loading a model from file.
	 */
	bool applyAttributeFilter() { return boolAttributeFilter; }

	/**
	 * AttributeFilter is used to limit the number of attribute read from the model input file.
	 * It is an array of length equal to the number of attributes in the model file.  Elements
	 * that contain a value < 0 are not loaded into memory.  Element values >= 0 are the
	 * index of the attributes array in memory into which the model values will be loaded.
	 * This variable is populated from values in inputFilter, which is the entity actually
	 * specified by calling application.
	 */
	vector<int>& getAttributeFilter() { return attributeFilter; }

	/**
	 * Array with of length equal to the number of attributes that will be loaded into memory.
	 * The elements are the indexes of the attributes in the model file that are to be loaded
	 * into memory.
	 */
	void setAttributeFilter(vector<int>& filter) { inputFilter = filter; }

	/**
	 * Check to ensure that this MetaData object contains all the information
	 * needed to construct a new GeoTessModel.
	 * @throws GeoTessExcepion if not complete.
	 */
	void checkComplete();

	void loadMetaData(IFStreamBinary& input);

	void loadMetaData(IFStreamAscii& input);

	int getRefCount() { return refCount; }

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
			os << endl << "ERROR in GeoTessMetaData::removeReference" << endl
					<< "Reference count (" << refCount << ") is already zero."
					<< endl;
			throw GeoTessException(os, __FILE__, __LINE__, 6005);
		}

		--refCount;
	}

	/**
	 * Returns true if reference count is zero.
	 */
	bool isNotReferenced() { return (refCount == 0) ? true : false; }

	/**
	 * Specify the number of vertices in the model.
	 */
	void setNVertices(const int& nvert) { nVertices = nvert; }

	/**
	 * Specify the name of the file from which the model was loaded.
	 *
	 * @param imf the name of the file from which the model was loaded.
	 */
	void setInputModelFile(const string& imf) { inputModelFile = imf; }

	/**
	 * Specify the name of the file from which the grid was loaded.
	 *
	 * @param igf the name of the file from which the grid was loaded.
	 */
	void setInputGridFile(const string& igf) { inputGridFile = igf; }

	/**
	 * Set the amount of time, in seconds, required to load the model.
	 *
	 * @param ltm the amount of time, in seconds, required to load the model.
	 */
	void setLoadTimeModel(double ltm) { loadTimeModel = ltm; }

	/**
	 * Set the name of the file to which the model has been written
	 *
	 * @param omf the name of the file to which the model has been written
	 */
	void setOutputModelFile(const string& omf) { outputModelFile = omf; }

	/**
	 * Set the name of the file to which the grid has been written
	 *
	 * @param ogf the name of the file to which the grid has been written
	 */
	void setOutputGridFile(const string& ogf) { outputGridFile = ogf; }

	/**
	 * Specify the amount of time, in seconds, required to write the model to file.
	 *
	 * @param wtm the amount of time, in seconds, required to write the model to file.
	 */
	void setWriteTimeModel(double wtm) { writeTimeModel = wtm; }

	// the following methods that involve Optimization type are all deprecated.
	// They no longer have any effect since GeoTess is always optimized for speed.
	const GeoTessOptimizationType& getOptimizationType() const { return GeoTessOptimizationType::SPEED; }
	void setOptimizationType(const GeoTessOptimizationType& ot);
	void setOptimizationType(const string& ot);

	///@endcond

};
// end class GeoTessMetaData

}// end namespace geotess

#endif  // GEOTESSMETADATA_OBJECT_H
