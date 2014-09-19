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



#include <math.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/seismology/magnitudes.h>

/*
static float __qml[] = { 0., 1.4, 1.4, 1.5, 1.6, 1.7, 1.9, 2.1, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.8, 2.8, 2.8, 2.9, 2.9, 3.0, 3.0, 3.0, 3.1, 3.1, 3.2, 3.2, 3.3, 3.3, 3.4, 3.4, 3.5, 3.5, 3.6, 3.65, 3.7, 3.7, 3.8, 3.8, 3.9, 3.9, 4.0, 4.0, 4.1, 4.1, 4.2, 4.2, 4.3, 4.3, 4.3, 4.4, 4.4, 4.5, 4.5, 4.5, 4.6, 4.6, 4.6, 4.6, 4.7, 4.7, 4.7, 4.7, 4.8, 4.8, 4.8, 4.8, 4.8, 4.9, 4.9, 4.9, 4.9, 4.9, 5.2, 5.4, 5.5, 5.7 };

static double lmagn(double amp, double dist, int *err)
{
	//   Computes ML from amplitudes
	//
	//   Distance correction for ML (after C.F.RICHTER (1958): Elementary
	//   Seismology. p 342, up to 600 km epicenter distance; and after G.SCHNEIDER
	//   (1975): Erdbeben. p 338, for 700-1000 km focal distance).
	//
	//   Based on a Fortran routine written by Winfried Hanka

	double del=0;
	int i0 = 0;

	*err = 0;

	if (amp <= 0.) {
		*err = 1;
		return 0.;
	}

	double kdist = dist*111.18;

	if (kdist > 1000) {
		// ML is not defined for dist > 1000 km
		*err = 1;
		return 0.;
	}

	if (kdist > 600) {
		// from 600-1000 km: 100 km steps
		del = 100.;
		i0 = 65;
	}
	else if (kdist > 100) {
		// from 100-600 km: 10 km steps
		del = 10.;
		i0 = 11;
	}

	else {
		// from 0-100 km: 5 km steps
		del = 5.;
		i0 = 1;
	}

	static int qlen = sizeof(__qml)/sizeof(float);

	int idist = int(kdist/del) + i0;
	if (idist>=qlen) {
		*err = -3;
		return 0;
	}
	double tanalpha = (__qml[idist+1] - __qml[idist])/del;
	int ixx = int(kdist/del);
	double xx = kdist - float(ixx)*del;
	double sm10 = xx*tanalpha + __qml[idist];

	return log10(amp) + sm10;
}
*/


namespace Seiscomp {
namespace Magnitudes {

bool
compute_ML(
	double amplitude, // in micrometers
	double delta,     // in degrees
	double depth,     // in kilometers
	double *mag)
{
	//   Computes ML from amplitudes
	//
	//   Distance correction for ML (after C.F.RICHTER (1958): Elementary
	//   Seismology. p 342, up to 600 km epicenter distance; and after G.SCHNEIDER
	//   (1975): Erdbeben. p 338, for 700-1000 km focal distance).
	//
	//   Based on a Fortran routine written by Winfried Hanka


	if (amplitude <= 0.)
		return false;

	double logA0; // Richter (1935): ML = log(A) - log(A0)
	double distkm = Math::Geo::deg2km(delta);

	// Approximation of previous ML computation by three
	// straight-line segments.
	// TODO: Make this configurable

	// distkm -logA0
	//      0    1.3
	//     60    2.8
	//    400    4.5
	//   1000    5.85

        if (distkm <= 60)
	        logA0 = -1.3 - distkm*0.025;
	else if (distkm <= 400)
		logA0 = -2.5 - distkm*0.005;
	else if (distkm <= 1000)
		logA0 = -3.6 - distkm*0.00225;
	else
		return false; // no MLv for dist > 1000 km

	*mag = log10(amplitude) - logA0;
	return true;
}
} // namespace Magnitudes
} // namespace Seiscomp
