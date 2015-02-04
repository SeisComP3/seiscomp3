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



#define SEISCOMP_COMPONENT AmplitudeProcessor

#include <seiscomp3/processing/amplitudeprocessor.h>
#include <seiscomp3/math/mean.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/interfacefactory.ipp>

#include <fstream>
#include <limits>


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::Processing::AmplitudeProcessor, SC_SYSTEM_CLIENT_API);

namespace Seiscomp {

namespace Processing {

IMPLEMENT_SC_ABSTRACT_CLASS_DERIVED(AmplitudeProcessor, TimeWindowProcessor, "AmplitudeProcessor");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::AmplitudeProcessor() {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::AmplitudeProcessor(const std::string& type)
 : _type(type) {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::AmplitudeProcessor(const Core::Time& trigger)
 : _trigger(trigger) {

	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::AmplitudeProcessor(const Core::Time& trigger, const std::string& type)
 : _trigger(trigger), _type(type) {

	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::init() {
	_enableUpdates = false;
	_enableResponses = false;
	_responseApplied = false;

	_config.saturationThreshold = -1;

	_config.noiseBegin = -35;
	_config.noiseEnd = -5;
	_config.signalBegin = -5;
	_config.signalEnd = 30;
	_config.snrMin = 3;

	_config.minimumDistance = 0;
	_config.maximumDistance = 180;
	_config.minimumDepth = -1E6;
	_config.maximumDepth = 1E6;

	_config.respTaper = 60.0;
	_config.respMinFreq = 0.00833333; // 120 secs
	_config.respMaxFreq = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::~AmplitudeProcessor() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::setUpdateEnabled(bool e) {
	_enableUpdates = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor::isUpdateEnabled() const {
	return _enableUpdates;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::setReferencingPickID(const std::string& pickID) {
	_pickID = pickID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& AmplitudeProcessor::referencingPickID() const {
	return _pickID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OPT(double) AmplitudeProcessor::noiseOffset() const {
	return _noiseOffset;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OPT(double) AmplitudeProcessor::noiseAmplitude() const {
	return _noiseAmplitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& AmplitudeProcessor::type() const {
	return _type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& AmplitudeProcessor::unit() const {
	return _unit;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::computeTimeWindow() {
	if ( !(bool)_trigger )
		setTimeWindow(Core::TimeWindow());

	Core::Time startTime = _trigger + Core::TimeSpan(_config.noiseBegin);
	Core::Time   endTime = _trigger + Core::TimeSpan(_config.signalEnd);

	setTimeWindow(Core::TimeWindow(startTime, endTime));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int AmplitudeProcessor::capabilities() const {
	return NoCapability;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor::supports(Capability c) const {
	return (capabilities() & c) > 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::IDList
AmplitudeProcessor::capabilityParameters(Capability cap) const {
	return IDList();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor::setParameter(Capability cap, const std::string &value) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::setUnit(const std::string &unit) {
	_unit = unit;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::reprocess(OPT(double) searchBegin,
                                   OPT(double) searchEnd) {
	if ( _stream.lastRecord ) {
		_searchBegin = searchBegin;
		_searchEnd = searchEnd;

		// Force recomputation of noise amplitude and noise offset
		_noiseAmplitude = Core::None;
		process(_stream.lastRecord.get());

		// Reset search window again
		_searchBegin = _searchEnd = Core::None;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::reset() {
	TimeWindowProcessor::reset();
	_noiseAmplitude = Core::None;
	_noiseOffset = Core::None;
	_responseApplied = false;
	_trigger = Core::Time();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::process(const Record *record, const DoubleArray &) {
	process(record);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::process(const Record *record) {
	// Sampling frequency has not been set yet
	if ( _stream.fsamp == 0.0 )
		return;

	int n = (int)_data.size();

	// signal and noise window relative to _continuous->startTime()
	double dt0  = _trigger - dataTimeWindow().startTime();
	double dt1  = dataTimeWindow().endTime() - dataTimeWindow().startTime();
	double dtn1 = dt0 + _config.noiseBegin;
	double dtn2 = dt0 + _config.noiseEnd;
	double dts1 = dt0 + _config.signalBegin;
	double dts2 = dt0 + _config.signalEnd;

	// Noise indicies
	int ni1 = int(dtn1*_stream.fsamp+0.5);
	int ni2 = int(dtn2*_stream.fsamp+0.5);

	if ( ni1 < 0 || ni2 < 0 ) {
		SEISCOMP_DEBUG("Noise data not available -> abort");
		setStatus(Error, 1);
		return;
	}

	if ( n < ni2 ) {
		// the noise window is not complete
		return;
	}


	// **** compute signal amplitude ********************************

	// these are the offsets of the beginning and end
	// of the signal window relative to the start of
	// the continuous record in samples
	int i1 = int(dts1*_stream.fsamp+0.5);
	int i2 = int(dts2*_stream.fsamp+0.5);

	//int progress = int(100.*(n-i1)/(i2-i1));
	int progress = int(100.*(dt1-dts1)/(dts2-dts1));
	if ( progress > 100 ) progress = 100;
	setStatus(InProgress, progress);

	if ( i1 < 0 ) i1 = 0;
	if ( i2 > n ) i2 = n;

	bool unlockCalculation = ((_enableUpdates && !_enableResponses) && progress > 0) || progress >= 100;

	if ( unlockCalculation ) {
		if ( _streamConfig[_usedComponent].gain == 0.0 ) {
			setStatus(MissingGain, 0);
			return;
		}

		// **** prepare the data to compute the noise
		prepareData(_data);
		if ( isFinished() )
			return;

		// **** compute noise amplitude *********************************
		// if the noise hasn't been measured yet...
		if ( !_noiseAmplitude ) {
			// compute pre-arrival data offset and noise amplitude

			double off = 0., amp = 0.;

			if ( !computeNoise(_data, ni1, ni2, &off, &amp) ) {
				SEISCOMP_DEBUG("Noise computation failed -> abort");
				setStatus(Error, 2);
				return;
			}

			_noiseOffset = off;
			_noiseAmplitude = amp;
		}

		AmplitudeIndex index;
		Result res;
		res.component = _usedComponent;
		res.record = record;
		res.period = -1;
		res.snr = -1;

		res.amplitude.value = -1;
		res.amplitude.lowerUncertainty = Core::None;
		res.amplitude.upperUncertainty = Core::None;

		index.index = -1;
		index.begin = 0;
		index.end = 0;

		double dtsw1, dtsw2;

		if ( _searchBegin ) {
			dtsw1 = dt0 + *_searchBegin;
			if ( dtsw1 < dts1 ) dtsw1 = dts1;
			if ( dtsw1 > dts2 ) dtsw1 = dts2;
		}
		else
			dtsw1 = dts1;

		if ( _searchEnd ) {
			dtsw2 = dt0 + *_searchEnd;
			if ( dtsw2 < dts1 ) dtsw2 = dts1;
			if ( dtsw2 > dts2 ) dtsw2 = dts2;
		}
		else
			dtsw2 = dts2;

		int si1 = int(dtsw1*_stream.fsamp+0.5);
		int si2 = int(dtsw2*_stream.fsamp+0.5);

		si1 = std::max(si1, i1);
		si2 = std::min(si2, i2);

		if ( !computeAmplitude(_data, i1, i2, si1, si2, *_noiseOffset,
		                       &index, &res.amplitude, &res.period, &res.snr) ) {
			if ( progress >= 100 ) {
				if ( status() == LowSNR )
					SEISCOMP_DEBUG("Amplitude %s computation for stream %s failed because of low SNR (%.2f < %.2f)",
					              _type.c_str(), record->streamID().c_str(), res.snr, _config.snrMin);
				else {
					SEISCOMP_DEBUG("Amplitude %s computation for stream %s failed -> abort",
					              _type.c_str(), record->streamID().c_str());
					setStatus(Error, 3);
				}

				_lastAmplitude = Core::None;
			}

			return;
		}

		if ( _lastAmplitude ) {
			if ( res.amplitude.value <= *_lastAmplitude ) {
				if ( progress >= 100 ) {
					setStatus(Finished, 100.);
					_lastAmplitude = Core::None;
				}

				return;
			}
		}

		_lastAmplitude = res.amplitude.value;

		double dt = index.index / _stream.fsamp;
		res.period /= _stream.fsamp;

		if ( index.begin > index.end ) std::swap(index.begin, index.end);

		// Update status information
		res.time.reference = dataTimeWindow().startTime() + Core::TimeSpan(dt);
		//res.time.begin = index.begin / _stream.fsamp;
		//res.time.end = index.end / _stream.fsamp;
		res.time.begin = (si1 - index.index) / _stream.fsamp;
		res.time.end = (si2 - index.index) / _stream.fsamp;

		if ( progress >= 100 ) {
			setStatus(Finished, 100.);
			_lastAmplitude = Core::None;
		}

		emitAmplitude(res);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor::handleGap(Filter *filter, const Core::TimeSpan& span,
                                   double lastSample, double nextSample,
                                   size_t missingSamples) {
	if ( _stream.dataTimeWindow.endTime()+span < timeWindow().startTime() ) {
		// Save trigger, because reset will unset it
		Core::Time t = _trigger;
		reset();
		_trigger = t;
		return true;
	}

	//TODO: Handle gaps
	setStatus(QCError, 1);
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::prepareData(DoubleArray &data) {
	Sensor *sensor = _streamConfig[_usedComponent].sensor();

	// When using full responses then all information needs to be set up
	// correctly otherwise an error is set
	if ( _enableResponses ) {
		if ( !sensor ) {
			setStatus(MissingResponse, 1);
			return;
		}

		if ( !sensor->response() ) {
			setStatus(MissingResponse, 2);
			return;
		}

		// If the unit cannot be converted into the internal
		// enum (what basically means "unknown") then the deconvolution
		// cannot be correctly. We do not want to assume a unit here
		// to prevent computation errors in case of bad configuration.
		SignalUnit unit;
		if ( !unit.fromString(sensor->unit().c_str()) ) {
			// Invalid unit string
			setStatus(IncompatibleUnit, 2);
			return;
		}

		int intSteps = 0;
		switch ( unit ) {
			case MeterPerSecond:
				break;
			case MeterPerSecondSquared:
				intSteps = 1;
				break;
			default:
				setStatus(IncompatibleUnit, 1);
				return;
		}

		if ( _responseApplied ) return;

		_responseApplied = true;

		if ( !deconvolveData(sensor->response(), _data, intSteps) ) {
			setStatus(DeconvolutionFailed, 0);
			return;
		}
	}
	else {
		// If the sensor is known then check the unit and skip
		// non velocity streams. Otherwise simply use the data
		// to be compatible to the old version. This will be
		// changed in the future and checked more strictly.
		if ( sensor ) {
			SignalUnit unit;
			if ( !unit.fromString(sensor->unit().c_str()) ) {
				// Invalid unit string
				setStatus(IncompatibleUnit, 4);
				return;
			}

			if ( unit != MeterPerSecond ) {
				setStatus(IncompatibleUnit, 3);
				return;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor::deconvolveData(Response *resp, DoubleArray &data,
                                        int numberOfIntegrations) {
	// Remove linear trend
	double m,n;
	Math::Statistics::computeLinearTrend(data.size(), data.typedData(), m, n);
	Math::Statistics::detrend(data.size(), data.typedData(), m, n);

	return resp->deconvolveFFT(data, _stream.fsamp, _config.respTaper,
	                           _config.respMinFreq, _config.respMaxFreq,
	                           numberOfIntegrations);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor::computeNoise(const DoubleArray &data, int i1, int i2, double *offset, double *amplitude) {
	// compute offset and rms within the time window
	if(i1<0) i1=0;
	if(i2<0) return false;
	if(i2>(int)data.size()) i2=(int)data.size();

	// If noise window is zero return an amplitude and offset of zero as well.
	if ( i2-i1 == 0 ) {
		*amplitude = 0;
		*offset = 0;
		return true;
	}

	DoubleArrayPtr d = static_cast<DoubleArray*>(data.slice(i1, i2));

	double ofs, amp;

	// compute pre-arrival offset
	ofs = d->median();
	// compute rms after removing offset
	amp = 2 * d->rms(ofs);

	if ( offset ) *offset = ofs;
	if ( amplitude ) *amplitude = amp;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor::setup(const Settings &settings) {
	try {
		if ( settings.getBool("amplitudes." + _type + ".enable") == false )
			return false;
	}
	catch ( ... ) {
		// In case the amplitude specific enable flag is not set,
		// check the global flag
		try {
			if ( settings.getBool("amplitudes.enable") == false )
				return false;
		}
		catch ( ... ) {}
	}

	if ( !settings.getValue(_enableResponses, "amplitudes." + _type + ".enableResponses") )
		settings.getValue(_enableResponses, "amplitudes.enableResponses");

	settings.getValue(_config.saturationThreshold, "amplitudes.saturationThreshold");
	settings.getValue(_config.snrMin, "amplitudes." + _type + ".minSNR");
	settings.getValue(_config.noiseBegin, "amplitudes." + _type + ".noiseBegin");
	settings.getValue(_config.noiseEnd, "amplitudes." + _type + ".noiseEnd");
	settings.getValue(_config.signalBegin, "amplitudes." + _type + ".signalBegin");
	settings.getValue(_config.signalEnd, "amplitudes." + _type + ".signalEnd");
	settings.getValue(_config.minimumDistance, "amplitudes." + _type + ".minDist");
	settings.getValue(_config.maximumDistance, "amplitudes." + _type + ".maxDist");
	settings.getValue(_config.minimumDepth, "amplitudes." + _type + ".minDepth");
	settings.getValue(_config.maximumDepth, "amplitudes." + _type + ".maxDepth");
	settings.getValue(_config.respTaper, "amplitudes." + _type + ".resp.taper");
	settings.getValue(_config.respMinFreq, "amplitudes." + _type + ".resp.minFreq");
	settings.getValue(_config.respMaxFreq, "amplitudes." + _type + ".resp.maxFreq");

	return TimeWindowProcessor::setup(settings);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::setTrigger(const Core::Time& trigger) throw(Core::ValueException) {
	if ( _trigger )
		throw Core::ValueException("The trigger has been set already");

	_trigger = trigger;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::Time AmplitudeProcessor::trigger() const {
	return _trigger;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::setPublishFunction(const PublishFunc& func) {
	_func = func;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::emitAmplitude(const Result &res) {
	if ( isEnabled() && _func )
		_func(this, res);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double AmplitudeProcessor::timeWindowLength(double distance) const {
	return _config.signalEnd;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::fill(size_t n, double *samples) {
	if ( _config.saturationThreshold > 0 ) {
		for ( size_t i = 0; i < n; ++i ) {
			if ( fabs(samples[i]) > _config.saturationThreshold ) {
				SEISCOMP_WARNING("%s: data clipped: %f > %f",
				                 _stream.lastRecord->streamID().c_str(),
				                 fabs(samples[i]), _config.saturationThreshold);
				setStatus(DataClipped, samples[i]);
				break;
			}
		}
	}

	TimeWindowProcessor::fill(n, samples);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::setHint(ProcessingHint hint, double value) {
	WaveformProcessor::setHint(hint, value);

	switch ( hint ) {
		case Distance:
			if ( (value < _config.minimumDistance) || (value > _config.maximumDistance) )
				setStatus(DistanceOutOfRange, value);
			else {
				double length = timeWindowLength(value);
				if ( length != _config.signalEnd ) {
					_config.signalEnd = length;
					computeTimeWindow();
					// When we are already finished, make sure the current amplitude
					// will be send immediately
					if ( _trigger + Core::TimeSpan(_config.signalEnd) <= dataTimeWindow().endTime() && _stream.lastRecord )
						process(_stream.lastRecord.get());
				}
			}
			break;

		case Depth:
			if ( (value < _config.minimumDepth) || (value > _config.maximumDepth) )
				setStatus(DepthOutOfRange, value);
			// To be defined
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const AmplitudeProcessor *
AmplitudeProcessor::componentProcessor(Component comp) const {
	if ( comp < VerticalComponent || comp > SecondHorizontalComponent )
		return NULL;

	if ( comp != (Component)_usedComponent ) return NULL;

	return this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DoubleArray *AmplitudeProcessor::processedData(Component comp) const {
	if ( comp < VerticalComponent || comp > SecondHorizontalComponent )
		return NULL;

	if ( comp != (Component)_usedComponent ) return NULL;

	return &continuousData();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::writeData() const {
	if ( !_stream.lastRecord ) return;

	const DoubleArray *data = processedData((Component)_usedComponent);
	if ( data == NULL ) return;

 	std::ofstream of((_stream.lastRecord->streamID() + "-" + type() + ".data").c_str());

	of << "#sampleRate: " << _stream.lastRecord->samplingFrequency() << std::endl;

	for ( int i = 0; i < data->size(); ++i )
		of << i << "\t" << (*data)[i] << std::endl;
	of.close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}
