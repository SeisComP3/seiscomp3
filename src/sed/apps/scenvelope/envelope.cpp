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


#include "envelope.h"

#define SEISCOMP_COMPONENT Envelope
#include <seiscomp3/logging/log.h>

#include <seiscomp3/logging/filerotator.h>
#include <seiscomp3/logging/channel.h>

#include <seiscomp3/core/system.h>

#include <seiscomp3/io/archive/xmlarchive.h>

#include <seiscomp3/datamodel/inventory_package.h>
#include <seiscomp3/datamodel/vs/vs_package.h>

#include <seiscomp3/client/inventory.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/io/records/mseedrecord.h>

#include <seiscomp3/math/filter/butterworth.h>

#include <seiscomp3/utils/files.h>
#include <seiscomp3/utils/timer.h>

#include <boost/bind.hpp>


using namespace std;
using namespace Seiscomp::DataModel;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {


#define NEW_OPT(var, ...) addOption(&var, __VA_ARGS__)
#define NEW_OPT_FREQ(var, ...) addOption(new FreqOption(&var, __VA_ARGS__))
#define NEW_OPT_CLI(var, ...) addOption(&var, NULL, __VA_ARGS__)
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Envelope::Config::Config() {
	saturationThreshold = 80;
	baselineCorrectionBufferLength = 60;
	useSC3Filter = false;
#ifndef SC3_SYNC_VERSION
	maxMessageCountPerSecond = 3000;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Envelope::Envelope(int argc, char **argv) : Application(argc, argv) {
	setAutoApplyNotifierEnabled(true);
	setInterpretNotifierEnabled(true);

	setLoadInventoryEnabled(true);
	setLoadConfigModuleEnabled(false);

	setPrimaryMessagingGroup("VS");
	setAutoAcquisitionStart(true);

	NEW_OPT(_config.streamsWhiteList, "envelope.streams.whitelist");
	NEW_OPT(_config.streamsBlackList, "envelope.streams.blacklist");
	NEW_OPT(_config.saturationThreshold, "envelope.saturationThreshold");
	NEW_OPT(_config.baselineCorrectionBufferLength, "envelope.baselineCorrectionBuffer");
	NEW_OPT(_config.useSC3Filter, "envelope.useSC3Filter");
#ifndef SC3_SYNC_VERSION
	NEW_OPT(_config.maxMessageCountPerSecond, "envelope.mps", "Messaging", "mps",
	        "Maximum number of messages per second. Important for time window based runs. "
	        "val <= 0 -> unlimited", true);
#endif
	NEW_OPT_CLI(_config.strTs, "Offline", "ts",
	            "Start time of data acquisition time window, requires also --te",
	            false, false);
	NEW_OPT_CLI(_config.strTe, "Offline", "te",
	            "End time of data acquisition time window, requires also --ts",
	            false, false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Envelope::~Envelope() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Envelope::createCommandLineDescription() {
	Application::createCommandLineDescription();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Envelope::validateParameters() {
	if ( !Application::validateParameters() ) return false;

	if ( !_config.strTs.empty() || !_config.strTe.empty() ) {
		if ( _config.strTs.empty() ) {
			cerr << "--te requires also --ts" << endl;
			return false;
		}

		if ( _config.strTe.empty() ) {
			cerr << "--ts requires also --te" << endl;
			return false;
		}

		if ( !_config.ts.fromString(_config.strTs.c_str(), "%F %T") ) {
			cerr << "Wrong start time format: expected %F %T, e.g. \"2010-01-01 12:00:00\"" << endl;
			return false;
		}

		if ( !_config.te.fromString(_config.strTe.c_str(), "%F %T") ) {
			cerr << "Wrong end time format: expected %F %T, e.g. \"2010-01-01 12:00:00\"" << endl;
			return false;
		}

		if ( _config.ts >= _config.te ) {
			cerr << "Acquisition time window is empty or of negative length" << endl;
			return true;
		}
	}

	if ( !isInventoryDatabaseEnabled() )
		setDatabaseEnabled(false, false);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Envelope::init() {
	if ( !Application::init() ) return false;

	// Construct stream firewall
	for ( size_t i = 0; i < _config.streamsWhiteList.size(); ++i ) {
		Core::trim(_config.streamsWhiteList[i]);
		if ( !_config.streamsWhiteList[i].empty() ) {
			SEISCOMP_DEBUG("Adding pattern to stream whitelist: %s",
			               _config.streamsWhiteList[i].c_str());
			_streamFirewall.allow.insert(_config.streamsWhiteList[i]);
		}
	}

	for ( size_t i = 0; i < _config.streamsBlackList.size(); ++i ) {
		Core::trim(_config.streamsBlackList[i]);
		if ( !_config.streamsBlackList[i].empty() ) {
			SEISCOMP_DEBUG("Adding pattern to stream blacklist: %s",
			               _config.streamsBlackList[i].c_str());
			_streamFirewall.deny.insert(_config.streamsBlackList[i]);
		}
	}

	Inventory *inv = Client::Inventory::Instance()->inventory();
	if ( inv == NULL ) {
		SEISCOMP_ERROR("Inventory not available");
		return false;
	}

	Core::Time now = Core::Time::GMT();

	for ( size_t n = 0; n < inv->networkCount(); ++n ) {
		Network *net = inv->network(n);
		try { if ( net->end() < now ) continue; }
		catch ( ... ) {}

		for ( size_t s = 0; s < net->stationCount(); ++s ) {
			Station *sta = net->station(s);
			try { if ( sta->end() < now ) continue; }
			catch ( ... ) {}

			// Find velocity and strong-motion streams
			DataModel::WaveformStreamID tmp(net->code(), sta->code(), "", "", "");

			Stream *maxVel, *maxAcc;
			maxVel = Private::findStreamMaxSR(sta, now,
			                                  Processing::WaveformProcessor::MeterPerSecond,
			                                  &_streamFirewall);
			maxAcc = Private::findStreamMaxSR(sta, now,
			                                  Processing::WaveformProcessor::MeterPerSecondSquared,
			                                  &_streamFirewall);

			if ( !maxAcc && !maxVel ) {
				SEISCOMP_WARNING("%s.%s: no usable velocity and acceleration channel found",
				                 net->code().c_str(), sta->code().c_str());
				continue;
			}

			// Add velocity data if available
			if ( maxVel ) {
				tmp.setLocationCode(maxVel->sensorLocation()->code());
				tmp.setChannelCode(maxVel->code().substr(0,2));
				addProcessor(maxVel->sensorLocation(), tmp, now, "velocity", "vel");
			}

			// Add velocity data if available
			if ( maxAcc ) {
				tmp.setLocationCode(maxAcc->sensorLocation()->code());
				tmp.setChannelCode(maxAcc->code().substr(0,2));
				addProcessor(maxAcc->sensorLocation(), tmp, now, "accelerometric", "acc");
			}
		}
	}

	if ( _config.ts.valid() ) recordStream()->setStartTime(_config.ts);
	if ( _config.te.valid() ) recordStream()->setEndTime(_config.te);

	_creationInfo.setAgencyID(agencyID());
	_creationInfo.setAuthor(author());

	// We do not need lookup objects by publicID
	PublicObject::SetRegistrationEnabled(false);

	_sentMessages = 0;
	_sentMessagesTotal = 0;

#ifndef SC3_SYNC_VERSION
	_mpsReset.setCallback(boost::bind(&Envelope::resetMPSCount, this));
	_mpsReset.setTimeout(1);
	_mpsReset.start();
#endif

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Envelope::addProcessor(SensorLocation *loc, const WaveformStreamID &id,
                            const Core::Time &timestamp, const char *type,
                            const char *short_type) {
	DataModel::ThreeComponents tc;
	DataModel::WaveformStreamID tmp(id);
	try {
		DataModel::getThreeComponents(
			tc, loc, id.channelCode().c_str(), timestamp
		);
	}
	catch ( exception &e ) {
		SEISCOMP_ERROR("%s: cannot query three components: %s: "
		               "%s channels not used",
		               Private::toStreamID(tmp).c_str(),
		               e.what(), type);
	}

	for ( int i = 0; i < 3; ++i ) {
		if ( tc.comps[i] == NULL ) continue;
		tmp.setChannelCode(tc.comps[i]->code());
		if ( !_streamFirewall.isAllowed(Private::toStreamID(tmp)) ) continue;

		SEISCOMP_INFO("%s: +%s", Private::toStreamID(tmp).c_str(), short_type);
		recordStream()->addStream(tmp.networkCode(), tmp.stationCode(),
		                          tmp.locationCode(), tmp.channelCode());
		ProcessorPtr proc = new Processor(_config.baselineCorrectionBufferLength);
		switch ( i ) {
			case ThreeComponents::Vertical:
				proc->setUsedComponent(Processing::WaveformProcessor::Vertical);
				proc->setName("Z");
				break;
			case ThreeComponents::FirstHorizontal:
				proc->setUsedComponent(Processing::WaveformProcessor::FirstHorizontal);
				proc->setName("H1");
				break;
			case ThreeComponents::SecondHorizontal:
				proc->setUsedComponent(Processing::WaveformProcessor::SecondHorizontal);
				proc->setName("H2");
				break;
		}

		Processing::StreamPtr stream = new Processing::Stream;
		stream->init(tmp.networkCode(), tmp.stationCode(),
		             tmp.locationCode(), tmp.channelCode(),
		             timestamp);

		proc->streamConfig((Processing::WaveformProcessor::Component)proc->usedComponent()) = *stream;
		proc->setWaveformID(tmp);
		proc->setSaturationThreshold(_config.saturationThreshold);
		proc->useVSFilterImplementation(!_config.useSC3Filter);

		if ( proc->streamConfig((Processing::WaveformProcessor::Component)proc->usedComponent()).gain == 0.0 ) {
			SEISCOMP_WARNING("%s: -%s: gain not defined (= 0.0)",
			                 short_type, Private::toStreamID(tmp).c_str());
			continue;
		}

		proc->setPublishFunction(boost::bind(&Envelope::emitResult, this, _1, _2, _3, _4, _5, _6));
		_processors[Private::toStreamID(tmp)] = proc;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Envelope::emitResult(const Processor *proc,
                          double acc, double vel, double disp,
                          const Core::Time &timestamp, bool clipped) {
	_creationInfo.setCreationTime(Core::Time::GMT());

	// Objects with empty publicID are created since they are currently
	// not sent as notifiers and stored in the database
	VS::EnvelopePtr envelope = new VS::Envelope("");
	envelope->setCreationInfo(_creationInfo);
	envelope->setNetwork(proc->waveformID().networkCode());
	envelope->setStation(proc->waveformID().stationCode());
	envelope->setTimestamp(timestamp);

	VS::EnvelopeChannelPtr cha = new VS::EnvelopeChannel("");
	cha->setName(proc->name());
	cha->setWaveformID(proc->waveformID());

	VS::EnvelopeValuePtr val;

	// Add acceleration value
	val = new VS::EnvelopeValue;
	val->setValue(acc);
	val->setType("acc");
	if ( clipped ) val->setQuality(VS::EnvelopeValueQuality(VS::clipped));
	cha->add(val.get());

	// Add velocity value
	val = new VS::EnvelopeValue;
	val->setValue(vel);
	val->setType("vel");
	if ( clipped ) val->setQuality(VS::EnvelopeValueQuality(VS::clipped));
	cha->add(val.get());

	// Add displacement value
	val = new VS::EnvelopeValue;
	val->setValue(disp);
	val->setType("disp");
	if ( clipped ) val->setQuality(VS::EnvelopeValueQuality(VS::clipped));
	cha->add(val.get());

	envelope->add(cha.get());

	Core::DataMessagePtr msg = new Core::DataMessage;
	msg->attach(envelope.get());

	/* -- DEBUG --
	Core::DataMessage *tmp = &msg;
	IO::XMLArchive ar;
	ar.create("-");
	ar.setFormattedOutput(true);
	ar << tmp;
	ar.close();
	*/

	if ( connection() ) {
		connection()->send(msg.get());
		++_sentMessages;
		++_sentMessagesTotal;

#ifndef SC3_SYNC_VERSION
		if ( _config.maxMessageCountPerSecond > 0 ) {
			bool first = true;
			while ( !isExitRequested() && _sentMessages > _config.maxMessageCountPerSecond ) {
				if ( first )
					SEISCOMP_WARNING("Throttling message output, more than %d allowed messages sent per second",
					                 _config.maxMessageCountPerSecond);
				first = false;
				Core::msleep(100);
			}
		}
#else
		// Request a sync token every 100 messages
		if ( _sentMessages > 100 ) {
			_sentMessages = 0;
			// Tell the record acquisition to request synchronization and to
			// stop sending records until the sync is completed.
			requestSync();
		}
#endif
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Envelope::run() {
	_appStartTime = Core::Time::GMT();
	return Application::run();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Envelope::done() {
	Core::Time now = Core::Time::GMT();
	int secs = (now-_appStartTime).seconds();
	SEISCOMP_INFO("Sent %ld messages with an average of %d messages per second",
	              (long int)_sentMessagesTotal, (int)(secs > 0?_sentMessagesTotal/secs:_sentMessagesTotal));
	Application::done();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Envelope::handleRecord(Record *rec) {
	RecordPtr tmp(rec);
	string sid = rec->streamID();

	Processors::iterator it = _processors.find(sid);
	if ( it != _processors.end() )
		it->second->feed(rec);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#ifndef SC3_SYNC_VERSION
void Envelope::resetMPSCount() {
	_sentMessages = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif
}
