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



#define SEISCOMP_COMPONENT AmplitudeMjma

#include <seiscomp3/processing/amplitudes/Mjma.h>
#include <seiscomp3/math/mean.h>
#include <seiscomp3/math/filter/seismometers.h>
#include <seiscomp3/math/restitution/fft.h>


using namespace Seiscomp::Math;

namespace Seiscomp {

namespace Processing {


REGISTER_AMPLITUDEPROCESSOR(AmplitudeProcessor_Mjma, "Mjma");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_Mjma::AmplitudeProcessor_Mjma()
	: AmplitudeProcessor("Mjma") {
	setSignalEnd(150.);
	setMinSNR(0);
	setMaxDist(20);
	setMaxDepth(80);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_Mjma::AmplitudeProcessor_Mjma(const Core::Time& trigger)
	: AmplitudeProcessor(trigger, "Mjma") {
	setSignalEnd(150.);
	setMinSNR(0);
	setMaxDist(20);
	setMaxDepth(20);
	computeTimeWindow();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_Mjma::initFilter(double fsamp) {
	if ( !_enableResponses ) {
		AmplitudeProcessor::setFilter(
			new Filtering::IIR::Seismometer5secFilter<double>(Velocity)
		);
	}
	else
		AmplitudeProcessor::setFilter(NULL);

	AmplitudeProcessor::initFilter(fsamp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_Mjma::deconvolveData(Response *resp,
                                             DoubleArray &data,
                                             int numberOfIntegrations) {
	Math::Restitution::FFT::TransferFunctionPtr tf =
		resp->getTransferFunction(numberOfIntegrations);

	if ( tf == NULL ) {
		setStatus(DeconvolutionFailed, 0);
		return false;
	}

	
	Math::SeismometerResponse::Seismometer5sec paz(Math::Velocity);
	Math::Restitution::FFT::PolesAndZeros seis5sec(paz);

	Math::Restitution::FFT::TransferFunctionPtr cascade =
		*tf / seis5sec;

	// Remove linear trend
	double m,n;
	Math::Statistics::computeLinearTrend(data.size(), data.typedData(), m, n);
	Math::Statistics::detrend(data.size(), data.typedData(), m, n);

	return Math::Restitution::transformFFT(data.size(), data.typedData(),
	                                         _stream.fsamp, cascade.get(),
	                                         _config.respTaper, _config.respMinFreq, _config.respMaxFreq);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_Mjma::computeAmplitude(
		const DoubleArray &data,
		size_t i1, size_t i2,
		size_t si1, size_t si2,
		double offset,AmplitudeIndex *dt,
		AmplitudeValue *amplitude,
		double *period, double *snr)
{
	double amax;

	size_t imax = find_absmax(data.size(), data.typedData(), si1, si2, offset);
	amax = fabs(data[imax] - offset);
	dt->index = imax;

	if ( *_noiseAmplitude == 0. )
		*snr = 1000000.0;
	else
		*snr = amax / *_noiseAmplitude;

	if ( *snr < _config.snrMin ) {
		setStatus(LowSNR, *snr);
		return false;
	}

	*period = -1;

	amplitude->value = amax;

	if ( _streamConfig[_usedComponent].gain != 0.0 )
		amplitude->value /= _streamConfig[_usedComponent].gain;
	else {
		setStatus(MissingGain, 0.0);
		return false;
	}

	// - convert to micrometer
	amplitude->value *= 1E06;

	// - estimate peak-to-peak from absmax amplitude
	amplitude->value *= 2.0;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double AmplitudeProcessor_Mjma::timeWindowLength(double distance_deg) const {
	// Minimal S/SW group velocity.
	//
	// This is very approximate and may need refinement. Usually the Lg
	// group velocity is around 3.2-3.6 km/s. By setting v_min to 3 km/s,
	// we are probably on the safe side. We add 30 s to coount for rupture
	// duration, which may, however, nit be sufficient.
	double v_min = 3;

	double distance_km = distance_deg*111.2;
	double windowLength = distance_km/v_min + 30;
	return windowLength;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}
