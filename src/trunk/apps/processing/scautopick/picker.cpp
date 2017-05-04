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

#include <seiscomp3/client/inventory.h>

#include <seiscomp3/processing/application.h>
#include <seiscomp3/processing/response.h>
#include <seiscomp3/processing/sensor.h>

#include <seiscomp3/io/archive/xmlarchive.h>

#include <seiscomp3/math/geo.h>
#include <seiscomp3/math/filter.h>

#include <seiscomp3/seismology/magnitudes.h>

#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/sensorlocation.h>
#include <seiscomp3/datamodel/stream.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/datamodel/config_package.h>

#include <iomanip>
#include <boost/bind.hpp>

#include "picker.h"
#include "detector.h"


using namespace std;
using namespace Seiscomp::Client;
using namespace Seiscomp::Math;
using namespace Seiscomp::Processing;


#define MESSAGE_LIMIT 500
#define LOG_PICKS


namespace {

char statusFlag(const Seiscomp::DataModel::Pick* pick) {
	try {
		if ( pick->evaluationMode() == Seiscomp::DataModel::AUTOMATIC )
			return 'A';
		else
			return 'M';
	}
	catch ( ... ) {}
	return 'A';
}


bool contains(const Seiscomp::Core::TimeWindow &tw, const Seiscomp::Core::Time &time) {
	if ( !time.valid() ) return false;

	if ( tw.startTime().valid() && tw.endTime().valid() )
		return tw.contains(time);

	if ( tw.startTime().valid() )
		return time >= tw.startTime();

	if ( tw.endTime().valid() )
		return time < tw.endTime();

	return true;
}


ostream& operator<<(ostream& o, const Seiscomp::Core::Time& time) {
	o << time.toString("%Y/%m/%d %H:%M:%S.") << (time.microseconds() / 1000);
	return o;
}

ostream& operator<<(ostream& o, const Seiscomp::DataModel::Pick* pick) {
	o
	 << setiosflags(ios::left)
	 << "BEGIN " << pick->publicID() << endl
	 << "    " << setw(17) << "net" << "= " << pick->waveformID().networkCode() << endl
	 << "    " << setw(17) << "sta" << "= " << pick->waveformID().stationCode() << endl
	 << "    " << setw(17) << "loc" << "= " << pick->waveformID().locationCode() << endl
	 << "    " << setw(17) << "cha" << "= " << pick->waveformID().channelCode() << endl
	 << "    " << setw(17) << "on"  << "= " << pick->time().value() << endl
	 << "    " << setw(17) << "phase" << "= " << pick->phaseHint().code() << endl;
	try {
		o << "    " << setw(17) << "timestamp"  << "= " << pick->creationInfo().creationTime() << endl;
	}
	catch ( Seiscomp::Core::ValueException& ) {}
	o
	 << "END" << endl;

	return o;
}

ostream& operator<<(ostream& o, const Seiscomp::DataModel::Amplitude* amp) {
	o
	 << setiosflags(ios::left)
	 << "BEGIN " << amp->publicID() << endl
	 << "    " << setw(17) << "net" << "= " << amp->waveformID().networkCode() << endl
	 << "    " << setw(17) << "sta" << "= " << amp->waveformID().stationCode() << endl
	 << "    " << setw(17) << "loc" << "= " << amp->waveformID().locationCode() << endl
	 << "    " << setw(17) << "cha" << "= " << amp->waveformID().channelCode() << endl
	 << "    " << setw(17) << "on"  << "= " << (amp->timeWindow().reference() + Seiscomp::Core::TimeSpan(amp->timeWindow().begin())) << endl
	 << "    " << setw(17) << "off"  << "= " << (amp->timeWindow().reference() + Seiscomp::Core::TimeSpan(amp->timeWindow().end())) << endl
	 << "    " << setw(17) << "period"  << "= ";

	try {
		o << amp->period().value() << endl;
	}
	catch ( ... ) {
		o << "N.D." << endl;
	}

	o << "    " << setw(17) << "ampl_" + amp->type() << "= " << amp->amplitude().value() << endl;
	if ( !amp->pickID().empty() )
		o << "    " << setw(17) << "pick" << "= " << amp->pickID() << endl;
	try {
		o << "    " << setw(17) << "timestamp"  << "= " << amp->creationInfo().creationTime() << endl;
	}
	catch ( Seiscomp::Core::ValueException& ) {}
	o << "END" << endl;

	return o;
}

}


namespace Seiscomp {

namespace Applications {

namespace Picker {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
App::App(int argc, char **argv) : Processing::Application(argc, argv) {
	setLoadInventoryEnabled(true);
	setLoadConfigModuleEnabled(true);

	setPrimaryMessagingGroup("PICK");
	addMessagingSubscription("CONFIG");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
App::~App() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::createCommandLineDescription() {
	StreamApplication::createCommandLineDescription();
	commandline().addOption("Database", "db-disable", "Do not use the database at all");

	commandline().addGroup("Mode");
	commandline().addOption("Mode", "offline", "Do not connect to a messaging server");
	commandline().addOption("Mode", "amplitudes", "Enable/disable computation of amplitudes", &_config.calculateAmplitudes);
	commandline().addOption("Mode", "test", "Do not send any object");
	commandline().addOption("Mode", "playback", "Use playback mode that does not set a request time window and works best with files");
	commandline().addOption("Mode", "ep", "Same as offline but outputs all result as an event parameters XML file");
	commandline().addOption("Mode", "dump-config", "Dump the configuration and exit");
	commandline().addOption("Mode", "dump-records", "Dump records to ASCII when in offline mode");

	commandline().addGroup("Settings");
	commandline().addOption("Settings", "filter", "The filter used for picking", &_config.defaultFilter, false);
	commandline().addOption("Settings", "time-correction", "The time correction in seconds for a pick", &_config.defaultTimeCorrection);
	commandline().addOption("Settings", "buffer-size", "The waveform ringbuffer size in seconds", &_config.ringBufferSize);
	commandline().addOption("Settings", "before", "The timespan in seconds before now to start picking", &_config.leadTime);
	commandline().addOption("Settings", "init-time", "The initialization (inactive) time after the first record arrived per trace", &_config.initTime);

	commandline().addOption("Settings", "trigger-on", "The trigger-on threshold", &_config.defaultTriggerOnThreshold);
	commandline().addOption("Settings", "trigger-off", "The trigger-off threshold", &_config.defaultTriggerOffThreshold);
	commandline().addOption("Settings", "trigger-dead-time", "The dead-time after a pick has been detected", &_config.triggerDeadTime);
	commandline().addOption("Settings", "ampl-max-time-window", "The timewindow length after pick to calculate 'max' amplitude", &_config.amplitudeMaxTimeWindow);
	commandline().addOption("Settings", "min-ampl-offset", "The amplitude offset for amplitude dependend dead time calculation", &_config.amplitudeMinOffset);
	commandline().addOption("Settings", "gap-tolerance", "The maximum gap length to tolerate (reset otherwise)", &_config.maxGapLength);
	commandline().addOption("Settings", "gap-interpolation", "Enables/disables the linear interpolation of gaps", &_config.interpolateGaps);
	commandline().addOption("Settings", "any-stream", "Use all/configured received Z streams for picking", &_config.useAllStreams);
	commandline().addOption("Settings", "send-detections", "If a picker is configured send detections as well");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::validateParameters() {
	_config.init(commandline());
	setMessagingEnabled(!_config.offline);
	bool disableDB = commandline().hasOption("db-disable") ||
	                 (!isInventoryDatabaseEnabled() && !isConfigDatabaseEnabled());

	setDatabaseEnabled(!disableDB, true);

	if ( _config.ringBufferSize < 0 ) {
		cerr << "The buffer-size must not be negative" << endl;
		return false;
	}

	if ( _config.initTime < 0 ) {
		cerr << "The init-time must not be negative" << endl;
		return false;
	}

	if ( _config.triggerDeadTime < 0 ) {
		cerr << "The trigger dead-time must not be negative" << endl;
		return false;
	}

	if ( _config.amplitudeMaxTimeWindow < 0 ) {
		cerr << "The amplitude-max-time-window must not be negative" << endl;
		return false;
	}

	if ( _config.amplitudeMinOffset < 0 ) {
		cerr << "The min-amplitude-offset must not be negative" << endl;
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::initConfiguration() {
	if ( !Processing::Application::initConfiguration() )
		return false;

	_config.init(this);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::init() {
	if ( !StreamApplication::init() ) return false;

	_sentMessages = 0;
	_logPicks = addOutputObjectLog("pick", primaryMessagingGroup());
	_logAmps = addOutputObjectLog("amplitude", _config.amplitudeGroup);

	if ( !_config.pickerType.empty() ) {
		// Check availability of configured picker type
		PickerPtr proc = PickerFactory::Create(_config.pickerType.c_str());
		if ( !proc ) {
			SEISCOMP_ERROR("Picker '%s' is not available, abort application", _config.pickerType.c_str());
			return false;
		}
	}

	_stationConfig.setDefault(
		StreamConfig(_config.defaultTriggerOnThreshold,
		             _config.defaultTriggerOffThreshold,
		             _config.defaultTimeCorrection,
		             _config.defaultFilter)
	);

	streamBuffer().setTimeSpan(_config.ringBufferSize);

	_playbackMode = commandline().hasOption("playback");

	// Only set start time if playback option is not set. Without a time window
	// set, a file source will forward all records to the application otherwise
	// they are filtered according to the time window.
	if ( !_playbackMode )
		recordStream()->setStartTime(Core::Time::GMT() - Core::TimeSpan(_config.leadTime));

	if ( configModule() != NULL ) {
		_config.useAllStreams = false;
		//cerr << "Reading configured streams:" << endl;

		_stationConfig.read(&configuration(), configModule(), name());
	}

	// Components to acquire
	bool acquireComps[3];
	acquireComps[0] = true;
	acquireComps[1] = false;
	acquireComps[2] = false;

	if ( !_config.secondaryPickerType.empty() ) {
		SecondaryPickerPtr proc = SecondaryPickerFactory::Create(_config.secondaryPickerType.c_str());
		if ( proc == NULL ) {
			SEISCOMP_ERROR("Unknown secondary picker: %s", _config.secondaryPickerType.c_str());
			return false;
		}

		if ( proc->usedComponent() == WaveformProcessor::Horizontal ||
		     proc->usedComponent() == WaveformProcessor::Any ||
		     proc->usedComponent() == WaveformProcessor::FirstHorizontal ) {
			acquireComps[1] = true;
		}

		if ( proc->usedComponent() == WaveformProcessor::Horizontal ||
		     proc->usedComponent() == WaveformProcessor::Any ||
		     proc->usedComponent() == WaveformProcessor::SecondHorizontal ) {
			acquireComps[2] = true;
		}
	}

	std::string logAmplTypes;
	for ( StringSet::iterator it = _config.amplitudeList.begin();
	      it != _config.amplitudeList.end(); ) {
		AmplitudeProcessorPtr proc = AmplitudeProcessorFactory::Create(it->c_str());
		logAmplTypes += " * ";
		if ( !proc ) {
			logAmplTypes += *it;
			logAmplTypes += ": Disabled (unknown type)";
			_config.amplitudeList.erase(it++);
		}
		else {
			logAmplTypes += *it;
			logAmplTypes += ": OK";
			if ( _config.amplitudeUpdateList.find(*it) != _config.amplitudeUpdateList.end() )
				logAmplTypes += " (updates enabled)";
			++it;

			if ( proc->usedComponent() == WaveformProcessor::Horizontal ||
			     proc->usedComponent() == WaveformProcessor::Any ||
			     proc->usedComponent() == WaveformProcessor::FirstHorizontal ) {
				acquireComps[1] = true;
			}

			if ( proc->usedComponent() == WaveformProcessor::Horizontal ||
			     proc->usedComponent() == WaveformProcessor::Any ||
			     proc->usedComponent() == WaveformProcessor::SecondHorizontal ) {
				acquireComps[2] = true;
			}
		}

		logAmplTypes += '\n';
	}

	Core::Time now = Core::Time::GMT();
	DataModel::Inventory *inv = Client::Inventory::Instance()->inventory();

	for ( StationConfig::const_iterator it = _stationConfig.begin();
	      it != _stationConfig.end(); ++it ) {

		// Ignore wildcards
		if ( it->first.first == "*" ) continue;
		if ( it->first.second == "*" ) continue;

		// Ignore undefined channels
		if ( it->second.channel.empty() ) continue;

		// Ignore disabled channels
		if ( !it->second.enabled ) {
			SEISCOMP_INFO("Detector on station %s.%s disabled by config",
			              it->first.first.c_str(), it->first.second.c_str());
			continue;
		}

		DataModel::SensorLocation *loc =
			Client::Inventory::Instance()->getSensorLocation(it->first.first, it->first.second, it->second.locCode, now);

		string channel = it->second.channel;
		char compCode = 'Z';
		bool isFixedChannel = channel.size() > 2;

		DataModel::ThreeComponents chan;

		if ( loc )
			DataModel::getThreeComponents(chan, loc, it->second.channel.substr(0,2).c_str(), now);

		DataModel::Stream *compZ = chan.comps[DataModel::ThreeComponents::Vertical];
		DataModel::Stream *compN = chan.comps[DataModel::ThreeComponents::FirstHorizontal];
		DataModel::Stream *compE = chan.comps[DataModel::ThreeComponents::SecondHorizontal];

		if ( !isFixedChannel ) {
			if ( compZ )
				channel = compZ->code();
			else
				channel += compCode;
		}

		std::string streamID = it->first.first + "." + it->first.second + "." + it->second.locCode + "." + channel;

		SEISCOMP_INFO("Adding detection channel %s", streamID.c_str());

		_streamIDs.insert(streamID);

		recordStream()->addStream(it->first.first, it->first.second, it->second.locCode, channel);

		if ( compZ && acquireComps[0] ) {
			streamID = it->first.first + "." + it->first.second + "." + it->second.locCode + "." + compZ->code();
			if ( _streamIDs.find(streamID) == _streamIDs.end() )
				recordStream()->addStream(it->first.first, it->first.second, it->second.locCode, compZ->code());
		}

		if ( compN && acquireComps[1] ) {
			streamID = it->first.first + "." + it->first.second + "." + it->second.locCode + "." + compN->code();
			if ( _streamIDs.find(streamID) == _streamIDs.end() )
				recordStream()->addStream(it->first.first, it->first.second, it->second.locCode, compN->code());
		}

		if ( compE && acquireComps[2] ) {
			streamID = it->first.first + "." + it->first.second + "." + it->second.locCode + "." + compE->code();
			if ( _streamIDs.find(streamID) == _streamIDs.end() )
				recordStream()->addStream(it->first.first, it->first.second, it->second.locCode, compE->code());
		}

		if ( _playbackMode ) {
			// Figure out all historic epochs for locations and channels and
			// subscribe to all channels covering all epochs. This is only
			// required in playback mode if historic data is fed into the picker
			// and where the current time check does not apply.

			for ( size_t n = 0; n < inv->networkCount(); ++n ) {
				DataModel::Network *net = inv->network(n);
				if ( net->code() != it->first.first ) continue;

				for ( size_t s = 0; s < net->stationCount(); ++s ) {
					DataModel::Station *sta = net->station(s);
					if ( sta->code() != it->first.second ) continue;

					for ( size_t l = 0; l < sta->sensorLocationCount(); ++l ) {
						DataModel::SensorLocation *loc = sta->sensorLocation(l);
						if ( loc->code() != it->second.locCode ) continue;

						for ( size_t c = 0; c < loc->streamCount(); ++c ) {
							DataModel::Stream *cha = loc->stream(c);

							if ( it->second.channel.compare(0, 2, cha->code(), 0, 2) == 0 ) {
								streamID = it->first.first + "." + it->first.second + "." + it->second.locCode + "." + cha->code();
								if ( _streamIDs.find(streamID) == _streamIDs.end() )
									recordStream()->addStream(it->first.first, it->first.second, it->second.locCode, cha->code());
							}
						}
					}
				}
			}
		}
	}


	if ( _streamIDs.empty() ) {
		if ( _config.useAllStreams )
			SEISCOMP_INFO("No stations added (empty module configuration?)");
		else {
			SEISCOMP_ERROR("No stations added (empty module configuration?) and thus nothing to do");
			return false;
		}
	}
	else
		SEISCOMP_INFO("%d stations added", (int)_streamIDs.size());

	SEISCOMP_INFO("\nAmplitudes to calculate:\n%s", logAmplTypes.c_str());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::run() {
	if ( commandline().hasOption("dump-config") ) {
		_config.dump();
		printf("\n");
		_stationConfig.dump();
		return true;
	}

	if ( commandline().hasOption("ep") )
		_ep = new DataModel::EventParameters;

	return Processing::Application::run();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::done() {
	if ( _ep ) {
		IO::XMLArchive ar;
		ar.create("-");
		ar.setFormattedOutput(true);
		ar << _ep;
		ar.close();
		_ep = NULL;
	}

	Processing::Application::done();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::addObject(const string& parentID, DataModel::Object* o) {
	Processing::Application::addObject(parentID, o);

	// TODO: handle QC objects
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::removeObject(const string& parentID, DataModel::Object* o) {
	Processing::Application::removeObject(parentID, o);

	// TODO: handle QC objects
	//       call enableStation or enableStream to enable/disable a
	//       certain station/stream
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::updateObject(const string& parentID, DataModel::Object* o) {
	Processing::Application::updateObject(parentID, o);

	// TODO: handle QC objects
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::initComponent(Processing::WaveformProcessor *proc,
                        Processing::WaveformProcessor::Component comp,
                        const Core::Time &time,
                        const std::string &streamID,
                        const std::string &networkCode,
                        const std::string &stationCode,
                        const std::string &locationCode,
                        const std::string &channelCode,
                        bool metaDataRequired) {
	StreamMap::iterator it = _streams.find(streamID);
	if ( it != _streams.end() && contains(it->second->epoch, time) ) {
		proc->streamConfig(comp) = *it->second;
		if ( proc->streamConfig(comp).gain == 0.0 && metaDataRequired ) {
			SEISCOMP_ERROR("No gain for stream %s", streamID.c_str());
			return false;
		}
	}
	else {
		// Load sensor, responses usw.
		Processing::StreamPtr stream = new Processing::Stream;
		_streams[streamID] = stream;

		stream->init(networkCode, stationCode, locationCode, channelCode, time);
		if ( (stream->gain == 0.0) && metaDataRequired ) {
			SEISCOMP_ERROR("No gain for stream %s", streamID.c_str());
			return false;
		}

		proc->streamConfig(comp) = *stream;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::initProcessor(Processing::WaveformProcessor *proc,
                        Processing::WaveformProcessor::StreamComponent comp,
                        const Core::Time &time,
                        const std::string &streamID,
                        const std::string &networkCode,
                        const std::string &stationCode,
                        const std::string &locationCode,
                        const std::string &channelCode,
                        bool metaDataRequired) {
	switch ( comp ) {
		case Processing::WaveformProcessor::Vertical:
			if ( !initComponent(proc,
			                    Processing::WaveformProcessor::VerticalComponent,
			                    time, streamID, networkCode, stationCode,
			                    locationCode, channelCode, metaDataRequired) ) {
				SEISCOMP_ERROR("%s.%s.%s.%s: failed to setup vertical component",
				               networkCode.c_str(), stationCode.c_str(),
				               locationCode.c_str(), channelCode.substr(0,2).c_str());
				return false;
			}
			break;

		case Processing::WaveformProcessor::FirstHorizontal:
			if ( !initComponent(proc,
			                    Processing::WaveformProcessor::FirstHorizontalComponent,
			                    time, streamID, networkCode, stationCode,
			                    locationCode, channelCode, metaDataRequired) ) {
				SEISCOMP_ERROR("%s.%s.%s.%s: failed to setup first horizontal component",
				               networkCode.c_str(), stationCode.c_str(),
				               locationCode.c_str(), channelCode.substr(0,2).c_str());
				return false;
			}
			break;

		case Processing::WaveformProcessor::SecondHorizontal:
			if ( !initComponent(proc,
			                    Processing::WaveformProcessor::SecondHorizontalComponent,
			                    time, streamID, networkCode, stationCode,
			                    locationCode, channelCode, metaDataRequired) ) {
				SEISCOMP_ERROR("%s.%s.%s.%s: failed to setup second horizontal component",
				               networkCode.c_str(), stationCode.c_str(),
				               locationCode.c_str(), channelCode.substr(0,2).c_str());
				return false;
			}
			break;

		case Processing::WaveformProcessor::Horizontal:
		{
			// Find the two horizontal components of given location code
			DataModel::ThreeComponents chans;
			string streamID1, streamID2;
			string channelCode1, channelCode2;

			DataModel::SensorLocation *loc =
				Client::Inventory::Instance()->getSensorLocation(
					networkCode, stationCode, locationCode, time
				);

			if ( loc == NULL ) {
				SEISCOMP_ERROR("%s.%s: location code '%s' not found",
				               networkCode.c_str(), stationCode.c_str(),
				               locationCode.c_str());
				return false;
			}

			// Extract the first two characters of the channel code
			DataModel::getThreeComponents(chans, loc, channelCode.substr(0, 2).c_str(), time);
			if ( chans.comps[DataModel::ThreeComponents::FirstHorizontal] != NULL ) {
				channelCode1 = chans.comps[DataModel::ThreeComponents::FirstHorizontal]->code();
				streamID1 = networkCode + "." + stationCode + "." +
				            locationCode + "." + channelCode1;
			}
			else {
				SEISCOMP_ERROR("%s.%s.%s.%s: 1st horizontal component not available",
				               networkCode.c_str(), stationCode.c_str(),
				               locationCode.c_str(), channelCode.substr(0,2).c_str());
				return false;
			}

			if ( chans.comps[DataModel::ThreeComponents::SecondHorizontal] != NULL ) {
				channelCode2 = chans.comps[DataModel::ThreeComponents::SecondHorizontal]->code();
				streamID2 = networkCode + "." + stationCode + "." +
				            locationCode + "." + channelCode2;
			}
			else {
				SEISCOMP_ERROR("%s.%s.%s.%s: 2nd horizontal component not available",
				               networkCode.c_str(), stationCode.c_str(),
				               locationCode.c_str(), channelCode.substr(0,2).c_str());
				return false;
			}

			if ( !initComponent(proc, Processing::WaveformProcessor::FirstHorizontalComponent,
			                    time, streamID1, networkCode, stationCode,
			                    locationCode, channelCode1, metaDataRequired) ||
			     !initComponent(proc, Processing::WaveformProcessor::SecondHorizontalComponent,
			                    time, streamID2, networkCode, stationCode,
			                    locationCode, channelCode2, metaDataRequired) ) {
				SEISCOMP_ERROR("%s.%s.%s.%s: failed to setup horizontal components",
				               networkCode.c_str(), stationCode.c_str(),
				               locationCode.c_str(), channelCode.substr(0,2).c_str());
				return false;
			}
			break;
		}

		case Processing::WaveformProcessor::Any:
		{
			// Find the all three components of given location code
			DataModel::ThreeComponents chans;
			string streamID0, streamID1, streamID2;
			string channelCode0, channelCode1, channelCode2;

			DataModel::SensorLocation *loc =
				Client::Inventory::Instance()->getSensorLocation(
					networkCode, stationCode, locationCode, time
				);

			if ( loc == NULL ) {
				SEISCOMP_ERROR("%s.%s: location code '%s' not found",
				               networkCode.c_str(), stationCode.c_str(),
				               locationCode.c_str());
				return false;
			}

			// Extract the first two characters of the channel code
			DataModel::getThreeComponents(chans, loc, channelCode.substr(0, 2).c_str(), time);
			if ( chans.comps[DataModel::ThreeComponents::Vertical] != NULL ) {
				channelCode0 = chans.comps[DataModel::ThreeComponents::Vertical]->code();
				streamID0 = networkCode + "." + stationCode + "." +
				            locationCode + "." + channelCode0;
			}
			else if ( metaDataRequired ) {
				SEISCOMP_ERROR("%s.%s.%s.%s: 1st horizontal component not available",
				               networkCode.c_str(), stationCode.c_str(),
				               locationCode.c_str(), channelCode.substr(0,2).c_str());
				return false;
			}

			if ( chans.comps[DataModel::ThreeComponents::FirstHorizontal] != NULL ) {
				channelCode1 = chans.comps[DataModel::ThreeComponents::FirstHorizontal]->code();
				streamID1 = networkCode + "." + stationCode + "." +
				            locationCode + "." + channelCode1;
			}
			else if ( metaDataRequired ) {
				SEISCOMP_ERROR("%s.%s.%s.%s: 1st horizontal component not available",
				               networkCode.c_str(), stationCode.c_str(),
				               locationCode.c_str(), channelCode.substr(0,2).c_str());
				return false;
			}

			if ( chans.comps[DataModel::ThreeComponents::SecondHorizontal] != NULL ) {
				channelCode2 = chans.comps[DataModel::ThreeComponents::SecondHorizontal]->code();
				streamID2 = networkCode + "." + stationCode + "." +
				            locationCode + "." + channelCode2;
			}
			else if ( metaDataRequired ) {
				SEISCOMP_ERROR("%s.%s.%s.%s: 2nd horizontal component not available",
				               networkCode.c_str(), stationCode.c_str(),
				               locationCode.c_str(), channelCode.substr(0,2).c_str());
				return false;
			}

			if ( !initComponent(proc, Processing::WaveformProcessor::VerticalComponent,
			                    time, streamID0, networkCode, stationCode,
			                    locationCode, channelCode0, metaDataRequired) ||
			     !initComponent(proc, Processing::WaveformProcessor::FirstHorizontalComponent,
			                    time, streamID1, networkCode, stationCode,
			                    locationCode, channelCode1, metaDataRequired) ||
			     !initComponent(proc, Processing::WaveformProcessor::SecondHorizontalComponent,
			                    time, streamID2, networkCode, stationCode,
			                    locationCode, channelCode2, metaDataRequired) ) {
				SEISCOMP_ERROR("%s.%s.%s.%s: failed to setup components",
				               networkCode.c_str(), stationCode.c_str(),
				               locationCode.c_str(), channelCode.substr(0,2).c_str());
				return false;
			}
			break;
		}

		default:
			break;
	}

	const StreamConfig *sc = _stationConfig.get(&configuration(), configModuleName(),
	                                            networkCode, stationCode);
	return proc->setup(Settings(configModuleName(), networkCode, stationCode,
	                            locationCode, channelCode, &configuration(),
	                            sc?sc->parameters.get():NULL));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::initDetector(const string &streamID,
                       const string &networkCode,
                       const string &stationCode,
                       const string &locationCode,
                       const string &channelCode,
                       const Core::Time &time) {
	double trigOn = _config.defaultTriggerOnThreshold;
	double trigOff = _config.defaultTriggerOffThreshold;
	double tcorr = _config.defaultTimeCorrection;
	string filter = _config.defaultFilter;
	bool sensitivityCorrection = false;

	const StreamConfig *sc = _stationConfig.get(&configuration(), configModuleName(),
	                                            networkCode, stationCode);
	if ( sc != NULL ) {
		if ( !sc->enabled ) {
			SEISCOMP_INFO("Detector on station %s.%s disabled by config",
			              networkCode.c_str(), stationCode.c_str());
			return true;
		}

		if ( sc->triggerOn ) trigOn = *sc->triggerOn;
		if ( sc->triggerOff ) trigOff = *sc->triggerOff;
		if ( !sc->filter.empty() ) filter = sc->filter;
		if ( sc->timeCorrection ) tcorr = *sc->timeCorrection;
		sensitivityCorrection = sc->sensitivityCorrection;
	}

	DetectorPtr detector = new Detector(trigOn, trigOff, _config.initTime);

	Processing::WaveformProcessor::Filter *detecFilter;
	string filterError;
	detecFilter = Processing::WaveformProcessor::Filter::Create(filter, &filterError);
	if ( !detecFilter ) {
		SEISCOMP_WARNING("%s: compiling filter failed: %s: %s", streamID.c_str(),
		                 filter.c_str(), filterError.c_str());
		return false;
	}

	detector->setDeadTime(_config.triggerDeadTime);
	detector->setAmplitudeTimeWindow(_config.amplitudeMaxTimeWindow);
	//picker->setAmplitudeTimeWindow(0.0);
	detector->setMinAmplitudeOffset(_config.amplitudeMinOffset);
	detector->setDurations(_config.minDuration, _config.maxDuration);
	detector->setFilter(detecFilter);
	detector->setOffset(tcorr);
	detector->setGapTolerance(_config.maxGapLength);
	detector->setGapInterpolationEnabled(_config.interpolateGaps);
	detector->setSensitivityCorrection(sensitivityCorrection);
	detector->setPublishFunction(boost::bind(&App::emitDetection, this, _1, _2, _3));

	if ( _config.calculateAmplitudes )
		detector->setAmplitudePublishFunction(boost::bind(&App::emitAmplitude, this, _1, _2));

	if ( !initProcessor(detector.get(), detector->usedComponent(),
	                    time, streamID, networkCode, stationCode, locationCode, channelCode, sensitivityCorrection) )
		return false;

	SEISCOMP_DEBUG("%s: created detector", streamID.c_str());

	addProcessor(networkCode, stationCode, locationCode, channelCode, detector.get());

	SEISCOMP_DEBUG("Number of processors: %lu", (unsigned long)processorCount());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::handleNewStream(const Record *rec) {
	if ( _config.useAllStreams || _streamIDs.find(rec->streamID()) != _streamIDs.end() )
		if ( !initDetector(rec->streamID(), rec->networkCode(), rec->stationCode(),
		                   rec->locationCode(), rec->channelCode(), rec->startTime()) ) {
			SEISCOMP_ERROR("%s: initialization failed: abort operation",
			               rec->streamID().c_str());
			exit(1);
		}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::processorFinished(const Record *rec, WaveformProcessor *wp) {
	std::stringstream ss;

	if ( wp->status() == Processing::WaveformProcessor::LowSNR )
		ss << "SNR " << wp->statusValue() << " too low";
	else if ( wp->status() > Processing::WaveformProcessor::Terminated )
		ss << "ERROR (" << wp->status().toString() << "," << wp->statusValue() << ")";
	else
		ss << "OK";

	SEISCOMP_DEBUG("%s:%s: %s", rec != NULL?rec->streamID().c_str():"-",
	                           wp->className(),
	                           ss.str().c_str());

	// If its a secondary processor remove it from the tracked item list
	ProcReverseMap::iterator pit = _procLookup.find(wp);
	if ( pit == _procLookup.end() ) return;

	ProcMap::iterator mit = _runningStreamProcs.find(pit->second);

	_procLookup.erase(pit);

	if ( mit == _runningStreamProcs.end() ) return;

	ProcList &list = mit->second;
	for ( ProcList::iterator it = list.begin(); it != list.end(); ) {
		if ( it->proc == wp ) {
			SEISCOMP_DEBUG("Removed finished processor from stream procs");
			it = list.erase(it);
		}
		else
			++it;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::addSecondaryPicker(const Core::Time &onset, const Record *rec, const std::string &pickID) {
	// Add secondary picker
	SecondaryPickerPtr proc = SecondaryPickerFactory::Create(_config.secondaryPickerType.c_str());
	if ( proc == NULL ) {
		SEISCOMP_WARNING("Could not create secondary picker: %s", _config.secondaryPickerType.c_str());
		this->exit(1);
		return;
	}

	SecondaryPicker::Trigger trigger;
	trigger.onset = onset;
	proc->setTrigger(trigger);
	proc->setPublishFunction(boost::bind(&App::emitSPick, this, _1, _2));
	proc->setReferencingPickID(pickID);

	if ( !initProcessor(proc.get(), proc->usedComponent(),
	                    onset, rec->streamID(),
	                    rec->networkCode(), rec->stationCode(), rec->locationCode(), rec->channelCode(), true) )
		return;

	SEISCOMP_DEBUG("%s: created secondary picker %s (rec ref: %d)",
	               rec->streamID().c_str(), _config.secondaryPickerType.c_str(),
	               rec->referenceCount());

	switch ( proc->usedComponent() ) {
		case Processing::WaveformProcessor::Vertical:
			addProcessor(rec->networkCode(), rec->stationCode(),
			             rec->locationCode(), proc->streamConfig(Processing::WaveformProcessor::VerticalComponent).code(),
			             proc.get());
			break;
		case Processing::WaveformProcessor::FirstHorizontal:
			addProcessor(rec->networkCode(), rec->stationCode(),
			             rec->locationCode(), proc->streamConfig(Processing::WaveformProcessor::FirstHorizontalComponent).code(),
			             proc.get());
			break;
		case Processing::WaveformProcessor::SecondHorizontal:
			addProcessor(rec->networkCode(), rec->stationCode(),
			             rec->locationCode(), proc->streamConfig(Processing::WaveformProcessor::SecondHorizontalComponent).code(),
			             proc.get());
			break;
		case Processing::WaveformProcessor::Horizontal:
			addProcessor(rec->networkCode(), rec->stationCode(),
			             rec->locationCode(), proc->streamConfig(Processing::WaveformProcessor::FirstHorizontalComponent).code(),
			             proc.get());
			addProcessor(rec->networkCode(), rec->stationCode(),
			             rec->locationCode(), proc->streamConfig(Processing::WaveformProcessor::SecondHorizontalComponent).code(),
			             proc.get());
			break;
		case Processing::WaveformProcessor::Any:
			addProcessor(rec->networkCode(), rec->stationCode(),
			             rec->locationCode(), proc->streamConfig(Processing::WaveformProcessor::VerticalComponent).code(),
			             proc.get());
			addProcessor(rec->networkCode(), rec->stationCode(),
			             rec->locationCode(), proc->streamConfig(Processing::WaveformProcessor::FirstHorizontalComponent).code(),
			             proc.get());
			addProcessor(rec->networkCode(), rec->stationCode(),
			             rec->locationCode(), proc->streamConfig(Processing::WaveformProcessor::SecondHorizontalComponent).code(),
			             proc.get());
			break;
	}

	ProcList &list = _runningStreamProcs[rec->streamID()];
	SEISCOMP_DEBUG("check for expired procs (got %d in list)", (int)list.size());

	// Check for secondary procs that are still running but where the
	// end time is before onset and remove them
	// ...
	for ( ProcList::iterator it = list.begin(); it != list.end(); ) {
		if ( it->dataEndTime <= onset ) {
			SEISCOMP_DEBUG("Remove expired proc 0x%lx", (long int)it->proc);
			if ( /*it->proc != NULL*/true ) {
				SEISCOMP_INFO("Remove expired running processor %s on %s",
				              it->proc->className(), rec->streamID().c_str());

				if ( it->proc->status() == Processing::WaveformProcessor::LowSNR )
					SEISCOMP_DEBUG("  -> status: SNR(%f) too low", it->proc->statusValue());
				else if ( it->proc->status() > Processing::WaveformProcessor::Terminated )
					SEISCOMP_DEBUG("  -> status: ERROR (%s, %f)",
					               it->proc->status().toString(), it->proc->statusValue());
				else
					SEISCOMP_DEBUG("  -> status: OK");

				// Remove processor from application
				removeProcessor(it->proc);

				// Remove its reverse lookup
				ProcReverseMap::iterator pit = _procLookup.find(it->proc);
				if ( pit != _procLookup.end() ) _procLookup.erase(pit);
			}

			// Remove it from the run list
			it = list.erase(it);
		}
		else
			++it;
	}

	// addProcessor can feed the requested time window with cached records
	// so the proc might be finished already. This needs a test otherwise
	// the registered pointer is invalid later when checking for expired
	// procs.
	if ( !proc->isFinished() ) {
		// Register the secondary procs running on the verticals
		list.push_back(ProcEntry(proc->safetyTimeWindow().endTime(), proc.get()));
		_procLookup[proc.get()] = rec->streamID();
		SEISCOMP_DEBUG("%s: registered proc 0x%lx",
		               rec->streamID().data(), (long int)proc.get());
	}
	else
		SEISCOMP_DEBUG("%s: proc finished already", rec->streamID().data());

	SEISCOMP_DEBUG("Number of processors: %lu", (unsigned long)processorCount());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::addAmplitudeProcessor(AmplitudeProcessorPtr proc,
                                const Record *rec, const string &pickID) {
	proc->setPublishFunction(boost::bind(&App::emitAmplitude, this, _1, _2));
	proc->setReferencingPickID(pickID);

	if ( !initProcessor(proc.get(), proc->usedComponent(),
	                    proc->trigger(), rec->streamID(),
	                    rec->networkCode(), rec->stationCode(), rec->locationCode(), rec->channelCode(), true) )
		return;

	if ( _config.amplitudeUpdateList.find(proc->type()) != _config.amplitudeUpdateList.end() )
		proc->setUpdateEnabled(true);
	else
		proc->setUpdateEnabled(false);

	switch ( proc->usedComponent() ) {
		case Processing::WaveformProcessor::Vertical:
			addProcessor(rec->networkCode(), rec->stationCode(),
			             rec->locationCode(), proc->streamConfig(Processing::WaveformProcessor::VerticalComponent).code(),
			             proc.get());
			break;
		case Processing::WaveformProcessor::FirstHorizontal:
			addProcessor(rec->networkCode(), rec->stationCode(),
			             rec->locationCode(), proc->streamConfig(Processing::WaveformProcessor::FirstHorizontalComponent).code(),
			             proc.get());
			break;
		case Processing::WaveformProcessor::SecondHorizontal:
			addProcessor(rec->networkCode(), rec->stationCode(),
			             rec->locationCode(), proc->streamConfig(Processing::WaveformProcessor::SecondHorizontalComponent).code(),
			             proc.get());
			break;
		case Processing::WaveformProcessor::Horizontal:
			addProcessor(rec->networkCode(), rec->stationCode(),
			             rec->locationCode(), proc->streamConfig(Processing::WaveformProcessor::FirstHorizontalComponent).code(),
			             proc.get());
			addProcessor(rec->networkCode(), rec->stationCode(),
			             rec->locationCode(), proc->streamConfig(Processing::WaveformProcessor::SecondHorizontalComponent).code(),
			             proc.get());
			break;
		case Processing::WaveformProcessor::Any:
			addProcessor(rec->networkCode(), rec->stationCode(),
			             rec->locationCode(), proc->streamConfig(Processing::WaveformProcessor::VerticalComponent).code(),
			             proc.get());
			addProcessor(rec->networkCode(), rec->stationCode(),
			             rec->locationCode(), proc->streamConfig(Processing::WaveformProcessor::FirstHorizontalComponent).code(),
			             proc.get());
			addProcessor(rec->networkCode(), rec->stationCode(),
			             rec->locationCode(), proc->streamConfig(Processing::WaveformProcessor::SecondHorizontalComponent).code(),
			             proc.get());
			break;
	}

	SEISCOMP_DEBUG("Number of processors: %lu", (unsigned long)processorCount());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::emitTrigger(const Processing::Detector *pickProc,
                      const Record *rec, const Core::Time &time) {
	PickerPtr proc = PickerFactory::Create(_config.pickerType.c_str());
	if ( !proc ) {
		SEISCOMP_ERROR("Unable to create '%s' picker, no picking possible", _config.pickerType.c_str());
		return;
	}

	proc->setTrigger(time);
	proc->setMargin(60.);
	proc->setPublishFunction(boost::bind(&App::emitPPick, this, _1, _2));

	if ( !initProcessor(proc.get(), proc->usedComponent(),
	                    time, rec->streamID(),
	                    rec->networkCode(), rec->stationCode(), rec->locationCode(), rec->channelCode(), false) )
		return;

	SEISCOMP_DEBUG("%s: created picker %s",
	               rec->streamID().c_str(), _config.pickerType.c_str());

	addProcessor(rec->networkCode(), rec->stationCode(),
	             rec->locationCode(), rec->channelCode(), proc.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::emitPPick(const Processing::Picker *proc,
                    const Processing::Picker::Result &res)
{
	PickMap::iterator it = _lastPicks.find(res.record->streamID());
	if ( it != _lastPicks.end() ) {
		if ( it->second->time().value() == res.time ) {
			SEISCOMP_WARNING("Duplicate pick on %s at %s: ignoring",
			                 res.record->streamID().c_str(),
			                 res.time.iso().c_str());
			return;
		}
	}

	DataModel::PickPtr pick;
	if ( hasCustomPublicIDPattern() ) {
		pick = DataModel::Pick::Create();

		if ( !pick ) {
			SEISCOMP_WARNING("Duplicate pick ignored");
			return;
		}
	}
	else {
		std::string pickID = res.time.toString("%Y%m%d.%H%M%S.%2f-") + _config.pickerType +
		                     "-" + res.record->streamID();
		pick = DataModel::Pick::Create(pickID);

		if ( !pick ) {
			SEISCOMP_WARNING("Duplicate pick %s ignored", pickID.c_str());
			return;
		}
	}

	Core::Time now = Core::Time::GMT();
	DataModel::CreationInfo ci;
	ci.setCreationTime(now);
	ci.setAgencyID(agencyID());
	ci.setAuthor(author());
	pick->setCreationInfo(ci);

	if ( res.polarity ) {
		switch ( *res.polarity ) {
			case Processing::Picker::POSITIVE:
				pick->setPolarity(DataModel::PickPolarity(DataModel::POSITIVE));
				break;
			case Processing::Picker::NEGATIVE:
				pick->setPolarity(DataModel::PickPolarity(DataModel::NEGATIVE));
				break;
			case Processing::Picker::UNDECIDABLE:
				pick->setPolarity(DataModel::PickPolarity(DataModel::UNDECIDABLE));
				break;
			default:
				break;
		}
	}

	DataModel::TimeQuantity pickTime(res.time);
	if ( res.timeLowerUncertainty >= 0 && res.timeUpperUncertainty >= 0 &&
	     res.timeLowerUncertainty == res.timeUpperUncertainty )
		pickTime.setUncertainty(res.timeUpperUncertainty);
	else {
		if ( res.timeLowerUncertainty >= 0 )
			pickTime.setLowerUncertainty(res.timeLowerUncertainty);
		if ( res.timeUpperUncertainty >= 0 )
			pickTime.setUpperUncertainty(res.timeUpperUncertainty);
	}

	pick->setTime(pickTime);
	pick->setMethodID(proc->methodID());
	pick->setFilterID(proc->filterID());

	// If the detections should be sent as well set the repicked Pick mode
	// to manual to distinguish between detected picks and picked picks.
	if ( _config.sendDetections )
		pick->setEvaluationMode(DataModel::EvaluationMode(DataModel::MANUAL));
	else
		pick->setEvaluationMode(DataModel::EvaluationMode(DataModel::AUTOMATIC));

	pick->setPhaseHint(DataModel::Phase(_config.phaseHint));
	pick->setWaveformID(DataModel::WaveformStreamID(
		res.record->networkCode(),
		res.record->stationCode(),
		res.record->locationCode(),
		res.record->channelCode(),
		"")
	);

	_lastPicks[res.record->streamID()] = pick;

	DataModel::TimeWindow tw;
	tw.setReference(res.time);
	tw.setBegin(res.timeWindowBegin);
	tw.setEnd(res.timeWindowEnd);

	DataModel::AmplitudePtr amp;
	if ( hasCustomPublicIDPattern() )
		amp = DataModel::Amplitude::Create();
	else
		amp = DataModel::Amplitude::Create(pick->publicID() + ".snr");

	if ( amp ) {
		amp->setCreationInfo(ci);

		amp->setPickID(pick->publicID());
		amp->setType("snr");

		amp->setWaveformID(pick->waveformID());
		amp->setTimeWindow(tw);

		amp->setSnr(res.snr);
		amp->setAmplitude(DataModel::RealQuantity(res.snr));
	}

#ifdef LOG_PICKS
	if ( !isMessagingEnabled() && !_ep ) {
		printf("%s %-2s %-6s %-3s %-2s %6.1f %10.3f %4.1f %c %s\n",
		       res.time.toString("%Y-%m-%d %H:%M:%S.%1f").c_str(),
		       res.record->networkCode().c_str(), res.record->stationCode().c_str(),
		       res.record->channelCode().c_str(),
		       res.record->locationCode().empty()?"__":res.record->locationCode().c_str(),
		       res.snr, -1.0, -1.0, 'A', pick->publicID().c_str());
	}
#endif

	SEISCOMP_DEBUG("%s: emit P pick %s", res.record->streamID().c_str(), pick->publicID().c_str());
	logObject(_logPicks, now);
	logObject(_logAmps, now);

	if ( connection() && !_config.test ) {
		// Send pick
		DataModel::NotifierPtr n = new DataModel::Notifier("EventParameters", DataModel::OP_ADD, pick.get());
		DataModel::NotifierMessagePtr m = new DataModel::NotifierMessage;
		m->attach(n.get());
		connection()->send(m.get());
		++_sentMessages;

		// Send amplitude
		if ( amp ) {
			n = new DataModel::Notifier("EventParameters", DataModel::OP_ADD, amp.get());
			m = new DataModel::NotifierMessage;
			m->attach(n.get());
			connection()->send(_config.amplitudeGroup, m.get());
			++_sentMessages;
		}
	}

	if ( _ep ) {
		_ep->add(pick.get());
		if ( amp )
			_ep->add(amp.get());
	}

	if ( !_config.secondaryPickerType.empty() )
		addSecondaryPicker(res.time, res.record, pick->publicID());

	if ( _config.calculateAmplitudes ) {
		for ( StringSet::iterator it = _config.amplitudeList.begin();
		      it != _config.amplitudeList.end(); ++it ) {
			AmplitudeProcessorPtr proc = AmplitudeProcessorFactory::Create(it->c_str());
			if ( !proc ) continue;

			proc->setTrigger(res.time);
			addAmplitudeProcessor(proc.get(), res.record, pick->publicID());
		}
	}

	// Request a sync token every n messages to not flood the message bus
	// and to prevent a disconnect by the master
	if ( _sentMessages > MESSAGE_LIMIT ) {
		_sentMessages = 0;
		// Tell the record acquisition to request synchronization and to
		// stop sending records until the sync is completed.
		SEISCOMP_DEBUG("Synchronize with messaging");
		requestSync();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::emitSPick(const Processing::SecondaryPicker *proc,
                    const Processing::SecondaryPicker::Result &res) {
	DataModel::PickPtr pick;
	if ( hasCustomPublicIDPattern() ) {
		pick = DataModel::Pick::Create();

		if ( !pick ) {
			SEISCOMP_WARNING("Duplicate pick ignored");
			return;
		}
	}
	else {
		std::string pickID = res.time.toString("%Y%m%d.%H%M%S.%2f-") + _config.secondaryPickerType +
		                     "-" + res.record->streamID();
		pick = DataModel::Pick::Create(pickID);

		if ( !pick ) {
			SEISCOMP_WARNING("Duplicate pick %s ignored", pickID.c_str());
			return;
		}
	}

	Core::Time now = Core::Time::GMT();
	DataModel::CreationInfo ci;
	ci.setCreationTime(now);
	ci.setAgencyID(agencyID());
	ci.setAuthor(author());
	pick->setCreationInfo(ci);

	DataModel::TimeQuantity pickTime(res.time);
	if ( res.timeLowerUncertainty >= 0 && res.timeUpperUncertainty >= 0 &&
	     res.timeLowerUncertainty == res.timeUpperUncertainty )
		pickTime.setUncertainty(res.timeUpperUncertainty);
	else {
		if ( res.timeLowerUncertainty >= 0 )
			pickTime.setLowerUncertainty(res.timeLowerUncertainty);
		if ( res.timeUpperUncertainty >= 0 )
			pickTime.setUpperUncertainty(res.timeUpperUncertainty);
	}

	pick->setTime(pickTime);
	pick->setMethodID(proc->methodID());
	pick->setFilterID(proc->filterID());
	pick->setEvaluationMode(DataModel::EvaluationMode(DataModel::AUTOMATIC));
	pick->setPhaseHint(DataModel::Phase(res.phaseCode));
	pick->setWaveformID(DataModel::WaveformStreamID(
		res.record->networkCode(),
		res.record->stationCode(),
		res.record->locationCode(),
		res.record->channelCode(),
		"")
	);

	DataModel::CommentPtr comment;
	if ( !proc->referencingPickID().empty() ) {
		comment = new DataModel::Comment;
		comment->setId("RefPickID");
		comment->setText(proc->referencingPickID());
		pick->add(comment.get());
	}

#ifdef LOG_PICKS
	if ( !isMessagingEnabled() && !_ep ) {
		printf("%s %-2s %-6s %-3s %-2s %6.1f %10.3f %4.1f %c %s\n",
		       res.time.toString("%Y-%m-%d %H:%M:%S.%1f").c_str(),
		       res.record->networkCode().c_str(), res.record->stationCode().c_str(),
		       res.record->channelCode().c_str(),
		       res.record->locationCode().empty()?"__":res.record->locationCode().c_str(),
		       res.snr, -1.0, -1.0, 'A', pick->publicID().c_str());
	}
#endif

	SEISCOMP_DEBUG("%s: emit S pick %s", res.record->streamID().c_str(), pick->publicID().c_str());
	logObject(_logPicks, now);

	if ( connection() && !_config.test ) {
		// Send pick
		DataModel::NotifierPtr n = new DataModel::Notifier("EventParameters", DataModel::OP_ADD, pick.get());
		DataModel::NotifierMessagePtr m = new DataModel::NotifierMessage;
		m->attach(n.get());

		if ( comment ) {
			n = new DataModel::Notifier(pick->publicID(), DataModel::OP_ADD, comment.get());
			m->attach(n.get());
		}

		connection()->send(m.get());
		++_sentMessages;
	}

	if ( _ep )
		_ep->add(pick.get());

	// Request a sync token every n messages to not flood the message bus
	// and to prevent a disconnect by the master
	if ( _sentMessages > MESSAGE_LIMIT ) {
		_sentMessages = 0;
		// Tell the record acquisition to request synchronization and to
		// stop sending records until the sync is completed.
		SEISCOMP_DEBUG("Synchronize with messaging");
		requestSync();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::emitDetection(const Processing::Detector *proc, const Record *rec, const Core::Time& time) {
	if ( !_config.pickerType.empty() ) {
		emitTrigger(proc, rec, time);

		if ( !_config.sendDetections ) return;
	}

	Core::Time now = Core::Time::GMT();
	DataModel::PickPtr pick;
	if ( hasCustomPublicIDPattern() )
		pick = DataModel::Pick::Create();
	else
		pick = DataModel::Pick::Create(time.toString("%Y%m%d.%H%M%S.%2f-") + rec->streamID());

	DataModel::CreationInfo ci;
	ci.setCreationTime(now);
	ci.setAgencyID(agencyID());
	ci.setAuthor(author());
	pick->setCreationInfo(ci);
	pick->setTime(time);
	pick->setMethodID(proc->methodID());

	// Set filterID
	string filter = _config.defaultFilter;

	const StreamConfig *sc = _stationConfig.get(&configuration(), configModuleName(),
	                                            rec->networkCode(), rec->stationCode());
	if ( sc != NULL )
		if ( !sc->filter.empty() ) filter = sc->filter;

	pick->setFilterID(filter);

	pick->setEvaluationMode(DataModel::EvaluationMode(DataModel::AUTOMATIC));
	pick->setPhaseHint(DataModel::Phase(_config.phaseHint));
	pick->setWaveformID(DataModel::WaveformStreamID(
		rec->networkCode(),
		rec->stationCode(),
		rec->locationCode(),
		rec->channelCode(),
		"")
	);

	static_cast<const Detector*>(proc)->setPickID(pick->publicID());

#ifdef LOG_PICKS
	if ( !isMessagingEnabled() && !_ep ) {
		//cout << pick.get();
		printf("%s %-2s %-6s %-3s %-2s %6.1f %10.3f %4.1f %c %s\n",
		       pick->time().value().toString("%Y-%m-%d %H:%M:%S.%1f").c_str(),
		       rec->networkCode().c_str(), rec->stationCode().c_str(),
		       rec->channelCode().c_str(),
		       rec->locationCode().empty()?"__":rec->locationCode().c_str(),
		       -1.0, -1.0, -1.0, statusFlag(pick.get()),
		       pick->publicID().c_str());
	}
#endif

	SEISCOMP_DEBUG("%s: emit detection %s", rec->streamID().c_str(), pick->publicID().c_str());
	logObject(_logPicks, now);

	if ( connection() && !_config.test ) {
		DataModel::NotifierPtr n = new DataModel::Notifier("EventParameters", DataModel::OP_ADD, pick.get());
		DataModel::NotifierMessagePtr m = new DataModel::NotifierMessage;
		m->attach(n.get());
		connection()->send(m.get());
		++_sentMessages;
	}

	if ( _ep )
		_ep->add(pick.get());

	if ( !_config.secondaryPickerType.empty() )
		addSecondaryPicker(time, rec, pick->publicID());

	if ( _config.calculateAmplitudes ) {
		for ( StringSet::iterator it = _config.amplitudeList.begin();
		      it != _config.amplitudeList.end(); ++it ) {
			AmplitudeProcessorPtr proc = AmplitudeProcessorFactory::Create(it->c_str());
			if ( !proc ) continue;

			proc->setTrigger(time);
			addAmplitudeProcessor(proc.get(), rec, pick->publicID());
		}
	}

	// Request a sync token every n messages to not flood the message bus
	// and to prevent a disconnect by the master
	if ( _sentMessages > MESSAGE_LIMIT ) {
		_sentMessages = 0;
		// Tell the record acquisition to request synchronization and to
		// stop sending records until the sync is completed.
		SEISCOMP_DEBUG("Synchronize with messaging");
		requestSync();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::emitAmplitude(const AmplitudeProcessor *ampProc,
                        const AmplitudeProcessor::Result &res) {

	if ( _config.dumpRecords && _config.offline )
		ampProc->writeData();

	bool update = true;
	DataModel::TimeWindow tw;
	tw.setReference(res.time.reference);
	tw.setBegin(res.time.begin);
	tw.setEnd(res.time.end);

	DataModel::AmplitudePtr amp = (DataModel::Amplitude*)ampProc->userData();
	Core::Time now = Core::Time::GMT();

	if ( amp == NULL ) {
		if ( hasCustomPublicIDPattern() )
			amp = DataModel::Amplitude::Create();
		else
			amp = DataModel::Amplitude::Create(ampProc->referencingPickID() + "." + ampProc->type());

		if ( !amp ) {
			SEISCOMP_INFO("Internal error: duplicate amplitudeID?");
			return;
		}

		DataModel::CreationInfo ci;
		ci.setCreationTime(now);
		ci.setAgencyID(agencyID());
		ci.setAuthor(author());
		amp->setCreationInfo(ci);

		amp->setPickID(ampProc->referencingPickID());
		amp->setType(ampProc->type());

		amp->setWaveformID(DataModel::WaveformStreamID(
			res.record->networkCode(),
			res.record->stationCode(),
			res.record->locationCode(),
			res.record->channelCode(),
			"")
		);

		ampProc->setUserData(amp.get());

		update = false;
	}
	else {
		try {
			amp->creationInfo().setModificationTime(now);
		}
		catch ( Core::ValueException &e ) {
			DataModel::CreationInfo ci;
			ci.setModificationTime(now);
			amp->setCreationInfo(ci);
		}
	}

	amp->setUnit(ampProc->unit());
	amp->setTimeWindow(tw);
	if ( res.period > 0 ) amp->setPeriod(DataModel::RealQuantity(res.period));
	if ( res.snr >= 0 ) amp->setSnr(res.snr);
	amp->setAmplitude(
		DataModel::RealQuantity(
			res.amplitude.value, Core::None,
			res.amplitude.lowerUncertainty, res.amplitude.upperUncertainty,
			Core::None
		)
	);

#ifdef LOG_PICKS
	if ( !isMessagingEnabled() && !_ep ) {
		//cout << amp.get();
		if ( amp->type() == "snr" || amp->type() == "mb" ) {
			printf("%s %-2s %-6s %-3s %-2s %6.1f %10.3f %4.1f %c %s\n",
			       ampProc->trigger().toString("%Y-%m-%d %H:%M:%S.%1f").c_str(),
			       res.record->networkCode().c_str(), res.record->stationCode().c_str(),
			       res.record->channelCode().c_str(),
			       res.record->locationCode().empty()?"__":res.record->locationCode().c_str(),
			       amp->type() == "snr"?res.amplitude.value:-1.0, amp->type() == "mb"?res.amplitude.value:-1.0,
			       amp->type() == "mb"?res.period:-1.0, 'A',
			       amp->pickID().c_str());
		}
	}
#endif

	SEISCOMP_DEBUG("Emit amplitude %s, proc = 0x%lx, %s", amp->publicID().c_str(), (long int)ampProc, ampProc->type().c_str());
	logObject(_logAmps, now);

	if ( connection() && !_config.test ) {
		DataModel::NotifierPtr n = new DataModel::Notifier("EventParameters", update?DataModel::OP_UPDATE:DataModel::OP_ADD, amp.get());
		DataModel::NotifierMessagePtr m = new DataModel::NotifierMessage;
		m->attach(n.get());
		if ( !connection()->send(_config.amplitudeGroup, m.get()) && !update ) {
			ampProc->setUserData(NULL);
		}
		else
			++_sentMessages;
	}

	if ( _ep )
		_ep->add(amp.get());

	// Request a sync token every n messages to not flood the message bus
	// and to prevent a disconnect by the master
	if ( _sentMessages > MESSAGE_LIMIT ) {
		_sentMessages = 0;
		// Tell the record acquisition to request synchronization and to
		// stop sending records until the sync is completed.
		SEISCOMP_DEBUG("Synchronize with messaging");
		requestSync();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}

}
