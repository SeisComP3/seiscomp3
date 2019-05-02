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

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"
#include "GeoTessException.h"
#include "GeoTessUtils.h"
#include "GeoTessMetaData.h"
#include "GeoTessData.h"
#include "GeoTessDataValue.h"
#include "GeoTessDataArray.h"
#include "IFStreamAscii.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

int       GeoTessData::aClassCount    = 0;

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

GeoTessData::~GeoTessData()
{
	--aClassCount;
}

GeoTessData* GeoTessData::getData(IFStreamAscii& ifs, GeoTessMetaData& gtmd)
{
	// switch on the DataType and create a new Data object ... if not defined
	// throw an error

	int n = gtmd.getNAttributes();
	if (gtmd.applyAttributeFilter())
	{
		switch (gtmd.getDataType().ordinal())
		{
		case 0 : // DOUBLE
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<double>(ifs, gtmd.getAttributeFilter()) :
					(GeoTessData*) new GeoTessDataArray<double>(ifs, n, gtmd.getAttributeFilter()));
		case 1 : // FLOAT
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<float>(ifs, gtmd.getAttributeFilter()) :
					(GeoTessData*) new GeoTessDataArray<float>(ifs, n, gtmd.getAttributeFilter()));
		case 2 : // LONG
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<LONG_INT>(ifs, gtmd.getAttributeFilter()) :
					(GeoTessData*) new GeoTessDataArray<LONG_INT>(ifs, n, gtmd.getAttributeFilter()));
		case 3 : // INT
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<int>(ifs, gtmd.getAttributeFilter()) :
					(GeoTessData*) new GeoTessDataArray<int>(ifs, n, gtmd.getAttributeFilter()));
		case 4 : // SHORT
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<short>(ifs, gtmd.getAttributeFilter()) :
					(GeoTessData*) new GeoTessDataArray<short>(ifs, n, gtmd.getAttributeFilter()));
		case 5 : // BYTE
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<byte>(ifs, gtmd.getAttributeFilter()) :
					(GeoTessData*) new GeoTessDataArray<byte>(ifs, n, gtmd.getAttributeFilter()));
		default :
			ostringstream os;
			os << endl << "ERROR in Data::getData" << endl
					<< gtmd.getDataType().toString() << " is not a recognized data type." << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 5001);
		}
	}
	else
	{
		switch (gtmd.getDataType().ordinal())
		{
		case 0 : // DOUBLE
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<double>(ifs) :
					(GeoTessData*) new GeoTessDataArray<double>(ifs, n));
		case 1 : // FLOAT
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<float>(ifs) :
					(GeoTessData*) new GeoTessDataArray<float>(ifs, n));
		case 2 : // LONG
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<LONG_INT>(ifs) :
					(GeoTessData*) new GeoTessDataArray<LONG_INT>(ifs, n));
		case 3 : // INT
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<int>(ifs) :
					(GeoTessData*) new GeoTessDataArray<int>(ifs, n));
		case 4 : // SHORT
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<short>(ifs) :
					(GeoTessData*) new GeoTessDataArray<short>(ifs, n));
		case 5 : // BYTE
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<byte>(ifs) :
					(GeoTessData*) new GeoTessDataArray<byte>(ifs, n));
		default :
			ostringstream os;
			os << endl << "ERROR in Data::getData" << endl
					<< gtmd.getDataType().toString() << " is not a recognized data type." << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 5002);
		}
	}

	// can't get here

	return (GeoTessData*) NULL;
}

GeoTessData* GeoTessData::getData(IFStreamBinary& ifs, GeoTessMetaData& gtmd)
{
	// switch on the DataType and create a new Data object ... if not defined
	// throw an error

	int n = gtmd.getNAttributes();
	if (gtmd.applyAttributeFilter())
	{
		switch (gtmd.getDataType().ordinal())
		{
		case 0 : // DOUBLE
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<double>(ifs, gtmd.getAttributeFilter()) :
					(GeoTessData*) new GeoTessDataArray<double>(ifs, n, gtmd.getAttributeFilter()));
		case 1 : // FLOAT
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<float>(ifs, gtmd.getAttributeFilter()) :
					(GeoTessData*) new GeoTessDataArray<float>(ifs, n, gtmd.getAttributeFilter()));
		case 2 : // LONG
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<LONG_INT>(ifs, gtmd.getAttributeFilter()) :
					(GeoTessData*) new GeoTessDataArray<LONG_INT>(ifs, n, gtmd.getAttributeFilter()));
		case 3 : // INT
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<int>(ifs, gtmd.getAttributeFilter()) :
					(GeoTessData*) new GeoTessDataArray<int>(ifs, n, gtmd.getAttributeFilter()));
		case 4 : // SHORT
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<short>(ifs, gtmd.getAttributeFilter()) :
					(GeoTessData*) new GeoTessDataArray<short>(ifs, n, gtmd.getAttributeFilter()));
		case 5 : // BYTE
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<byte>(ifs, gtmd.getAttributeFilter()) :
					(GeoTessData*) new GeoTessDataArray<byte>(ifs, n, gtmd.getAttributeFilter()));
		default :
			ostringstream os;
			os << endl << "ERROR in Data::getData" << endl
					<< gtmd.getDataType().toString() << " is not a recognized data type." << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 5003);
		}
	}
	else
	{
		switch (gtmd.getDataType().ordinal())
		{
		case 0 : // DOUBLE
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<double>(ifs) :
					(GeoTessData*) new GeoTessDataArray<double>(ifs, n));
		case 1 : // FLOAT
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<float>(ifs) :
					(GeoTessData*) new GeoTessDataArray<float>(ifs, n));
		case 2 : // LONG
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<LONG_INT>(ifs) :
					(GeoTessData*) new GeoTessDataArray<LONG_INT>(ifs, n));
		case 3 : // INT
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<int>(ifs) :
					(GeoTessData*) new GeoTessDataArray<int>(ifs, n));
		case 4 : // SHORT
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<short>(ifs) :
					(GeoTessData*) new GeoTessDataArray<short>(ifs, n));
		case 5 : // BYTE
			return (n == 1 ? (GeoTessData*) new GeoTessDataValue<byte>(ifs) :
					(GeoTessData*) new GeoTessDataArray<byte>(ifs, n));
		default :
			ostringstream os;
			os << endl << "ERROR in Data::getData" << endl
					<< gtmd.getDataType().toString() << " is not a recognized data type." << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 5004);
		}
	}

	// can't get here

	return (GeoTessData*) NULL;
}

GeoTessData* GeoTessData::getData(const GeoTessDataType& dataType, int nAttributes)
{
	switch (dataType.ordinal())
	{
	case 0 : // DOUBLE
		return (nAttributes == 1 ? (GeoTessData*) new GeoTessDataValue<double>(0.) :
				(GeoTessData*) new GeoTessDataArray<double>(nAttributes));
	case 1 : // FLOAT
		return (nAttributes == 1 ? (GeoTessData*) new GeoTessDataValue<float>(0.F) :
				(GeoTessData*) new GeoTessDataArray<float>(nAttributes));
	case 2 : // LONG
		return (nAttributes == 1 ? (GeoTessData*) new GeoTessDataValue<LONG_INT>(0) :
				(GeoTessData*) new GeoTessDataArray<LONG_INT>(nAttributes));
	case 3 : // INT
		return (nAttributes == 1 ? (GeoTessData*) new GeoTessDataValue<int>(0) :
				(GeoTessData*) new GeoTessDataArray<int>(nAttributes));
	case 4 : // SHORT
		return (nAttributes == 1 ? (GeoTessData*) new GeoTessDataValue<short>(0) :
				(GeoTessData*) new GeoTessDataArray<short>(nAttributes));
	case 5 : // BYTE
		return (nAttributes == 1 ? (GeoTessData*) new GeoTessDataValue<byte>(0) :
				(GeoTessData*) new GeoTessDataArray<byte>(nAttributes));
	default :
		ostringstream os;
		os << endl << "ERROR in Data::getData" << endl
				<< dataType.toString() << " is not a recognized data type." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 5004);
	}
// can't get here
return (GeoTessData*) NULL;
}

GeoTessData* GeoTessData::getData(double values[], const int& size)
{ return (size == 1 ?  (GeoTessData*)new GeoTessDataValue<double>(values[0]) : (GeoTessData*)new GeoTessDataArray<double>(values, size)); }

GeoTessData* GeoTessData::getData(float values[], const int& size)
{ return (size == 1 ?  (GeoTessData*)new GeoTessDataValue<float>(values[0]) : (GeoTessData*)new GeoTessDataArray<float>(values, size)); }

GeoTessData* GeoTessData::getData(LONG_INT values[], const int& size)
{ return (size == 1 ?  (GeoTessData*)new GeoTessDataValue<LONG_INT>(values[0]) : (GeoTessData*)new GeoTessDataArray<LONG_INT>(values, size)); }

GeoTessData* GeoTessData::getData(int values[], const int& size)
{ return (size == 1 ?  (GeoTessData*)new GeoTessDataValue<int>(values[0]) : (GeoTessData*)new GeoTessDataArray<int>(values, size)); }

GeoTessData* GeoTessData::getData(short values[], const int& size)
{ return (size == 1 ?  (GeoTessData*)new GeoTessDataValue<short>(values[0]) : (GeoTessData*)new GeoTessDataArray<short>(values, size)); }

GeoTessData* GeoTessData::getData(byte values[], const int& size)
{ return (size == 1 ?  (GeoTessData*)new GeoTessDataValue<byte>(values[0]) : (GeoTessData*)new GeoTessDataArray<byte>(values, size)); }


// repeat with vectors

GeoTessData* GeoTessData::getData(const vector<double>& values)
{ return (values.size() == 1 ?  (GeoTessData*)new GeoTessDataValue<double>(values[0]) : (GeoTessData*)new GeoTessDataArray<double>(values)); }

GeoTessData* GeoTessData::getData(const vector<float>& values)
{ return (values.size() == 1 ?  (GeoTessData*)new GeoTessDataValue<float>(values[0]) : (GeoTessData*)new GeoTessDataArray<float>(values)); }

GeoTessData* GeoTessData::getData(const vector<LONG_INT>& values)
{ return (values.size() == 1 ?  (GeoTessData*)new GeoTessDataValue<LONG_INT>(values[0]) : (GeoTessData*)new GeoTessDataArray<LONG_INT>(values)); }

GeoTessData* GeoTessData::getData(const vector<int>& values)
{ return (values.size() == 1 ?  (GeoTessData*)new GeoTessDataValue<int>(values[0]) : (GeoTessData*)new GeoTessDataArray<int>(values)); }

GeoTessData* GeoTessData::getData(const vector<short>& values)
{ return (values.size() == 1 ?  (GeoTessData*)new GeoTessDataValue<short>(values[0]) : (GeoTessData*)new GeoTessDataArray<short>(values)); }

GeoTessData* GeoTessData::getData(const vector<byte>& values)
{ return (values.size() == 1 ?  (GeoTessData*)new GeoTessDataValue<byte>(values[0]) : (GeoTessData*)new GeoTessDataArray<byte>(values)); }

} // end namespace geotess
