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

#ifndef DATAARRAY_OBJECT_H
#define DATAARRAY_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <fstream>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "GeoTessDataType.h"
#include "GeoTessData.h"
#include "IFStreamAscii.h"
#include "IFStreamBinary.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

//class IFStreamAscii;

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Manages a 1D array of data values attached to a single grid node.
 *
 * Manages the data values attached to single grid node defined as an array of
 * T type attributes (See DataType for supported types). The size of the array
 * is always nAttributes as defined in the Data object.
 */
template<typename T>
class GEOTESS_EXP_IMP GeoTessDataArray : public GeoTessData
{
private:

	/**
	 * Number of values stored by this DataArray.
	 */
	int											nValues;

	/**
	 * Array of values stored by this DataArray.
	 */
	T*											values;

	/**
	 * Default constructor. Not Used.
	 */
	GeoTessDataArray() : GeoTessData(), nValues(0), values(NULL) {};

public:

	/**
	 * Standard constructor. Copies the contents of the new DataArray from the
	 * provided input array.
	 */
	GeoTessDataArray(T v[], const int& n) : GeoTessData(), nValues(n), values(NULL)
	{
		values = new T [nValues];
		for (int i = 0; i < nValues; ++i) values[i] = v[i];
	}

	/**
	 * Standard constructor. Copies the contents of the new DataArray from the
	 * provided input array.
	 */
	GeoTessDataArray(const vector<T>& v) : GeoTessData(), nValues(v.size()), values(NULL)
	{
		values = new T [nValues];
		for (int i = 0; i < nValues; ++i) values[i] = v[i];
	}

	/**
	 * Standard constructor. Creates a new array of n entries and initializes each to 0
	 */
	GeoTessDataArray(const int& n) : GeoTessData(), nValues(n), values(NULL)
	{
		values = new T [nValues];
		for (int i = 0; i < n; ++i) values[i] = (T) 0;
	}

	/// @cond PROTECTED  Turn off doxygen documentation until 'endcond' is found

	/**
	 * Standard constructor. Reads the contents of the new DataArray from the
	 * provided input file stream.
	 */
	GeoTessDataArray(IFStreamAscii& ifs, int n) : GeoTessData(), nValues(n), values(NULL)
	{
		if (nValues > 0)
		{
			values = new T [nValues];
			for (int i = 0; i < nValues; ++i) ifs.readType(values[i]);
		}
	}

	/**
	 * Standard constructor. Reads the contents of the new DataArray from the
	 * provided input file stream.
	 */
	GeoTessDataArray(IFStreamBinary& ifs, int n) : GeoTessData(), nValues(n), values(NULL)
	{
		if (nValues > 0)
		{
			values = new T [nValues];
			ifs.readTypeArray(values, nValues);
		}
	}

	/**
	 * Standard constructor. Reads value from the provided input file stream.
	 * @param ifs the input stream
	 * @param n the number of values that are to be loaded into memory
	 * @param filter a vector containing n or more integers.  It musts contain
	 * every integer >= 0 and < n.  In addition, it can contain a bunch of negative
	 * numbers.  The number of values that will be read from the file is filter.size().
	 * Entries with negative numbers will be discarded.
	 */
	GeoTessDataArray(IFStreamAscii& ifs, int n, vector<int>& filter) : GeoTessData(),  nValues(n), values(NULL)
	{
		if (nValues > 0)
		{
			values = new T [nValues];
			T val;
			for (int i=0; i<(int)filter.size(); ++i)
			{
				ifs.readType(val);
				if (filter[i] >= 0)
					values[filter[i]] = val;
			}
		}
	}

	/**
	 * Standard constructor. Reads value from the provided input file stream.
	 * @param ifs the input stream
	 * @param n the number of values that are to be loaded into memory
	 * @param filter a vector containing n or more integers.  It musts contain
	 * every integer >= 0 and < n.  In addition, it can contain a bunch of negative
	 * numbers.  The number of values that will be read from the file is filter.size().
	 * Entries with negative numbers will be discarded.
	 */
	GeoTessDataArray(IFStreamBinary& ifs, int n, vector<int>& filter) : GeoTessData(),  nValues(n), values(NULL)
	{
		if (nValues > 0)
		{
			values = new T [nValues];

			T val;
			for (int i=0; i<(int)filter.size(); ++i)
			{
				ifs.readType(val);
				if (filter[i] >= 0)
					values[filter[i]] = val;
			}
		}
	}

	/**
	 * Destructor.
	 */
	virtual									~GeoTessDataArray()
	{ if (values != NULL) delete [] values; };

	/**
	 * Writes the contents of this DataArray to the provided output file stream.
	 */
	virtual void write(IFStreamBinary& ofs)
	{
		for (int i = 0; i < nValues; ++i) ofs.writeType(values[i]);
	}

	/**
	 * Writes the contents of this DataArray to the provided output file stream.
	 */
	virtual void write(IFStreamAscii& ofs)
	{
		for (int i = 0; i < nValues; ++i)
		{ ofs.writeString(" "); ofs.writeType(values[i]); }
	}

	///@endcond

	/*
	 * Return DataType.
	 */
	virtual const GeoTessDataType&	getDataType() const { return GeoTessDataType::NONE; };

	/**
	 * Returns the number of entries in the array of values.
	 */
	virtual int			size() const { return nValues; };

	virtual LONG_INT getMemory()
	{ return (LONG_INT)sizeof(GeoTessDataArray<T>) + (LONG_INT)nValues * (LONG_INT)sizeof(T); }

	/**
	 * Return true if the input DataArray<T> object (d) equals this DataArray<T>
	 * object.
	 */
	bool				operator == (const GeoTessDataArray<T>& d) const
	{
		if (!GeoTessData::operator==(d)) return false;
		if (nValues != d.nValues) return false;

		for (int i = 0; i < nValues; ++i)
			if ((values[i] != d.values[i]) &&
					!(isNaN(i) && d.isNaN(i))) return false;

		return true;
	}

	/**
	 * Return true if the input DataArray<T> object (d) equals this DataArray<T>
	 * object.
	 */
	virtual bool		operator == (const GeoTessData& d) const
	{	return operator==(*((const GeoTessDataArray<T>*) &d)); }

	/**
	 * Returns value defined for the input attribute index as a double
	 */
	virtual double		getDouble(int attributeIndex) const
	{ return (double) values[attributeIndex]; }

	/**
	 * Returns value defined for the input attribute index as a float
	 */
	virtual float		getFloat(int attributeIndex) const
	{ return (float) values[attributeIndex]; };

	/**
	 * Returns value defined for the input attribute index as a long
	 */
	virtual LONG_INT	getLong(int attributeIndex) const
	{ return (LONG_INT) values[attributeIndex]; };

	/**
	 * Returns value defined for the input attribute index as an int
	 */
	virtual int			getInt(int attributeIndex) const
	{ return (int) values[attributeIndex]; };

	/**
	 * Returns value defined for the input attribute index as a short
	 */
	virtual short		getShort(int attributeIndex) const
	{ return (short) values[attributeIndex]; };

	/**
	 * Returns value defined for the input attribute index as a byte
	 */
	virtual byte		getByte(int attributeIndex) const
	{ return (byte) values[attributeIndex]; };

	/**
	 * Returns the attribute at the input attribute index as a double value.
	 */
	virtual void						getValue(int attributeIndex, double& val) const
	{ val = (double) values[attributeIndex]; };

	/**
	 * Returns the attribute at the input attribute index as a float value.
	 */
	virtual void						getValue(int attributeIndex, float& val) const
	{ val = (float) values[attributeIndex]; };

	/**
	 * Returns the attribute at the input attribute index as a long value.
	 */
	virtual void						getValue(int attributeIndex, LONG_INT& val) const
	{ val = (LONG_INT) values[attributeIndex]; };

	/**
	 * Returns the attribute at the input attribute index as a int value.
	 */
	virtual void						getValue(int attributeIndex, int& val) const
	{ val = (int) values[attributeIndex]; };

	/**
	 * Returns the attribute at the input attribute index as a short value.
	 */
	virtual void						getValue(int attributeIndex, short& val) const
	{ val = (short) values[attributeIndex]; };

	/**
	 * Returns the attribute at the input attribute index as a byte value.
	 */
	virtual void						getValue(int attributeIndex, byte& val) const
	{ val = (byte) values[attributeIndex]; };

	/**
	 * Copy the first n values into the supplied array as a double value.
	 */
	virtual void getValues(double vals[], const int& n)
	{ for (int i=0; i<n && i<nValues; ++i) vals[i] = (double) values[i]; }

	/**
	 * Copy the first n values into the supplied array as a float value.
	 */
	virtual void getValues(float vals[], const int& n)
	{ for (int i=0; i<n && i<nValues; ++i) vals[i] = (float) values[i]; }

	/**
	 * Copy the first n values into the supplied array as a LONG_INT value.
	 */
	virtual void getValues(LONG_INT vals[], const int& n)
	{ for (int i=0; i<n && i<nValues; ++i) vals[i] = (LONG_INT) values[i]; }

	/**
	 * Copy the first n values into the supplied array as an int value.
	 */
	virtual void getValues(int vals[], const int& n)
	{ for (int i=0; i<n && i<nValues; ++i) vals[i] = (int) values[i]; }

	/**
	 * Copy the first n values into the supplied array as a short value.
	 */
	virtual void getValues(short vals[], const int& n)
	{ for (int i=0; i<n && i<nValues; ++i) vals[i] = (short) values[i]; }

	/**
	 * Copy the first n values into the supplied array as a byte value.
	 */
	virtual void getValues(byte vals[], const int& n)
	{ for (int i=0; i<n && i<nValues; ++i) vals[i] = (byte) values[i]; }

	/**
	 * Set the value at the input attribute index to the input value.
	 */
	virtual GeoTessData&						setValue(int attributeIndex, double v)
	{ values[attributeIndex] = (T) v; return *this; }

	/**
	 * Set the value at the input attribute index to the input value.
	 */
	virtual GeoTessData&						setValue(int attributeIndex, float v)
	{ values[attributeIndex] = (T) v; return *this; }

	/**
	 * Set the value at the input attribute index to the input value.
	 */
	virtual GeoTessData&						setValue(int attributeIndex, LONG_INT v)
	{ values[attributeIndex] = (T) v; return *this; }

	/**
	 * Set the value at the input attribute index to the input value.
	 */
	virtual GeoTessData&						setValue(int attributeIndex, int v)
	{ values[attributeIndex] = (T) v; return *this; }

	/**
	 * Set the value at the input attribute index to the input value.
	 */
	virtual GeoTessData&						setValue(int attributeIndex, short v)
	{ values[attributeIndex] = (T) v; return *this; };

	/**
	 * Set the value at the input attribute index to the input value.
	 */
	virtual GeoTessData&						setValue(int attributeIndex, byte v)
	{ values[attributeIndex] = (T) v; return *this; }

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
	 * Returns a deep copy of this DataArray<T> object.
	 */
	virtual GeoTessDataArray<T>*   copy()
	{
		return new GeoTessDataArray<T>(values, nValues);
	}

}; // end class DataArray

/// @cond PROTECTED  Turn off doxygen documentation until 'endcond' is found

/**
 * Template specialization for isNaN()
 */
template<>
inline bool	GeoTessDataArray<double>::isNaN(int attributeIndex) const
{
	return (isnan(values[attributeIndex]));
}

/**
 * Template specialization for isNaN()
 */
template<>
inline bool	GeoTessDataArray<float>::isNaN(int attributeIndex) const
{
	double v = (double) values[attributeIndex];
	return (isnan(v));
}

/**
 * Template specialization for getDataType()
 */
template<>
inline const GeoTessDataType&	GeoTessDataArray<double>::getDataType() const
{
	return GeoTessDataType::DOUBLE;
}

/**
 * Template specialization for getDataType()
 */
template<>
inline const GeoTessDataType&	GeoTessDataArray<float>::getDataType() const
{
	return GeoTessDataType::FLOAT;
}

/**
 * Template specialization for getDataType()
 */
template<>
inline const GeoTessDataType&	GeoTessDataArray<LONG_INT>::getDataType() const
{
	return GeoTessDataType::LONG;
}

/**
 * Template specialization for getDataType()
 */
template<>
inline const GeoTessDataType&	GeoTessDataArray<int>::getDataType() const
{
	return GeoTessDataType::INT;
}

/**
 * Template specialization for getDataType()
 */
template<>
inline const GeoTessDataType&	GeoTessDataArray<short>::getDataType() const
{
	return GeoTessDataType::SHORT;
}

/**
 * Template specialization for getDataType()
 */
template<>
inline const GeoTessDataType&	GeoTessDataArray<byte>::getDataType() const
{
	return GeoTessDataType::BYTE;
}

///@endcond

} // end namespace geotess

#endif  // DATAARRAY_OBJECT_H
