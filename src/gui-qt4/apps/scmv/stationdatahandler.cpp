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


#include "stationdatahandler.h"

#include <algorithm>
#include <limits>

#define SEISCOMP_COMPONENT mapview
#include <seiscomp3/logging/log.h>

#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/math/math.h>

#include "debug.h"


using namespace Seiscomp;


bool RecordHandlerCompare(double a0, double a1) {
	return fabs(a0) < fabs(a1);
}




RecordHandler::RecordHandler()
 : _recordLifespan(5*60),
   _sampleLifespan(5*60){

	_velocityLimits[0] = 0;
	_velocityLimits[1] = 200;
	_velocityLimits[2] = 400;
	_velocityLimits[3] = 800;
	_velocityLimits[4] = 1500;
	_velocityLimits[5] = 4000;
	_velocityLimits[6] = 12000;
	_velocityLimits[7] = 30000;
	_velocityLimits[8] = 60000;
	_velocityLimits[9] = 150000;
}




void RecordHandler::handle(StationData* stationData, Record* record) {
	RecordPtr recordAutoPtr(record);

	if ( stationData->gmGain == 0 ) return;
	if ( !record->data() ) return;

	int dataSize = record->data()->size();
	if ( dataSize <= 0 ) return;

	ArrayPtr array = record->data()->copy(Array::DOUBLE);
	double* data = static_cast<double*>(const_cast<void*>(array->data()));
	if ( !data ) return;

	try {
		stationData->gmFilter->setSamplingFrequency(record->samplingFrequency());
		stationData->gmFilter->apply(dataSize, data);
	}
	catch (Core::GeneralException &e) {
		SEISCOMP_WARNING("Could not filter record %s.%s.%s.%s (%fHz): %s",
		                 record->networkCode().c_str(),
		                 record->stationCode().c_str(),
		                 record->locationCode().c_str(),
		                 record->channelCode().c_str(),
		                 record->samplingFrequency(), e.what());
		return;
	}

	double* begin = data;
	double* maximumSample = std::max_element(begin, data + dataSize,
	                                         std::ptr_fun(RecordHandlerCompare));

	*maximumSample = fabs(*maximumSample);

	if ( stationData->gmMaximumSample < *maximumSample ||
			Core::Time::GMT() - stationData->gmSampleReceiveTime > _sampleLifespan ) {
		stationData->gmSampleReceiveTime = Core::Time::GMT();
		stationData->gmMaximumSample= *maximumSample;
	}

	try { stationData->gmRecordReceiveTime = record->endTime(); }
	catch ( Core::ValueException& ) { stationData->gmRecordReceiveTime = Core::Time::GMT(); }

	double velocity = stationData->gmMaximumSample / stationData->gmGain * 1E9;

	stationData->gmColor = calculateColorFromVelocity(velocity);
}




void RecordHandler::update(StationData* stationData) {
	if ( stationData->gmRecordReceiveTime + Core::TimeSpan(_recordLifespan) < Core::Time::GMT() )
		stationData->gmColor = SCApp->scheme().colors.gm.gmNotSet;
}




QColor RecordHandler::interpolate(double x, int x0, int x1,
                                  const QColor& c0, const QColor& c1) const {
	const double basis = x1 - x0;
	const double factor = x - x0;
	int red = c0.red() + static_cast<int>((c1.red() - c0.red()) / basis * factor);
	int green = c0.green() + static_cast<int>((c1.green() - c0.green()) / basis * factor);
	int blue = c0.blue() + static_cast<int>((c1.blue() - c0.blue()) / basis * factor);

	return QColor(red, green, blue);
}




QColor RecordHandler::calculateColorFromVelocity(double velocity) {
	if ( velocity < _velocityLimits[0] )
		return SCApp->scheme().colors.gm.gmNotSet;

	if ( velocity >= _velocityLimits[0] && velocity < _velocityLimits[1] )
		return SCApp->scheme().colors.gm.gm0;

	if ( velocity >= _velocityLimits[1] && velocity < _velocityLimits[2] )
		return interpolate(velocity, _velocityLimits[1], _velocityLimits[2],
		                   SCApp->scheme().colors.gm.gm1,
		                   SCApp->scheme().colors.gm.gm2);

	if ( velocity >= _velocityLimits[2] && velocity < _velocityLimits[3] )
		return interpolate(velocity, _velocityLimits[2], _velocityLimits[3],
		                   SCApp->scheme().colors.gm.gm2,
		                   SCApp->scheme().colors.gm.gm3);

	if ( velocity >= _velocityLimits[3] && velocity < _velocityLimits[4] )
		return interpolate(velocity, _velocityLimits[3], _velocityLimits[4],
		                   SCApp->scheme().colors.gm.gm3,
		                   SCApp->scheme().colors.gm.gm4);

	if ( velocity >= _velocityLimits[4] && velocity < _velocityLimits[5] )
		return interpolate(velocity, _velocityLimits[4], _velocityLimits[5],
		                   SCApp->scheme().colors.gm.gm4,
		                   SCApp->scheme().colors.gm.gm5);

	if ( velocity >= _velocityLimits[5] && velocity < _velocityLimits[6] )
		return interpolate(velocity, _velocityLimits[5], _velocityLimits[6],
		                   SCApp->scheme().colors.gm.gm5,
		                   SCApp->scheme().colors.gm.gm6);

	if ( velocity >= _velocityLimits[6] && velocity < _velocityLimits[7] )
		return interpolate(velocity, _velocityLimits[6], _velocityLimits[7],
		                   SCApp->scheme().colors.gm.gm6,
		                   SCApp->scheme().colors.gm.gm7);

	if ( velocity >= _velocityLimits[7] && velocity < _velocityLimits[8] )
		return interpolate(velocity, _velocityLimits[7], _velocityLimits[8],
		                   SCApp->scheme().colors.gm.gm7,
		                   SCApp->scheme().colors.gm.gm8);

	if ( velocity >= _velocityLimits[8] && velocity < _velocityLimits[9] )
		return  interpolate(velocity, _velocityLimits[8], _velocityLimits[9],
		                   SCApp->scheme().colors.gm.gm8,
		                   SCApp->scheme().colors.gm.gm9);

	return SCApp->scheme().colors.gm.gm9;

}




void RecordHandler::setRecordLifeSpan(double span) {
	_recordLifespan = span;
}




TriggerHandler::TriggerHandler()
 : _defaultPickTriggerLifeSpan(double(20.0)),
   _minimumAmplitudeTriggerLifeSpan(60.0),
   _maximumAmplitudeTriggerLifeSpan(double(4*60.0)),
   _pickLifeSpan(10*60.0) {
}




void TriggerHandler::handle(StationData* stationData, DataModel::Amplitude* amplitude) {
	try {
		stationData->tAmplitudeTime = amplitude->timeWindow().reference();
	}
	catch ( ... ) {
		// Nothing to do
		return;
	}

	stationData->tAmplitude            = amplitude;
	//stationData->tAmplitudeTime        = Core::Time::GMT();
	stationData->isTriggering          = true;

	double amplitudeValue = amplitude->amplitude().value();
	stationData->frameSize = calculateFrameSizeFromAmplitude(amplitudeValue);
	stationData->tAmplitudeTriggerLifeSpan = calculateTriggerLifeSpanFromAmplitude(amplitudeValue);

#ifdef DEBUG_AMPLITUDES
	std::cout << "Station " << stationData->id << " has amplitudeValue " << amplitudeValue
	          << " framesize " << stationData->frameSize << " and a triggerLifeSpan of "
	          << stationData->tAmplitudeTriggerLifeSpan << " sec." << std::endl;
#endif
}




void TriggerHandler::handle(StationData* stationData, DataModel::Pick* pick) {
	try {
		stationData->tPickTime = pick->time().value();
	}
	catch ( ... ) {
		return;
	}

	stationData->tPickCache.push_back(pick);
	//stationData->tPickTime = Core::Time::GMT();

	if ( isAmplitudeExpired(stationData) ) {
		stationData->isTriggering = true;
		stationData->frameSize    = STATION_DEFAULT_PICK_TRIGGER_FRAME_SIZE;
	}
}




void TriggerHandler::update(StationData* stationData) {
	if ( hasValidAmplitude(stationData) ) {
		if ( isAmplitudeExpired(stationData) ) {
			reset(stationData);
		}
	}
	else if ( hasValidPick(stationData) ) {
		if ( isPickExpired(stationData) ) {
			reset(stationData);
		}
	}

	cleanupPickCollection(stationData);
}




void TriggerHandler::setPickLifeSpan(double timeSpan) {
	_pickLifeSpan = timeSpan;
}




void TriggerHandler::cleanupPickCollection(StationData* stationData) {
	std::list<DataModel::PickPtr>::iterator it = stationData->tPickCache.begin();
	for ( ; it != stationData->tPickCache.end(); ++it ) {
		if ( Core::Time::GMT() - (*it)->time() > _pickLifeSpan )
			it = stationData->tPickCache.erase(it);
	}
}




int TriggerHandler::calculateFrameSizeFromAmplitude(double amplitude) const {
	double frameSize = pow(amplitude, 0.125) * SCScheme.map.stationSize * 0.5;
	if ( frameSize > STATION_MAXIMUM_FRAME_SIZE )
		frameSize = STATION_MAXIMUM_FRAME_SIZE;
	else if ( frameSize < STATION_MINIMUM_FRAME_SIZE )
		frameSize = STATION_MINIMUM_FRAME_SIZE;

	return static_cast<int>(Math::round(frameSize));
}




Core::TimeSpan TriggerHandler::calculateTriggerLifeSpanFromAmplitude(double amplitude) {
	int minimumAmplitude = 3;
	int offset = static_cast<int>(
			exp((_minimumAmplitudeTriggerLifeSpan.seconds() + 177) * log(2.0) * 0.01) - minimumAmplitude
	);
	//fenlo: bugfix: check range before(!) assigning double value to TimeSpan to prevent throwing exception (range of TimeSpan is somewhat limited)
	double triggerLifeSpanValue = 
		static_cast<double>((100*log(amplitude + offset)/log(2.0)) - 177);

	if( triggerLifeSpanValue < double(_minimumAmplitudeTriggerLifeSpan) )
		triggerLifeSpanValue = double(_minimumAmplitudeTriggerLifeSpan);
	else if ( triggerLifeSpanValue > double(_maximumAmplitudeTriggerLifeSpan) )
		triggerLifeSpanValue = double(_maximumAmplitudeTriggerLifeSpan);

	return triggerLifeSpanValue;

	/* original code:
	Core::TimeSpan triggerLifeSpan =
			static_cast<double>((100*log(amplitude + offset)/log(2.0)) - 177);

	if (triggerLifeSpan < _minimumAmplitudeTriggerLifeSpan )
		triggerLifeSpan = _minimumAmplitudeTriggerLifeSpan;
	else if ( triggerLifeSpan > _maximumAmplitudeTriggerLifeSpan )
		triggerLifeSpan = _maximumAmplitudeTriggerLifeSpan;

	return triggerLifeSpan;
	*/
}




bool TriggerHandler::isAmplitudeExpired(StationData* stationData) const {
	Core::TimeSpan passedTime = Core::Time::GMT() - stationData->tAmplitudeTime;
	return passedTime > stationData->tAmplitudeTriggerLifeSpan;
}




bool TriggerHandler::isPickExpired(StationData* stationData) const {
	return Core::Time::GMT() - stationData->tPickTime > _defaultPickTriggerLifeSpan;
}




bool TriggerHandler::hasValidAmplitude(StationData* stationData) const {
	return stationData->tAmplitudeTime != Core::Time::Null;
}




bool TriggerHandler::hasValidPick(StationData* stationData) const {
	return stationData->tPickTime != Core::Time::Null;
}




void TriggerHandler::reset(StationData* stationData) const {
	stationData->frameSize      = STATION_DEFAULT_FRAME_SIZE;
	stationData->isTriggering   = false;

	stationData->tPickTime      = Core::Time::Null;
	stationData->tAmplitudeTime = Core::Time::Null;
}




void QCHandler::handle(StationData* stationData, Seiscomp::DataModel::WaveformQuality* waveformQuality) {
	std::string qcParameterName = waveformQuality->parameter();
	QCParameter::Parameter parameter = resolveQualityControlParameter(qcParameterName);

	setQualityControlValues(parameter, stationData, waveformQuality);
	calculateQualityControlStatus(parameter, stationData);
	setGlobalQualityControlStatus(stationData);

	if ( parameter == QCParameter::DELAY )
		stationData->qcColor = calculateDelayQualityControlColor(waveformQuality->value());
}


void QCHandler::init(const Seiscomp::Config::Config &config) {
	for ( int i = 0; i < QCParameter::Parameter::Quantity; ++i ) {
		_thresholds[i].initialized = false;

		try { _thresholds[i].value[0] = config.getDouble(std::string("scmv.qc.") + QCParameter::EParameterNames::name(i) + ".OK"); }
		catch ( ... ) {}

		try { _thresholds[i].value[1] = config.getDouble(std::string("scmv.qc.") + QCParameter::EParameterNames::name(i) + ".WARNING"); }
		catch ( ... ) {}
	}
	
}


void QCHandler::update(StationData* ) {
}




QCParameter::Parameter QCHandler::resolveQualityControlParameter(const std::string& parameter) {
	if ( parameter == "availability" )
		return QCParameter::AVAILABILITY;
	else if ( parameter == "gaps count" )
		return QCParameter::GAPS_COUNT;
	else if ( parameter == "delay" )
		return QCParameter::DELAY;
	else if ( parameter == "gaps interval" )
		return QCParameter::GAPS_INTERVAL;
	else if ( parameter == "gaps length" )
		return QCParameter::GAPS_LENGTH;
	else if ( parameter == "latency" )
		return QCParameter::LATENCY;
	else if ( parameter == "offset" )
		return QCParameter::OFFSET;
	else if ( parameter == "overlaps interval" )
		return QCParameter::OVERLAPS_INTERVAL;
	else if ( parameter == "overlaps length" )
		return QCParameter::OVERLAPS_LENGTH;
	else if ( parameter == "overlaps count" )
		return QCParameter::OVERLAPS_COUNT;
	else if ( parameter == "rms" )
		return QCParameter::RMS;
	else if ( parameter == "spikes interval" )
		return QCParameter::SPIKES_INTERVAL;
	else if ( parameter == "spikes amplitude" )
		return QCParameter::SPIKES_AMPLITUDE;
	else if ( parameter == "spikes count" )
		return QCParameter::SPIKES_COUNT;
	else if ( parameter == "timing quality" )
		return QCParameter::TIMING_QUALITY;
	else
		SEISCOMP_ERROR("Could not resolve qc parameter %s", parameter.c_str());

	return QCParameter::EParameterQuantity;
}





void QCHandler::setQualityControlValues(QCParameter::Parameter parameter, StationData* stationData,
                                        DataModel::WaveformQuality* waveformQuality) {
	StationData::QCData& qcData = stationData->qcDataMap[parameter];

	double qcValue = waveformQuality->value();
	qcData.value = qcValue;

	try {
		qcData.lowerUncertainty = waveformQuality->lowerUncertainty();
	} catch(Seiscomp::Core::ValueException& e) {
		qcData.lowerUncertainty = std::numeric_limits<double>::quiet_NaN();
	}

	try {
		qcData.upperUncertainty = waveformQuality->upperUncertainty();
	} catch(Seiscomp::Core::ValueException& e) {
		qcData.upperUncertainty = std::numeric_limits<double>::quiet_NaN();
	}
}




void QCHandler::calculateQualityControlStatus(QCParameter::Parameter parameter, StationData* stationData) {
	stationData->qcDataMap[parameter].status = QCStatus::OK;
	double value = stationData->qcDataMap[parameter].value;

	/*
	if ( !_thresholds[parameter].initialized )
		stationData->qcDataMap[parameter].status = QCStatus::NOT_SET;
	else {
		if ( value > _thresholds[parameter].value[1] )
			stationData->qcDataMap[parameter].status = QCStatus::ERROR;
		else if ( value > _thresholds[parameter].value[0] )
			stationData->qcDataMap[parameter].status = QCStatus::WARNING;
	}
	*/

	switch ( parameter ) {
		case QCParameter::DELAY:
			if ( value > 60 )
				stationData->qcDataMap[parameter].status = QCStatus::WARNING;
			break;

		case QCParameter::LATENCY:
			if ( value > 60 )
				stationData->qcDataMap[parameter].status = QCStatus::WARNING;
			break;

		case QCParameter::TIMING_QUALITY:
			if ( value < 50 )
				stationData->qcDataMap[parameter].status = QCStatus::WARNING;
			break;

		case QCParameter::GAPS_LENGTH:
			if ( value > 0 )
				stationData->qcDataMap[parameter].status = QCStatus::ERROR;
			break;

		case QCParameter::OFFSET:
			if ( value > 100000 )
				stationData->qcDataMap[parameter].status = QCStatus::WARNING;
			break;

		case QCParameter::RMS:
			if ( value < 10 )
				stationData->qcDataMap[parameter].status = QCStatus::ERROR;
			break;

		default:
			stationData->qcDataMap[parameter].status = QCStatus::NOT_SET;
			break;
	}
}




void QCHandler::setGlobalQualityControlStatus(StationData* stationData) {
	stationData->qcGlobalStatus = QCStatus::NOT_SET;

	StationData::QcDataMap::iterator it = stationData->qcDataMap.begin();
	for ( ; it != stationData->qcDataMap.end(); it++ ) {
		if ( stationData->qcGlobalStatus < it->second.status )
			stationData->qcGlobalStatus = it->second.status;
	}
}




QColor QCHandler::calculateDelayQualityControlColor(double delayValue) {
	if ( delayValue > QC_DELAY6 )
		return SCScheme.colors.qc.delay7;

	if ( delayValue <= QC_DELAY6 && delayValue > QC_DELAY5 )
		return SCScheme.colors.qc.delay6;

	if ( delayValue <= QC_DELAY5 && delayValue > QC_DELAY4 )
		return SCScheme.colors.qc.delay5;

	if ( delayValue <= QC_DELAY4 && delayValue > QC_DELAY3 )
		return SCScheme.colors.qc.delay4;

	if ( delayValue <= QC_DELAY3 && delayValue > QC_DELAY2 )
		return SCScheme.colors.qc.delay3;

	if ( delayValue <= QC_DELAY2 && delayValue > QC_DELAY1 )
		return SCScheme.colors.qc.delay2;

	if ( delayValue <= QC_DELAY1 && delayValue > QC_DELAY0 )
		return SCScheme.colors.qc.delay1;

	return SCScheme.colors.qc.delay0;
}


