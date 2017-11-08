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



#include <seiscomp3/processing/magnitudes/Mwp.h>
#include <seiscomp3/seismology/magnitudes.h>
#include <math.h>

namespace Seiscomp {

namespace Processing {


IMPLEMENT_SC_CLASS_DERIVED(MagnitudeProcessor_Mwp, MagnitudeProcessor, "MagnitudeProcessor_Mwp");
REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_Mwp, "Mwp");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_Mwp::MagnitudeProcessor_Mwp()
 : MagnitudeProcessor("Mwp") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_Mwp::computeMagnitude(
	double amplitude, // in micrometers per second // XXX ???
	double,           // period is unused
	double delta,     // in degrees
	double depth,     // in kilometers
	const DataModel::Origin *hypocenter,
	const DataModel::SensorLocation *receiver,
	double &value)
{
	if ( amplitude <= 0 )
		return AmplitudeOutOfRange;

	bool status = Magnitudes::compute_Mwp(amplitude*1.E-9, delta, value);
	value = correctMagnitude(value);
	return status ? OK : Error;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_Mwp::estimateMw(
	double magnitude,
	double &estimation,
	double &stdError)
{
	const double a=1.31, b=-1.91; // Whitmore et al. (2002)
	estimation = a * magnitude + b;

	stdError = 0.4; // Fixme

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}
