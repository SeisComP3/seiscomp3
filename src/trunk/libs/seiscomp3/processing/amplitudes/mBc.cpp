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



#define SEISCOMP_COMPONENT AmplitudemBc

#include <seiscomp3/processing/amplitudes/mBc.h>
#include <limits>
#include "mBc_measure.h"


namespace Seiscomp {

namespace Processing {

// #define M_CAPITAL_B_DEFAULT_WINDOW_LENGTH 60 // Same as for mB since it is only a default

IMPLEMENT_SC_CLASS_DERIVED(AmplitudeProcessor_mBc, AmplitudeProcessor_mB, "AmplitudeProcessor_mBc");
REGISTER_AMPLITUDEPROCESSOR(AmplitudeProcessor_mBc, "mBc");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_mBc::AmplitudeProcessor_mBc() {
	_type = "mBc";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_mBc::AmplitudeProcessor_mBc(const Core::Time& trigger)
: AmplitudeProcessor_mB(trigger) {
	_type = "mBc";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_mBc::computeAmplitude(const DoubleArray &data,
                                              size_t i1, size_t i2,
                                              size_t si1, size_t si2,
                                              double offset, AmplitudeIndex *dt,
                                              AmplitudeValue *amplitude,
                                              double *period, double *snr) {
	// see also amplitudeprocessor_m_B.cpp
	int n = si2-si1;
	const double *v = data.typedData() + si1;
	Measurement_mBc measurement(n);
	measurement.set_offset(offset);
	measurement.feed(n, v);

	double pmax = -1;

	// Bei Mwp bestimmt man amax zur Berechnung des SNR. Die eigentliche
	// Amplitude ist aber was anderes! Daher ist SNR-Bestimmung auch
	// magnitudenspezifisch!

	if ( *_noiseAmplitude == 0. )
		*snr = 1000000.0;
	else    // measurement.vmax is the same as used in mB
		*snr = measurement.vmax / *_noiseAmplitude;

	// SNR check
	if (*snr < _config.snrMin) {
		setStatus(LowSNR, *snr);
		return false;
	}

	dt->index = si1 + measurement.icum;
	*period = pmax;
	amplitude->value = measurement.vcum;
	if ( _streamConfig[_usedComponent].gain != 0.0 )
		amplitude->value /= _streamConfig[_usedComponent].gain;
	else {
		setStatus(MissingGain, 0.0);
		return false;
	}

	// Convert m/s to nm/s
	amplitude->value *= 1.E09;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}

}
