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

#ifndef DATAVALUE_OBJECT_H
#define DATAVALUE_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <fstream>
#include <string>
#include <climits>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "GeoTessDataType.h"
#include "GeoTessData.h"
#include "IFStreamAscii.h"
#include "IFStreamBinary.h"
#include "GeoTessUtils.h"
#include "GeoTessDataArray.h"

// **** _BEGIN GEOTESS NAMESPACE_ *********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Manages a single data value attached to a grid node.
 *
 * Manages the data values attached to single grid node defined as a single value
 * of type T (See DataType for supported types). The number of stored values is
 * always 1 for this object.
 */
template<typename T>
class GEOTESS_EXP_IMP GeoTessDataValue : public GeoTessData
{
private:

	/**
	 * The single value element of type T of this DataValue<T> object.
	 */
	T												value;

public:

	/**
	 * Standard constructor. Sets value.
	 */
	GeoTessDataValue(T v) : GeoTessData(), value(v) {};

	/**
	 * Default constructor.
	 */
	GeoTessDataValue() : GeoTessData(), value(0) {};

	/// @cond PROTECTED  Turn off doxygen documentation until 'endcond' is found

	/**
	 * Standard constructor. Reads value from the provided input file stream.
	 */
	GeoTessDataValue(IFStreamBinary& ifs) : GeoTessData(), value(0)
	{ ifs.readType(value); };

	/**
	 * Standard constructor. Reads value from the provided input file stream.
	 */
	GeoTessDataValue(IFStreamAscii& ifs) : GeoTessData(), value(0)
	{ ifs.readType(value); };

	/**
	 * Standard constructor. Reads value from the provided input file stream.
	 */
	GeoTessDataValue(IFStreamBinary& ifs, vector<int>& filter) : GeoTessData(), value(0)
	{
		T val;
		for (int i=0; i<(int)filter.size(); ++i)
		{
			ifs.readType(val);
			if (filter[i] == 0)
				value = val;
		}
	}

	/**
	 * Standard constructor. Reads value from the provided input file stream.
	 */
	GeoTessDataValue(IFStreamAscii& ifs, vector<int>& filter) : GeoTessData(), value(0)
	{
		T val;
		for (int i=0; i<(int)filter.size(); ++i)
		{
			ifs.readType(val);
			if (filter[i] == 0)
				value = val;
		}
	}

	/**
	 * Destructor.
	 */
	virtual									~GeoTessDataValue() {};

	virtual LONG_INT getMemory() { return (LONG_INT)sizeof(GeoTessDataValue<T>); }

	/**
	 * Output this DataValue<T> to the provided output file stream.
	 */
	virtual void						write(IFStreamAscii& ofs)
	{ ofs.writeString(" "); ofs.writeType(value); };

	/**
	 * Output this DataValue<T> to the provided output file stream.
	 */
	virtual void						write(IFStreamBinary& ofs) { ofs.writeType(value); };

	///@endcond

	/*
	 * Return DataType.
	 */
	virtual const GeoTessDataType&	getDataType() const { return GeoTessDataType::NONE; };

	/**
	 * Return size.
	 */
	virtual int							size() const { return 1; };

	/**
	 * Return true if the input DataValue<T> object (d) equals this DataValue<T>
	 * object.
	 */
	bool operator == (const GeoTessDataValue<T>& d) const
	{ return (GeoTessData::operator==(d) && ((value == d.value) || (isNaN(0) && d.isNaN(0)))); }

	/**
	 * Return true if the input DataValue<T> object (d) equals this DataValue<T>
	 * object.
	 */
	virtual bool operator == (const GeoTessData& d) const { return operator==(*((const GeoTessDataValue<T>*) &d)); }

	/**
	 * Returns value as one of six intrinsic types.
	 */
	virtual double					getDouble(int attributeIndex) const
	{ return attributeIndex == 0 ? (double) value : NaN_DOUBLE; }

	virtual float						getFloat(int attributeIndex) const
	{ return attributeIndex == 0 ? (float) value : NaN_FLOAT; };

	virtual LONG_INT						getLong(int attributeIndex) const
	{ return attributeIndex == 0 ? (LONG_INT) value : LONG_MIN; };

	virtual int							getInt(int attributeIndex) const
	{ return attributeIndex == 0 ? (int) value : INT_MIN; };

	virtual short						getShort(int attributeIndex) const
	{ return attributeIndex == 0 ? (short) value : SHRT_MIN; };

	virtual byte						getByte(int attributeIndex) const
	{ return attributeIndex == 0 ? (byte) value : SCHAR_MIN; };


	/**
	 * Returns the attribute at the input attribute index as a double value.
	 */
	virtual void			getValue(int attributeIndex, double& val) const
	{ val = attributeIndex == 0 ? (double) value : NaN_DOUBLE; };

	/**
	 * Returns the attribute at the input attribute index as a float value.
	 */
	virtual void			getValue(int attributeIndex, float& val) const
	{ val = attributeIndex == 0 ? (float) value : NaN_FLOAT; };

	/**
	 * Returns the attribute at the input attribute index as a long value.
	 */
	virtual void			getValue(int attributeIndex, LONG_INT& val) const
	{ val = attributeIndex == 0 ? (LONG_INT) value : LONG_MIN; };

	/**
	 * Returns the attribute at the input attribute index as a int value.
	 */
	virtual void			getValue(int attributeIndex, int& val) const
	{ val = attributeIndex == 0 ? (int) value : INT_MIN; };

	/**
	 * Returns the attribute at the input attribute index as a short value.
	 */
	virtual void			getValue(int attributeIndex, short& val) const
	{ val = attributeIndex == 0 ? (short) value : SHRT_MIN; };

	/**
	 * Returns the attribute at the input attribute index as a byte value.
	 */
	virtual void			getValue(int attributeIndex, byte& val) const
	{ val = attributeIndex == 0 ? (byte) value : SCHAR_MIN; };

	/**
	 * Copy value into the supplied array at index 0 as a double value.
	 */
	virtual void            getValues(double values[], const int& n) { values[0] = (double) value; }

	/**
	 * Copy value into the supplied array at index 0 as a float value.
	 */
	virtual void            getValues(float values[], const int& n) { values[0] = (float) value; }

	/**
	 * Copy value into the supplied array at index 0 as a LONG_INT value.
	 */
	virtual void            getValues(LONG_INT values[], const int& n) { values[0] = (LONG_INT) value; }

	/**
	 * Copy value into the supplied array at index 0 as an int value.
	 */
	virtual void            getValues(int values[], const int& n) { values[0] = (int) value; }

	/**
	 * Copy value into the supplied array at index 0 as a short value.
	 */
	virtual void            getValues(short values[], const int& n) { values[0] = (short) value; }

	/**
	 * Copy value into the supplied array at index 0 as a byte value.
	 */
	virtual void            getValues(byte values[], const int& n) { values[0] = (byte) value; }

	/**
	 * Set the value at the input attribute index to the input intrinsic.
	 */
	virtual GeoTessData&						setValue(int attributeIndex, double v)
	{ if (attributeIndex == 0) value = (T) v; return *this; };

	virtual GeoTessData&						setValue(int attributeIndex, float v)
	{ if (attributeIndex == 0) value = (T) v; return *this; };

	virtual GeoTessData&						setValue(int attributeIndex, LONG_INT v)
	{ if (attributeIndex == 0) value = (T) v; return *this; };

	virtual GeoTessData&						setValue(int attributeIndex, int v)
	{ if (attributeIndex == 0) value = (T) v; return *this; };

	virtual GeoTessData&						setValue(int attributeIndex, short v)
	{ if (attributeIndex == 0) value = (T) v; return *this; };

	virtual GeoTessData&						setValue(int attributeIndex, byte v)
	{ if (attributeIndex == 0) value = (T) v; return *this; };

	/**
	 * Returns true if the specified attribute is NaN. when Data values
	 * are of type Byte, Short, Int and Long, this method always returns
	 * false since those types do not support NaN. float and double types
	 * are overridden and returns true if value isNaN and false
	 * otherwise.
	 *
	 * @param attributeIndex The attribute value to be tested.
	 *
	 * @return true if the value of the specified attribute is NaN.
	 */
	virtual bool            isNaN(int attributeIndex) const {return false;};

	/**
	 * Returns a copy of this DataValue.
	 */
	virtual GeoTessData*           copy()
	{
		return new GeoTessDataValue<T>(value);
	}

}; // end class DataValue

/// @cond PROTECTED  Turn off doxygen documentation until 'endcond' is found

/**
 * Template specialization for getDataType()
 */
template<>
inline bool	GeoTessDataValue<double>::isNaN(int attributeIndex) const
{
	return (isnan(value));
}

/**
 * Template specialization for getDataType()
 */
template<>
inline bool	GeoTessDataValue<float>::isNaN(int attributeIndex) const
{
	double v = (double) value;
	return (isnan(v));
}

/**
 * Template specialization for getDataType()
 */
template<>
inline const GeoTessDataType&	GeoTessDataValue<double>::getDataType() const
{
	return GeoTessDataType::DOUBLE;
}

/**
 * Template specialization for getDataType()
 */
template<>
inline const GeoTessDataType&	GeoTessDataValue<float>::getDataType() const
{
	return GeoTessDataType::FLOAT;
}

/**
 * Template specialization for getDataType()
 */
template<>
inline const GeoTessDataType&	GeoTessDataValue<LONG_INT>::getDataType() const
{
	return GeoTessDataType::LONG;
}

/**
 * Template specialization for getDataType()
 */
template<>
inline const GeoTessDataType&	GeoTessDataValue<int>::getDataType() const
{
	return GeoTessDataType::INT;
}

/**
 * Template specialization for getDataType()
 */
template<>
inline const GeoTessDataType&	GeoTessDataValue<short>::getDataType() const
{
	return GeoTessDataType::SHORT;
}

/**
 * Template specialization for getDataType()
 */
template<>
inline const GeoTessDataType&	GeoTessDataValue<byte>::getDataType() const
{
	return GeoTessDataType::BYTE;
}

///@endcond

} // end namespace geotess

#endif  // DATAVALUE_OBJECT_H
