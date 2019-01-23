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

#ifndef POLYGONFACTORY_H_
#define POLYGONFACTORY_H_

// **** _SYSTEM INCLUDES_ ******************************************************

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <vector>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"
#include "GeoTessUtils.h"
#include "GeoTessGreatCircle.h"
#include "IFStreamAscii.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Factory method with a static method getPolygon() that returns
 * a pointer to either a Polygon or Polygon3D object.
 *
 * Factory method with static method getPolygon() that returns
 * a pointer to a Polygon or Polygon3D object.
 *
 */
class GEOTESS_EXP_IMP GeoTessPolygonFactory
{
public:

	static GeoTessPolygon* getPolygon(string inputFileName)
	{
		if (inputFileName.find(".kmz", inputFileName.length() - 4) != string::npos
				|| inputFileName.find(".kml", inputFileName.length() - 4) != string::npos)
		{
			ostringstream os;
			os << endl << "ERROR in Polygon::constructor" << endl
					<< "Cannot read files in kml or kmz format (Google Earth)." << endl
					<< "GeoTessExplorer has a utility to translate to ascii." << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 10201);
		}

		GeoTessPolygon* polygon = NULL;

		vector<string> records;

		IFStreamAscii input;
		input.openForRead(inputFileName);
		string line;
		while (input.readLine(line))
		{
			line = CPPUtils::uppercase_string(CPPUtils::trim(line));
			if (line.length() > 0 && line.find('#') != 0)
				records.push_back(line);
		}
		input.close();

		if (records[0].find("POLYGON3D") == 0)
		{
			polygon = new GeoTessPolygon3D();
			polygon->loadAscii(records);
		}
		else
		{
			polygon = new GeoTessPolygon();
			polygon->loadAscii(records);
		}

		return polygon;
	}


}; // end class PolygonFactory

} // end namespace geotess

#endif /* POLYGONFACTORY_H_ */
