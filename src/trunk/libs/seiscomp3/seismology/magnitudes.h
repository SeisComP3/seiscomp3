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



#ifndef __SEISCOMP_SEISMOLOGY_MAGNITUDES_H__
#define __SEISCOMP_SEISMOLOGY_MAGNITUDES_H__

#include <seiscomp3/core.h>

namespace Seiscomp {
namespace Magnitudes {

/**
 * Compute Mwp after Tsuboi(1999) with linear correction from GFZ
 * @param  amplitude  maximum reading of absolute integrated displacement in m*s
 * @param  delta      epicentral distance in degrees
 * @param  mag        return value
 * @param  offset     taken from fit against Harvard Mw (Mwp=Mw*slope+offset)
 * @param  slope      taken from fit against Harvard Mw 
 * @param  alpha      P-wave velocity along ray path in m/s
 * @param  rho        mass density along ray path in kg/m^3
 * @param  fp         radiation pattern influence factor
 * @return false      Mwp undefined for given epicentral distance
 * @return true       Mwp calculated successfully
 */
SC_SYSTEM_CORE_API
bool compute_Mwp(
	double amplitude,
	double delta,
	double &mag,     // resulting magnitude
	double offset = 0, double slope = 1,
//	double offset=2.25,
//	double slope=1./1.6,
	double alpha=7900.,
	double rho=3400.,
	double fp=1.);



/**
 * Compute Mjma after Katsumata (1988) PRELIMINARY!!! only valid for depth < 60 km !!!
 * @param  amplitude  maximum reading of absolute integrated displacement in m*s
 * @param  delta      epicentral distance in degrees
 * @return false      Mjma undefined for given epicentral distance
 * @return true       Mwp calculated successfully
 */
SC_SYSTEM_CORE_API
bool compute_Mjma(
	double amplitude,
	double delta,     // in degrees
	double depth,     // in kilometers
	double& Mjma);

/**
 * Mjma variant for BMG/Indonesia
 */
SC_SYSTEM_CORE_API
bool compute_Mbmg(
	double amplitude,
	double delta,	  // in degrees
	double depth,	  // in kilometers
	double& Mbmg);


SC_SYSTEM_CORE_API
bool
compute_mb(
	double amplitude, // in micrometers
	double period,    // in seconds
	double delta,     // in degrees
	double depth,     // in kilometers
	double *mag);     // resulting magnitude


SC_SYSTEM_CORE_API
bool
compute_mb_fromVelocity(
	double amplitude, // in micrometers/second
	double delta,     // in degrees
	double depth,     // in kilometers
	double *mag);     // resulting magnitude


SC_SYSTEM_CORE_API
bool
compute_ML(
	double amplitude, // in micrometers
	double delta,     // in degrees
	double depth,     // in kilometers
	double *mag);     // resulting magnitude

}
}

#endif
