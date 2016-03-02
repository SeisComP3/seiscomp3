/***************************************************************************
 *   Copyright (C) by ETHZ/SED, GNS New Zealand, GeoScience Australia      *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Developed by gempa GmbH                                               *
 ***************************************************************************/

/*
 The code of ARIAS_05_95 was ported from a Mathlab code written
 by Carlo Cauzzi, ETHZ/SED Zurich/Switzerland.
 */


#define SEISCOMP_COMPONENT PGAV

#include "pgav.h"
#include <seiscomp3/logging/log.h>
#include <seiscomp3/math/mean.h>
#include <seiscomp3/math/fft.h>
#include <seiscomp3/math/filter/butterworth.h>
#include <seiscomp3/math/filter/stalta.h>
#include <seiscomp3/math/restitution/fft.h>

#include <fstream>


using namespace std;
using namespace Seiscomp::Math;


namespace Seiscomp {

namespace Processing {


namespace {


//#define SC3_STALTA
double FIXED_PERIODS[] = {
	0,
	-1,
	0.01,
	0.02,
	0.03,
	0.04,
	0.05,
	0.075,
	0.1,
	0.11,
	0.12,
	0.13,
	0.14,
	0.15,
	0.16,
	0.17,
	0.18,
	0.19,
	0.2,
	0.22,
	0.24,
	0.26,
	0.28,
	0.3,
	0.32,
	0.34,
	0.36,
	0.38,
	0.4,
	0.42,
	0.44,
	0.46,
	0.48,
	0.5,
	0.55,
	0.6,
	0.65,
	0.7,
	0.75,
	0.8,
	0.85,
	0.9,
	0.95,
	1,
	1.1,
	1.2,
	1.3,
	1.4,
	1.5,
	1.6,
	1.7,
	1.8,
	1.9,
	2,
	2.2,
	2.4,
	2.6,
	2.8,
	3,
	3.2,
	3.4,
	3.6,
	3.8,
	4,
	4.2,
	4.4,
	4.6,
	4.8,
	5,
	5.5,
	6,
	6.5,
	7,
	7.5,
	8,
	8.5,
	9,
	9.5,
	10
};


template<typename TYPE>
class STALTA : public Filtering::InPlaceFilter<TYPE> {
	public:
		STALTA(double lenSTA=2, double lenLTA=50, double fsamp=1.) {
			_lenSTA = lenSTA;
			_lenLTA = lenLTA;
			_sampleCount = 0;
			_initLength = 0;
			setSamplingFrequency(fsamp);
		}

		// Apply the picker in place to the (previously filtered) data.
		void apply(int ndata, TYPE *data) {
			double inlta = 1./_numLTA, insta = 1./_numSTA;
#ifndef SC3_STALTA
			//bool incsc = false;
#endif

			for (int i=0; i<ndata; ++i, ++data) {
#ifdef SC3_STALTA
				if (_sampleCount < _initLength) {
					// immediately after initialization
					_avg = (_avg*_sampleCount + *data)/(_sampleCount+1);
					_lta = (_sampleCount*_lta+fabs(*data))/(_sampleCount+1);
					_sta = _lta;
					*data = 1.;
					++_sampleCount;
				}
				else {
					// normal behaviour
					_avg = (_avg * (_numLTA-1) + *data) * inlta;

					double q = (_sta - _lta)*inlta;
					_lta += q;
					_sta += (fabs(*data-_avg) - _sta)*insta;
					*data = (TYPE)(_sta/_lta);
				}
#else
	#if 0
				if ( _sampleCount < _numLTA ) {
					_avg = (_avg * _sampleCount + *data)/(_sampleCount+1);
					_lta = (_lta * _sampleCount + fabs(*data-_avg))/(_sampleCount+1);
					incsc = true;
				}
				else {
					_avg = (_avg * (_numLTA-1) + *data)*inlta;
					_lta = (_lta * (_numLTA-1) + fabs(*data-_avg))*inlta;
					incsc = false;
				}

				if ( _sampleCount < _numLTA ) {
					_sta = (_sta * _sampleCount + fabs(*data-_avg))/(_sampleCount+1);
					*data = 1.0;
					++_sampleCount;
				}
				else {
					_sta = (_sta * (_numSTA-1) + fabs(*data-_avg))*insta;
					*data = (TYPE)(_sta/_lta);
					if ( incsc ) ++_sampleCount;
				}
	#else
				if ( _sampleCount < _numLTA ) {
					_avg = (_avg * _sampleCount + *data)/(_sampleCount+1);
					_sta = _lta = (_lta * _sampleCount + fabs(*data-_avg))/(_sampleCount+1);
					*data = 1.0;
					++_sampleCount;
				}
				else {
					_avg = (_avg * (_numLTA-1) + *data)*inlta;
					_sta = (_sta * (_numSTA-1) + fabs(*data-_avg))*insta;
					_lta = (_lta * (_numLTA-1) + fabs(*data-_avg))*inlta;
					*data = (TYPE)(_sta/_lta);
				}
	#endif
#endif
			}
		}

		void reset() {
			_sampleCount = 0;
			_sta =  0.;	// initial STA
			_lta =  0.;	// initial LTA set in apply()
			_avg =  0.;
		}

		void setSamplingFrequency(double fsamp) {
			_fsamp  = fsamp;
			_numSTA = int(_lenSTA*fsamp+0.5);
			_numLTA = int(_lenLTA*fsamp+0.5);
			_initLength = _numLTA/2;
			reset();
		}


		int setParameters(int n, const double *params) {
			if ( n != 2 ) return 2;
			_lenSTA = params[0];
			_lenLTA = params[1];
			return 2;
		}

		Filtering::InPlaceFilter<TYPE>* clone() const {
			return new STALTA<TYPE>(_lenSTA, _lenLTA, _fsamp);
		}


	protected:
		// config
		int _numSTA, _numLTA,
		    _sampleCount, _initLength;
		double _lenSTA, _lenLTA, _fsamp;

		// state
		double _sta, _lta, _avg; // must be double
};


void ButterworthBandpass_Acausal(std::vector<Complex> &spec,
                                 double startFreq, double df,
                                 int order, double loFreq, double hiFreq) {
	double freq = df;
	double exp = order*2.0;
	for ( size_t i = 0; i < spec.size(); ++i ) {
		double v = pow(freq/loFreq,exp);
		spec[i] *= sqrt((v/(1+v))/(1+pow(freq/hiFreq, exp)));
		freq += df;
	}
}

void ButterworthHiPass_Acausal(std::vector<Complex> &spec,
                               double startFreq, double df,
                               int order, double loFreq) {
	double freq = df;
	double exp = order*2.0;
	for ( size_t i = 0; i < spec.size(); ++i ) {
		double v = pow(freq/loFreq,exp);
		spec[i] *= sqrt(v/(1+v));
		freq += df;
	}
}

void ButterworthLoPass_Acausal(std::vector<Complex> &spec,
                               double startFreq, double df,
                               int order, double hiFreq) {
	double freq = df;
	double exp = order*2.0;
	for ( size_t i = 0; i < spec.size(); ++i ) {
		spec[i] *= sqrt(1.0/(1+pow(freq/hiFreq, exp)));
		freq += df;
	}
}

template <typename T>
void costaper(int n, T *inout, int istart, int iend, int estart, int eend) {
	int taperLength = iend - istart;

	for ( int i = 0; i < istart; ++i )
		inout[i] = 0;

	for ( int i = 0; i < taperLength; ++i ) {
		double frac = double(i)/taperLength;
		inout[istart+i] *= 0.5*(1-cos(M_PI*frac));
	}

	taperLength = eend - estart;

	for ( int i = 0; i < taperLength; ++i ) {
		double frac = double(i)/taperLength;
		inout[estart+i] *= 0.5*(1+cos(M_PI*frac));
	}

	for ( int i = eend; i < n; ++i )
		inout[i] = 0;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int PGAV::Config::freqFromString(double &val, const std::string &str) {
	bool isNyquist = false;
	size_t maxPos = str.size();
	if ( str.size() >= 8 && str.compare(str.size()-8,8,"fNyquist") == 0 ) {
		maxPos -= 8;
		isNyquist = true;
	}

	double tmp;
	if ( !Core::fromString(tmp, str.substr(0, maxPos)) ) return 1;

	if ( tmp < 0 ) return 2;
	if ( isNyquist ) tmp *= -1;

	val = tmp;
	return 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PGAV::Config::freqToString(std::string &str, double val) {
	if ( val >= 0 )
		str = Core::toString(val);
	else
		str = Core::toString(-val) + "fNyquist";
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PGAV::PGAV(const Seiscomp::Core::Time& trigger) {
	_trigger = trigger;
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PGAV::~PGAV() {
	setFilter(NULL);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::setEventWindow(double preEventWindow, double totalEventWindow) {
	_config.preEventWindowLength = preEventWindow;
	_config.totalTimeWindowLength = totalEventWindow;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::setSTALTAParameters(double sta, double lta, double ratio, double margin) {
	_config.STAlength = sta;
	_config.LTAlength = lta;
	_config.STALTAratio = ratio;
	_config.STALTAmargin = margin;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::setResponseSpectrumParameters(double damping) {
	_config.dampings.clear();
	_config.dampings.push_back(damping);
	_config.fixedPeriods = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::setResponseSpectrumParameters(const std::vector<double> &dampings) {
	_config.dampings = dampings;
	_config.fixedPeriods = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::setResponseSpectrumParameters(double damping, int nPeriods,
                                         double Tmin, double Tmax,
                                         bool logarithmicSpacing) {
	_config.dampings.clear();
	_config.dampings.push_back(damping);
	_config.naturalPeriods = nPeriods;
	_config.naturalPeriodsLog = logarithmicSpacing;
	_config.Tmin = Tmin;
	_config.Tmax = Tmax;
	_config.fixedPeriods = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::setResponseSpectrumParameters(const std::vector<double> &dampings,
                                         int nPeriods,
                                         double Tmin, double Tmax,
                                         bool logarithmicSpacing) {
	_config.dampings = dampings;
	_config.naturalPeriods = nPeriods;
	_config.naturalPeriodsLog = logarithmicSpacing;
	_config.Tmin = Tmin;
	_config.Tmax = Tmax;
	_config.fixedPeriods = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::setAftershockRemovalEnabled(bool e) {
	_config.aftershockRemoval = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::setPreEventCutOffEnabled(bool e) {
	_config.preEventCutOff = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::setDeconvolutionEnabled(bool e) {
	_config.useDeconvolution = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::setDurationScale(double s) {
	_config.durationScale = s;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::setPostDeconvolutionFilterParams(int o, double loFreq, double hiFreq) {
	_config.PDorder = o;
	_config.loPDFreq = loFreq;
	_config.hiPDFreq = hiFreq;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::setFilterParams(int o, double loFreq, double hiFreq) {
	_config.filterOrder = o;

	if ( _config.filterOrder > 0 ) {
		_config.loFilterFreq = loFreq;
		_config.hiFilterFreq = hiFreq;
	}
	else {
		_config.loFilterFreq = 0;
		_config.hiFilterFreq = 0;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::setSaturationThreshold(double t) {
	_config.saturationThreshold = t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::setPadLength(double len) {
	_config.padLength = len;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::setClipTmaxToLowestFilterFrequency(bool f) {
	_config.clipTmax = f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::setNonCausalFiltering(bool f, double taperLength) {
	_config.noncausal = f;
	_config.taperLength = taperLength;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PGAV::setup(const Settings &settings) {
	string tmp;
	double tmpVal;

	if ( !TimeWindowProcessor::setup(settings) ) return false;

	settings.getValue(_config.preEventWindowLength, "PGAV.preEventWindowLength");
	settings.getValue(_config.totalTimeWindowLength, "PGAV.totalTimeWindowLength");
	settings.getValue(_config.STAlength, "PGAV.STA");
	settings.getValue(_config.LTAlength, "PGAV.LTA");
	settings.getValue(_config.STALTAratio, "PGAV.STALTAratio");
	settings.getValue(_config.STALTAmargin, "PGAV.STALTAmargin");
	settings.getValue(_config.aftershockRemoval, "PGAV.aftershockRemoval");
	settings.getValue(_config.saturationThreshold, "PGAV.saturationThreshold");
	settings.getValue(_config.useDeconvolution, "PGAV.deconvolution");
	settings.getValue(_config.noncausal, "PGAV.noncausal");
	settings.getValue(_config.PDorder, "PGAV.pd.order");

	if ( settings.getValue(tmp, "PGAV.pd.loFreq") ) {
		if ( Config::freqFromString(tmpVal, tmp) ) {
			SEISCOMP_ERROR("%s.%s.PGAV.pd.loFreq: invalid value '%s'",
			               settings.networkCode.c_str(),
			               settings.stationCode.c_str(), tmp.c_str());
			return false;
		}

		_config.loPDFreq = tmpVal;
	}

	if ( settings.getValue(tmp, "PGAV.pd.hiFreq") ) {
		if ( Config::freqFromString(tmpVal, tmp) ) {
			SEISCOMP_ERROR("%s.%s.PGAV.pd.hiFreq: invalid value '%s'",
			               settings.networkCode.c_str(),
			               settings.stationCode.c_str(), tmp.c_str());
			return false;
		}

		_config.hiPDFreq = tmpVal;
	}

	settings.getValue(_config.filterOrder, "PGAV.filter.order");
	if ( settings.getValue(tmp, "PGAV.filter.loFreq") ) {
		if ( Config::freqFromString(tmpVal, tmp) ) {
			SEISCOMP_ERROR("%s.%s.PGAV.filter.loFreq: invalid value '%s'",
			               settings.networkCode.c_str(),
			               settings.stationCode.c_str(), tmp.c_str());
			return false;
		}

		_config.loFilterFreq = tmpVal;
	}

	if ( settings.getValue(tmp, "PGAV.filter.hiFreq") ) {
		if ( Config::freqFromString(tmpVal, tmp) ) {
			SEISCOMP_ERROR("%s.%s.PGAV.filter.hiFreq: invalid value '%s'",
			               settings.networkCode.c_str(),
			               settings.stationCode.c_str(), tmp.c_str());
			return false;
		}

		_config.hiFilterFreq = tmpVal;
	}

	string naturalPeriods;
	if ( settings.getValue(naturalPeriods, "PGAV.naturalPeriods") ) {
		if ( naturalPeriods == "fixed" )
			_config.fixedPeriods = true;
		else {
			_config.fixedPeriods = false;
			if ( !Core::fromString(_config.naturalPeriods, naturalPeriods) ) {
				SEISCOMP_ERROR("%s.%s.PGAV.naturalPeriods: expected either "
				               "'fixed' or an integer value, got '%s'",
				               settings.networkCode.c_str(),
				               settings.stationCode.c_str(),
				               naturalPeriods.c_str());
				return false;
			}
		}
	}

	settings.getValue(_config.naturalPeriodsLog, "PGAV.naturalPeriods.log");
	settings.getValue(_config.Tmin, "PGAV.Tmin");
	settings.getValue(_config.Tmax, "PGAV.Tmax");

	if ( _config.totalTimeWindowLength <= 0 ) {
		SEISCOMP_ERROR("%s.%s.PGAV.totalTimeWindowLength <= 0: ",
		               settings.networkCode.c_str(),
		               settings.stationCode.c_str());
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::finish() {
	if ( status() == WaitingForData || status() == InProgress ) {
		_force = true;
		process(lastRecord(), _data);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::computeTimeWindow() {
	setTimeWindow(
		Core::TimeWindow(
			_trigger-Core::TimeSpan(_config.preEventWindowLength),
			_config.totalTimeWindowLength
		)
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const PGAV::ResponseSpectra &PGAV::responseSpectra() const {
	return _responseSpectra;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define CONTINUE_PROCESSING_WHEN_CHECK_FAILS
void PGAV::process(const Record *record, const DoubleArray &) {
	double offset;

	_duration = Core::None;
	_loFilter = _hiFilter = 0;
	_velocity = true;
	_processed = false;

	// Sampling frequency has not been set yet
	if ( _stream.fsamp == 0.0 )
		return;

	if ( _streamConfig[_usedComponent].gain == 0.0 ) {
		setStatus(MissingGain, 0);
		return;
	}

	SignalUnit gainUnit;
	if ( !gainUnit.fromString(_streamConfig[_usedComponent].gainUnit.c_str()) ) {
		// Invalid unit string
		setStatus(IncompatibleUnit, 2);
		return;
	}

	// Only velocity and acceleration is currently supported
	switch ( gainUnit ) {
		case MeterPerSecond:
			_velocity = true;
			break;
		case MeterPerSecondSquared:
			_velocity = false;
			break;
		default:
			setStatus(IncompatibleUnit, 1);
			return;
	}

#ifdef CONTINUE_PROCESSING_WHEN_CHECK_FAILS
	// Set values to OK. If any successive check fails the status will
	// be set accordingly
	_processed = true;
	setStatus(Finished, 100);
#endif

	// signal and noise window relative to _continuous->startTime()
	double dttrig  = _trigger - dataTimeWindow().startTime();
	double dtn1 = dttrig - _config.preEventWindowLength;
	double dtn2 = dtn1 + _config.totalTimeWindowLength;
	double dt = 1.0 / _stream.fsamp;

	// Trigger index
	int ti = int(dttrig*_stream.fsamp+0.5);
	// Noise start index
	int noise0i = int(dtn1*_stream.fsamp+0.5);
	int noise1i = ti;
	int sig0i = ti;
	int sig1i = int(dtn2*_stream.fsamp+0.5);

	if ( noise0i < 0 ) {
		//SEISCOMP_ERROR("%d samples missing at beginning", -noise0i);
		//setStatus(Error, 1);
		//return;
		noise0i = 0;
	}

	if ( ti < 0 ) ti = 0;

	int n = (int)_data.size();

	if ( n < sig1i ) {
		// time window not complete
		if ( !_force ) {
			setStatus(InProgress, n*100.0/sig1i);
			return;
		}

		sig1i = n;
	}

	SEISCOMP_DEBUG("> processing %s", record->streamID().c_str());

	// Cut data
	if ( noise0i > 0 || sig1i < n ) {
		ti -= noise0i;
		noise1i -= noise0i;
		sig0i -= noise0i;
		sig1i -= noise0i;
		n -= noise0i;

		// Move data time window accordingly
		if ( noise0i > 0 ) {
			_stream.dataTimeWindow.setStartTime(
				_stream.dataTimeWindow.startTime() + Core::TimeSpan(noise0i*dt)
			);

			SEISCOMP_DEBUG(">  cut %.2fs from beginning", noise0i*dt);
		}

		// Move data
		for ( int i = 0; i < sig1i; ++i )
			_data[i] = _data[noise0i+i];

		// Cut end data
		if ( sig1i < n )
			_stream.dataTimeWindow.setEndTime(
				_stream.dataTimeWindow.startTime() + Core::TimeSpan(sig1i*dt)
			);

		_data.resize(sig1i);
		noise0i = 0;
		n = (int)_data.size();
	}

	SEISCOMP_DEBUG(">  trigger = %s", _trigger.iso().c_str());
	SEISCOMP_DEBUG(">  time window = %s ~ %s",
	               _stream.dataTimeWindow.startTime().iso().c_str(),
	               _stream.dataTimeWindow.endTime().iso().c_str());

	// -------------------------------------------------------------------
	// Saturation check
	// -------------------------------------------------------------------
	if ( _config.saturationThreshold >= 0 ) {
		double maxCounts = (_config.saturationThreshold * 0.01) * (1 << 23);
		for ( int i = 0; i < n; ++i ) {
			if ( fabs(_data[i]) > maxCounts ) {
				setStatus(DataClipped, _data[i]);
#ifndef CONTINUE_PROCESSING_WHEN_CHECK_FAILS
				return;
#endif
			}
		}
	}


	// -------------------------------------------------------------------
	// Get maximum raw counts
	// -------------------------------------------------------------------
	int maxCountIndex;
	find_max(sig1i, _data.typedData(), 0, sig1i, &maxCountIndex, &_maximumRawValue);
	SEISCOMP_DEBUG(">  max counts = %.1f", _maximumRawValue);


	// -------------------------------------------------------------------
	// Apply gain
	// -------------------------------------------------------------------
	SEISCOMP_DEBUG(">  gain = %.2f", _streamConfig[_usedComponent].gain);

	for ( int i = 0; i < n; ++i )
		_data[i] /= _streamConfig[_usedComponent].gain;


	// -------------------------------------------------------------------
	// Check STA/LTA ratio
	// -------------------------------------------------------------------
	STALTA<double> stalta(
		std::min(_config.STAlength, _config.preEventWindowLength),
		std::min(_config.LTAlength, dt*(ti-noise0i)), _stream.fsamp
	);

	double maxSTALTA = 0;

	// Compute maximum STA/LTA ratio within [t-5s;t+5s]
	int i0 = noise0i;
	int i1 = sig1i;

	// A positive STA/LTA margin limits the STA/LTA check
	// only around the P onset, a negative margin uses the full
	// data
	if ( _config.STALTAmargin >= 0 ) {
		i0 = (int)(ti - _config.STALTAmargin*_stream.fsamp);
		i1 = (int)(ti + _config.STALTAmargin*_stream.fsamp);
	}

	for ( int i = noise0i; i < sig1i; ++i ) {
		double v = _data[i];
		stalta.apply(1, &v);
		if ( i < i0 || i > i1 ) continue;
		// Only check STA/LTA within the above time window
		if ( v > maxSTALTA ) maxSTALTA = v;
	}

	SEISCOMP_DEBUG(">  STA/LTA(%.1f,%.1f) = %.2f, threshold reached: %s",
	               std::min(_config.STAlength, _config.preEventWindowLength),
	               std::min(_config.LTAlength, dt*(ti-noise0i)),
	               maxSTALTA, maxSTALTA >= _config.STALTAratio?"yes":"no");

	// Return if minimum STA/LTA ratio is not reached
	if ( maxSTALTA < _config.STALTAratio ) {
		setStatus(LowSNR, maxSTALTA);
#ifndef CONTINUE_PROCESSING_WHEN_CHECK_FAILS
		return;
#endif
	}


	// -------------------------------------------------------------------
	// Convert to acceleration units
	// -------------------------------------------------------------------

	// Convert velocity to acceleration if necessary
	if ( gainUnit == MeterPerSecond ) {
		// Differentiate data to acceleration
		double last = _data[0];
		_data[0] = 0;

		for ( int i = 1; i < n; ++i ) {
			double m = (_data[i] - last) * _stream.fsamp;
			last = _data[i];
			_data[i] = m;
		}

		gainUnit = MeterPerSecondSquared;
	}


	// -------------------------------------------------------------------
	// Compute pre event cut-off
	// -------------------------------------------------------------------
	if ( _config.preEventCutOff ) {
		// TODO: Implement
		int i0 = (int)(ti - 15.0*_stream.fsamp);
		int i1 = ti;

		if ( i0 < 0 ) i0 = 0;

		// Initialize the minimum difference to 1.2 with 0.01
		double minDiff = 0.01;
		int minIdx = ti;

		stalta = STALTA<double>(0.1, 2.0, _stream.fsamp);
		for ( int i = i0; i < i1; ++i ) {
			double v = _data[i];
			stalta.apply(1, &v);
			// Compute the minimum distance from 1.2 because 1.2 won't
			// be reached exactly
			v = fabs(v-1.2);
			if ( v < minDiff ) {
				minDiff = v;
				minIdx = i;
			}
		}

		// Adjust the pre event window
		noise0i = std::max(0, (int)(minIdx - 15.5*_stream.fsamp));
		noise1i = std::max(0, (int)(minIdx - 0.5*_stream.fsamp));
		SEISCOMP_DEBUG(">  trigger offset = %.1fs", (minIdx-ti)*dt);
		sig0i = minIdx;
	}


	// -------------------------------------------------------------------
	// Compute offset of pre event time window
	// -------------------------------------------------------------------
	// If the noise window is empty, set the offset to 0
	if ( noise1i == noise0i )
		offset = 0.0;
	else {
		// Compute the offset
		offset = Math::Statistics::mean(
		                     noise1i-noise0i, _data.typedData()+noise0i
		                );

		// Remove the offset
		for ( int i = 0; i < sig1i; ++i )
			_data[i] -= offset;
	}

	SEISCOMP_DEBUG(">  noise offset = %f", offset);


	// -------------------------------------------------------------------
	// Signal duration
	// -------------------------------------------------------------------
	DoubleArray Ia;
	double Ias = M_PI/(2*9.81)*dt;

	Ia.resize(n);

	// Ia = cumsum(_processedData^2)*Ias
	double sum = 0;
	for ( int i = 0; i < n; ++i ) {
		sum += _data[i]*_data[i];
		Ia[i] = sum*Ias;
	}

	double Imax = Ia[Ia.size()-1];
	if ( Imax == 0 ) {
		SEISCOMP_DEBUG(">  asr failed: code 101");
		setStatus(Error, 101.0);
#ifndef CONTINUE_PROCESSING_WHEN_CHECK_FAILS
		return;
#endif
	}

	double iImax = 1.0 / Imax;
	double t05, t95;

	t05 = 0.1;
	t95 = 0.1;

	// Ia = Ia / Imax
	for ( int i = 0; i < n; ++i ) {
		Ia[i] *= iImax;
		if ( (Ia[i] > 0.03) && (Ia[i] < 0.05) )
			t05 = i;

		if ( (Ia[i] > 0.93) && (Ia[i] < 0.95) )
			t95 = i;
	}

	if ( t95 < t05 )
		t95 = n-1;

	// Convert to seconds
	t05 *= dt;
	t95 *= dt;

	// Store signal duration
	_duration = t95 - t05;

	SEISCOMP_DEBUG(">  signal duration = %.2fs, t05 = %.2fs, t95 = %.2fs",
	               *_duration, t05, t95);


	// -------------------------------------------------------------------
	// Aftershock removal
	// -------------------------------------------------------------------
	if ( _config.aftershockRemoval ) {
		double d = std::max(Math::round(*_duration / 3), _config.preEventWindowLength);
		int tcut = n;

		int j = 0;
		int z = 0;

		// Search for aftershock
		for ( int i = 1; i < sig1i-1; ++i ) {
			// Second derivative of relative Arias intensity
			double d2Irel = (Ia[i+1]-2*Ia[i]+Ia[i-1]) * _stream.fsamp*_stream.fsamp;

			// If the curvature is negative, counter j is increased to d
			if ( d2Irel < -7 )
				j = (int)(i+d*_stream.fsamp);

			// Where i equals j (presumably in the plateau of the raltive Arias
			// intensity), we start looking for a positive curvature, i.e. a
			// new eq. signal on the coda
			if ( (int)i == j ) {
				if ( fabs(d2Irel) > 5 ) {
					// The cut time is the time of large positive curvature
					// minus 5 seconds
					int tc = (int)(j-(5.0*_stream.fsamp));
					if ( tc < tcut )
						tcut = tc;
					++z;
				}

				++j;
			}
		}

		// Clip amplitude computation window to tcut
		if ( tcut < sig1i ) {
			sig1i = std::max(0, tcut);
			SEISCOMP_DEBUG(">  aftershock removal trimmed signal to %fs", sig1i*dt);
		}
		else if ( _config.durationScale > 0 ) {
			sig1i = (int)((t05 + _config.durationScale* *_duration) * _stream.fsamp);
			SEISCOMP_DEBUG(">  trimmed signal to %fs", sig1i*dt);
		}
	}
	else if ( _config.durationScale > 0 ) {
		sig1i = (int)((t05 + _config.durationScale* *_duration) * _stream.fsamp);
		SEISCOMP_DEBUG(">  trimmed signal to %fs", sig1i*dt);
	}

	// Clip sig1i to number of samples
	sig1i = std::min(sig1i, n);
	SEISCOMP_DEBUG(">  end of signal = %s (%.2fs after trigger)",
	               (_stream.dataTimeWindow.startTime() + Core::TimeSpan(sig1i*dt)).iso().c_str(),
	               (sig1i-ti)*dt);

	double fNyquist = (_stream.fsamp * 0.5);
	std::vector<Complex> spectrum;
	double df = 0.0;

	if ( _config.noncausal || _config.useDeconvolution ) {
		double Tzpad;

		if ( _config.padLength < 0 ) {
			Tzpad = 1.5*std::max(_config.PDorder, _config.filterOrder);

			if ( _config.loPDFreq > 0 && _config.PDorder > 0 &&
				 _config.loFilterFreq > 0 && _config.filterOrder > 0 )
				Tzpad /= std::min(_config.loPDFreq, _config.loFilterFreq);
			else if ( _config.loPDFreq > 0 && _config.PDorder > 0 )
				Tzpad /= _config.loPDFreq;
			else if ( _config.loFilterFreq > 0 && _config.filterOrder > 0 )
				Tzpad /= _config.loFilterFreq;
			else
				Tzpad = 0.0;

			Tzpad *= 0.5;
		}
		else
			Tzpad = _config.padLength;

		int numZeros = Tzpad * _stream.fsamp;

		if ( numZeros > 0 ) {
			SEISCOMP_DEBUG(">  padding %.1f secs / %d samples of zeros on either side",
			               Tzpad, numZeros);
			DoubleArray zeros(numZeros);
			zeros.fill(0);

			_data.prepend(&zeros);
			_data.append(&zeros);

			_stream.dataTimeWindow.setStartTime(
				_stream.dataTimeWindow.startTime() - Core::TimeSpan(Tzpad)
			);

			_stream.dataTimeWindow.setEndTime(
				_stream.dataTimeWindow.endTime() + Core::TimeSpan(Tzpad)
			);

			// Taper data
			double taperLength = _config.taperLength;

			// If negative, use 20% of pre event window
			if ( taperLength < 0 )
				taperLength = 0.2*_config.preEventWindowLength*0.5;

			if ( taperLength > 0 ) {
				int numTaperSamples = taperLength * _stream.fsamp;
				SEISCOMP_DEBUG(">  taper %.1f secs / %d samples on either side",
				               taperLength, numTaperSamples);

				costaper(_data.size(), _data.typedData(),
				         numZeros, numZeros + numTaperSamples,
				         _data.size() - numZeros - numTaperSamples, _data.size() - numZeros);
			}

			ti += numZeros;
			noise1i += numZeros;
			sig0i += numZeros;
			sig1i += numZeros;
			n += numZeros;
		}

		// Compute frequency spectrum of trace
		Math::fft(spectrum, _data.size(), _data.typedData());
		df = fNyquist / spectrum.size();
	}

	// -------------------------------------------------------------------
	// Deconvolve data
	// -------------------------------------------------------------------
	if ( _config.useDeconvolution ) {
		Sensor *sensor = _streamConfig[_usedComponent].sensor();

		// When using full responses then all information needs to be set up
		// correctly otherwise an error is set
		if ( !sensor ) {
			SEISCOMP_DEBUG(">  no sensor information but deconvolution is enabled");
			setStatus(MissingResponse, 1);
			return;
		}

		if ( !sensor->response() ) {
			SEISCOMP_DEBUG(">  no responses but deconvolution is enabled");
			setStatus(MissingResponse, 2);
			return;
		}

		Math::Restitution::FFT::TransferFunctionPtr tf =
			sensor->response()->getTransferFunction();

		if ( tf == NULL ) {
			SEISCOMP_DEBUG(">  deconvolution failed, no transferfunction");
			setStatus(DeconvolutionFailed, 1);
			return;
		}

		tf->deconvolve(spectrum, df, df);
		SEISCOMP_DEBUG(">  applied deconvolution");

		// -------------------------------------------------------------------
		// Optional post-deconvolution filter
		// -------------------------------------------------------------------
		if ( _config.PDorder > 0 ) {
			double fmin = _config.loPDFreq;
			double fmax = _config.hiPDFreq;

			if ( fmin < 0 ) fmin = fabs(fmin) * fNyquist;
			if ( fmax < 0 ) fmax = fabs(fmax) * fNyquist;

			_loPDFilter = fmin;
			_hiPDFilter = fmax;

			if ( fmin > 0 && fmax > 0 ) {
				ButterworthBandpass_Acausal(spectrum, df, df, _config.PDorder, fmin, fmax);
				SEISCOMP_DEBUG(">  post deconvolution filter with bp%d_%.4f_%.4f", _config.PDorder, fmin, fmax);
			}
			else if ( fmin > 0 ) {
				ButterworthHiPass_Acausal(spectrum, df, df, _config.PDorder, fmin);
				SEISCOMP_DEBUG(">  post deconvolution filter with hp%d_%.4f", _config.PDorder, fmin);
			}
			else if ( fmax > 0 ) {
				ButterworthLoPass_Acausal(spectrum, df, df, _config.PDorder, fmax);
				SEISCOMP_DEBUG(">  post deconvolution filter with lp%d_%.4f", _config.PDorder, fmax);
			}
			else
				SEISCOMP_DEBUG(">  no post deconvolution filter applied: disabled corner freqs (%f,%f)",
				               fmin, fmax);
		}
		else
			SEISCOMP_DEBUG(">  no post deconvolution filter applied: order <= 0 (%d)",
			               _config.PDorder);

		// Convert back to time domain
		if ( !_config.noncausal )
			Math::ifft(_data.size(), _data.typedData(), spectrum);
	}
	else
		SEISCOMP_DEBUG(">  no deconvolution applied (disabled)");

	// -------------------------------------------------------------------
	// Filter
	// -------------------------------------------------------------------
	if ( _config.filterOrder > 0 ) {
		double fmin = _config.loFilterFreq;
		double fmax = _config.hiFilterFreq;

		if ( fmin < 0 ) fmin = fabs(fmin) * fNyquist;
		if ( fmax < 0 ) fmax = fabs(fmax) * fNyquist;

		// Set the used filter frequencies
		_loFilter = fmin;
		_hiFilter = fmax;

		if ( fmin > 0 && fmax > 0 ) {
			if ( _config.noncausal )
				ButterworthBandpass_Acausal(spectrum, df, df, _config.filterOrder, fmin, fmax);
			else {
				Math::Filtering::IIR::ButterworthHighpass<double> hp(_config.filterOrder, fmin );
				hp.setSamplingFrequency(_stream.fsamp);
				hp.apply(_data.size(), _data.typedData());
				Math::Filtering::IIR::ButterworthLowpass<double> lp(_config.filterOrder, fmax);
				lp.setSamplingFrequency(_stream.fsamp);
				lp.apply(_data.size(), _data.typedData());

				/*
				Math::Filtering::IIR::ButterworthBandpass<double> bp(_config.filterOrder, fmin, fmax);
				bp.setSamplingFrequency(_comp.fsamp);
				bp.apply(_data.size(), _data.typedData());
				*/
			}

			SEISCOMP_DEBUG(">  filter: bp%d_%.4f_%.4f", _config.filterOrder, fmin, fmax);
		}
		else if ( fmin > 0 ) {
			if ( _config.noncausal )
				ButterworthHiPass_Acausal(spectrum, df, df, _config.filterOrder, fmin );
			else {
				Math::Filtering::IIR::ButterworthHighpass<double> hp(_config.filterOrder, fmin );
				hp.setSamplingFrequency(_stream.fsamp);
				hp.apply(_data.size(), _data.typedData());
			}

			SEISCOMP_DEBUG(">  filter: hp%d_%.4f", _config.filterOrder, fmin );
		}
		else if ( fmax > 0 ) {
			if ( _config.noncausal )
				ButterworthLoPass_Acausal(spectrum, df, df, _config.filterOrder, fmax);
			else {
				Math::Filtering::IIR::ButterworthLowpass<double> lp(_config.filterOrder, fmax);
				lp.setSamplingFrequency(_stream.fsamp);
				lp.apply(_data.size(), _data.typedData());
			}

			SEISCOMP_DEBUG(">  filter: lp%d_%.4f", _config.filterOrder, fmax);

		}
		else
			SEISCOMP_DEBUG(">  no filter applied: disabled corner freqs (%f,%f)", fmin, fmax);
	}
	else
		SEISCOMP_DEBUG(">  no filter applied: filter order <= 0 (%d)", _config.filterOrder);

	if ( _config.noncausal )
		// Convert back to time domain
		Math::ifft(_data.size(), _data.typedData(), spectrum);

	int pgai, pgvi;

	// -------------------------------------------------------------------
	// Compute PGV/PGA
	// -------------------------------------------------------------------

	SEISCOMP_DEBUG(">  computing maxima in time window %s ~ %s",
	               (_stream.dataTimeWindow.startTime() + Core::TimeSpan(sig0i*dt)).iso().c_str(),
	               (_stream.dataTimeWindow.startTime() + Core::TimeSpan(sig1i*dt)).iso().c_str());

	// Velocity
	if ( gainUnit == MeterPerSecond ) {
		pgvi = find_absmax(sig1i, _data.typedData(), sig0i, sig1i, 0.0);
		_pgv = fabs(_data[pgvi]);

		_pga = -1.0;

		// Differentiate data to acceleration
		for ( int i = 1; i < sig1i; ++i ) {
			double m = (_data[i] - _data[i-1]) * _stream.fsamp;
			double v = fabs(m);
			if ( (i >= sig0i) && (v > _pga) ) {
				_pga = v;
				pgai = i;
			}
		}
	}
	// Acceleration
	else {
		pgai = find_absmax(sig1i, _data.typedData(), sig0i, sig1i, 0.0);
		_pga = fabs(_data[pgai]);

		_pgv = -1.0;

		// Integrate data to velocity using trapezoidal rule
		double sum = 0.0;
		double as = 0.5 * dt;

		//std::ofstream of((record->streamID() + ".dat").c_str());

		for ( int i = 1; i < sig1i; ++i ) {
			double a = (_data[i-1] + _data[i])*as;
			sum += a;
			//of << dt*i << "\t" << _data[i] << "\t" << sum << endl;
			double v = fabs(sum);

			if ( (i >= sig0i) && (v > _pgv) ) {
				pgvi = i;
				_pgv = v;
			}
		}
	}

	SEISCOMP_DEBUG(">  PGA = %e", _pga);
	SEISCOMP_DEBUG(">  PGV = %e", _pgv);


	// -------------------------------------------------------------------
	// Calculate response spectra
	// -------------------------------------------------------------------
	double Tmax = _config.Tmax;

	if ( _config.clipTmax ) {
		double loFreq = std::max(_config.loPDFreq, _config.loFilterFreq);
		// Use an epsilon of 1E-20 to take it as zero
		if ( loFreq > 1E-20 ) {
			double tmpTmax = 1.0 / loFreq;
			if ( tmpTmax < Tmax ) {
				Tmax = tmpTmax;
				SEISCOMP_DEBUG(">  adjust Tmax = %f to stay within filter bands", Tmax);
			}
		}
	}

	vector<double> T;

	if ( !_config.fixedPeriods ) {
		// add basic vibration periods needed for Shakemaps
		T.push_back(0.3);
		T.push_back(1.0);
		T.push_back(3.0);

		if ( _config.naturalPeriods > 1 ) {
			int nT = _config.naturalPeriods-1;

			if ( _config.naturalPeriodsLog ) {
				if ( _config.Tmin != 0.0 && _config.Tmax != 0.0 ) {
					double logTmin = log10(_config.Tmin);
					double logTmax = log10(_config.Tmax);

					double dT = (logTmax-logTmin)/nT;
					for ( int i = 0; i < nT; ++i ) {
						double v = pow(10.0, logTmin+i*dT);
						if ( v <= Tmax ) T.push_back(v);
					}

					if ( _config.Tmax <= Tmax ) T.push_back(_config.Tmax);
				}
				else
					SEISCOMP_DEBUG(">  given natural periods ignored: log(0) is not defined");
			}
			else {
				double dT = (_config.Tmax-_config.Tmin)/nT;
				for ( int i = 0; i < nT; ++i ) {
					double v = _config.Tmin+i*dT;
					if ( v <= Tmax ) T.push_back(v);
				}

				if ( _config.Tmax <= Tmax ) T.push_back(_config.Tmax);
			}
		}
		else
			T.push_back(_config.Tmin);
	}
	else {
		if ( _config.clipTmax ) {
			SEISCOMP_DEBUG(">  using fixed natural periods table cut by Tmax = %f", Tmax);
			int len = sizeof(FIXED_PERIODS)/sizeof(double);
			for ( int i = 0; i < len; ++i ) {
				if ( FIXED_PERIODS[i] <= Tmax )
					T.push_back(FIXED_PERIODS[i]);
			}
		}
		else {
			SEISCOMP_DEBUG(">  using fixed natural periods table");
			T.assign(FIXED_PERIODS, FIXED_PERIODS + sizeof(FIXED_PERIODS)/sizeof(double));
		}
	}

	_responseSpectra.clear();
	for ( size_t di = 0; di < _config.dampings.size(); ++di ) {
		// Convert from percent
		double zeta = _config.dampings[di]*0.01;

		_responseSpectra.push_back(DampingResponseSpectrum(_config.dampings[di], ResponseSpectrum()));
		ResponseSpectrum &spectrum = _responseSpectra.back().second;
		spectrum.resize(T.size());

		for ( size_t i = 0; i < T.size(); ++i ) {
			spectrum[i].period = T[i];

			if ( T[i] == 0 ) {
				spectrum[i].sd = _pga;
				spectrum[i].psa = _pga;
				continue;
			}
			else if ( T[i] == -1 ) {
				spectrum[i].sd = _pgv;
				spectrum[i].psa = _pgv;
				continue;
			}

			double K = (2*M_PI)/T[i];
			double C = 2*zeta*K;
			double beta = 0.25;
			double gamma = 0.5;
			K *= K; // K = K^2

			double B = 1.0/(beta*dt*dt) + (gamma*C)/(beta*dt);
			double A = B + K;
			double E = 1.0/(beta*dt) + (gamma/beta-1)*C;
			double G = 1.0/(2*beta)-1.0;

			double x = 0;
			double xp = 0;
			double xpp = _data[0];
			double maxx = x;

			for ( int j = 1; j < sig1i; ++j ) {
				// f = -f: thats why -_data[j] is used
				double xn = (-_data[j]+B*x+E*xp+G*xpp)/A;
				double xppn = (xn-x-dt*xp-dt*dt*xpp/2+dt*dt*beta*xpp)/(beta*dt*dt);
				double xpn = xp+dt*xpp+dt*gamma*(xppn-xpp);

				x = xn;
				xpp = xppn;
				xp = xpn;

				xn = fabs(x);

				// Save max(fabs(x))
				if ( xn > maxx ) maxx = xn;
			}

			spectrum[i].sd = maxx;
			spectrum[i].psa = maxx*K;
		}
	}

#ifndef CONTINUE_PROCESSING_WHEN_CHECK_FAILS
	_processed = true;
	setStatus(Finished, 100);
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PGAV::init() {
	// No safety margin
	setMargin(Core::TimeSpan(0.0));
	setEventWindow(60, 360);
	setSTALTAParameters(1,60,3,5.0);
	setResponseSpectrumParameters(5, 100, 0, 5, false);
	setAftershockRemovalEnabled(true);
	setPreEventCutOffEnabled(true);
	setDeconvolutionEnabled(false);
	setDurationScale(1.5);
	setPostDeconvolutionFilterParams(0,0,0);
	setFilterParams(0,0,0);
	setNonCausalFiltering(false, -1);
	setPadLength(-1);
	setClipTmaxToLowestFilterFrequency(true);

	_config.saturationThreshold = -1;

	_maximumRawValue = 0;
	_force = false;
	_loFilter = _hiFilter = 0;

	computeTimeWindow();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
