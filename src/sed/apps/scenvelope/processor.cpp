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


#include "processor.h"
#include "util.h"

#define SEISCOMP_COMPONENT Envelope
#include <seiscomp3/logging/log.h>
#include <seiscomp3/math/mean.h>
#include <seiscomp3/math/filter/butterworth.h>



namespace Seiscomp {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AverageBuffer::AverageBuffer(int size) {
	_empty = true;
	_bins.resize(size, 0);
	_sum = 0;
	_front = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AverageBuffer::push(double val) {
	// First push initializes all bins with val
	if ( _empty ) {
		fill(val);
		_empty = false;
		return;
	}

	// Remove overwritten value from sum
	_sum -= _bins[_front];

	// Store new value
	_bins[_front] = val;

	// Add new value to sum
	_sum += val;

	// Increate pointer and wrap it if necessary
	++_front;
	if ( _front >= _bins.size() ) _front = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AverageBuffer::fill(double val) {
	std::fill(_bins.begin(), _bins.end(), val);
	_sum = val * _bins.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double AverageBuffer::sum() const {
	return _sum;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double AverageBuffer::average() const {
	return _sum / (double)_bins.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Processor::Config::Config() {
	saturationThreshold = 80;
	interval = 1000;
	useVSFilterImplementation = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Processor::setPublishFunction(const PublishFunc& func) {
	_func = func;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Processor::useVSFilterImplementation(bool f) {
	_config.useVSFilterImplementation = f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Processor::reset() {
	// Reset waveform processor
	Processing::WaveformProcessor::reset();

	// Clear/reset all static filters and buffers
	_baselineCorrection0.clear();
	_baselineCorrection1.clear();
	_baselineCorrection2.clear();
	_toVelocity.reset();
	_toAcceleration.reset();
	_toDisplacement.reset();

	// Reset the filter by cloning them. reset() is not part of the filter
	// interface yet
	if ( _filter0 ) _filter0 = _filter0->clone();
	if ( _filter1 ) _filter1 = _filter1->clone();

	// Reset start and end time
	_currentStartTime = _currentEndTime = Core::Time();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Processor::setupTimeWindow(const Core::Time &ref) {
	_currentStartTime = ref;
	if ( _currentStartTime.microseconds() > 0 )
		_currentStartTime.setUSecs(0);

	_currentEndTime = _currentStartTime + Core::TimeSpan(1,0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Processor::init(const Record *rec) {
	_samplePool.reset((int)_stream.fsamp+1);

	if ( _config.useVSFilterImplementation ) {
		_filter0 = new ButterworthFilter(4, 1.0/3.0);
		_filter1 = new ButterworthFilter(4, 1.0/3.0);
	}
	else {
		_filter0 = new Math::Filtering::IIR::ButterworthHighpass<double>(4, 1.0/3.0);
		_filter1 = new Math::Filtering::IIR::ButterworthHighpass<double>(4, 1.0/3.0);
	}

	// Propagate sampling frequencies
	_filter0->setSamplingFrequency(_stream.fsamp);
	_filter1->setSamplingFrequency(_stream.fsamp);
	_toVelocity.setSamplingFrequency(_stream.fsamp);
	_toAcceleration.setSamplingFrequency(_stream.fsamp);
	_toDisplacement.setSamplingFrequency(_stream.fsamp);

	// Set sampling interval
	_dt = Core::TimeSpan(1.0 / _stream.fsamp);
	setupTimeWindow(rec->startTime());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Processor::handleGap(Filter *, const Core::TimeSpan &ts, double, double,
                          size_t missingSamples) {
	SEISCOMP_WARNING("%s: detected gap of %.6f secs or %lu samples: reset processing",
	                 Private::toStreamID(_waveformID).c_str(), (double)ts,
	                 (unsigned long)missingSamples);
	// Flush what is possible
	flush();
	// Reset the processor
	reset();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Processor::process(const Record *rec, const DoubleArray &data) {
	// First record after reset?
	if ( !_stream.initialized ) {
		// Try to setup all internal variables
		if ( !init(rec) ) return;
	}
	// Mismatching sampling frequency: reset
	else if ( rec->samplingFrequency() != _stream.fsamp ) {
		SEISCOMP_INFO("%s: mismatching sampling frequency (%f != %f): reset",
		              rec->streamID().c_str(), _stream.fsamp,
		              rec->samplingFrequency());
		reset();
		// Try to setup all internal variables
		if ( !init(rec) ) return;
	}

	// Record time window is after the current time window -> flush
	// existing samples and setup the new interval
	if ( rec->startTime() >= _currentEndTime ) {
		flush();
		setupTimeWindow(rec->startTime());
	}

	Core::Time ts = rec->startTime();
	// Process all samples
	for ( int i = 0; i < data.size(); ++i ) {
		if ( ts >= _currentEndTime ) {
			// Flush existing pool
			flush();
			// Step to next time span
			_currentStartTime = _currentEndTime;
			_currentEndTime = _currentStartTime + Core::TimeSpan(1,0);
		}
		_samplePool.push(data[i]);
		ts += _dt;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Processor::flush() {
	size_t n = _samplePool.size();
	double *data = _samplePool.samples;
	bool clipped = false;

	// Skip empty sample pool
	if ( n == 0 ) return;

	// -------------------------------------------------------------------
	// Saturation check
	// -------------------------------------------------------------------
	if ( _config.saturationThreshold >= 0 ) {
		double maxCounts = (_config.saturationThreshold * 0.01) * (2 << 23);
		for ( size_t i = 0; i < n; ++i ) {
			if ( fabs(data[i]) > maxCounts ) clipped = true;
		}
	}

	// -------------------------------------------------------------------
	// Sensitivity correction
	// -------------------------------------------------------------------
	double scorr = 1.0 / _streamConfig[_usedComponent].gain;
	for ( size_t i = 0; i < n; ++i ) data[i] *= scorr;

	// -------------------------------------------------------------------
	// Baseline correction and filtering
	// -------------------------------------------------------------------
	double amp0 = getValue(n, data, NULL, _baselineCorrection0, _filter0.get());

	// -------------------------------------------------------------------
	// Conversion to ACC, VEL, DISP
	// -------------------------------------------------------------------
	SignalUnit unit;
	if ( !unit.fromString(_streamConfig[_usedComponent].gainUnit.c_str()) ) {
		SEISCOMP_ERROR("%s: internal error: invalid gain unit '%s'",
		               Private::toStreamID(_waveformID).c_str(),
		               _streamConfig[_usedComponent].gainUnit.c_str());
		return;
	}

	double vel, acc, disp;
	std::vector<double> tmp;
	double *vel_data;

	switch ( unit ) {
		case MeterPerSecond:
			vel = amp0;
			tmp.assign(data, data+n);
			vel_data = &tmp[0];
			acc = getAcceleration(n, data);
			break;
		case MeterPerSecondSquared:
			acc = amp0;
			vel = getVelocity(n, data);
			vel_data = data;
			break;
		default:
			SEISCOMP_ERROR("%s: internal error: unsupported gain unit '%s'",
			               Private::toStreamID(_waveformID).c_str(),
			               _streamConfig[_usedComponent].gainUnit.c_str());
			return;
	}

	disp = getDisplacement(n, vel_data);

	// Publish result
	if ( _func )
		_func(this, acc, vel, disp, _currentStartTime, clipped);

	_samplePool.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Processor::getValue(size_t n, double *samples,
                           Math::Filtering::InPlaceFilter<double> *conversion,
                           AverageBuffer &buffer,
                           Math::Filtering::InPlaceFilter<double> *filter) {
	if ( conversion ) conversion->apply((int)n, samples);

	double mean = Math::Statistics::mean(n, samples);
	buffer.push(mean);
	mean = buffer.average();
	for ( size_t i = 0; i < n; ++i ) samples[i] -= mean;

	if ( filter != NULL ) filter->apply(n, samples);

	return fabs(samples[find_absmax((int)n, samples, 0, n, 0.0)]);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Processor::getAcceleration(size_t n, double *samples) {
	return getValue(n, samples, &_toAcceleration, _baselineCorrection1, _filter1.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Processor::getVelocity(size_t n, double *samples) {
	return getValue(n, samples, &_toVelocity, _baselineCorrection1, _filter1.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Processor::getDisplacement(size_t n, double *samples) {
	return getValue(n, samples, &_toDisplacement, _baselineCorrection2, NULL);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
