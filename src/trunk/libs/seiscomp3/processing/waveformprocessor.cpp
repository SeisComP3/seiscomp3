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


#define SEISCOMP_COMPONENT WaveformProcessor

#include <seiscomp3/processing/waveformprocessor.h>
#include <seiscomp3/processing/waveformoperator.h>
#include <seiscomp3/logging/log.h>

#include <boost/bind.hpp>


namespace Seiscomp {

namespace Processing {

IMPLEMENT_SC_ABSTRACT_CLASS(WaveformProcessor, "WaveformProcessor");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WaveformProcessor::StreamState::StreamState()
: lastSample(0), neededSamples(0), receivedSamples(0), initialized(false),
  fsamp(0.0), filter(NULL) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WaveformProcessor::StreamState::~StreamState() {
	if ( filter != NULL ) delete filter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WaveformProcessor::WaveformProcessor(const Core::TimeSpan &initTime,
                                     const Core::TimeSpan &gapThreshold)
 : _enabled(true),
   _initTime(initTime), _gapThreshold(gapThreshold), _usedComponent(Vertical) {

	_gapTolerance = 0.;
	_enableGapInterpolation = false;
	_enableSaturationCheck = false;
	_saturationThreshold = -1;
	reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WaveformProcessor::~WaveformProcessor() {
	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Stream &WaveformProcessor::streamConfig(Component c) {
	return _streamConfig[c];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Stream &WaveformProcessor::streamConfig(Component c) const {
	return _streamConfig[c];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WaveformProcessor::setFilter(Filter *filter) {
	if ( _stream.filter ) delete _stream.filter;
	_stream.filter = filter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WaveformProcessor::setOperator(WaveformOperator *op) {
	if ( _operator ) _operator->setStoreFunc(WaveformOperator::StoreFunc());

	_operator = op;

	if ( _operator ) _operator->setStoreFunc(boost::bind(&WaveformProcessor::store, this, _1));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WaveformProcessor::setGapTolerance(const Core::TimeSpan &length) {
	_gapTolerance = length;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Core::TimeSpan &WaveformProcessor::gapTolerance() const {
	return _gapTolerance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WaveformProcessor::reset() {
	Filter *tmp = _stream.filter;

	_stream = StreamState();

	if ( _operator ) _operator->reset();

	if ( tmp != NULL ) {
		_stream.filter = tmp->clone();
		delete tmp;
	}

	_status = WaitingForData;
	_statusValue = 0.;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WaveformProcessor::setEnabled(bool e) {
	if ( _enabled == e ) return;
	_enabled = e;
	_enabled?enabled():disabled();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WaveformProcessor::setGapInterpolationEnabled(bool enable) {
	_enableGapInterpolation = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WaveformProcessor::isGapInterpolationEnabled() const {
	return _enableGapInterpolation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WaveformProcessor::setSaturationCheckEnabled(bool enable) {
	_enableSaturationCheck = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WaveformProcessor::setSaturationThreshold(double t) {
	_saturationThreshold = t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WaveformProcessor::isSaturationCheckEnabled() const {
	return _enableSaturationCheck;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::TimeWindow WaveformProcessor::dataTimeWindow() const {
	return _stream.dataTimeWindow;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Record *WaveformProcessor::lastRecord() const {
	return _stream.lastRecord.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WaveformProcessor::initFilter(double fsamp) {
	_stream.fsamp = fsamp;
	_stream.neededSamples = static_cast<size_t>(_initTime * _stream.fsamp + 0.5);
	if ( _stream.filter )
		_stream.filter->setSamplingFrequency(fsamp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double WaveformProcessor::samplingFrequency() const {
	return _stream.fsamp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WaveformProcessor::feed(const Record *rec) {
	if ( !_operator ) return store(rec);
	Status stat = _operator->feed(rec);
	if ( stat > Terminated ) {
		setStatus(stat, -1);
		return false;
	}
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WaveformProcessor::store(const Record *record) {
	if ( _status > InProgress ) return false;
	if ( record->data() == NULL ) return false;

	DoubleArrayPtr arr = (DoubleArray*)record->data()->copy(Array::DOUBLE);

	if ( _stream.lastRecord ) {
		if ( record == _stream.lastRecord ) return false;

		Core::TimeSpan gap = record->startTime() - _stream.dataTimeWindow.endTime() - Core::TimeSpan(0,1);
		double gapSecs = (double)gap;

		if ( gap > _gapThreshold ) {
			size_t gapsize = static_cast<size_t>(ceil(_stream.fsamp * gapSecs));
			bool handled = handleGap(_stream.filter, gap, _stream.lastSample, (*arr)[0], gapsize);
			if ( handled )
				SEISCOMP_DEBUG("[%s] detected gap of %.6f secs or %lu samples (handled)",
				               record->streamID().c_str(), (double)gap, (unsigned long)gapsize);
			else {
				SEISCOMP_DEBUG("[%s] detected gap of %.6f secs or %lu samples (NOT handled): status = %s",
				               record->streamID().c_str(), (double)gap, (unsigned long)gapsize,
				               status().toString());
				if ( _status > InProgress ) return false;
			}
		}
		else if ( gapSecs < 0 ) {
			size_t gapsize = static_cast<size_t>(ceil(-_stream.fsamp * gapSecs));
			if ( gapsize > 1 ) return false;
		}

		// update the received data timewindow
		_stream.dataTimeWindow.setEndTime(record->endTime());
	}

	// NOTE: Do not use else here, because lastRecord can be set NULL
	//       when calling reset() in handleGap(...)
	if ( !_stream.lastRecord ) {
		initFilter(record->samplingFrequency());

		// update the received data timewindow
		_stream.dataTimeWindow = record->timeWindow();
		/*
		std::cerr << "Received first record for " << record->streamID() << ", "
		          << className() << " [" << record->startTime().iso() << " - " << record->endTime().iso() << std::endl;
		*/
		if ( _stream.filter ) {
			_stream.filter->setStartTime(record->startTime());
			_stream.filter->setStreamID(record->networkCode(), record->stationCode(),
			                            record->locationCode(), record->channelCode());
		}
	}

	// Fill the values and do the actual filtering
	fill(arr->size(), arr->typedData());
	if ( _status > InProgress ) return false;

	if ( !_stream.initialized ) {
		if ( _stream.receivedSamples > _stream.neededSamples ) {
			//_initialized = true;
			process(record, *arr);
			// NOTE: To allow derived classes to notice modification of the variable
			//       _initialized, it is necessary to set this after calling process.
			_stream.initialized = true;
		}
	}
	else
		// Call process to cause a derived processor to work on the data.
		process(record, *arr);

	_stream.lastRecord = record;
	_stream.lastSample = (*arr)[arr->size()-1];

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int WaveformProcessor::feedSequence(const RecordSequence *sequence) {
	int count = 0;

	if ( sequence == NULL ) return count;
	for ( RecordSequence::const_iterator it = sequence->begin();
	      it != sequence->end(); ++it )
		if ( feed(it->get()) ) ++count;

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WaveformProcessor::setUserData(Core::BaseObject *obj) const {
	_userData = obj;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::BaseObject *WaveformProcessor::userData() const {
	return _userData.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WaveformProcessor::fill(size_t n, double *samples) {
	_stream.receivedSamples += n;

	if ( _enableSaturationCheck ) {
		for ( size_t i = 0; i < n; ++i ) {
			if ( fabs(samples[i]) >= _saturationThreshold ) {
				setStatus(DataClipped, samples[i]);
				break;
			}
		}
	}

	if ( _stream.filter ) _stream.filter->apply(n, samples);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WaveformProcessor::handleGap(Filter *filter, const Core::TimeSpan& span,
                                  double lastSample, double nextSample,
                                  size_t missingSamples) {
	if ( span <= _gapTolerance ) {
		if ( _enableGapInterpolation ) {
			double delta = nextSample - lastSample;
			double step = 1./(double)(missingSamples+1);
			double di = step;
			for ( size_t i = 0; i < missingSamples; ++i, di += step ) {
				double value = lastSample + di*delta;
				fill(1, &value);
			}
		}

		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WaveformProcessor::isFinished() const {
	return _status > InProgress;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WaveformProcessor::setStatus(Status status, double value) {
	_status = status;
	_statusValue = value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WaveformProcessor::terminate() {
	setStatus(Terminated, _status);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WaveformProcessor::Status WaveformProcessor::status() const {
	return _status;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double WaveformProcessor::statusValue() const {
	return _statusValue;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WaveformProcessor::close() const {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WaveformProcessor::parseSaturationThreshold(const Settings &settings,
                                                 const std::string &optionName) {
	std::string saturationThreshold;

	if ( settings.getValue(saturationThreshold, optionName) ) {
		Core::trim(saturationThreshold);

		if ( saturationThreshold == "false" ) {
			setSaturationCheckEnabled(false);
			return true;
		}

		// Parse it
		size_t atPos = saturationThreshold.find('@');
		double sat_thrs = -1;

		if ( atPos == std::string::npos ) {
			// This is an absolute value
			if ( !Core::fromString(sat_thrs, saturationThreshold) ) {
				SEISCOMP_ERROR("Invalid saturation threshold: %s", saturationThreshold.c_str());
				return false;
			}
		}
		else {
			std::string strBits = saturationThreshold.substr(atPos+1);
			std::string strValue;
			int bits;
			double value;

			if ( strBits.empty() ) {
				SEISCOMP_ERROR("No effective bits specified: %s",
				               saturationThreshold.c_str());
				return false;
			}

			if ( !Core::fromString(bits, strBits) ) {
				SEISCOMP_ERROR("Invalid saturation threshold bits: %s",
				               saturationThreshold.c_str());
				return false;
			}

			if ( bits <= 0 || bits > 64 ) {
				SEISCOMP_ERROR("Number of effective bits out of range: %d", bits);
				return false;
			}

			strValue = saturationThreshold.substr(0, atPos);
			Core::trim(strValue);

			if ( strValue.empty() ) {
				SEISCOMP_ERROR("Saturation threshold relative value is empty: %s",
				               saturationThreshold.c_str());
				return false;
			}

			bool isPercent = false;
			if ( *strValue.rbegin() == '%' ) {
				isPercent = true;
				strValue.resize(strValue.size()-1);
			}

			if ( strValue.empty() ) {
				SEISCOMP_ERROR("Saturation threshold relative value is empty: %s",
				               saturationThreshold.c_str());
				return false;
			}

			if ( !Core::fromString(value, strValue) ) {
				SEISCOMP_ERROR("Invalid saturation threshold relative value: %s",
				               saturationThreshold.c_str());
				return false;
			}

			if ( isPercent )
				value *= 0.01;

			if ( value < 0 || value > 1 ) {
				SEISCOMP_ERROR("Number of relative value out of range [0,1]: %f", value);
				return false;
			}

			sat_thrs = (1 << bits) * value;
		}

		setSaturationThreshold(sat_thrs);
		setSaturationCheckEnabled(true);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WaveformProcessor::setup(const Settings &settings) {
	if ( !parseSaturationThreshold(settings, "waveforms.saturationThreshold") )
		return false;

	return Processor::setup(settings);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}
