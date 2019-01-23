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

#include <iostream>
#include <iomanip>

// **** _LOCAL INCLUDES_ *******************************************************

#include "GeoTessPolygon.h"
#include "GeoTessModel.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

// TOLERANCE is the angular distance between two points, in radians, such that
// if the angular separation is less than this value the two points are
// considered equal.  Two points at the surface of the earth that are 1e-7
// radians apart are separated by about 0.6 meters.
double GeoTessPolygon::TOLERANCE = 1e-7;

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

GeoTessPolygon::GeoTessPolygon()
: referencePoint(NULL), referenceIn(false), global(false), lonFirst(false), refCount(0), attachment(NULL)
{ }

GeoTessPolygon::GeoTessPolygon(vector<double*>& points)
: referencePoint(NULL), referenceIn(false), global(false), lonFirst(false), refCount(0), attachment(NULL)
{
	setup(points);
}

GeoTessPolygon::GeoTessPolygon(const double* center, double radius, int nEdges)
: referencePoint(NULL), referenceIn(false), global(false), lonFirst(false), refCount(0), attachment(NULL)
{
	vector<double*> points;
	points.reserve((size_t)nEdges);
	double* firstPoint = new double[3];
	if (!GeoTessUtils::isPole(center))
		GeoTessUtils::moveNorth(center, radius, firstPoint);
	else
	{
		double pole[3] = { 0., 1., 0. };
		GeoTessUtils::rotate(center, pole, radius, firstPoint);
	}
	points.push_back(firstPoint);

	double theta = 2. * PI/ (double) nEdges;
	for (int i = 1; i < nEdges; ++i)
	{
		double* point = new double[3];
		GeoTessUtils::rotate(firstPoint, center, i * theta, point);
		points.push_back(point);
	}

	referencePoint = new double[3];
	referencePoint[0] = center[0];
	referencePoint[1] = center[1];
	referencePoint[2] = center[2];
	referenceIn = true;

	setup(points);
}

GeoTessPolygon::GeoTessPolygon(string inputFileName)
: referencePoint(NULL), referenceIn(false), global(false), lonFirst(false), refCount(0), attachment(NULL)
{
	if (inputFileName.find(".kmz", inputFileName.length() - 4) != string::npos
			|| inputFileName.find(".kml", inputFileName.length() - 4) != string::npos)
	{
		ostringstream os;
		os << endl << "ERROR in Polygon::constructor" << endl
				<< "Cannot read files in kml or kmz format (Google Earth)." << endl
				<< "GeoTessExplorer has a utility to translate to ascii." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 10002);
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

	loadAscii(records);
}

GeoTessPolygon::~GeoTessPolygon()
{
	if (refCount > 0)
	{
		ostringstream os;
		os << endl << "ERROR in Polygon::~Polygon" << endl
			 << "Reference count (" << refCount << ") is not zero." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 10004);
	}

	  for (size_t i=0; i<edges.size(); ++i)
	  {
		  delete[] edges[i]->getFirst();
		  delete edges[i];
	  }

	  edges.clear();

	if (referencePoint != NULL)
	{
		delete[] referencePoint;
		referencePoint = NULL;
	}
}

void GeoTessPolygon::loadAscii(vector<string>& records)
{
	string reference="";

	// load all the lines that are not comments into a buffer
	vector<string> tokens;
	vector< vector<double> > values;
	double val1, val2;

	for (int i=0; i<(int)records.size(); ++i)
	{
		if (records[i].find("global") == 0 || records[i].find("GLOBAL") == 0)
		{
			global = true;
			referenceIn = true;

			// check to see if another word appears on the line.
			// if there is another word, and it is not 'in'
			// then referenceIn is false.
			vector<string> tokens;
			CPPUtils::tokenizeString(records[i], ", ", tokens);
			if (tokens.size() > 1)
				referenceIn = tokens[1].find("in") == 0
						|| tokens[1].find("IN") == 0;

			// set the reference point to the north pole.
			referencePoint = new double[3];
			referencePoint[0] = 1;
			referencePoint[1] = 0;
			referencePoint[2] = 0;
			edges.clear();
			return;
		}
		else if (records[i].find("lat") == 0 || records[i].find("LAT") == 0)
			lonFirst = false;
		else if (records[i].find("lon") == 0 || records[i].find("LON") == 0)
			lonFirst = true;
		else if (records[i].find("reference") == 0 || records[i].find("REFERENCE") == 0)
			reference = records[i];
		else
		{
			// if the line can be parsed as two values of type double,
			// parse them and add them to vector of values.
			// If not, the line is simply ignored.
			CPPUtils::tokenizeString(records[i], ", ", tokens);
			if (tokens.size() == 2)
			{
				val1 = CPPUtils::stod(tokens[0]);
				if (!isnan(val1))
				{
					val2 = CPPUtils::stod(tokens[1]);
					if (!isnan(val2))
					{
						vector<double> pair;
						pair.push_back(val1);
						pair.push_back(val2);
						values.push_back(pair);
					}
				}
			}
		}
	}

	vector<double*> points;
	points.reserve(values.size());

	if (lonFirst)
		for (size_t i=0; i<values.size(); ++i)
			points.push_back(GeoTessUtils::getVectorDegrees(values[i][1], values[i][0]));
	else
		for (size_t i=0; i<values.size(); ++i)
			points.push_back(GeoTessUtils::getVectorDegrees(values[i][0], values[i][1]));

	if (reference.length() > 0)
	{
		// refpoint should consist of 4 tokens: refpoint lat lon in|out
		stringstream ref(reference);
		string s, inout;
		double lat, lon;
		if (lonFirst)
			ref >> s >> lon >> lat >> inout;
		else
			ref >> s >> lat >> lon >> inout;

		referencePoint = GeoTessUtils::getVectorDegrees(lat, lon);

		referenceIn = inout.find("in") == 0 || inout.find("IN") == 0;
	}

	setup(points);

}

void GeoTessPolygon::write(const string& outputFileName)
{
	IFStreamAscii output;
	output.openForWrite(outputFileName);
	output.writeStringNL("POLYGON");
	output.writeStringNL("lat-lon");
	output.writeString("referencePoint ");
	output.writeString(GeoTessUtils::getLatLonString(referencePoint));
	output.writeStringNL(referenceIn ? " in" : " out");
	output.writeString(str(false, true, -180.));
	output.close();
}

/**
 * @param points
 *            an array of double[3]
 * @throws PolygonException
 * @throws
 */
void GeoTessPolygon::setup(vector<double*>& points)
{
	// search for points at the end of the list that are
	// equal to the first point in the list.
	// Remove and delete all that are found.
	int newSize = points.size();
	for (int i=points.size()-1; i > 0; --i)
	{
		if (GeoTessUtils::dot(points[i], points[0]) > cos(TOLERANCE))
		{
			delete[] points[i];
			points[i] = NULL;
			--newSize;
		}
		else
			break;
	}

	while ((int)points.size() > newSize)
		points.pop_back();


	if (points.size() < 3)
	{
		ostringstream os;
		os << endl << "ERROR in Polygon::setup" << endl
				<< "Cannot create a polygon with less than 3 points." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 10005);

	}

	// find the location of the vector sum of all the points.
	double center[3];
	if (referencePoint == NULL)
		GeoTessUtils::center(points, center);

	// there will be a GreatCircle edge for every vertex
	edges.reserve(points.size());
	int previous = 0;


	for (size_t i=1; i<points.size(); ++i)
	{
		if (GeoTessUtils::dot(points[previous], points[i]) > cos(TOLERANCE))
		{
			delete[] points[i];
			points[i] = NULL;
		}
		else
		{
			// create a great circle from previous to current point with
			// no intermediate point.  Specify that points are not to be
			// deleted in gc's destructor and that shortest path is to
			// be used.
			GeoTessGreatCircle* gc = new GeoTessGreatCircle();
			gc->set(points[previous], NULL, points[i], true, false);
			edges.push_back(gc);
			previous = i;
		}
	}

	if (GeoTessUtils::dot(points[previous], points[0]) < cos(TOLERANCE))
	{
		GeoTessGreatCircle* gc = new GeoTessGreatCircle();
		gc->set(points[previous], NULL, points[0], true, false);
		edges.push_back(gc);
	}


	if (referencePoint == NULL)
	{
		referencePoint = new double[3];
		referenceIn = false;

		// deal with degenerate case where the vector sum of the points is zero.
		// One way this can happen is if all the points are evenly distributed along
		// a great circle that encircles the globe (pretty unlikely).  In this case,
		// the selection of referencePoint and referencePointIn is pretty arbitrary.
		if (center[0] == 0. && center[1] == 0. && center[1] == 0.)
		{
			center[0] = center[1] = center[2] = 0.1;
			do
			{
				referencePoint[0] = center[0];
				referencePoint[1] = center[1]*=2;
				referencePoint[2] = center[2]*=4;
				GeoTessUtils::normalize(referencePoint);
			}
			while(onBoundary(referencePoint));
		}
		else
		{

			// flip the referencePoint so that it is on the opposite side of the Earth
			referencePoint[0] = -center[0];
			referencePoint[1] = -center[1];
			referencePoint[2] = -center[2];

			if (!onBoundary(center))
			{
				referenceIn = contains(center);
				referencePoint[0] = center[0];
				referencePoint[1] = center[1];
				referencePoint[2] = center[2];
			}
		}
	}

}

/**
 *
 * @param x
 *            double[3] unit vector
 * @return int
 * @throws PolygonException
 * @throws GreatCircleException
 */
int GeoTessPolygon::edgeCrossings(GeoTessGreatCircle& gcRef)
{
	// for every edge that has its last point on gcRef, we need to know the index
	// of the next edge that does not have its last point on gcRef.

	int j;
	int ncrossings=0;

	vector<int> nextEdge;
	nextEdge.reserve(edges.size());

	// for every edge[i], set nextEdge[i] = -1 if edge[i].getLast() is not on gcRef.
	for (int i=0; i<(int)edges.size(); ++i)
		if (abs(GeoTessUtils::dot(edges[i]->getLast(), gcRef.getNormal())) > sin(TOLERANCE))
			nextEdge.push_back(-1);
		else
			nextEdge.push_back(0);

	// for every edge[i] that has its last point on gcRef, set nextEdge[i]
	// equal to the index of the next edge[j] such that edge[j].getLast()
	// is not on gcRef.
	for (int i=0; i<(int)nextEdge.size(); ++i)
		if (nextEdge[i] == 0)
		{
			j = (i+1) % edges.size();
			while (nextEdge[j] >= 0)
				j = (j+1) % edges.size();
			nextEdge[i] = j;
		}

	bool* checked = new bool[edges.size()];
	for (int i=0; i<(int)edges.size(); ++i)
		checked[i] = false;

	// find j0 such that edge[j0].getFirst() is not on gcRef
	int firstj=0;
	for (int i=0; i<(int)edges.size(); ++i)
		if (abs(GeoTessUtils::dot(edges[i]->getFirst(), gcRef.getNormal())) > sin(TOLERANCE))
		{
			firstj=i;
			break;
		}

	// loop over all the edges, in order, starting from edge[firstj];
	j=firstj;
	do
	{
		if (nextEdge[j] >= 0)
		{
			// the last point of edge[j] is on gcRef.
			// Find edge[k] such that edge[k].getLast() is not on gcRef.
			int k = nextEdge[j];

			// figure out if there is a crossing here.  If edge[j] and edge[k]
			// are on different sides of gcRef, and the distance from reference
			// point to evaluation point is greater than the distance from the
			// reference point to edge[k].getFirst(), then a crossing occurred.
			if (GeoTessUtils::dot(edges[j]->getFirst(), gcRef.getNormal()) *
					GeoTessUtils::dot(edges[k]->getLast(), gcRef.getNormal()) < 0
					&& gcRef.getDistance() >= gcRef.getDistance(edges[k]->getFirst()))
				++ncrossings;

			// all the edges from edge[j] to edge[k] inclusive, have been checked
			// for crossings.  Record that fact.
			checked[j] = true;
			while (j != k)
			{
				j = (j+1) % edges.size();
				checked[j] = true;
			}
		}
		else
			j = (j+1) % edges.size();
	}
	while (j != firstj);

	//cout << "Polygon::edgeCrossings(GreatCircle& gcRef)1 = " << ncrossings << endl;

	// if referencePoint is inside and number of edge crossings is even,
	// or if referencePoint is outside and number of crossings is odd,
	// then x is inside the polygon, otherwise, it is outside.

	// loop over all the edges and count the intersections.
	double intersection[3];
	for (int i=0; i<(int)edges.size(); ++i)
		if (!checked[i] && gcRef.getIntersection(*edges[i], true, intersection))
				++ncrossings;
	delete[] checked;

	//cout << "Polygon::edgeCrossings(GreatCircle& gcRef)2 = " << ncrossings << endl << endl;

	return ncrossings;
}

/**
 * Returns a String containing all the points that define the polygon with
 * one lon, lat pair per record. lats and lons are in degrees.
 *
 * @param repeatFirstPoint
 *
 * @param latFirst if true, points are listed as lat, lon. If false, order is
 * lon, lat.
 *
 * @param minLongitude longitudes will be adjusted so that they fall in the range minLongitude
 * to (minLongitude+360).
 *
 * @return String
 */
string GeoTessPolygon::str(const bool& repeatFirstPoint, const bool& latFirst,
		const double& minLongitude)
{
	if (global)
		return "GLOBAL\n";

	ostringstream os;

	os << setiosflags(ios::fixed) << setprecision(6);
	double lat, lon;
	for (int i=0; i<(int)edges.size(); ++i)
	{
		lat = GeoTessUtils::getLatDegrees(edges[i]->getFirst());
		lon = GeoTessUtils::getLonDegrees(edges[i]->getFirst());
		while (lon < minLongitude)
			lon += 360.;
		while (lon >= minLongitude + 360)
			lon -= 360.;

		if (latFirst)
			os << setw(10) << lat << " " << setw(11) << lon << endl;
		else
			os << setw(11) << lon << " " << setw(10) << lat << endl;
	}

	if (repeatFirstPoint)
	{
		lat = GeoTessUtils::getLatDegrees(edges[0]->getFirst());
		lon = GeoTessUtils::getLonDegrees(edges[0]->getFirst());
		while (lon < minLongitude)
			lon += 360.;
		while (lon >= minLongitude + 360)
			lon -= 360.;

		if (latFirst)
			os << setw(10) << lat << " " << setw(11) << lon << endl;
		else
			os << setw(11) << lon << " " << setw(10) << lat << endl;
	}
	return os.str();
}

} // end namespace geotess
