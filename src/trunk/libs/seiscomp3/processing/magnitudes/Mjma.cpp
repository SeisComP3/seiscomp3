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



#include <seiscomp3/processing/magnitudes/Mjma.h>
#include <seiscomp3/seismology/magnitudes.h>
#include <seiscomp3/math/geo.h>


#define DELTA_MIN 0.3
#define DELTA_MAX 20.

#define DEPTH_MAX 80


namespace Seiscomp {

namespace Processing {


REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_Mjma, "Mjma");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_Mjma::MagnitudeProcessor_Mjma()
 : MagnitudeProcessor("Mjma") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_Mjma::computeMagnitude(
	double amplitude, const std::string &unit,
	double, double, double delta, double depth,
	const DataModel::Origin *, const DataModel::SensorLocation *,
	const DataModel::Amplitude *,
	double &value) {
	if ( delta < DELTA_MIN || delta > DELTA_MAX )
		return DistanceOutOfRange;

	if ( amplitude <= 0 )
		return AmplitudeOutOfRange;

	// Clip depth to 0
	if ( depth < 0 ) depth = 0;

	if ( depth > DEPTH_MAX )
		return DepthOutOfRange;

	double a1 = 1.73, a2 = 0., a3 = -0.83;
	double r = Math::Geo::deg2km(delta);

	// Convert amplitude unit from millimeters to micrometers
	value = correctMagnitude(log10(amplitude) + a1*log10(r) + a2*r + a3 + 0.44);

	return OK;
}


}

}
