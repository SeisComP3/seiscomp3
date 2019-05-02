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

#ifndef GEO_DATA_OBJECT_H
#define GEO_DATA_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <string>
#include <fstream>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPGlobals.h"
#include "GeoTessDataType.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

class GeoTessMetaData;
class IFStreamAscii;
class IFStreamBinary;
template<typename T> class GeoTessDataArray;
template<typename T> class GeoTessDataValue;

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Abstract base class that manages the data values attached to a single
 * grid point.
 *
 * Abstract class that manages the data values attached to single grid point in
 * the model. Data objects have no information about the position in the model
 * where the data is located.
 */
class GEOTESS_EXP_IMP GeoTessData
{
  private:

		/**
		 * The static class instance count.
		 */
		static int				aClassCount;

	public:

		/**
		 * Returns the class name.
		 */
		static  string		class_name() { return "Data"; };

		/**
		 * Returns the class size.
		 */
		virtual int			class_size() const { return (int) sizeof(GeoTessData); };

		/**
		 * Returns the class instance count.
		 */
		static  int			class_count() { return aClassCount; };

		virtual LONG_INT getMemory() = ABSTRACT;

		/**
		 * Return true if the input Data object data type is the same as this
		 * Data object.
		 */
		virtual bool		operator == (const GeoTessData& d) const
		{	return (getDataType() == d.getDataType()); }

		/**
		 * Factory method that will return a GeoTessData of the specified GeoTessDataType
		 * with all values initialized to 0.
		 *
		 * @param dataType the type of the requested GeoTessData object.
		 * @param nAttributes
		 * @return a GeoTessData object of the correct derived type
		 */
		static GeoTessData* getData(const GeoTessDataType& dataType, int nAttributes);

		/**
		 * Factory method that will return a pointer to a Data object of the
		 * correct derived type (DataValue if size==1 or DataValue if size > 1).
		 * The data values are copied from the supplied array of values into new Data structures.
		 * <p>Caller assumes ownership of the returned Data object.
		 * @param values an array of data values
		 * @param size the number of data values in the array
		 * @return  either a DataArray or DataValue object of the correct intrinsic type.
		 */
		static GeoTessData* getData(double values[], const int& size);
		static GeoTessData* getData(const vector<double>& values);

		/**
		 * Factory method that will return a pointer to a Data object of the
		 * correct derived type (DataValue if size==1 or DataValue if size > 1).
		 * The data values are copied from the supplied array of values into new Data structures.
		 * <p>Caller assumes ownership of the returned Data object.
		 * @param values an array of data values
		 * @param size the number of data values in the array
		 * @return  a pointer to either a DataArray or DataValue object of the correct intrinsic type.
		 */
		static GeoTessData* getData(float values[], const int& size);
		static GeoTessData* getData(const vector<float>& values);

		/**
		 * Factory method that will return a pointer to a Data object of the
		 * correct derived type (DataValue if size==1 or DataValue if size > 1).
		 * The data values are copied from the supplied array of values into new Data structures.
		 * <p>Caller assumes ownership of the returned Data object.
		 * @param values an array of data values
		 * @param size the number of data values in the array
		 * @return  either a DataArray or DataValue object of the correct intrinsic type.
		 */
		static GeoTessData* getData(LONG_INT values[], const int& size);
		static GeoTessData* getData(const vector<LONG_INT>& values);

		/**
		 * Factory method that will return a pointer to a Data object of the
		 * correct derived type (DataValue if size==1 or DataValue if size > 1).
		 * The data values are copied from the supplied array of values into new Data structures.
		 * <p>Caller assumes ownership of the returned Data object.
		 * @param values an array of data values
		 * @param size the number of data values in the array
		 * @return  either a DataArray or DataValue object of the correct intrinsic type.
		 */
		static GeoTessData* getData(int values[], const int& size);
		static GeoTessData* getData(const vector<int>& values);

		/**
		 * Factory method that will return a pointer to a Data object of the
		 * correct derived type (DataValue if size==1 or DataValue if size > 1).
		 * The data values are copied from the supplied array of values into new Data structures.
		 * <p>Caller assumes ownership of the returned Data object.
		 * @param values an array of data values
		 * @param size the number of data values in the array
		 * @return  either a DataArray or DataValue object of the correct intrinsic type.
		 */
		static GeoTessData* getData(short values[], const int& size);
		static GeoTessData* getData(const vector<short>& values);

		/**
		 * Factory method that will return a pointer to a Data object of the
		 * correct derived type (DataValue if size==1 or DataValue if size > 1).
		 * The data values are copied from the supplied array of values into new Data structures.
		 * <p>Caller assumes ownership of the returned Data object.
		 * @param values an array of data values
		 * @param size the number of data values in the array
		 * @return  either a DataArray or DataValue object of the correct intrinsic type.
		 */
		static GeoTessData* getData(byte values[], const int& size);
		static GeoTessData* getData(const vector<byte>& values);

		/**
		 * Retrieve the DataType of this Data object. One of
		 * DOUBLE, FLOAT, LONG, INT SHORT, BYTE
		 * @return the DataType of this Data object.
		 */
		virtual const GeoTessDataType& getDataType() const = ABSTRACT;

		/**
		 * Retrieve the number of attributes stored in this Data object.
		 * @return  the number of attributes stored in this Data object.
		 */
		virtual int			size() const = ABSTRACT;

		/**
		 * Retrieve the value of the attribute at the specified attribute index
		 * as a double value.
		 * @return  the value of the attribute at the specified attribute index
		 */
		virtual double		getDouble(int attributeIndex) const = ABSTRACT;

		/**
		 * Retrieve the value of the attribute at the specified attribute index
		 * as a float value.
		 * @return  the value of the attribute at the specified attribute index
		 */
		virtual float		getFloat(int attributeIndex) const = ABSTRACT;

		/**
		 * Retrieve the value of the attribute at the specified attribute index
		 * as a LONG_INT value.
		 * @return  the value of the attribute at the specified attribute index
		 */
		virtual LONG_INT	getLong(int attributeIndex) const = ABSTRACT;

		/**
		 * Retrieve the value of the attribute at the specified attribute index
		 * as a int value.
		 * @return  the value of the attribute at the specified attribute index
		 */
		virtual int			getInt(int attributeIndex) const = ABSTRACT;

		/**
		 * Retrieve the value of the attribute at the specified attribute index
		 * as a short value.
		 * @return  the value of the attribute at the specified attribute index
		 */
		virtual short		getShort(int attributeIndex) const = ABSTRACT;

		/**
		 * Retrieve the value of the attribute at the specified attribute index
		 * as a bute value.
		 * @return  the value of the attribute at the specified attribute index
		 */
		virtual byte		getByte(int attributeIndex) const = ABSTRACT;

		/**
		 * Retrieve the attribute at the input attribute index as a double value.
		 */
		virtual void		getValue(int attributeIndex, double& val) const = ABSTRACT;

		/**
		 * Retrieve the attribute at the input attribute index as a float value.
		 */
		virtual void		getValue(int attributeIndex, float& val) const = ABSTRACT;

		/**
		 * Retrieve the attribute at the input attribute index as a LONG_INT value.
		 */
		virtual void		getValue(int attributeIndex, LONG_INT& val) const = ABSTRACT;

		/**
		 * Retrieve the attribute at the input attribute index as a int value.
		 */
		virtual void		getValue(int attributeIndex, int& val) const = ABSTRACT;

		/**
		 * Retrieve the attribute at the input attribute index as a short value.
		 */
		virtual void		getValue(int attributeIndex, short& val) const = ABSTRACT;

		/**
		 * Retrieve the attribute at the input attribute index as a byte value.
		 */
		virtual void		getValue(int attributeIndex, byte& val) const = ABSTRACT;

		/**
		 * Copy the first n data values into the supplied array.
		 * @param values the array into which data values will be copied.
		 * @param n the number of values to copy.
		 */
		virtual void        getValues(double values[], const int& n) = ABSTRACT;

		/**
		 * Copy the first n data values into the supplied array.
		 * @param values the array into which data values will be copied.
		 * @param n the number of values to copy.
		 */
		virtual void        getValues(float values[], const int& n) = ABSTRACT;

		/**
		 * Copy the first n data values into the supplied array.
		 * @param values the array into which data values will be copied.
		 * @param n the number of values to copy.
		 */
		virtual void        getValues(LONG_INT values[], const int& n) = ABSTRACT;

		/**
		 * Copy the first n data values into the supplied array.
		 * @param values the array into which data values will be copied.
		 * @param n the number of values to copy.
		 */
		virtual void        getValues(int values[], const int& n) = ABSTRACT;

		/**
		 * Copy the first n data values into the supplied array.
		 * @param values the array into which data values will be copied.
		 * @param n the number of values to copy.
		 */
		virtual void        getValues(short values[], const int& n) = ABSTRACT;

		/**
		 * Copy the first n data values into the supplied array.
		 * @param values the array into which data values will be copied.
		 * @param n the number of values to copy.
		 */
		virtual void        getValues(byte values[], const int& n) = ABSTRACT;


		/**
		 * Set the value of the specified attributeIndex and return a
		 * reference to this Data object.
		 * @param attributeIndex the index of the attribute that is to be modified.
		 * @param v the new value that is to replace the old value.
		 * @return a reference to this Data object.
		 */
		virtual GeoTessData&		setValue(int attributeIndex, double v) = ABSTRACT;

		/**
		 * Set the value of the specified attributeIndex and return a
		 * reference to this Data object.
		 * @param attributeIndex the index of the attribute that is to be modified.
		 * @param v the new value that is to replace the old value.
		 * @return a reference to this Data object.
		 */
		virtual GeoTessData&		setValue(int attributeIndex, float v) = ABSTRACT;

		/**
		 * Set the value of the specified attributeIndex and return a
		 * reference to this Data object.
		 * @param attributeIndex the index of the attribute that is to be modified.
		 * @param v the new value that is to replace the old value.
		 * @return a reference to this Data object.
		 */
		virtual GeoTessData&		setValue(int attributeIndex, LONG_INT v) = ABSTRACT;

		/**
		 * Set the value of the specified attributeIndex and return a
		 * reference to this Data object.
		 * @param attributeIndex the index of the attribute that is to be modified.
		 * @param v the new value that is to replace the old value.
		 * @return a reference to this Data object.
		 */
		virtual GeoTessData&		setValue(int attributeIndex, int v) = ABSTRACT;

		/**
		 * Set the value of the specified attributeIndex and return a
		 * reference to this Data object.
		 * @param attributeIndex the index of the attribute that is to be modified.
		 * @param v the new value that is to replace the old value.
		 * @return a reference to this Data object.
		 */
		virtual GeoTessData&		setValue(int attributeIndex, short v) = ABSTRACT;

		/**
		 * Set the value of the specified attributeIndex and return a
		 * reference to this Data object.
		 * @param attributeIndex the index of the attribute that is to be modified.
		 * @param v the new value that is to replace the old value.
		 * @return a reference to this Data object.
		 */
		virtual GeoTessData&		setValue(int attributeIndex, byte v) = ABSTRACT;


		/**
		 * Fill the Data object values with the input fillValue, casting the
		 * input value to the intrinsic type of this Data object if necessary.
		 * @param fillValue the value that is to replace all existing
		 * Data values.
		 */
		template <typename T>
		void fill(T fillValue) { for (int i=0; i<size(); ++i) setValue(i,fillValue);}

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
		virtual bool              isNaN(int attributeIndex) const = ABSTRACT;


		/// @cond PROTECTED  Turn off doxygen documentation until 'endcond' is found

		/**
		 * Default constructor.
		 */
								GeoTessData() { ++aClassCount; };

		/**
		 * Default destructor.
		 */
		virtual					~GeoTessData();

		/**
		 * Make a copy of this data.
		 * <p>Caller assumes ownership of the returned Data object.
		 */
		virtual GeoTessData*     copy() = ABSTRACT;

		/**
		 * Returns a new Data object of the type specified by the DataType
		 * parameter (dataType) read from the input file stream.
		 * <p>Caller assumes ownership of the returned Data object.
		 */
		static GeoTessData*			getData(IFStreamBinary& ifs, GeoTessMetaData& gtmd);

		/**
		 * Factory method that will return a Data of the correct derived type.
		 * The data are read from a IFStreamAscii object which is assumed to be
		 * at the correct position in the underlying ascii stream.
		 */
		static GeoTessData*			getData(IFStreamAscii& input,  GeoTessMetaData& gtmd);

		/**
		 * Abstract function that writes this Data derived object to the input
		 * output file stream.
		 */
		virtual void			write(IFStreamBinary& ofs) = ABSTRACT;

		/**
		 * Abstract function that writes this Data derived object to the input
		 * output file stream.
		 */
		virtual void			write(IFStreamAscii& ofs) = ABSTRACT;

		///@endcond

}; // end class Data

} // end namespace geotess

#endif  // DATA_OBJECT_H
