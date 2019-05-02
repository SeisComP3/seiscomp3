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

//#include <iostream>
//#include <iomanip>

// **** _LOCAL INCLUDES_ *******************************************************

#include "GeoTessPolygon.h"
#include "GeoTessPolygon3D.h"
#include "GeoTessHorizon.h"
#include "GeoTessHorizonLayer.h"
#include "GeoTessHorizonDepth.h"
#include "GeoTessHorizonRadius.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

GeoTessPolygon3D::GeoTessPolygon3D(string inputFileName)
: GeoTessPolygon(), bottom(NULL), top(NULL)
{
	if (inputFileName.find(".kmz", inputFileName.length() - 4) != string::npos
			|| inputFileName.find(".kml", inputFileName.length() - 4) != string::npos)
	{
		ostringstream os;
		os << endl << "ERROR in Polygon::constructor" << endl
				<< "Cannot read files in kml or kmz format (Google Earth)." << endl
				<< "GeoTessExplorer has a utility to translate to ascii." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 10101);
	}

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
		loadAscii(records);
	else
	{
		ostringstream os;
		os << endl << "ERROR in Polygon3D::constructor" << endl
				<< "Expecting file to to start with string 'POLYGON3D' but first line is" << endl
				<< records[0] << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 10102);
	}

	input.close();

	//cout << toString(true, true) << endl;
}

GeoTessPolygon3D::~GeoTessPolygon3D()
{
	if (top != NULL)
		delete top;
	if (bottom != NULL)
		delete bottom;
}

bool GeoTessPolygon3D::contains(GeoTessPosition& position)
{
	map<int, double> weights;
	position.getWeights(weights, 1.0);
	return weights.count(-1) == 0;
}

void GeoTessPolygon3D::loadAscii(vector<string>& records)
{
	for (int i=0; i<(int)records.size(); ++i)
		if (records[i].find("BOTTOM") != string::npos || records[i].find("TOP") != string::npos)
		{
			vector<string> tokens;
			CPPUtils::tokenizeString(records[i], " ", tokens);

			if (tokens.size() == 4)
			{
				double x = CPPUtils::stod(tokens[2]);
				if (isnan(x))
				{
					ostringstream os;
					os << endl << "ERROR in Polygon3D::loadAscii" << endl
							<< "Trying to parse 4 tokens defining top or bottom horizon." << endl
							<< "Line = " << records[i] << endl
							<< "Cannot convert 3rd token to double: " << tokens[2] << endl;
					throw GeoTessException(os, __FILE__, __LINE__, 10103);
				}

				int layer = CPPUtils::stoi(tokens[3]);
				if (layer == -999999)
				{
					ostringstream os;
					os << endl << "ERROR in Polygon3D::loadAscii" << endl
							<< "Trying to parse 4 tokens defining top or bottom horizon." << endl
							<< "Line = " << records[i] << endl
							<< "Cannot convert 4th token to int: " << tokens[3] << endl;
					throw GeoTessException(os, __FILE__, __LINE__, 10104);
				}

				if (tokens[0].find("TOP") == 0 && tokens[1].find("LAYER") == 0)
					top = new GeoTessHorizonLayer(x, layer);
				else if (tokens[0].find("TOP") == 0 && tokens[1].find("DEPTH") == 0)
					top = new GeoTessHorizonDepth(x, layer);
				else if (tokens[0].find("TOP") == 0 && tokens[1].find("RADIUS") == 0)
					top = new GeoTessHorizonRadius(x, layer);
				else if (tokens[0].find("BOTTOM") == 0 && tokens[1].find("LAYER") == 0)
					bottom = new GeoTessHorizonLayer(x, layer);
				else if (tokens[0].find("BOTTOM") == 0 && tokens[1].find("DEPTH") == 0)
					bottom = new GeoTessHorizonDepth(x, layer);
				else if (tokens[0].find("BOTTOM") == 0 && tokens[1].find("RADIUS") == 0)
					bottom = new GeoTessHorizonRadius(x, layer);
				else
				{
					ostringstream os;
					os << endl << "ERROR in Polygon3D::loadAscii" << endl
							<< "Expected to find [top | bottom] [depth | radius | layer] value layerIndex" << endl
							<< "but actually found: " << records[i] << endl;
					throw GeoTessException(os, __FILE__, __LINE__, 10105);
				}
			}
		}

	GeoTessPolygon::loadAscii(records);

}

void GeoTessPolygon3D::write(const string& outputFileName)
{
	IFStreamAscii output;
	output.openForWrite(outputFileName);
	output.writeStringNL("POLYGON3D");
	output.writeStringNL("top "+top->str());
	output.writeStringNL("bottom "+bottom->str());
	output.writeStringNL("lat-lon");
	output.writeString("referencePoint ");
	output.writeString(GeoTessUtils::getLatLonString(referencePoint));
	output.writeStringNL(referenceIn ? " in" : " out");
	output.writeString(str(false, true, -180.));
	output.close();
}

} // end namespace geotess
