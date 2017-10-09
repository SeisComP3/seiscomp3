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



#define SEISCOMP_COMPONENT AmplitudeMwp

#include <seiscomp3/processing/amplitudes/Mwp.h>
#include <seiscomp3/math/filter/iirintegrate.h>
#include<seiscomp3/math/filter/butterworth.h>

#include <seiscomp3/math/geo.h>

#include <limits>

extern "C" {

#include "mwp_utils.h"

}

using namespace Seiscomp::Math::Filtering::IIR;

namespace Seiscomp {

namespace Processing {

namespace {


/*
double calcSlope(DoubleArray& ar, double dt) {
	double varx = 0.;
	double covxy = 0.;
	double tq = 0.;
	double yq = 0.;
	int n = ar.size();

	for ( int i = 0; i < ar.size(); ++i ) {
		tq += i*dt;
		yq += ar[i];
	}

	tq /= n;
	yq /= n;

	for ( int i = 0; i < ar.size() ; ++i ) {
		varx += pow((i*dt-tq),2);
		covxy+=(ar[i]-yq)*(i*dt-tq);
	}

	varx /= n;
	covxy = covxy/n;
	double slope = covxy/varx;

	return slope;
}
*/

}


IMPLEMENT_SC_ABSTRACT_CLASS_DERIVED(AmplitudeProcessor_Mwp, AmplitudeProcessor, "AmplitudeProcessor_Mwp");
REGISTER_AMPLITUDEPROCESSOR(AmplitudeProcessor_Mwp, "Mwp");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_Mwp::AmplitudeProcessor_Mwp()
	: AmplitudeProcessor("Mwp") {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_Mwp::AmplitudeProcessor_Mwp(const Core::Time& trigger)
	: AmplitudeProcessor(trigger, "Mwp") {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_Mwp::init() {
//	setSignalEnd(120.);
	setSignalEnd(95.); // be on the safe side... otherwise we might measure the S-wave at regional distances!!!
	setNoiseStart(-240.);
	setMinDist(5);
	setMaxDist(105);
	setMinSNR(3);
	computeTimeWindow();

	_epizentralDistance = -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



/*
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_Mwp::initFilter(double fsamp) {
	AmplitudeProcessor::setFilter(
		new Math::Filtering::IIR::ButterworthHighpass<double>(3,.01, fsamp)
	);

	AmplitudeProcessor::initFilter(fsamp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
*/

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double AmplitudeProcessor_Mwp::timeWindowLength(double distance) const {
	// make sure the measurement is not contaminated by S energy
	double tdist = 11.5*distance; // distance-dependent time difference between P and S
	return tdist < _config.signalEnd ? tdist :_config.signalEnd;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_Mwp::computeAmplitude(const DoubleArray &data,
                                              size_t i1, size_t i2,
                                              size_t si1, size_t si2, double offset,
                                              AmplitudeIndex *dt,
                                              AmplitudeValue *amplitude,
                                              double *period, double *snr) {
	size_t imax = find_absmax(data.size(), (const double*)data.data(), si1, si2, offset);
	double amax = fabs(data[imax] - offset);

	if ( *_noiseAmplitude == 0. )
		*snr = 1000000.0;
	else
		*snr = amax / *_noiseAmplitude;

/*
	if ( *snr < _config.snrMin ) {
		setStatus(LowSNR, *snr);
		_processedData = continuousData();
		return false;
	}
*/
	int onset = i1, n=i2; // XXX

	_processedData.resize(n);

	for ( int i = 0; i < n; ++i )
		_processedData[i] = (data[i] - offset) / _streamConfig[_usedComponent].gain;

	// Apply mild highpass to take care of long-period noise.
	// This is required unless the stations are exceptionally good.
	ButterworthHighpass<double> *hp = new ButterworthHighpass<double>(2,.008, _stream.fsamp);

	Mwp_demean(n, _processedData.typedData(), onset);
	Mwp_taper (n, _processedData.typedData(), onset);
	hp->apply(n, _processedData.typedData());
	Mwp_double_integration(n, _processedData.typedData(), onset, _stream.fsamp);
	// apply high pass a second time
	hp->reset();
	hp->apply(n, _processedData.typedData());
	delete hp;

	// Amplitude in nanometers
	amplitude->value = 1.E9*Mwp_amplitude(si2, _processedData.typedData(), si1, &onset);

	dt->index = onset; // FIXME
	*period = 0.0;


	// Now check the SNR of the doubly integrated trace.
	// Perhaps we can skip the initial SNR test completely!
	*snr = Mwp_SNR(n, _processedData.typedData(), i1);
	if ( *snr < _config.snrMin ) {
		setStatus(LowSNR, *snr);
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_Mwp::setHint(ProcessingHint hint, double value) {
	if ( hint == Distance )
		_epizentralDistance = value;

	AmplitudeProcessor::setHint(hint, value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DoubleArray *AmplitudeProcessor_Mwp::processedData(Component comp) const {
	if ( comp != (Component)_usedComponent ) return NULL;
	return &_processedData;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}
