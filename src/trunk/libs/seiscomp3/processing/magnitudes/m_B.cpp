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



#include <seiscomp3/processing/magnitudes/m_B.h>
#include <seiscomp3/seismology/magnitudes.h>
#include <math.h>

namespace Seiscomp {
namespace Processing {


namespace {

std::string ExpectedAmplitudeUnit = "nm/s";

}


IMPLEMENT_SC_CLASS_DERIVED(MagnitudeProcessor_mB, MagnitudeProcessor, "MagnitudeProcessor_mB");
REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_mB, "mB");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_mB::MagnitudeProcessor_mB()
 : MagnitudeProcessor("mB") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

MagnitudeProcessor_mB::MagnitudeProcessor_mB(const std::string& type)
 : MagnitudeProcessor(type) {}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_mB::computeMagnitude(
        double amplitude, const std::string &unit,
        double, double,
        double delta, double depth,
        const DataModel::Origin *, const DataModel::SensorLocation *,
        const DataModel::Amplitude *, double &value) {
	// Clip depth to 0
	if ( depth < 0 ) depth = 0;

	if ( amplitude <= 0 )
		return AmplitudeOutOfRange;

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) )
		return InvalidAmplitudeUnit;

	bool status = Magnitudes::compute_mb(amplitude*1.E-3, 2*M_PI, delta, depth+1, &value);
	value -= 0.14; // HACK until we have an optimal calibration function
	value = correctMagnitude(value);
	return status ? OK : Error;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_mB::estimateMw(
	double magnitude,
	double &Mw_estimate,
	double &Mw_stdError)
{
	const double a=1.30, b=-2.18;
//	if ( magnitude>=6 ) {
		Mw_estimate = a * magnitude + b;
//	}
//	else {
//		// This is to limit the difference between mB and Mw(mB)
//		// FIXME hack to be revised...
//		estimation = a * 6. + b + 0.7*(magnitude-6);
//	}

	Mw_stdError = 0.4;

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
