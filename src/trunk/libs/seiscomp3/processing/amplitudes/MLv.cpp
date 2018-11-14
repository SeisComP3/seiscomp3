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



#define SEISCOMP_COMPONENT AmplitudeMLv

#include <seiscomp3/processing/amplitudes/MLv.h>
#include <seiscomp3/math/mean.h>
#include <seiscomp3/math/filter/seismometers.h>
#include <seiscomp3/math/restitution/fft.h>


using namespace Seiscomp::Math;

namespace Seiscomp {
namespace Processing {


IMPLEMENT_SC_CLASS_DERIVED(AmplitudeProcessor_MLv, AbstractAmplitudeProcessor_ML, "AmplitudeProcessor_MLv");
REGISTER_AMPLITUDEPROCESSOR(AmplitudeProcessor_MLv, "MLv");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_MLv::AmplitudeProcessor_MLv()
: AbstractAmplitudeProcessor_ML("MLv") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_MLv::AmplitudeProcessor_MLv(const Core::Time &trigger)
: AbstractAmplitudeProcessor_ML(trigger, "MLv") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_MLv::computeAmplitude(const DoubleArray &data,
                                              size_t i1, size_t i2,
                                              size_t si1, size_t si2,
                                              double offset,
                                              AmplitudeIndex *dt, AmplitudeValue *amplitude,
                                              double *period, double *snr) {
	bool r = AbstractAmplitudeProcessor_ML::computeAmplitude(data, i1, i2, si1, si2,
	                                                         offset, dt, amplitude,
	                                                         period, snr);
	if ( !r ) return false;

	// Apply empirical correction for measuring ML on the vertical component.
	// Normally ML is measured on both horizontal components and the average
	// is taken.
	amplitude->value *= 2.0;
	return true;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}
