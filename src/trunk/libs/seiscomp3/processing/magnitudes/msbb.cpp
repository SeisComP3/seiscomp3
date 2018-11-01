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



#include <seiscomp3/processing/magnitudes/msbb.h>
#include <seiscomp3/seismology/magnitudes.h>


#define DELTA_MIN 2.
#define DELTA_MAX 160.

#define DEPTH_MAX 100


namespace Seiscomp {
namespace Processing {


namespace {

std::string ExpectedAmplitudeUnit = "m";

}


IMPLEMENT_SC_CLASS_DERIVED(MagnitudeProcessor_msbb, MagnitudeProcessor, "MagnitudeProcessor_msbb");
REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_msbb, "Ms(BB)");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_msbb::MagnitudeProcessor_msbb()
 : MagnitudeProcessor("Ms(BB)") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_msbb::computeMagnitude(
	double amplitude, const std::string &unit,
	double period, double snr,
	double delta, double depth,
	const DataModel::Origin *hypocenter,
	const DataModel::SensorLocation *receiver,
	const DataModel::Amplitude *,
	double &value) {
	if ( amplitude <= 0 )
		return AmplitudeOutOfRange;

	if ( delta < DELTA_MIN || delta > DELTA_MAX )
		return DistanceOutOfRange;

	// Clip depth to 0
	if ( depth < 0 ) depth = 0;

	if ( depth > DEPTH_MAX )
		return DepthOutOfRange; // strictly speaking it would be 60 km

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) )
		return InvalidAmplitudeUnit;

	// Convert amplitude unit from meters to micrometers
	value = correctMagnitude(log10((amplitude*1E06)/(2*M_PI)) + 1.66*log10(delta) + 3.3);

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
