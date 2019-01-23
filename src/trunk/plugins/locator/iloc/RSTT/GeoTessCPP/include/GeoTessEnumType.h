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

#ifndef ENUMTYPE_OBJECT_H
#define ENUMTYPE_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <string>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess
{

// **** _FORWARD REFERENCES_ ***************************************************

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief The base class for all "enum" types.
 *
 * The base class for all "enum" types. Contains the name string and ordinal of
 * the enum and functions to return those attributes. Also defines operator
 * overloads for equality (==) and non-equality (!=).
 */
class GEOTESS_EXP_IMP GeoTessEnumType
{
protected:

	/**
	 * Private default constructor. Not used.
	 */
	GeoTessEnumType()
			: aName(""), aOrdinal(-1)
	{
	}

	/**
	 * Private copy constructor. Not used.
	 */
	GeoTessEnumType(const GeoTessEnumType& et)
			: aName(et.aName), aOrdinal(et.aOrdinal)
	{
	}

	/**
	 * Private assignment operator. Not used.
	 */
	GeoTessEnumType& operator=(const GeoTessEnumType& et)
	{
		return *this;
	}

	/**
	 * The string name of this enum.
	 */
	const string aName;

	/**
	 * The ordinal of this enum.
	 */
	const int aOrdinal;

	/**
	 * Standard constuctor. Protected so that only derived types (public enums)
	 * which inherit this object can actually create one.
	 */
	GeoTessEnumType(const string& name, int ordinal)
			: aName(name), aOrdinal(ordinal)
	{
	}

	/**
	 * Returns the EnumType from the input array whose name matches the input
	 * string. Null is returned if no match is found.
	 *
	 * @param s     The input string for which a match in array is returned
	 *              (or null).
	 * @param array The array from which a match for s will be sought.
	 * @param n     The size of the input array.
	 * @return      The match of s in the names of array or null if one was
	 *              not found.
	 */
	static const GeoTessEnumType* valueOf(const string& s,
			GeoTessEnumType const* const * const array, int n);

public:

	/**
	 * Standard destructor.
	 */
	virtual ~GeoTessEnumType() { }

	/**
	 * Returns this Enums name.
	 */
	string toString() const { return aName; }

	/**
	 * Returns this Enums name.
	 */
	string name() const { return aName; }

	/**
	 * Returns this Enums ordinal.
	 */
	int ordinal() const { return aOrdinal; }

	/**
	 * Equals operator.
	 */
	friend bool operator==(const GeoTessEnumType& x, const GeoTessEnumType& y) { return &(x) == &(y); }

	/**
	 * Not equals operator.
	 */
	friend bool operator!=(const GeoTessEnumType& x, const GeoTessEnumType &y) { return !(x == y); }

};
// end class EnumType

/**
 * Stream operator that will print the input EnumTypes name to the stream.
 */
inline std::ostream& operator<<(std::ostream& o, GeoTessEnumType& s)
{ return o << s.toString(); }

} // end namespace geotess

#endif  // ENUMTYPE_OBJECT_H
