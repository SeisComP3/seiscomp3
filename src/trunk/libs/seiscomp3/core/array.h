/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SC_CORE_ARRAY_H__
#define __SC_CORE_ARRAY_H__

#include <seiscomp3/core/baseobject.h>
#include <string>


namespace Seiscomp {

DEFINE_SMARTPOINTER(Array);

/**
 * Generic abstract base class of certain array types.
 */
class SC_SYSTEM_CORE_API Array : public Seiscomp::Core::BaseObject {
	DECLARE_SC_CLASS(Array);

	public:
		//! Specifies the supported array data types.
		enum DataType {
			CHAR,
			INT,
			FLOAT,
			DOUBLE,
			DATETIME,
			STRING,
			COMPLEX_FLOAT,
			COMPLEX_DOUBLE,
			DT_QUANTITY
		};
	

	protected:
		//! Initializing Constructor
		Array(DataType dt);

	public:
		//! Destructor
		virtual ~Array();
	
		//! Returns the data type of the array
		DataType dataType() const { return _datatype; }
	
		//! Returns a clone of the array
		Array* clone() const;

		//! Returns a copy of the array of the specified data type.
		virtual Array* copy(DataType dt) const = 0;
	
		//! Returns the data address pointer.
		virtual const void *data() const = 0;
	
		//! Returns the size of the array.
		virtual int size() const = 0;

		//! Resizes the array
		virtual void resize(int size) = 0;

		//! Drops all elements.
		virtual void clear() = 0;
	
		//! Returns the number of bytes of an array element.
		virtual int bytes() const = 0;

		//! Appends the given array to this array.
		virtual void append(const Array*) = 0;

		//! Concatenates the given array to this array.
		//		virtual void concatenate(Array*) = 0;

		//! Returns the slice m...n-1 of the array
		virtual Array* slice(int m, int n) const = 0;

		//! Converts the array into a binary stream of
		//! chars
		std::string str() const;

	private:
		DataType _datatype;
};

}

#endif
