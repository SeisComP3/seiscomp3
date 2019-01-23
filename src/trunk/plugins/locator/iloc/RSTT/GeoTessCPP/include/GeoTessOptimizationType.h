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

#ifndef OPTIMIZATIONTYPE_OBJECT_H
#define OPTIMIZATIONTYPE_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <string>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "GeoTessEnumType.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess
{

// **** _FORWARD REFERENCES_ ***************************************************

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Enumeration of the optimization strategies supported by GeoTess
 * including OptimizationType::SPEED and OptimizationType::MEMORY.
 *
 * Optimize for speed or memory
 */
class GEOTESS_EXP_IMP GeoTessOptimizationType: public GeoTessEnumType
{
private:

	/**
	 * Private default constructor. Not used.
	 */
	GeoTessOptimizationType() : GeoTessEnumType()
	{
	}

	/**
	 * Private copy constructor. Not used.
	 */
	GeoTessOptimizationType(const GeoTessOptimizationType& ot) : GeoTessEnumType(ot)
	{
	}

	/**
	 * Private assignment operator. Not used.
	 */
	GeoTessOptimizationType& operator=(const GeoTessOptimizationType& ot)
	{
		return *this;
	}

	/**
	 * Total number of enums of this type that were created.
	 */
	static const int aSize;

	/**
	 * Inlined ordinal generator that increments the ordinal for each new enum
	 * created by this type.
	 */
	static inline int nextOrdinal()
	{
		static int firstOrdinal = 0;
		return firstOrdinal++;
	}

	/**
	 * Standard constructor.
	 *
	 * @param name The string name for this new enum instance.
	 */
	GeoTessOptimizationType(const string& name)
			: GeoTessEnumType(name, nextOrdinal())
	{
	}

public:

	/**
	 * The actual enums for this type.
	 */
	static const GeoTessOptimizationType SPEED;
	static const GeoTessOptimizationType MEMORY;

	/**
	 * The array containing all enums declared above.
	 */
	static const GeoTessOptimizationType* aArray[];

	/**
	 * Standard Destructor.
	 */
	virtual ~GeoTessOptimizationType()
	{
	}

//		/**
//		 * Assignment Operator.
//		 */
//		OptimizationType& OptimizationType::operator=(const OptimizationType& ot)
//		{
//			EnumType::operator=(ot);
//
//			return *this;
//		}

	/**
	 * Returns a pointer to the enum whose string matches the input string. If
	 * no match is found null is returned.
	 *
	 * @param s The input string for which a match in array is returned (or null).
	 * @return The match of s in the names of array or null if one was not found.
	 */
	static const GeoTessOptimizationType* valueOf(const string& s)
	{
		return (const GeoTessOptimizationType*) GeoTessEnumType::valueOf(s,
				(GeoTessEnumType const* const * const ) aArray, aSize);
	}
	;

	/**
	 * Returns the array of all enums for this type.
	 */
	static GeoTessOptimizationType const* const * const values()
	{
		return aArray;
	}

	/**
	 * Returns the total number of enums of this type.
	 */
	static int size()
	{
		return aSize;
	}

};
// end class OptimizationType

}// end namespace geotess

#endif  // OPTIMIZATIONTYPE_OBJECT_H
