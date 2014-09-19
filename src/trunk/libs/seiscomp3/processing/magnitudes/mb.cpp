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



#include <seiscomp3/processing/magnitudes/mb.h>
#include <seiscomp3/seismology/magnitudes.h>


namespace Seiscomp {

namespace Processing {


IMPLEMENT_SC_CLASS_DERIVED(MagnitudeProcessor_mb, MagnitudeProcessor, "MagnitudeProcessor_mb");
REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_mb, "mb");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_mb::MagnitudeProcessor_mb()
 : MagnitudeProcessor("mb") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_mb::computeMagnitude(
	double amplitude, // in micrometers per second
	double period,      // in seconds
	double delta,     // in degrees
	double depth,     // in kilometers
	double &value)
{
	// Clip depth to 0
	if ( depth < 0 ) depth = 0;

	// maximum allowed period is 3 s according to IASPEI standard (pers. comm. Peter Bormann)
	if ( period < 0.4 || period > 3.0 )
		return PeriodOutOfRange;

	// amplitude is nanometers, whereas compute_mb wants micrometers
	bool valid = Magnitudes::compute_mb(amplitude*1.E-3, period, delta, depth+1, &value);
	value = correctMagnitude(value);
	return valid ? OK : Error;
}


}

}
