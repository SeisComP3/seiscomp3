/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * GNU Affero General Public License Usage                                 *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 *                                                                         *
 * Other Usage                                                             *
 * Alternatively, this file may be used in accordance with the terms and   *
 * conditions contained in a signed written agreement between you and      *
 * gempa GmbH.                                                             *
 ***************************************************************************/



#include <seiscomp3/processing/magnitudes/Ms20.h>
#include <seiscomp3/seismology/magnitudes.h>


#define DELTA_MIN 2.
#define DELTA_MAX 160.

#define DEPTH_MAX 100


namespace Seiscomp {
namespace Processing {


namespace {

std::string ExpectedAmplitudeUnit = "nm";

}


IMPLEMENT_SC_CLASS_DERIVED(MagnitudeProcessor_ms20, MagnitudeProcessor, "MagnitudeProcessor_ms20");
REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_ms20, "Ms_20");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_ms20::MagnitudeProcessor_ms20()
 : MagnitudeProcessor("Ms_20") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_ms20::computeMagnitude(
	double amplitude, const std::string &unit,
	double period, double snr,
	double delta, double depth,
	const DataModel::Origin *hypocenter,
	const DataModel::SensorLocation *receiver,
	const DataModel::Amplitude *,
	double &value) {
	if ( amplitude <= 0 )
		return AmplitudeOutOfRange;

	// allowed periods are 18 - 22 s acocrding to IASPEI standard (IASPEI recommendations of magnitude working group, 2013)
	if ( period < 18. || period > 22.0 )
		return PeriodOutOfRange;

	if ( delta < DELTA_MIN || delta > DELTA_MAX )
		return DistanceOutOfRange;

	// Clip depth to 0
	if ( depth < 0 ) depth = 0;

	if ( depth > DEPTH_MAX )
		return DepthOutOfRange; // strictly speaking it would be 60 km

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) )
		return InvalidAmplitudeUnit;

	// Use amplitude in nm
	value = correctMagnitude(log10((amplitude)/(period)) + 1.66*log10(delta) + 0.3);

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
