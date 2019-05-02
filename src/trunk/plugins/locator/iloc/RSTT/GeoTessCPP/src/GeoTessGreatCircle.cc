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

#include "GeoTessGreatCircle.h"
#include "GeoTessModel.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

/**
 * Destructor
 */
GeoTessGreatCircle::~GeoTessGreatCircle()
{
	clear();
}

GeoTessGreatCircle::GeoTessGreatCircle(const double* first, const double* last,  const bool& shortestPath)
: distance(-1.), firstPoint(NULL), lastPoint(NULL), deleteFirst(true), deleteLast(true), trnsfrm(NULL)
{
	firstPoint = new double[3];
	firstPoint[0] = first[0];
	firstPoint[1] = first[1];
	firstPoint[2] = first[2];

	lastPoint = new double[3];
	lastPoint[0] = last[0];
	lastPoint[1] = last[1];
	lastPoint[2] = last[2];

	initialize(NULL, shortestPath);
}

GeoTessGreatCircle::GeoTessGreatCircle(const double* first, const double* middle, const double* last,  const bool& shortestPath)
: distance(-1.), firstPoint(NULL), lastPoint(NULL), deleteFirst(true), deleteLast(true), trnsfrm(NULL)
{
	firstPoint = new double[3];
	firstPoint[0] = first[0];
	firstPoint[1] = first[1];
	firstPoint[2] = first[2];

	lastPoint = new double[3];
	lastPoint[0] = last[0];
	lastPoint[1] = last[1];
	lastPoint[2] = last[2];

	initialize(middle, shortestPath);
}

GeoTessGreatCircle::GeoTessGreatCircle(const double* first, const double& dist, const double& direction)
: distance(-1.), firstPoint(NULL), lastPoint(NULL), deleteFirst(true), deleteLast(true), trnsfrm(NULL)
{
	firstPoint = new double[3];
	firstPoint[0] = first[0];
	firstPoint[1] = first[1];
	firstPoint[2] = first[2];

	// first find a point that is 90 degrees away from firstPoint, in specified direction.
	if (!GeoTessUtils::moveDistAz(firstPoint, PI/2, direction, moveDirection))
	{
		ostringstream os;
		os << endl << "ERROR in GreatCircle::GreatCircle" << endl
				<< "firstPoint of GreatCircle is one of the poles" << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 11001);

	}

	GeoTessUtils::crossNormal(firstPoint, moveDirection, normal);

	lastPoint = new double[3];
	GeoTessUtils::move(firstPoint, moveDirection, dist, lastPoint);

}

GeoTessGreatCircle::GeoTessGreatCircle(GeoTessGreatCircle& other)
{
	firstPoint = new double[3];
	firstPoint[0] = other.firstPoint[0];
	firstPoint[1] = other.firstPoint[1];
	firstPoint[2] = other.firstPoint[2];

	lastPoint = new double[3];
	lastPoint[0] = other.lastPoint[0];
	lastPoint[1] = other.lastPoint[1];
	lastPoint[2] = other.lastPoint[2];

	normal[0] = other.normal[0];
	normal[1] = other.normal[1];
	normal[2] = other.normal[2];

	moveDirection[0] = other.moveDirection[0];
	moveDirection[1] = other.moveDirection[1];
	moveDirection[2] = other.moveDirection[2];

	distance = other.distance;

	if (other.trnsfrm)
	{
		trnsfrm = CPPUtils::new2DArray<double>(3, 3);

		trnsfrm[0][0] = other.trnsfrm[0][0];
		trnsfrm[0][1] = other.trnsfrm[0][1];
		trnsfrm[0][2] = other.trnsfrm[0][2];
		trnsfrm[1][0] = other.trnsfrm[1][0];
		trnsfrm[1][1] = other.trnsfrm[1][1];
		trnsfrm[1][2] = other.trnsfrm[1][2];
		trnsfrm[2][0] = other.trnsfrm[2][0];
		trnsfrm[2][1] = other.trnsfrm[2][1];
		trnsfrm[2][2] = other.trnsfrm[2][2];
	}
	else
		trnsfrm = NULL;
}

void GeoTessGreatCircle::operator = (GeoTessGreatCircle& other)
{
	firstPoint[0] = other.firstPoint[0];
	firstPoint[1] = other.firstPoint[1];
	firstPoint[2] = other.firstPoint[2];

	lastPoint[0] = other.lastPoint[0];
	lastPoint[1] = other.lastPoint[1];
	lastPoint[2] = other.lastPoint[2];

	normal[0] = other.normal[0];
	normal[1] = other.normal[1];
	normal[2] = other.normal[2];

	moveDirection[0] = other.moveDirection[0];
	moveDirection[1] = other.moveDirection[1];
	moveDirection[2] = other.moveDirection[2];

	distance = other.distance;

	if (other.trnsfrm)
	{
		if (trnsfrm == NULL)
			trnsfrm = CPPUtils::new2DArray<double>(3, 3);

		trnsfrm[0][0] = other.trnsfrm[0][0];
		trnsfrm[0][1] = other.trnsfrm[0][1];
		trnsfrm[0][2] = other.trnsfrm[0][2];
		trnsfrm[1][0] = other.trnsfrm[1][0];
		trnsfrm[1][1] = other.trnsfrm[1][1];
		trnsfrm[1][2] = other.trnsfrm[1][2];
		trnsfrm[2][0] = other.trnsfrm[2][0];
		trnsfrm[2][1] = other.trnsfrm[2][1];
		trnsfrm[2][2] = other.trnsfrm[2][2];
	}
	else
		trnsfrm = NULL;
}

void GeoTessGreatCircle::clear()
{
	if (deleteFirst && firstPoint != NULL) delete[] firstPoint;

	if (deleteLast && lastPoint != NULL) delete[] lastPoint;

	if (trnsfrm != NULL) CPPUtils::delete2DArray<double>(trnsfrm);

	firstPoint = lastPoint = NULL;
	trnsfrm = NULL;

	distance = -1;
}

void GeoTessGreatCircle::set(double* first, double* middle, double* last, const bool &shortestPath, const bool& deleteWhenDone)
{
	clear();

	deleteFirst = deleteLast = deleteWhenDone;

	firstPoint = first;
	lastPoint = last;

	initialize(middle, shortestPath);

	if (deleteWhenDone && middle != NULL) { delete[] middle; middle = NULL; }

}

void GeoTessGreatCircle::set(double* first, const double& dist, const double& direction,
		const bool& deleteWhenDone)
{
	clear();

	firstPoint = first;
	lastPoint = new double[3];

	deleteFirst = deleteWhenDone;
	deleteLast = true;

	// first find a point that is 90 degrees away from firstPoint, in specified direction.
	if (!GeoTessUtils::moveDistAz(firstPoint, PI/2, direction, moveDirection))
	{
		ostringstream os;
		os << endl << "ERROR in GreatCircle::GreatCircle" << endl
				<< "firstPoint of GreatCircle is one of the poles" << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 11002);

	}

	GeoTessUtils::crossNormal(firstPoint, moveDirection, normal);

	GeoTessUtils::move(firstPoint, moveDirection, dist, lastPoint);

}

void GeoTessGreatCircle::initialize(const double* middle, const bool &shortestPath)
{
	// find the unit vector normal to the plane of the great circle
	// firstPoint cross lastPoint. If firstPoint on left and lastPoint on
	// right, normal points away the observer.

	if (GeoTessUtils::crossNormal(firstPoint, lastPoint, normal) == 0.)
	{
		// calculation of normal failed.
		// Distance must be either 0 or PI so dot will be +/- 1.
		distance = GeoTessUtils::dot(firstPoint, lastPoint) > 0 ? 0. : PI;

		if (middle == NULL || GeoTessUtils::crossNormal(firstPoint, middle, normal) == 0.)
		{
			// calculation of normal failed again
			double point[3] = {0., 0., 1.};
			if (GeoTessUtils::crossNormal(firstPoint, point, normal) == 0.)
			{
				// calculation of normal failed again
				point[0] = 0.;
				point[1] = 1.;
				point[2] = 0.;
				if (GeoTessUtils::crossNormal(firstPoint, point, normal) == 0.)
				{
					// calculation of normal failed again
					point[0] = 1.;
					point[1] = 0.;
					point[2] = 0.;
					if (GeoTessUtils::crossNormal(firstPoint, point, normal) == 0.)
					{
						// calculation of normal failed again
						ostringstream os;
						os << endl << "ERROR in GreatCircle::initialize" << endl;
						os << "Unable to determine normal to great circle path." << endl;
						if (firstPoint[0]*firstPoint[0] + firstPoint[1]*firstPoint[1] +
								firstPoint[2]*firstPoint[2] < 1e-6)
							os << "firstPoint is not a unit vector (length==0)!" << endl;

						throw GeoTessException(os, __FILE__, __LINE__, 11003);

					}
				}
			}
		}
	}

	if (!shortestPath)
	{
		if (distance >= 0.) distance = 2.*PI;
		normal[0] = -normal[0];
		normal[1] = -normal[1];
		normal[2] = -normal[2];
	}

	if (GeoTessUtils::crossNormal(normal, firstPoint, moveDirection) < 0.999999)
	{
		ostringstream os;
		os << endl << "ERROR in GreatCircle::initialize" << endl
				<< "firstPoint and normal are not orthogonal" << endl
				<< "firstPoint = " << GeoTessUtils::getLatLonString(firstPoint) << endl
				<< "lastPoint = " << GeoTessUtils::getLatLonString(lastPoint) << endl
				<< "normal = " << GeoTessUtils::getLatLonString(normal) << endl
				<< "normal X firstPoint length = "
				<< GeoTessUtils::crossNormal(normal, firstPoint, moveDirection)
		<< endl;
		throw GeoTessException(os, __FILE__, __LINE__, 11004);

	}

}

/**
 * Retrieve a reference to the transform matrix owned by this GreatCircle.
 * Transform is a 3 x 3 matrix such that when a vector is multiplied by
 * transform, the vector will be projected onto the plane of this
 * GreatCircle. The z direction will point out of the plane of the great
 * circle in the direction of the observer (lastPoint cross firstPoint;
 * parallel to normal). The y direction will correspond to the mean of
 * firstPoint and lastPoint. The x direction will correspond to y cross z,
 * forming a right handed coordinate system.
 *
 * <p>Caller should not delete this array
 *
 * @return double**
 */
double** GeoTessGreatCircle::getTransform()
{
	if (trnsfrm == NULL)
	{
		/**
		 * Transform is a 3 x 3 matrix such that when a vector is multiplied
		 * by transform, the vector will be projected onto the plane of this
		 * GreatCircle. The z direction will point out of the plane of the
		 * great circle in the direction of the observer (lastPoint cross
		 * firstPoint; parallel to normal). The y direction will correspond
		 * to the mean of firstPoint and lastPoint. The x direction will
		 * correspond to y cross z, forming a right handed coordinate
		 * system.
		 */
		trnsfrm = CPPUtils::new2DArray<double>(3, 3);

		GeoTessUtils::rotate(firstPoint, normal, -getDistance()/2, trnsfrm[1]);

		trnsfrm[2][0] = -normal[0];
		trnsfrm[2][1] = -normal[1];
		trnsfrm[2][2] = -normal[2];

		GeoTessUtils::crossNormal(trnsfrm[1], trnsfrm[2], trnsfrm[0]);
	}
	return trnsfrm;
}

string GeoTessGreatCircle::toString()
{
	ostringstream os;
	os << "firstPoint = " << GeoTessUtils::getLatLonString(firstPoint) << endl;
	os << "lastPoint  = " << GeoTessUtils::getLatLonString(lastPoint) << endl;
	os << "normal     = " << GeoTessUtils::getLatLonString(normal) << endl;
	os << "distance   = " << CPPUtils::toDegrees(getDistance()) << endl;
	os << "azimuth    = " << GeoTessUtils::azimuthDegrees(firstPoint, lastPoint, -999.) << endl;
	os << "backaz     = " << GeoTessUtils::azimuthDegrees(lastPoint, firstPoint, -999.) << endl;
	return os.str();
}

} // end namespace geotess
