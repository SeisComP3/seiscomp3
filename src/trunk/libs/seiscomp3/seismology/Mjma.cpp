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

// --------------------- ATTENTION -------------------
// Things to consider when calculating the appropriate amplitude in scampmjma:
// - time window length: maximum displacement amplitude of either body or surface waves
// - frequency content: use only periods of 5 s or less (highpass)
// - use both NS/EW components (now it's only N)
// - integrate once for BH streams
// ------------------- END ATTENTION -----------------


#define DELTA_MIN 0.
#define DELTA_MAX 20.

// degrees to radians factor
#define D2R 0.01745329251994329509
#define EARTH_RADIUS 6370998.685023

namespace Seiscomp {
namespace Magnitudes {

//
// ### REDUNDANCY: this code is also situated in applications/scamp/mjma.cpp !!!
//
// /GITEWS_WP1000/SHARE/LITERATURE/Katsumata_1996_Mjma_BSSA.pdf :
// Mjma is determined by Tsuboi's formula (1954) as
//         Mjma = log10 A + 1.73 log10 Delta - 0.83
// where Delta is the epicentral distance (km) and A is the maxi-
// mum amplitude (um). A is given by A = SQRT(A_NS + A_EW), in
// which A_NS and A_EW are half the maximum peak-to-peak
// amplitudes of the horizontal components.
//
// here amp is in m and Delta in m
bool compute_Mjma(double amplitude,double delta, double depth, double& Mjma)
{
	if(delta<DELTA_MIN||delta>DELTA_MAX) return false;

	// a formula for depth > 60 exists (Katsumata, 1964) but is yet to be implemented
	if(depth > 80) return false; // strictly speaking it would be 60 km

	double A=amplitude*1e6; // m -> um
	double r=(D2R*delta)*EARTH_RADIUS/1e3; // ° -> m

	double a1 = 1.73, a2 = 0., a3 = -0.83;

// 	SEISCOMP_DEBUG("amp2Mjma(): amp= %e delta= %f t1= %f t2= %f t3= %f",amp,Delta,log10(amp),a1*log10(Delta),a2*Delta);
	Mjma = log10(A) + a1*log10(r) + a2*r + a3 + 0.44;

	return true;
}


bool compute_Mbmg(double amplitude,double delta, double depth, double& Mbmg)
{
	// TODO: calibrate
	return compute_Mjma(amplitude, delta, depth, Mbmg);
}

}
}
