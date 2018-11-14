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


#define SEISCOMP_COMPONENT SCQC

#include <seiscomp3/logging/log.h>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/datamodel/types.h>

#include <boost/visit_each.hpp>
#include <boost/any.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include<boost/tokenizer.hpp>

#include <math.h>

#include <seiscomp3/qc/qcprocessor.h>
#include "qcmessenger.h"
#include "qcbuffer.h"
#include "qcconfig.h"
#include "qcplugin.h"

#include <seiscomp3/core/interfacefactory.ipp>


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::Applications::Qc::QcPlugin, SC_QCPLUGIN_API);


namespace Seiscomp {
namespace Applications {
namespace Qc {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/**
* Returns for the given stream ID the corresponding WaveformStreamID object.
*
* @param: streamID, a string specifying the <network>.<station>.<location>.<channel>
* @return: a WaveformStreamID object
**/
DataModel::WaveformStreamID getWaveformID(const std::string &streamID) {

std::string s = streamID;
std::string::size_type dot;

DataModel::WaveformStreamID waveformID;

if((dot = s.find('.')) != std::string::npos) {
	waveformID.setNetworkCode(std::string(s, 0, dot));
	s = std::string(s, dot + 1, std::string::npos);

	if((dot = s.find('.')) != std::string::npos) {
		waveformID.setStationCode(std::string(s, 0, dot));
		s = std::string(s, dot + 1, std::string::npos);
	
		if((dot = s.find('.')) != std::string::npos) {
			waveformID.setLocationCode(std::string(s, 0, dot));
			waveformID.setChannelCode(std::string(s, dot + 1, std::string::npos));
		}
	}
}
return waveformID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IMPLEMENT_SC_ABSTRACT_CLASS(QcPlugin, "QcPlugin");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QcPlugin::QcPlugin() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QcPlugin::~QcPlugin() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QcPlugin::init(QcApp* app, QcConfig *cfg, std::string streamID) {
	_app = app;
	_qcConfig = cfg;
	_streamID = streamID;
	_qcMessenger = _app->qcMessenger();
	_firstRecord = true; 
	_qcBuffer = new QcBuffer(_app->archiveMode() ? _qcConfig->archiveBuffer() : _qcConfig->buffer());

	if (! _app->archiveMode() && _qcConfig->reportTimeout() != 0) {
		_timer.restart();
		SEISCOMP_INFO("using report timeout %d s for %s", _qcConfig->reportTimeout(), _name.c_str());
		_app->addTimeout(boost::bind(&QcPlugin::onTimeout, this));
	}

	_app->doneSignal.connect(boost::bind(&QcPlugin::done, this));

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcPlugin::generateNullReport() const {
	for (size_t i = 0; i < _parameterNames.size(); ++i) {
		SEISCOMP_DEBUG("%s: generateNullReport[%ld]: OK", _streamID.c_str(), (long int)i);

		DataModel::WaveformQuality* obj = new DataModel::WaveformQuality();
		obj->setWaveformID(getWaveformID(_streamID));
		obj->setCreatorID(_app->creatorID());
		obj->setCreated(Core::Time::GMT());
		obj->setStart(Core::Time::GMT());
		obj->setEnd(Core::Time::GMT());
		obj->setType("report");
		obj->setParameter(_parameterNames[i]);
		obj->setValue(0.0);
		obj->setLowerUncertainty(0.0);
		obj->setUpperUncertainty(0.0);
		obj->setWindowLength(-1.0);
		
		pushObject(DataModel::Object::Cast(obj));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcPlugin::generateReport(const QcBuffer *buf) const {
	if ( buf->empty() ) return;

	double mean_ = mean(buf);
	double stdDev_ = stdDev(buf, mean_);

	DataModel::WaveformQuality* obj = new DataModel::WaveformQuality();
	obj->setWaveformID(getWaveformID(_streamID));
	obj->setCreatorID(_app->creatorID());
	obj->setCreated(Core::Time::GMT());
	obj->setStart(buf->startTime());
	obj->setEnd(buf->endTime());
	obj->setType("report");
	obj->setParameter(_parameterNames[0]);
	obj->setValue(mean_);
	obj->setLowerUncertainty(stdDev_);
	obj->setUpperUncertainty(stdDev_);
	obj->setWindowLength((double)buf->length());

	pushObject(obj);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcPlugin::generateAlert(const QcBuffer *shortBuffer,
                             const QcBuffer *longBuffer) const {
	if ( shortBuffer->empty() || longBuffer->empty() ) return;

	//shortBuffer->info(); Short Term buffer info
	//lta->info(); Long Term buffer info

	double sta = mean(shortBuffer);
	//double staStdDev = stdDev(shortBuffer, sta);

	double lta = mean(longBuffer);
	double ltaStdDev = stdDev(longBuffer, lta);

	double relative = 0.0;

	//! HACK
	if (ltaStdDev != 0.0)
		relative =  100.0 - ( (ltaStdDev - fabs(lta - sta)) / ltaStdDev ) * 100.0;

	string f = "\033[32m"; // green colour

	if (fabs(relative) > *(_qcConfig->alertThresholds().begin())) { // HACK multi thresholds not yet implemented!
		
		DataModel::WaveformQuality* obj = new DataModel::WaveformQuality();
		obj->setWaveformID(getWaveformID(_streamID));
		obj->setCreatorID(_app->creatorID());
		obj->setCreated(Core::Time::GMT());
		obj->setStart(shortBuffer->startTime());
		obj->setEnd(shortBuffer->endTime());
		obj->setType("alert");
		obj->setParameter(_parameterNames[0]);
		obj->setValue(sta);
		obj->setLowerUncertainty(relative);
		obj->setUpperUncertainty(lta);
		obj->setWindowLength((double)shortBuffer->length());

		pushObject(obj);
	
		f = "\033[31m"; // red colour
		SEISCOMP_WARNING("%s %s %s %.0f%% \033[30m  %.3f %.3f", _streamID.c_str(), _parameterNames[0].c_str(), f.c_str(), relative, fabs(relative), lta);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcPlugin::pushObject(DataModel::Object *obj) const {
	_objects.push(obj);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//! pass Objects to qcMessenger
// false = data messages; true = notifier messages
void QcPlugin::sendObjects(bool notifier) {
	while ( !_objects.empty() ) {
		_qcMessenger->attachObject(_objects.front().get(), notifier);
		_objects.pop();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double QcPlugin::mean(const QcBuffer *qcb) const {
	if ( qcb->size() < 1 ) return 0.0;

	double sum = 0.0;

	for ( QcBuffer::const_iterator p = qcb->begin(); p != qcb->end(); ++p )
		sum += boost::any_cast<double>((*p)->parameter);

	return sum / qcb->size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double QcPlugin::stdDev(const QcBuffer* qcb, double mean) const {
	if ( qcb->size() < 2 ) return 0.0;

	double sum = 0.0;

	for ( QcBuffer::const_iterator p = qcb->begin(); p != qcb->end(); ++p )
		sum += pow(boost::any_cast<double>((*p)->parameter) - mean, 2);

	return sqrt(sum / (qcb->size() - 1));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcPlugin::update() {
	QcParameter *qcp = _qcProcessor->getState();

	if ( _qcProcessor->isValid() )
		_qcBuffer->push_back(qcp);

	sendMessages(qcp->recordEndTime);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcPlugin::sendMessages(const Core::Time &rectime) {
	_timer.restart();

	if ( _firstRecord ) {
		_lastArchiveTime = rectime;
		_lastReportTime = rectime;
		_lastAlertTime = rectime;
		_firstRecord = false;
	}

	if ( _qcBuffer->empty() )
		return;

	//! DEBUG
	if ( rectime == Core::Time() ) {
		try {
			SEISCOMP_DEBUG("%s: %d sec timeout reached for stream: %s.", _name.c_str(),  _qcConfig->reportTimeout(), _streamID.c_str());
		}
		catch ( QcConfigException ) {}
	}

	Core::TimeSpan diff;

	//! A R C H I V E
	if ( _qcConfig->archiveInterval() >= 0 && rectime != Core::Time() ) {
		diff = rectime - _lastArchiveTime;
		if ( diff > Core::TimeSpan(_qcConfig->archiveInterval()) || _app->exitRequested() ) {
			QcBufferCPtr archiveBuffer = _qcBuffer->qcParameter(_qcConfig->archiveBuffer());
			if ( !archiveBuffer->empty() ) {
				generateReport(archiveBuffer.get());
				sendObjects(true); // as notifier msg
				_lastArchiveTime = rectime;
			}
		}
	}

	//! R E P O R T
	if ( _qcConfig->reportInterval() >= 0 ) {
		diff = rectime - _lastReportTime;
		if ( diff > Core::TimeSpan(_qcConfig->reportInterval()) || rectime == Core::Time()) {
			QcBufferCPtr reportBuffer = _qcBuffer->qcParameter(_qcConfig->reportBuffer());
			generateReport(reportBuffer.get());
			sendObjects(false);
			_lastReportTime = rectime;
		}
	}
	
	//! A L E R T
	// in archive mode we don't want alert msg
	if ( !_app->archiveMode() && _qcConfig->alertInterval() >= 0 ) {
		diff = rectime - _lastAlertTime;
		if ( ( diff > Core::TimeSpan(_qcConfig->alertInterval()) && (int)_qcBuffer->length() >= _qcConfig->alertBuffer() ) || rectime == Core::Time()) {
			QcBufferCPtr alertBuffer = _qcBuffer->qcParameter(_qcConfig->alertBuffer());
			if (alertBuffer->empty()) return;
			generateAlert(alertBuffer.get(), _qcBuffer.get());
			sendObjects(false);
			_lastAlertTime = rectime;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QcProcessor* QcPlugin::qcProcessor() {
	return _qcProcessor.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcPlugin::onTimeout() {
	if ( (double)_timer.elapsed() > _qcConfig->reportTimeout() ) {
		timeoutTask();
		_timer.restart();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcPlugin::timeoutTask() {
	// NOOP ----- if needed, implement this in derived classes
	SEISCOMP_WARNING("[%s] TimeOut specified, but no timeoutTask was defined for this QcPlugin.", registeredName().c_str());

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcPlugin::done() {
	sendMessages(Core::Time());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}

