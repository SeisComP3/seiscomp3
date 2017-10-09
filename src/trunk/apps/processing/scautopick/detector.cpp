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




#define SEISCOMP_COMPONENT Autopick
#include <seiscomp3/logging/log.h>

#include "detector.h"

namespace Seiscomp {

namespace Applications {

namespace Picker {

namespace {

class AmplitudeProcessor_SNR : public Processing::AmplitudeProcessor {
	public:
		AmplitudeProcessor_SNR(const Core::Time& trigger)
		: Processing::AmplitudeProcessor(trigger, "snr") {}

	protected:
		bool computeAmplitude(const DoubleArray &data,
		                      size_t i1, size_t i2,
		                      size_t si1, size_t si2,
		                      double offset,AmplitudeIndex *dt,
		                      AmplitudeValue *amplitude,
		                      double *period, double *snr) {
			return false;
		}
};

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Detector::AmplitudeMaxProcessor::reset() {
	amplitude = Core::None;
	index = 0;
	pickIndex = 0;

	neededSamples = 0;
	processedSamples = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline bool Detector::AmplitudeMaxProcessor::isFinished() const {
	return neededSamples > 0 && processedSamples >= neededSamples;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline bool Detector::AmplitudeMaxProcessor::isRunning() const {
	return neededSamples > 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Detector::Detector(double initTime)
 : Processing::SimpleDetector(initTime) {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Detector::Detector(double on, double off, double initTime)
 : Processing::SimpleDetector(on, off, initTime) {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Detector::init() {
	setDeadTime(10.);
	setAmplitudeTimeWindow(10.);
	setMinAmplitudeOffset(3);
	setDurations(-1,-1);
	setSensitivityCorrection(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Detector::setDeadTime(const Core::TimeSpan &dt) {
	_deadTime = dt;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Detector::setAmplitudeTimeWindow(const Core::TimeSpan &dt) {
	_twMax = dt;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Detector::setMinAmplitudeOffset(double at) {
	_amplitudeThreshold = at;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Detector::setDurations(double minDur, double maxDur) {
	_minDuration = minDur;
	_maxDuration = maxDur;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Detector::setSensitivityCorrection(bool enable) {
	_sensitivityCorrection = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Detector::reset() {
	Processing::SimpleDetector::reset();
	_lastPick = Core::Time();
	_lastAmplitude = Core::None;
	_currentPick = Core::Time();
	_currentPickRecord = NULL;
	_minAmplitude = Core::None;
	_triggeredAmplitude = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Detector::fill(size_t n, double *samples) {
	if ( _sensitivityCorrection )
		_streamConfig[_usedComponent].applyGain(n, samples);

	Processing::SimpleDetector::fill(n, samples);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Detector::setAmplitudePublishFunction(const Processing::AmplitudeProcessor::PublishFunc& func) {
	_amplPublish = func;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Detector::setPickID(const std::string& id) const {
	_pickID = id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Detector::handleGap(Filter *filter, const Core::TimeSpan& gapLength,
                         double lastSample, double nextSample,
                         size_t missingSamples) {
	if ( SimpleDetector::handleGap(filter, gapLength, lastSample,
	                               nextSample, missingSamples) ) {
		if ( _amplProc.isRunning() )
			_amplProc.processedSamples += missingSamples;

		return true;
	}

	reset();
	_amplProc.reset();

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Detector::emitPick(const Record *rec, const Core::Time &t) {
	// Is there a last pick and a last snr amplitude? Then defer the pick
	// until the max amplitude has been calculated
	if ( _lastAmplitude && (bool)_lastPick ) {
		double dt = (double)(t - _lastPick);
		double deadTime = (double)_deadTime;

		if ( deadTime > 0 ) {
			double dtn = dt/deadTime;
			_minAmplitude = _amplitudeThreshold + *_lastAmplitude * exp(-dtn*dtn);
		}
		else
			_minAmplitude = _amplitudeThreshold;

		SEISCOMP_DEBUG("Time to last pick = %.2f sec, last amplitude = %.2f, minimum amplitude = %.2f", dt, *_lastAmplitude, *_minAmplitude);

		if ( *_minAmplitude > _triggeredAmplitude ) {
			_currentPick = t;
			_currentPickRecord = rec;

			SEISCOMP_DEBUG("Defer pick, minimum amplitude to reach is %.2f", *_minAmplitude);
			return false;
		}

		SEISCOMP_DEBUG("Deferred sending disabled, pick already reached minimum amplitude: %.2f >= %.2f", _triggeredAmplitude, *_minAmplitude);
	}

	return sendPick(rec, t);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Detector::sendPick(const Record* rec, const Core::Time& t) {
	bool res = SimpleDetector::emitPick(rec, t);
	_lastPick = t;

	_minAmplitude = Core::None;
	_currentPickRecord = NULL;
	_currentPick = Core::Time();

	return res;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Detector::process(const Record *record, const DoubleArray &filteredData) {
	_amplProc.pickIndex = 0;

	SimpleDetector::process(record, filteredData);

	if ( _amplProc.isRunning() ) {
		calculateMaxAmplitude(record, _amplProc.pickIndex, filteredData.size(), filteredData);
		if ( _amplProc.isFinished() )
			sendMaxAmplitude(record);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Detector::validateOn(const Record *record, size_t &i, const DoubleArray &filteredData) {
	_amplProc.reset();
	_amplProc.pickIndex = i;
	_amplProc.neededSamples = (size_t)(_twMax * record->samplingFrequency());

	_triggeredAmplitude = filteredData[i];
	_minAmplitude = Core::None;
	_pickID = "";

	SEISCOMP_DEBUG("[%s] trigger on = %s, amp = %f",
	               record->streamID().c_str(),
	               (record->startTime() + Core::TimeSpan(double(i)/record->samplingFrequency())).iso().c_str(),
	               _triggeredAmplitude);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Detector::validateOff(const Record *record, size_t i,
                           const DoubleArray &filteredData) {
	// No snr amplitude sent yet but an emitted pick?
	if ( _amplProc.isRunning() ) {
		calculateMaxAmplitude(record, _amplProc.pickIndex, i, filteredData);
		sendMaxAmplitude(record);
	}

	if ( _lastPick ) {
		Core::Time offTime;
		offTime = record->startTime() +
		          Core::TimeSpan(double(i)/record->samplingFrequency());
		SEISCOMP_DEBUG("[%s] trigger off = %s, amp = %f",
		               record->streamID().c_str(),
		               offTime.iso().c_str(),
		               filteredData[i]);
		SEISCOMP_DEBUG("[%s] duration = %.2f",
			       record->streamID().c_str(),
		               double(offTime - _lastPick));
		SEISCOMP_DEBUG("[%s] pick id is '%s'",
			       record->streamID().c_str(), _pickID.c_str());
	}

	// Pick window is gone, reset pickID
	_pickID = "";

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Detector::calculateMaxAmplitude(const Record *record, size_t i0, size_t i1, const DoubleArray &filteredData) {
	size_t i = i0;

	while ( i < i1 && _amplProc.processedSamples < _amplProc.neededSamples && i < (size_t)filteredData.size() ) {
		double amp = fabs(filteredData[i]);
		if ( !_amplProc.amplitude || amp > *_amplProc.amplitude ) {
			_amplProc.amplitude = amp;
			_amplProc.index = _amplProc.processedSamples;
		}
		++i;
		++_amplProc.processedSamples;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Detector::sendMaxAmplitude(const Record *record) {
	_lastAmplitude = _amplProc.amplitude;

	double offset = (double)_amplProc.index / record->samplingFrequency();

	_amplProc.reset();

	if ( !_lastAmplitude ) {
		SEISCOMP_ERROR("%s: This should never happen: no SNR amplitude to be sent", record->streamID().c_str());
		return;
	}

	if ( _minAmplitude && *_lastAmplitude < *_minAmplitude ) {
		SEISCOMP_DEBUG("%s: Skipping pick, minimum amplitude not reached: %.2f < %.2f", record->streamID().c_str(), *_lastAmplitude, *_minAmplitude);
		return;
	}

	// emit the current pick
	if ( _currentPickRecord && (bool)_currentPick )
		sendPick(_currentPickRecord.get(), _currentPick);

	Core::Time t = _lastPick + Core::TimeSpan(offset);
	if ( _amplPublish && isEnabled() ) {
		if ( _pickID.empty() ) {
			SEISCOMP_DEBUG("No valid pickID set (pick has not been sent), cancel sending 'snr' amplitude");
			return;
		}

		AmplitudeProcessor_SNR ampProc(_lastPick + this->offset());
		Processing::AmplitudeProcessor::Result res;
		ampProc.setReferencingPickID(_pickID);
		res.amplitude.value = *_lastAmplitude;
		res.period = 0.;
		res.snr = -1.;
		res.time.reference = t;
		res.time.begin = -offset;
		res.time.end = (double)_twMax-offset;
		res.record = record;
		_amplPublish(&ampProc, res);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}

}
