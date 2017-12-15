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



#define SEISCOMP_COMPONENT AmplitudemB

#include <seiscomp3/processing/amplitudes/m_B.h>
#include <limits>


namespace Seiscomp {

namespace Processing {

// 60 s should be OK for rupture durations up to ~100 s.
// This default value MUST NOT be increased because if the amplitudes are
// computed in the stream picker, which doesn't know of origins, it also
// doesn't know the S-P time. For distances of 5 degrees the S wave would
// leak into the time window thus contaminating the measurement.
#define M_CAPITAL_B_DEFAULT_WINDOW_LENGTH 60

IMPLEMENT_SC_CLASS_DERIVED(AmplitudeProcessor_mB, AmplitudeProcessor, "AmplitudeProcessor_mB");
REGISTER_AMPLITUDEPROCESSOR(AmplitudeProcessor_mB, "mB");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_mB::AmplitudeProcessor_mB()
: AmplitudeProcessor("mB") {
	setSignalEnd(M_CAPITAL_B_DEFAULT_WINDOW_LENGTH);
	setMinDist(5);
	setMaxDist(105);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_mB::AmplitudeProcessor_mB(const Core::Time& trigger)
: AmplitudeProcessor(trigger, "mB") {
	setSignalEnd(M_CAPITAL_B_DEFAULT_WINDOW_LENGTH);
	setMinDist(5);
	setMaxDist(105);
	computeTimeWindow();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double AmplitudeProcessor_mB::timeWindowLength(double distance) const {
	// make sure the measurement is not contaminated by S energy
	double tdist = 11.5*distance; // distance-dependent time difference between P and S
	return tdist < _config.signalEnd ? tdist :_config.signalEnd;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_mB::computeAmplitude(const DoubleArray &data,
                                             size_t i1, size_t i2,
                                             size_t si1, size_t si2,
                                             double offset,AmplitudeIndex *dt,
                                             AmplitudeValue *amplitude,
                                             double *period, double *snr) {
	/*
	* Low-level signal amplitude computation. This is magnitude specific.
	*
	* Input:
	*      f           double array of length n
	*      i1,i2       indices defining the measurement window,
	*                  0 <= i1 < i2 <= n
	*      offset      this is subtracted from the samples in f before
	*                  computation
	*
	* Output:
	*      dt          Point at which the measurement was mad/completed. May
	*                  be the peak time or end of integration.
	*      amplitude   amplitude. This may be a peak amplitude as well as a
	*                  sum or integral.
	*      period      dominant period of the signal. Optional. If the period
	*                  is not computed, set it to -1.
	*/
	int    imax = find_absmax(data.size(), data.typedData(), si1, si2, offset);
	double amax = fabs(data[imax] - offset);
	double pmax = -1;

	// Bei Mwp bestimmt man amax zur Berechnung des SNR. Die eigentliche
	// Amplitude ist aber was anderes! Daher ist SNR-Bestimmung auch
	// magnitudenspezifisch!

	if ( *_noiseAmplitude == 0. )
		*snr = 1000000.0;
	else
		*snr = amax / *_noiseAmplitude;

	// SNR check
	if (*snr < _config.snrMin) {
		setStatus(LowSNR, *snr);
		return false;
	}

	dt->index = imax;
	// Amplitudes are send in nanometers
	*period = pmax;

	amplitude->value = amax;

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
