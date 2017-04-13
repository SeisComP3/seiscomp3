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


#include <seiscomp3/math/geo.h>
#include <seiscomp3/processing/magnitudes/MLv.h>


namespace Seiscomp {
namespace Processing {


IMPLEMENT_SC_CLASS_DERIVED(MagnitudeProcessor_MLv, MagnitudeProcessor, "MagnitudeProcessor_MLv");
REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_MLv, "MLv");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_MLv::MagnitudeProcessor_MLv()
 : MagnitudeProcessor("MLv") {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor_MLv::setup(const Settings &settings) {
	MagnitudeProcessor::setup(settings);
	std::string logA0;

	try {
		logA0 = settings.getString("MLv.logA0");
	}
	catch ( ... ) {
		// This is the default
		logA0 = "0 -1.3;60 -2.8;400 -4.5;1000 -5.85";
	}

	logA0_dist.clear(); logA0_val.clear();
	std::istringstream iss(logA0);
	std::string item;
	while ( getline(iss, item,';') ) {
		std::istringstream iss_item(item);
		double dist, val;
		iss_item >> dist >> val;
		logA0_dist.push_back(dist);
		logA0_val.push_back(val);
	}

	try {
		maxDistanceKm = settings.getDouble("MLv.maxDistanceKm");
	}
	catch ( ... ) {
		maxDistanceKm = -1; // distance according to the logA0 range
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MagnitudeProcessor_MLv::logA0(double dist_km) const throw(Core::ValueException) {
	for ( size_t i=1; i<logA0_dist.size(); ++i ) {
		if ( logA0_dist[i-1] <= dist_km && dist_km <= logA0_dist[i] ) {
			double q = (dist_km-logA0_dist[i-1])/(logA0_dist[i]-logA0_dist[i-1]);
			return q*(logA0_val[i]-logA0_val[i-1])+logA0_val[i-1];
		}
	}

	throw Core::ValueException("distance out of range");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_MLv::computeMagnitude(
	double amplitude, // in millimeters per second
	double,           // period is unused
	double delta,     // in degrees
	double depth,     // in kilometers
	double &value)
{
	if ( amplitude <= 0 )
		return AmplitudeOutOfRange;

	// Clip depth to 0
	if ( depth < 0 ) depth = 0;

	double distanceKm = Math::Geo::deg2km(delta);
	if ( maxDistanceKm > 0 && distanceKm > maxDistanceKm )
		return DistanceOutOfRange;

	try {
		value = log10(amplitude) - logA0(distanceKm);
	}
	catch ( Core::ValueException & ) {
		return DistanceOutOfRange;
	}

	value = correctMagnitude(value);

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
