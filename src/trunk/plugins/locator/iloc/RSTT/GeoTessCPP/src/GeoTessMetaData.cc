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

#include "GeoTessMetaData.h"
#include "CpuTimer.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

GeoTessMetaData::GeoTessMetaData(const GeoTessMetaData& other) :
		description(other.description), nLayers(other.nLayers), nVertices(other.nVertices),
		layerNames(NULL), layerTessIds(NULL), dataType(other.dataType),
		nAttributes(other.nAttributes), attributeNames(NULL), attributeUnits(NULL),
		boolAttributeFilter(other.boolAttributeFilter),
		attributeFilterString(other.attributeFilterString),
		inputModelFile(other.inputModelFile),
		inputGridFile(other.inputGridFile), loadTimeModel(other.loadTimeModel),
		outputModelFile(other.outputModelFile), outputGridFile(other.outputGridFile),
		writeTimeModel(other.writeTimeModel), refCount(0),
		reuseGrids(true), modelSoftwareVersion(other.modelSoftwareVersion),
		modelGenerationDate(other.modelGenerationDate)
{
	if (nLayers > 0)
	{
		layerNames = new string[nLayers];
		layerTessIds = new int[nLayers];
		for (int i = 0; i < nLayers; ++i)
		{
			layerNames[i] = other.layerNames[i];
			layerTessIds[i] = other.layerTessIds[i];
		}
	}

	if (nAttributes > 0)
	{
		attributeNames = new string[nAttributes];
		attributeUnits = new string[nAttributes];
		for (int i = 0; i < nAttributes; ++i)
		{
			attributeNames[i] = other.attributeNames[i];
			attributeUnits[i] = other.attributeUnits[i];
		}
	}

	if (boolAttributeFilter)
		for (int i=0; i<(int)other.attributeFilter.size(); ++i)
			attributeFilter.push_back(other.attributeFilter[i]);

}

GeoTessMetaData::GeoTessMetaData(const string& inputFile)
: description(""), nLayers(0), nVertices(0), layerNames(NULL), layerTessIds(NULL),
  dataType(&GeoTessDataType::NONE), nAttributes(-1), attributeNames(NULL),
  attributeUnits(NULL), boolAttributeFilter(false), attributeFilterString(""),
  inputModelFile("none"), inputGridFile("none"), loadTimeModel(-1.0),
  outputModelFile("none"), outputGridFile("none"), writeTimeModel(-1.0), refCount(0),
  reuseGrids(true), modelSoftwareVersion(""), modelGenerationDate("")
{
	CpuTimer timr;
	setInputModelFile(inputFile);

	if (inputFile.find(".ascii", inputFile.length() - 6) != string::npos)
	{
		IFStreamAscii input;
		input.openForRead(inputFile);
		loadMetaData(input);
		input.close();
	}
	else
	{
		IFStreamBinary ifs(inputFile);
		ifs.setBoundaryAlignment(false);
		ifs.resetPos();
		loadMetaData(ifs);
	}

	setLoadTimeModel(timr.realTime() * 1e-3);
}

GeoTessMetaData& GeoTessMetaData::operator=(const GeoTessMetaData& other)
{
	// delete existing arrays if they exist.
	if (attributeNames != NULL) delete [] attributeNames;
	if (attributeUnits != NULL) delete [] attributeUnits;
	if (layerNames != NULL) delete [] layerNames;
	if (layerTessIds != NULL) delete [] layerTessIds;

	description = other.description;

	nVertices = other.nVertices;

	nLayers = other.nLayers;
	if (nLayers > 0)
	{
		layerNames = new string[nLayers];
		layerTessIds = new int[nLayers];
		for (int i = 0; i < nLayers; ++i)
		{
			layerNames[i] = other.layerNames[i];
			layerTessIds[i] = other.layerTessIds[i];
		}
	}
	else
	{
		layerNames = NULL;
		layerTessIds = NULL;
	}

	dataType = other.dataType;
	nAttributes = other.nAttributes;
	if (nAttributes > 0)
	{
		attributeNames = new string[nAttributes];
		attributeUnits = new string[nAttributes];
		for (int i = 0; i < nAttributes; ++i)
		{
			attributeNames[i] = other.attributeNames[i];
			attributeUnits[i] = other.attributeUnits[i];
		}
	}
	else
	{
		attributeNames = NULL;
		attributeUnits = NULL;
	}

	//optimization = other.optimization;
	inputModelFile = other.inputModelFile;
	inputGridFile = other.inputGridFile;
	loadTimeModel = other.loadTimeModel;
	outputModelFile = other.outputModelFile;
	outputGridFile = other.outputGridFile;
	writeTimeModel = other.writeTimeModel;
	refCount = 0;
	reuseGrids = other.reuseGrids;
	modelSoftwareVersion = other.modelSoftwareVersion;
	modelGenerationDate = other.modelGenerationDate;

	return *this;
}


GeoTessMetaData::~GeoTessMetaData()
{
	if (refCount > 0)
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessMetaData::~GeoTessMetaData" << endl
			 << "Reference count (" << refCount << ") is not zero." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6006);
	}

	if (attributeNames != NULL)
	{
		delete [] attributeNames;
		attributeNames = NULL;
	}
	if (attributeUnits != NULL)
	{
		delete [] attributeUnits;
		attributeUnits = NULL;
	}
	if (layerNames != NULL)
	{
		delete [] layerNames;
		layerNames = NULL;
	}
	if (layerTessIds != NULL) \
	{
		delete [] layerTessIds;
		layerTessIds = NULL;
	}
}

bool GeoTessMetaData::operator==(const GeoTessMetaData& other)
{
	//if (optimization->ordinal() != other.optimization->ordinal()) return false;

	if (dataType->ordinal() != other.dataType->ordinal()) return false;

	if (description != other.description) return false;

	if (nLayers != other.nLayers) return false;

	if (nAttributes != other.nAttributes) return false;

	for (int i=0; i<nLayers; ++i)
		if (layerNames[i] != other.layerNames[i] || layerTessIds[i] != other.layerTessIds[i])
			return false;

	for (int i=0; i<nAttributes; ++i)
		if (attributeNames[i] != other.attributeNames[i] || attributeUnits[i] != other.attributeUnits[i])
			return false;

	//if (inputModelFile != other.inputModelFile) return false;
	//if (inputGridFile != other.inputGridFile) return false;
	//if (loadTimeModel != other.loadTimeModel) return false;
	//if (outputModelFile != other.outputModelFile) return false;
	//if (outputGridFile != other.outputGridFile) return false;
	//if (writeTimeModel != other.writeTimeModel) return false;
	//if (reuseGrids != other.reuseGrids) return false;
	//if (modelSoftwareVersion != other.modelSoftwareVersion) return false;
	//if (modelGenerationDate != other.modelGenerationDate) return false;

	return true;
}


/**
 * Check to ensure that this MetaData object contains all the information needed to construct a
 * new GeoTessModel.
 *
 * @throws GeoTessException if incomplete.
 */
void GeoTessMetaData::checkComplete()
{
	ostringstream buf;

	if (description == "")
		buf << endl << "  description has not been specified.";

	if (layerNames == NULL)
		buf << endl << "  layerNames has not been specified.";
	else if (layerTessIds == NULL)
		buf << endl << "  layerTessIds has not been specified.";

	if (*dataType == GeoTessDataType::NONE)
		buf << endl << "  dataType has not been specified.";

	if (attributeNames == NULL)
		buf << endl << "  attributeNames has not been specified.";
	else if (attributeUnits == NULL)
		buf << endl << "  attributeUnits has not been specified.";
	else if (nAttributes < 0)
		buf << endl << "  nAttributes < 0.";

	if (modelSoftwareVersion == "")
		buf << endl << "  modelSoftwareVersion has not been specified.";

	if (modelGenerationDate == "")
		buf << endl << "  modelGenerationDate has not been specified.";

	string s = buf.str();
	if (s.size() > 0)
	{
  	ostringstream os;
		os	<< endl << "Error in GeoTessMetaData::checkComplete" << endl
				<< "  MetaData is not complete." << s << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6007);
	}
}

/**
 * Set the attribute names and units. The lengths of the two String[] must be equal.
 *
 * @param names
 * @param units
 * @throws GeoTessException
 */
void GeoTessMetaData::setAttributes(const vector<string>& names, const vector<string>& units)
{
	if (names.size() == 0)
	{
		ostringstream os;
		os	 << "Error in GeoTessMetaData::setAttributes" << endl
				<< "Attribute names input is empty" << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6008);
	}
	if (names.size() != units.size())
	{
		ostringstream os;
		os	 << "Error in GeoTessMetaData::setAttributes" << endl
				<< "Attribute names size (" << names.size()
				<< ") is not equal to units size (" << units.size() << ")" << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6009);
	}

	if (attributeNames != NULL) delete [] attributeNames;
	if (attributeUnits != NULL) delete [] attributeUnits;

	if (inputFilter.size() > 0)
	{
		// test inputFilter to ensure it has no duplicates, no
		// negative indexes and no indexes >= names.size().
		for (int i=0; i<(int)inputFilter.size(); ++i)
		{
			if (inputFilter[i] < 0)
			{
				ostringstream os;
				os	 << "attributeFilter[" << i << "] == " << inputFilter[i] <<
						" is negative" << endl;
				throw GeoTessException(os, __FILE__, __LINE__, 6010);
			}
			if (inputFilter[i] >= (int)names.size())
			{
				ostringstream os;
				os	 << "attributeFilter[" << i << "] == " << inputFilter[i] <<
						" is >= the number of attributes stored in the input file (" <<
						names.size() << ")" << endl <<
						getInputModelFile() << endl;
				throw GeoTessException(os, __FILE__, __LINE__, 6011);
			}
		}
		for (int i=0; i<(int)inputFilter.size()-1; ++i)
			for (int j=i+1; j<(int)inputFilter.size(); ++j)
				if (inputFilter[j] == inputFilter[i])
				{
					ostringstream os;
					os	 << "attributeFilter has duplicate entry " << inputFilter[j] << endl;
					throw GeoTessException(os, __FILE__, __LINE__, 6012);
				}


		attributeFilter.clear();
		attributeFilter.reserve(names.size());

		// attributeFilter has same number of elements as names.
		// Elements that are not going to be stored in memory will
		// be equal to -1.  Elements that are to be stored in memory
		// will have value equal to index in filtered attributeNames array.

		for (int i=0; i<(int)names.size(); ++i)
			attributeFilter.push_back(-1);
		for (int i=0; i<(int)inputFilter.size(); ++i)
			attributeFilter[inputFilter[i]] = i;

		nAttributes = (int)inputFilter.size();

		attributeNames = new string [nAttributes];
		attributeUnits = new string [nAttributes];

		for (int i=0; i<(int)names.size(); ++i)
			if (attributeFilter[i] >= 0)
			{
				attributeNames[attributeFilter[i]] = CPPUtils::trim(names[i], " \t");
				attributeUnits[attributeFilter[i]] = CPPUtils::trim(units[i], " \t");
			}

		boolAttributeFilter = true;
		ostringstream os;
		os << endl << "Attribute Filter:   Model in file contains " <<
				names.size() << " attributes" << endl;
		for (int i=0; i<(int)names.size(); ++i)
			os << (i==0?"   ":"; ") <<
			CPPUtils::trim(names[i], " \t") << " (" <<
			CPPUtils::trim(units[i], " \t") << ")";

		os << endl;
		attributeFilterString = os.str();
	}
	else
	{
		nAttributes = names.size();

		attributeNames = new string [nAttributes];
		attributeUnits = new string [nAttributes];

		for (int i = 0; i < nAttributes; ++i)
		{
			attributeNames[i] = CPPUtils::trim(names[i], " \t");
			attributeUnits[i] = CPPUtils::trim(units[i], " \t");
		}

		boolAttributeFilter = false;
		attributeFilterString = "";
	}

}

/**
 * Retrieve a string containing all the attribute names separated by ';'
 *
 * @return a string containing all the attribute names separated by ';'
 */
string GeoTessMetaData::getAttributeNamesString() const
{
	string s = attributeNames[0];
	for (int i = 1; i < nAttributes; ++i)
		s += "; " + attributeNames[i];
	return s;
}

/**
 * Retrieve a string containing all the attribute units separated by ';'
 *
 * @return a string containing all the attribute units separated by ';'
 */
string GeoTessMetaData::getAttributeUnitsString() const
{
	string s = attributeUnits[0];
	for (int i = 1; i < nAttributes; ++i)
		s += "; " + attributeUnits[i];
	return s;
}

/**
 * Retrieve the layer names in a single, semi-colon delimited string.
 *
 * @return the layer names in a single, semi-colon delimited string.
 */
string GeoTessMetaData::getLayerNamesString()
{
	string s = layerNames[0];
	for (int i = 1; i < nLayers; ++i)
		s += ";" + layerNames[i];
	return s;
}

/**
 * Retrieve the index of the specified attribute name, or -1 if the specified attribute does not
 * exist.
 *
 * @param name
 * @return the index of the specified attribute name, or -1 if the specified attribute does not
 *         exist.
 */
int	GeoTessMetaData::getAttributeIndex(string name)
{
	for (int i = 0; i < nAttributes; ++i)
		if (attributeNames[i] == name)
			return i;
	return -1;
}

/**
 * Specify the names of the layers supported by the model.
 *
 * @param layerNames the names of the layers supported by the model.
 */
void GeoTessMetaData::setLayerNames(vector<string>& layrNms)
{
	if ((layerTessIds != NULL) && ((int) layrNms.size() != nLayers))
	{
		ostringstream os;
		os	 << "Number of tess ids (" << nLayers << ") != number of layers ("
				<< layrNms.size() << ")" << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6013);
	}

	nLayers = layrNms.size();
	if (layerNames != NULL) delete [] layerNames;
	layerNames = new string [nLayers];

	for (int i = 0; i < nLayers; ++i)
		layerNames[i] = CPPUtils::trim(layrNms[i], " \t");

	// create layerTessIds if it does not exist and set with zeros.

	if (layerTessIds == NULL)
	{
		layerTessIds = new int [nLayers];
		for (int i = 0; i < nLayers; ++i) layerTessIds[i] = 0;
	}
}

/**
 * Set layerTessIds; an int[] with an entry for each layer specifying the index of the
 * tessellation that supports that layer.
 *
 * @param layerTessIds the layerTessIds to set
 * @throws GeoTessException if layerTessIds.length != layerNames.length
 */
void GeoTessMetaData::setLayerTessIds(vector<int>& layrTsIds)
{
	if ((layerNames != NULL) && ((int) layrTsIds.size() != nLayers))
	{
  	ostringstream os;
		os	<< "Number of tess ids (" << layrTsIds.size()
				<< ") != number of layers ("
			  << nLayers << ")" << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6014);
	}

	nLayers = layrTsIds.size();
	if (layerTessIds != NULL) delete [] layerTessIds;
	layerTessIds = new int [nLayers];
	for (int i = 0; i < nLayers; ++i) layerTessIds[i] = layrTsIds[i];
}

/**
 * Retrieve the index of the layer that has the specified name,
 * or -1.
 * @param layerName name of layer for which to search
 * @return index of layer, or -1.
 */
int GeoTessMetaData::getLayerIndex(const string& layerName) const
{
	for (int i=0; i < nLayers; ++i)
		if (layerNames[i] == layerName)
			return i;
	return -1;
}

void GeoTessMetaData::setDataType(const GeoTessDataType& dt)
{
	dataType = &dt;
	if (dataType == &GeoTessDataType::BYTE)
	{
		byte i = 0;
		if (sizeof(i) != 1)
		{
			ostringstream os;
			os	 << "Error in GeoTessMetaData::setDataType" << endl
					<< "Trying to set DataType::BYTE but sizeof(byte) is "
					<< sizeof(i) << " when it should be 1" << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 6015);
		}
	}
	else if (dataType == &GeoTessDataType::SHORT)
	{
		short i = 0;
		if (sizeof(i) != 2)
		{
			ostringstream os;
			os	 << "Error in GeoTessMetaData::setDataType" << endl
					<< "Trying to set DataType::SHORT but sizeof(short) is "
					<< sizeof(i) << " when it should be 2" << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 6016);
		}
	}
	else if (dataType == &GeoTessDataType::INT)
	{
		int i = 0;
		if (sizeof(i) != 4)
		{
			ostringstream os;
			os	 << "Error in GeoTessMetaData::setDataType" << endl
					<< "Trying to set DataType::INT but sizeof(int) is "
					<< sizeof(i) << " when it should be 4" << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 6017);
		}
	}
	else if (dataType == &GeoTessDataType::LONG)
	{
		LONG_INT i = 0;
		if (sizeof(i) != 8)
		{
			ostringstream os;
			os	 << "Error in GeoTessMetaData::setDataType" << endl
					<< "Trying to set DataType::LONG but sizeof(LONG_INT) is "
					<< sizeof(i) << " when it should be 8" << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 6018);
		}
	}
}

/**
 * Specify the type of the data that is stored in the model; Must be one of DOUBLE, FLOAT, LONG,
 * INT, SHORTINT, BYTE.
 *
 * @param dataType the dataType to set
 */
void GeoTessMetaData::setDataType(const string& dt)
{
	string dtut = CPPUtils::uppercase_string(CPPUtils::trimRight(dt, " \t"));
	const GeoTessDataType* dtype = GeoTessDataType::valueOf(dtut);
	if (!dtype)
	{
		ostringstream os;
		os	<< dtut << " is not a recognized data type " << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6019);
	}
	setDataType(*dtype);
}

void GeoTessMetaData::setOptimizationType(const GeoTessOptimizationType& ot)
{
	//optimization = &ot;
}

/**
 * Specify the type of the data that is stored in the model; Must be one of DOUBLE, FLOAT, LONG,
 * INT, SHORTINT, BYTE.
 *
 * @param dataType the dataType to set
 */
void GeoTessMetaData::setOptimizationType(const string& ot)
{
	string dtut = CPPUtils::uppercase_string(CPPUtils::trimRight(ot, " \t"));
	const GeoTessOptimizationType* otype = GeoTessOptimizationType::valueOf(dtut);
	if (!otype)
	{
		ostringstream os;
		os	<< dtut << " is not a recognized OptimizationType " << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6020);
	}
	setOptimizationType(*otype);
}

string GeoTessMetaData::toString() const
{ return toString(class_name(), -1L); }

/**
 * To string function.
 */
string GeoTessMetaData::toString(const string& className, LONG_INT memory) const
{
	int nbits = sizeof(10L) == 8 ? 64 : sizeof(10L) == 4 ? 32 : -1;

	ostringstream os;
	os << endl << className << ":" << endl
		<< "OS: " << CPPUtils::getOpSys()
		<< " " << nbits << "-bit mode" << endl
		<< "Input Model File: " << inputModelFile << endl
		<< "Input Grid File: " << inputGridFile << endl << endl
		<< "Generated by: " << modelSoftwareVersion
		<< "  " << modelGenerationDate << endl
		<< "Model Load Time: " << CPPUtils::dtos(loadTimeModel, "%.3f sec") << endl;

	if (memory >= 0)
		os << "Memory footprint: " << memory/1024./1024. << " MB" << endl;

	if (outputModelFile != "none")
	{
		os << "Output Model File: " << outputModelFile << endl
	       << "Output Grid File: " << outputGridFile << endl
		   << "Model Write Time: " << CPPUtils::dtos(writeTimeModel, "%.3f") << endl;
	}
	os << endl << "Model Description: " << endl
			<< description
			<< "<end description>"
			<< endl << endl;

	os << "EarthShape: " << earthShape.getShapeName() << endl;

	os << "DataType: " << dataType->name() << endl;

	os << attributeFilterString;

//	os << "Attributes: " << getAttributeNamesString() << endl
//		 << "Attribute Units: " << getAttributeUnitsString() << endl << endl;

	os << "Attributes: " << endl;
	for (int i=0; i<getNAttributes(); ++i)
		os << "  " << i << ":  " << getAttributeName(i)
		<< "  (" << getAttributeUnit(i) << ")" << endl;
	os << endl;

	os << "Layers: " <<  endl << "  Index  TessId    Name" << endl;

	for (int i = nLayers - 1; i >= 0; --i)
	{
		os << " "  << CPPUtils::itos(i, "%3d")
		   << "  "    << CPPUtils::itos(layerTessIds[i], "%6d")
		   << "     " << layerNames[i] << endl;
	}
	os << endl;

	return os.str();
}

void GeoTessMetaData::loadMetaData(IFStreamBinary &input)
{
	string s;

	// get the GeoTess name (GEOTESSMODEL) and validate

	string GTName;
	input.readCharArray(GTName, 12);
	if (GTName != "GEOTESSMODEL")
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessModel::loadModelBinary" << endl
			 << "  expected char array \"GEOTESSMODEL\" as first entry of file "
			 << "but found \"";
		for (int i = 0; i < 12; ++i)
			if ((GTName[i] != 127) && (GTName[i] > 31))
			  os << GTName[i];
			else
			  os << "[" << (int) GTName[i] << "]";
		os << "\" instead ..." << endl;

		throw GeoTessException(os, __FILE__, __LINE__, 6021);
	}

	// get the fileFormatVersion. Only recognized values right now are 1 and 2.

	int fileFormatVersion = input.readInt();
	if ((fileFormatVersion < 0) || (fileFormatVersion > 65536))
	{
		input.setByteOrderReverse(!input.isByteOrderReversalOn());
		input.decrementPos(CPPUtils::SINT);
		fileFormatVersion = input.readInt();
	}

	if (fileFormatVersion < 1 || fileFormatVersion > 2)
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessModel::loadModelBinary" << endl
			<< "This version of GeoTessJava (" << GeoTessUtils::getVersion()
			<< ") cannot read GeoTess files written in file format " << fileFormatVersion << "." << endl
			<< "Please update GeoTessJava to the latest version, available at www.sandia.gov/geotess" << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6022);
	}

	// read model population software version and file creation date
	input.readString(modelSoftwareVersion);

	input.readString(modelGenerationDate);

	if (fileFormatVersion >= 2)
	{
		string earthShapeName;
		input.readString(earthShapeName);
		setEarthShape(earthShapeName);
	}

	input.readString(s);
	setDescription(s);

	string names, units, lyrs;
	input.readString(names);
	input.readString(units);
	if (units.find_first_of(";") ==  0)
		units = " "+units;
	if (units.find_last_of(";") ==  units.length()-1)
		units = units+" ";
	setAttributes(names, units);

	input.readString(lyrs);
	setLayerNames(lyrs);

	input.readString(s);
	setDataType(s);

	nVertices = input.readInt();

	// an array of length nLayers where each element is the
	// index of the tessellation that supports that layer.
	vector<int> tessellations;
	int nTess = 0;
	for (int i = 0; i < nLayers; ++i)
	{
		tessellations.push_back(input.readInt());
		if (tessellations[i] > nTess)
			nTess = tessellations[i];
	}
	++nTess;
	setLayerTessIds(tessellations);
}

void GeoTessMetaData::loadMetaData(IFStreamAscii &input)
{
	// get the dataFileFormat. Only recognized value right now is 1.

	string line = input.readString();
	if (line != "GEOTESSMODEL")
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessModel::loadModelAscii" << endl
				<< "while trying to read file " << inputModelFile << endl
			 << "  expected GEOTESSMODEL as first line of file but found" << endl
			 << line << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6023);
	}

	int dataFileFormat = input.readInteger();
	if (dataFileFormat < 1 || dataFileFormat > 2)
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessModel::loadModelAscii" << endl
				<< "while trying to read file " << inputModelFile << endl
			<< "This version of GeoTessJava (" << GeoTessUtils::getVersion()
			<< ") cannot read GeoTess files written in file format " << dataFileFormat << "." << endl
			<< "Please update GeoTessJava to the latest version, available at www.sandia.gov/geotess" << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6024);
	}

	// read model population software version and file creation date

	input.readLine(modelSoftwareVersion);
	modelSoftwareVersion = CPPUtils::trim(modelSoftwareVersion);

	input.readLine(modelGenerationDate);
	modelGenerationDate = CPPUtils::trim(modelGenerationDate);

	if (dataFileFormat >= 2)
	{
		string earthShapeName;
		input.readString(earthShapeName);
		setEarthShape(earthShapeName);
		//input.readLine(line);
	}

	// read description header

	line = input.readString();
	if (line != "<model_description>")
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessModel::loadModelAscii" << endl
				<< "while trying to read file " << inputModelFile << endl
				<< "  Expected to read string '<model_description>' but found "
				<< line << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6025);
	}

	// read description

	description = "";
	input.readLine(line);
	while (line != "</model_description>")
	{
		CPPUtils::removeEOL(line);
		description += line + CPPUtils::NEWLINE;
		input.readLine(line);
	}

	// read attribute names and units

	string attributes = "";
	input.readLine(attributes);
	vector<string> tokens;
	CPPUtils::tokenizeString(attributes, ":", tokens);
	if (tokens[0] != "attributes")
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessModel::loadModelAscii" << endl
				<< "while trying to read file " << inputModelFile << endl
				<< "  Expected to read string starting with 'attributes:' but found "
				<< attributes << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6026);
	}
	vector<string> attrArray;
	CPPUtils::tokenizeString(tokens[1], ";", attrArray);

	string units = "";
	input.readLine(units);
	CPPUtils::tokenizeString(units, ":", tokens);
	if (tokens[0] != "units")
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessModel::loadModelAscii" << endl
				<< "  while trying to read file " << inputModelFile << endl
				<< "  Expected to read string starting with 'units:' but found "
				<< units << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6027);
	}
	vector<string> unitArray;
	CPPUtils::tokenizeString(tokens[1], ";", unitArray);

	setAttributes(attrArray, unitArray);

	string layers = "";
	input.readLine(layers);
	CPPUtils::tokenizeString(layers, ":", tokens);
	if (tokens[0] != "layers")
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessModel::loadModelAscii" << endl
				<< "while trying to read file " << inputModelFile << endl
				<< "  Expected to read string starting with 'layers:' but found "
				<< layers << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 6028);
	}
	vector<string> layrArray;
	CPPUtils::tokenizeString(tokens[1], ";", layrArray);

	setLayerNames(layrArray);

	string datatype = input.readString();
	setDataType(datatype);

	nVertices = input.readInteger();

	// an array of length nLayers where each element is the
	// index of the tessellation that supports that layer.
	vector<int> tessellations;
	tessellations.reserve(nLayers);
	int nTess = 0;
	for (int i = 0; i < nLayers; ++i)
	{
		tessellations.push_back(input.readInteger());
		if (tessellations[i] > nTess)
			nTess = tessellations[i];
	}
	++nTess;
	setLayerTessIds(tessellations);

}

} // end namespace geotess
