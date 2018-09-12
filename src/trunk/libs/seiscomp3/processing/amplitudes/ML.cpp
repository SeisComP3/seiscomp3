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



#define SEISCOMP_COMPONENT AmplitudeML

#include <seiscomp3/processing/amplitudes/ML.h>
#include <seiscomp3/math/mean.h>
#include <seiscomp3/math/filter/seismometers.h>
#include <seiscomp3/math/restitution/fft.h>


using namespace Seiscomp::Math;

namespace Seiscomp {

namespace Processing {


IMPLEMENT_SC_ABSTRACT_CLASS_DERIVED(AbstractAmplitudeProcessor_ML, AmplitudeProcessor, "AbstractAmplitudeProcessor_ML");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AbstractAmplitudeProcessor_ML::AbstractAmplitudeProcessor_ML(const std::string& type)
: AmplitudeProcessor(type) {
	setSignalEnd(150.);
	setMinSNR(0);
	setMaxDist(8);
	_computeAbsMax = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AbstractAmplitudeProcessor_ML::AbstractAmplitudeProcessor_ML(const Core::Time& trigger, const std::string &type)
: AmplitudeProcessor(trigger, type) {
	setSignalEnd(150.);
	setMinSNR(0);
	setMaxDist(8);
	computeTimeWindow();
	_computeAbsMax = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AbstractAmplitudeProcessor_ML::initFilter(double fsamp) {
	if ( !_enableResponses ) {
		AmplitudeProcessor::setFilter(
			new Filtering::IIR::WoodAndersonFilter<double>(Velocity, _config.woodAndersonResponse)
		);
	}
	else
		AmplitudeProcessor::setFilter(NULL);

	AmplitudeProcessor::initFilter(fsamp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int AbstractAmplitudeProcessor_ML::capabilities() const {
	return MeasureType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::IDList
AbstractAmplitudeProcessor_ML::capabilityParameters(Capability cap) const {
	if ( cap == MeasureType ) {
		IDList params;
		params.push_back("AbsMax");
		params.push_back("MinMax");
		return params;
	}

	return AmplitudeProcessor::capabilityParameters(cap);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AbstractAmplitudeProcessor_ML::setParameter(Capability cap, const std::string &value) {
	if ( cap == MeasureType ) {
		if ( value == "AbsMax" ) {
			_computeAbsMax = true;
			return true;
		}
		else if ( value == "MinMax" ) {
			_computeAbsMax = false;
			return true;
		}

		return false;
	}

	return AmplitudeProcessor::setParameter(cap, value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AbstractAmplitudeProcessor_ML::setup(const Settings &settings) {
	if ( !AmplitudeProcessor::setup(settings) ) return false;

	settings.getValue(_computeAbsMax, "amplitudes.ML.absMax");
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AbstractAmplitudeProcessor_ML::deconvolveData(Response *resp,
                                                   DoubleArray &data,
                                                   int numberOfIntegrations) {
	if ( numberOfIntegrations < -1 )
		return false;

	Math::Restitution::FFT::TransferFunctionPtr tf =
		resp->getTransferFunction(numberOfIntegrations < 0 ? 0 : numberOfIntegrations);

	if ( tf == NULL )
		return false;

	Math::SeismometerResponse::WoodAnderson paz(numberOfIntegrations < 0 ? Math::Displacement : Math::Velocity,
	                                            _config.woodAndersonResponse);
	Math::Restitution::FFT::PolesAndZeros woodAnderson(paz);
	Math::Restitution::FFT::TransferFunctionPtr cascade =
		*tf / woodAnderson;

	// Remove linear trend
	double m,n;
	Math::Statistics::computeLinearTrend(data.size(), data.typedData(), m, n);
	Math::Statistics::detrend(data.size(), data.typedData(), m, n);

	return Math::Restitution::transformFFT(data.size(), data.typedData(),
	                                       _stream.fsamp, cascade.get(),
	                                       _config.respTaper, _config.respMinFreq,
	                                       _config.respMaxFreq);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AbstractAmplitudeProcessor_ML::computeAmplitude(
		const DoubleArray &data,
		size_t i1, size_t i2,
		size_t si1, size_t si2,
		double offset,
		AmplitudeIndex *dt, AmplitudeValue *amplitude,
		double *period, double *snr) {
	double amax;

	if ( _computeAbsMax ) {
		size_t imax = find_absmax(data.size(), data.typedData(), si1, si2, offset);
		amax = fabs(data[imax] - offset);
		dt->index = imax;
	}
	else {
		int lmin, lmax;
		find_minmax(lmin, lmax, data.size(), data.typedData(), si1, si2, offset);
		amax = (data[lmax] - data[lmin]) * 0.5;
		dt->index = (lmin+lmax)*0.5;
		dt->begin = lmin - dt->index;
		dt->end = lmax - dt->index;
	}

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

	// - convert to millimeter
	amplitude->value *= 1E03;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double AbstractAmplitudeProcessor_ML::timeWindowLength(double distance_deg) const {
	// Minimal S/SW group velocity.
	//
	// This is very approximate and may need refinement. Usually the Lg
	// group velocity is around 3.2-3.6 km/s. By setting v_min to 3 km/s,
	// we are probably on the safe side. We add 30 s to count for rupture
	// duration, which may, however, not be sufficient.
	double v_min = 3;

	double distance_km = distance_deg*111.2;
	double windowLength = distance_km/v_min + 30;
	return windowLength < _config.signalEnd ? windowLength :_config.signalEnd;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}
