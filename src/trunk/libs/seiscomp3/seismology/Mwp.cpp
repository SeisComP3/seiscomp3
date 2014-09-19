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
#include <seiscomp3/seismology/magnitudes.h>

#define DELTA_MIN 3.
#define DELTA_MAX 105.

// degrees to radians factor
#define D2R 0.01745329251994329509
#define EARTH_RADIUS 6370998.685023

namespace Seiscomp {
namespace Magnitudes {

bool compute_Mwp(double amplitude, double delta, double &Mwp, double offset,double slope,double alpha,double rho,double fp)
{
	if(delta<DELTA_MIN||delta>DELTA_MAX) return false;

	double r=(D2R*delta)*EARTH_RADIUS; // convert delta to meters
	double momfac=4*M_PI*rho*pow(alpha,3.)*r/fp;
	double M0,Mw;

	M0=amplitude*momfac;
	Mw=(log10(M0)-9.1)/1.5;
	Mwp=(Mw-offset)/slope;

	return true;
}

}
}

