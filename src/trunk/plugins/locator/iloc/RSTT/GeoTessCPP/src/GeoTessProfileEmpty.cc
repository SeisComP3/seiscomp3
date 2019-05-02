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

#include "GeoTessUtils.h"
#include "GeoTessException.h"
#include "GeoTessProfileEmpty.h"
#include "IFStreamAscii.h"
#include "GeoTessDataValue.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

/**
 * Retrieve the value of the specified attribute interpolated at the
 * specified radius. Unsupported for ProfileEmpty
 */
double	GeoTessProfileEmpty::getValue(const GeoTessInterpolatorType& rInterpType,
											    		 int attributeIndex, double radius,
											    		 bool allowRadiusOutOfRange) const
{
	return NaN_DOUBLE;
}

/**
 * Retrieve a reference to all of the Data objects associated with this
 * Profile.
 */
GeoTessData**	GeoTessProfileEmpty::getData()
{
	ostringstream os;
	os << endl << "ERROR in ProfileEmpty::getData" << endl
		 << "Unsupported method call." << endl;
	throw GeoTessException(os, __FILE__, __LINE__, 4201);
	return (GeoTessData**) NULL;
}

/**
 * Retrieve a reference the i'th Data object
 */
GeoTessData*	GeoTessProfileEmpty::getData(int i)
{
	return (GeoTessData*) NULL;
}

/**
 * Retrieve a reference the i'th Data object
 */
const GeoTessData&	GeoTessProfileEmpty::getData(int i) const
{
	ostringstream os;
	os << endl << "ERROR in ProfileEmpty::getData" << endl
		 << "Unsupported method call." << endl;
	throw GeoTessException(os, __FILE__, __LINE__, 4203);
	return *(new GeoTessDataValue<int>());
}

/**
 * Get the Data object at the top of the profile.
 */
const GeoTessData&	GeoTessProfileEmpty::getDataTop() const
{
	ostringstream os;
	os << endl << "ERROR in ProfileEmpty::getDataTop" << endl
		 << "Unsupported method call." << endl;
	throw GeoTessException(os, __FILE__, __LINE__, 4204);
	return *(new GeoTessDataValue<int>());
}

/**
 * Get the Data object at the bottom of the profile.
 */
const GeoTessData&	GeoTessProfileEmpty::getDataBottom() const
{
	ostringstream os;
	os << endl << "ERROR in ProfileEmpty::getDataBottom" << endl
		 << "Unsupported method call." << endl;
	throw GeoTessException(os, __FILE__, __LINE__, 4205);
	return *(new GeoTessDataValue<int>());
}

} // end namespace geotess
