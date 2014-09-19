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


#include "amptool.h"
#include "util.h"

#include <seiscomp3/logging/filerotator.h>
#include <seiscomp3/logging/channel.h>

#include <seiscomp3/client/inventory.h>

#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/datamodel/parameter.h>
#include <seiscomp3/datamodel/parameterset.h>
#include <seiscomp3/datamodel/utils.h>

#include <seiscomp3/io/archive/xmlarchive.h>

#include <seiscomp3/processing/amplitudeprocessor.h>

#include <boost/bind.hpp>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::Client;
using namespace Seiscomp::Processing;
using namespace Seiscomp::DataModel;
using namespace Private;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmpTool::CompareWaveformStreamID::operator()(
	const Seiscomp::DataModel::WaveformStreamID &one,
	const Seiscomp::DataModel::WaveformStreamID &other
) const
{
	if ( one.networkCode() < other.networkCode() ) return true;
	if ( one.networkCode() > other.networkCode() ) return false;

	if ( one.stationCode() < other.stationCode() ) return true;
	if ( one.stationCode() > other.stationCode() ) return false;

	if ( one.locationCode() < other.locationCode() ) return true;
	if ( one.locationCode() > other.locationCode() ) return false;

	return one.channelCode() < other.channelCode();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmpTool::AmpTool(int argc, char **argv) : StreamApplication(argc, argv) {
	_fExpiry = 1.0; // one hour cache initially
	_fetchMissingAmplitudes = true;
	_minWeight = 0.5;

	setAutoApplyNotifierEnabled(true);
	setInterpretNotifierEnabled(true);

	setLoadInventoryEnabled(true);
	setLoadConfigModuleEnabled(true);

	setPrimaryMessagingGroup("AMPLITUDE");

	addMessagingSubscription("PICK");
	addMessagingSubscription("AMPLITUDE");
	addMessagingSubscription("LOCATION");

	setAutoAcquisitionStart(false);

	_amplitudeTypes.insert("MLv");
	_amplitudeTypes.insert("mb");
	_amplitudeTypes.insert("mB");

	_cache.setPopCallback(boost::bind(&AmpTool::removedFromCache, this, _1));

	_errorChannel = NULL;
	_errorOutput = NULL;

	_processingInfoChannel = NULL;
	_processingInfoOutput = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmpTool::~AmpTool() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmpTool::createCommandLineDescription() {
	Application::createCommandLineDescription();

	commandline().addOption("Messaging", "test", "Test mode, no messages are sent");
	commandline().addOption("Generic", "expiry,x", "Time span in hours after which objects expire", &_fExpiry, true);
	commandline().addOption("Generic", "origin-id,O", "OriginID to calculate amplitudes for", &_originID, true);
	commandline().addOption("Generic", "dump-records", "Dumps the filtered traces to ASCII when using -O");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmpTool::validateParameters() {
	if ( !_originID.empty() )
		setMessagingEnabled(false);

	_testMode = commandline().hasOption("test");

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmpTool::initConfiguration() {
	if ( !Client::StreamApplication::initConfiguration() )
		return false;

	try {
		std::vector<std::string> amplitudes = configGetStrings("amplitudes");
		_amplitudeTypes.clear();
		_amplitudeTypes.insert(amplitudes.begin(), amplitudes.end());
	}
	catch (...) {}

	try { _minWeight = configGetDouble("amptool.minimumPickWeight"); }
	catch ( ... ) {}

	_dumpRecords = commandline().hasOption("dump-records");

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmpTool::init() {
	if ( !StreamApplication::init() )
		return false;

	_inputPicks = addInputObjectLog("pick");
	_inputAmps = addInputObjectLog("amplitude");
	_inputOrgs = addInputObjectLog("origin");
	_outputAmps = addOutputObjectLog("amplitude", primaryMessagingGroup());

	_errorChannel = SEISCOMP_DEF_LOGCHANNEL("processing/error", Logging::LL_ERROR);
	_errorOutput = new Logging::FileRotatorOutput(Environment::Instance()->logFile("scamp-processing-error").c_str(),
	                                              60*60*24, 30);

	// Log into processing/info to avoid logging the same information into the global info channel
	_processingInfoChannel = SEISCOMP_DEF_LOGCHANNEL("processing/info", Logging::LL_INFO);
	_processingInfoOutput = new Logging::FileRotatorOutput(Environment::Instance()->logFile("scamp-processing-info").c_str(),
	                                                       60*60*24, 30);

	_errorOutput->subscribe(_errorChannel);
	_processingInfoOutput->subscribe(_processingInfoChannel);

	_cache.setTimeSpan(TimeSpan(_fExpiry*3600.));
	_cache.setDatabaseArchive(query());

	if ( query() )
		// Disable public object lookup in a query. In multithreaded
		// environment this can lead to crashes if both threads are
		// working on the same object since SmartPointers are not
		// thread-safe.
		query()->setPublicObjectCacheLookupEnabled(false);

	AmplitudeTypeList *services = AmplitudeProcessorFactory::Services();

	if ( services ) {
		_registeredAmplitudeTypes = *services;
		delete services;
	}

	std::string logAmplTypes;
	for ( AmplitudeList::iterator it = _amplitudeTypes.begin();
	      it != _amplitudeTypes.end(); ) {
		logAmplTypes += " * ";

		if ( std::find(_registeredAmplitudeTypes.begin(),
		               _registeredAmplitudeTypes.end(), *it) == _registeredAmplitudeTypes.end() ) {
			logAmplTypes += *it;
			logAmplTypes += ": Disabled (unknown type)";
			_amplitudeTypes.erase(it++);
		}
		else {
			logAmplTypes += *it;
			logAmplTypes += ": OK";
			++it;
		}

		logAmplTypes += '\n';
	}

	SEISCOMP_INFO("\nAmplitudes to calculate:\n%s", logAmplTypes.c_str());

	_timer.setTimeout(1);
	_timer.setCallback(boost::bind(&AmpTool::handleTimeout, this));

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmpTool::run() {
	if ( !_originID.empty() ) {
		OriginPtr org = Origin::Cast(query()->getObject(Origin::TypeInfo(), _originID));
		if ( !org ) {
			std::cerr << "Origin not found!" << std::endl;
			return false;
		}

		_fetchMissingAmplitudes = false;
		query()->loadArrivals(org.get());
		process(org.get());
		return true;
	}

	return StreamApplication::run();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmpTool::done() {
	Seiscomp::Client::StreamApplication::done();

	if ( _errorChannel ) delete _errorChannel;
	if ( _errorOutput ) delete _errorOutput;

	if ( _processingInfoChannel ) delete _processingInfoChannel;
	if ( _processingInfoOutput ) delete _processingInfoOutput;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmpTool::addObject(const std::string& parentID, DataModel::Object* object) {
	updateObject(parentID, object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmpTool::updateObject(const std::string &parentID, Object* object) {
	Pick *pick = Pick::Cast(object);
	if ( pick ) {
		logObject(_inputPicks, Time::GMT());
		feed(pick);
		return;
	}

	Amplitude *amp = Amplitude::Cast(object);
	if ( amp && !amp->pickID().empty() ) {
		logObject(_inputAmps, Time::GMT());
		PickPtr pick = _cache.get<Pick>(amp->pickID());
		// No pick, no amplitude
		if ( !pick )
			return;

		AmplitudeMap::iterator it = _pickAmplitudes.find(pick->publicID());
		// If there is no amplitude associated to the pick load all
		// amplitudes from the database
		if ( it == _pickAmplitudes.end() )
			loadAmplitudes(pick->publicID(), amp);
		feed(amp);
		return;
	}

	Origin *origin = Origin::Cast(object);
	if ( origin ) {
		logObject(_inputOrgs, Time::GMT());
		process(origin);
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmpTool::process(Origin *origin) {
	if ( !origin ) return;

	if ( Private::status(origin) == REJECTED ) {
		SEISCOMP_INFO("Ignoring origin %s with status = REJECTED",
		              origin->publicID().c_str());
		return;
	}

	if ( _amplitudeTypes.empty() ) {
		// No amplitudes to calculate
		return;
	}

	// Clear all processors
	_processors.clear();

	// Clear all station time windows
	_stationRequests.clear();

	// Typedef a pickmap entry containing the pick and
	// the distance of the station from the origin
	typedef pair<PickCPtr, double> PickStreamEntry;

	// Typedef a pickmap that maps a streamcode to a pick
	typedef map<string, PickStreamEntry> PickStreamMap;

	// This map is needed to find the earliest P pick of
	// a certain stream
	PickStreamMap pickStreamMap;

	_report << std::endl;
	_report << "Processing report for Origin: " << origin->publicID() << std::endl;
	_report << "-----------------------------------------------------------------" << std::endl;

	_report << " + Arrivals" << endl;

	for ( size_t i = 0; i < origin->arrivalCount(); ++i ) {
		Arrival *arr = origin->arrival(i);
		const string &pickID = arr->pickID();

		double weight = Private::arrivalWeight(arr);

		if ( Private::shortPhaseName(arr->phase().code()) != 'P' || weight < _minWeight ) {
			SEISCOMP_INFO("Ignoring pick '%s' weight=%.1f phase=%s",
			              pickID.c_str(), weight, arr->phase().code().c_str());
			continue;
		}

		PickPtr pick = _cache.get<Pick>(pickID);
		if ( !pick ) {
			SEISCOMP_LOG(_errorChannel, "Pick '%s' not found", pickID.c_str());
			_report << "   - " << pickID << " [pick not found]" << std::endl;
			continue;
		}

		DataModel::WaveformStreamID wfid = pick->waveformID();
		// Strip the component code because every AmplitudeProcessor
		// will use its own component to pick the amplitude on
		wfid.setChannelCode(wfid.channelCode().substr(0,2));

		string streamID = Private::toStreamID(wfid);
		PickStreamEntry &e = pickStreamMap[streamID];

		// When there is already a pick registered for this stream which has
		// been picked earlier, ignore the current pick
		if ( e.first && e.first->time().value() < pick->time().value() )
			continue;

		e.first = pick;
		e.second = Private::arrivalDistance(arr);
	}


	for ( PickStreamMap::iterator it = pickStreamMap.begin(); it != pickStreamMap.end(); ++it ) {
		PickCPtr pick = it->second.first;
		const string &pickID = pick->publicID();
		const Time &pickTime = pick->time().value();
		double distance = it->second.second;
		OPT(double) depth;

		AmplitudeRange amps = getAmplitudes(pickID);

		_report << "   + " << pickID << ", " << Private::toStreamID(pick->waveformID()) << std::endl;
		try {
			depth = origin->depth().value();
			_report << "     + depth = " << origin->depth() << std::endl;
		}
		catch ( ... ) {
			_report << "     - depth [not set]" << std::endl;
		}
		_report << "     + distance = " << distance << std::endl;

		for ( AmplitudeList::iterator ait = _amplitudeTypes.begin();
		      ait != _amplitudeTypes.end(); ++ait ) {

			if ( hasAmplitude(amps, *ait) ) {
				SEISCOMP_INFO("Skipping %s calculation for pick %s, amplitude exists already",
				              ait->c_str(), pickID.c_str());
				_report << "     - " << *ait << " [amplitude exists already]" << std::endl;
				continue;
			}

			AmplitudeProcessorPtr proc = AmplitudeProcessorFactory::Create(ait->c_str());
			if ( !proc ) {
				SEISCOMP_LOG(_errorChannel, "Failed to create AmplitudeProcessor %s", ait->c_str());
				_report << "     - " << *ait << " [amplitudeprocessor NULL]" << std::endl;
				continue;
			}

			proc->setTrigger(pickTime);
			proc->setReferencingPickID(pickID);

			int res = addProcessor(proc.get(), pick.get(), distance, depth);
			if ( res < 0 ) {
				// RecordStream not available
				if ( res == -2 ) return;
				continue;
			}

			proc->setPublishFunction(boost::bind(&AmpTool::emitAmplitude, this, _1, _2));
		}
	}

	if ( _processors.empty() ) {
		_report << " + Nothing to do, no processors to add" << std::endl;
		printReport();
		return;
	}

	for ( RequestMap::iterator it = _stationRequests.begin(); it != _stationRequests.end(); ++it ) {
		StationRequest &req = it->second;
		for ( WaveformIDSet::iterator wit = req.streams.begin(); wit != req.streams.end(); ++wit ) {
			const WaveformStreamID &wsid = *wit;
			recordStream()->addStream(wsid.networkCode(), wsid.stationCode(),
			                          wsid.locationCode(), wsid.channelCode(),
			                          req.timeWindow.startTime(),
			                          req.timeWindow.endTime());
		}

		_report << " + TimeWindow (" << it->first << "): " << req.timeWindow.startTime().toString("%F %T")
		        << ", " << req.timeWindow.endTime().toString("%F %T") << std::endl;
	}

	SEISCOMP_INFO("set stream timeout to 30 seconds");
	_acquisitionTimeout = 30;
	_firstRecord = true;

	_result << " + Processing" << std::endl;

	_acquisitionTimer.restart();
	_noDataTimer.restart();
	_hasRecordsReceived = false;

	SEISCOMP_INFO("Starting timeout monitor");
	_timer.start();
	readRecords(false);
	if ( _timer.isActive() ) _timer.stop();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int AmpTool::addProcessor(Processing::AmplitudeProcessor *proc,
                          const DataModel::Pick *pick,
                          double distance, OPT(double) depth) {
	WaveformProcessor::Component components[3];
	int componentCount = 0;

	// Lookup station parameters of config module
	Parameters *params = NULL;
	std::string stationID = pick->waveformID().networkCode() + "." +
	                        pick->waveformID().stationCode();
	ParameterMap::iterator it = _parameters.find(stationID);
	if ( it != _parameters.end() )
		params = it->second.get();
	else if ( configModule() != NULL ) {
		for ( size_t i = 0; i < configModule()->configStationCount(); ++i ) {
			ConfigStation *station = configModule()->configStation(i);

			if ( station->networkCode() != pick->waveformID().networkCode() ) continue;
			if ( station->stationCode() != pick->waveformID().stationCode() ) continue;

			Setup *setup = findSetup(station, name());
			if ( setup ) {
				ParameterSet *ps = ParameterSet::Find(setup->parameterSetID());

				if ( !ps ) {
					SEISCOMP_ERROR("Cannot find parameter set %s", setup->parameterSetID().c_str());
					continue;
				}

				ParametersPtr parameters = new Processing::Parameters;
				parameters->readFrom(ps);
				_parameters[stationID] = parameters;
				params = parameters.get();
			}
		}
	}

	switch ( proc->usedComponent() ) {
		case AmplitudeProcessor::Vertical:
			components[0] = WaveformProcessor::VerticalComponent;
			componentCount = 1;
			break;
		case AmplitudeProcessor::FirstHorizontal:
			components[0] = WaveformProcessor::FirstHorizontalComponent;
			componentCount = 1;
			break;
		case AmplitudeProcessor::SecondHorizontal:
			components[0] = WaveformProcessor::SecondHorizontalComponent;
			componentCount = 1;
			break;
		case AmplitudeProcessor::Horizontal:
			components[0] = WaveformProcessor::FirstHorizontalComponent;
			components[1] = WaveformProcessor::SecondHorizontalComponent;
			componentCount = 2;
			break;
		case AmplitudeProcessor::Any:
			components[0] = WaveformProcessor::VerticalComponent;
			components[1] = WaveformProcessor::FirstHorizontalComponent;
			components[2] = WaveformProcessor::SecondHorizontalComponent;
			componentCount = 3;
			break;
		default:
			_report << "     - " << proc->type() << " [unsupported component " << proc->usedComponent() << "]"
			        << std::endl;
			return -1;
	}


	std::string streamIDs[3];
	WaveformStreamID cwids[3];

	Client::Inventory *inv = Client::Inventory::Instance();
	DataModel::ThreeComponents tc;
	if ( inv ) {
		try {
			tc = inv->getThreeComponents(pick);
		}
		catch ( ... ) {}
	}

	for ( int i = 0; i < componentCount; ++i ) {
		cwids[i] = pick->waveformID();
		if ( tc.comps[components[i]] == NULL ) {
			SEISCOMP_LOG(_errorChannel, "no inventory information found for %s -> ignoring Arrival %s",
			             streamIDs[i].c_str(), pick->publicID().c_str());
			_report << "   - " << proc->type().c_str() << " [streams not found]" << std::endl;
			return -1;
		}

		cwids[i].setChannelCode(tc.comps[components[i]]->code());
		streamIDs[i] = Private::toStreamID(cwids[i]);

		if ( cwids[i].channelCode().empty() ) {
			SEISCOMP_LOG(_errorChannel, "invalid channel code in %s -> ignoring Arrival %s",
			             streamIDs[i].c_str(), pick->publicID().c_str());
			_report << "   - " << proc->type().c_str() << " [invalid channel code]" << std::endl;
			return -1;
		}

		StreamMap::iterator it = _streams.find(streamIDs[i]);
		if ( it != _streams.end() )
			proc->streamConfig(components[i]) = *it->second;
		else {
			Processing::StreamPtr stream = new Processing::Stream;
			stream->init(cwids[i].networkCode(),
			             cwids[i].stationCode(),
			             cwids[i].locationCode(),
			             cwids[i].channelCode(),
			             pick->time().value());
			_streams[streamIDs[i]] = stream;

			proc->streamConfig(components[i]) = *stream;
		}

		if ( proc->streamConfig(components[i]).gain == 0.0 ) {
			SEISCOMP_LOG(_errorChannel, "no gain found for %s -> ignoring Arrival %s",
			             streamIDs[i].c_str(), pick->publicID().c_str());
			_report << "   - " << proc->type().c_str() << " [gain not found]" << std::endl;
			return -1;
		}
	}


	// If initialization fails, abort
	if ( !proc->setup(
		Settings(
			configModuleName(),
			pick->waveformID().networkCode(),
			pick->waveformID().stationCode(),
			pick->waveformID().locationCode(),
			pick->waveformID().channelCode().substr(0,2),
			&configuration(), params)) ) {
		_report << "   - " << proc->type().c_str() << " [setup failed]" << std::endl;
		return -1;
	}


	if ( depth ) {
		proc->setHint(WaveformProcessor::Depth, *depth);
		if ( proc->isFinished() ) {
			_report << "     - " << proc->type() << " [" << proc->status().toString() << " (" << proc->statusValue() << ")]" << std::endl;
			return -1;
		}
	}

	proc->setHint(WaveformProcessor::Distance, distance);
	if ( proc->isFinished() ) {
		_report << "     - " << proc->type() << " [" << proc->status().toString() << " (" << proc->statusValue() << ")]" << std::endl;
		return -1;
	}

	proc->computeTimeWindow();

	if ( proc->isFinished() ) {
		_report << "     - " << proc->type() << " [" << proc->status().toString() << " (" << proc->statusValue() << ")]" << std::endl;
		// NOTE: Do not create a dummy amplitude that prevents the calculation
		//       for the next origin because initial criterias like Depth and
		//       distance can change significantly for the next origin
		//createDummyAmplitude(proc.get());
		return -1;
	}


	if ( _processors.empty() )
		openStream();

	if ( recordStream() == NULL ) {
		SEISCOMP_ERROR("Opening the acquisition stream failed");
		printReport();
		return -2;
	}

	for ( int i = 0; i < componentCount; ++i ) {
		pair<ProcessorMap::iterator, bool> handle =
			_processors.insert(ProcessorMap::value_type(streamIDs[i], ProcessorSlot()));

		// Update processors station time window
		StationRequest &req = _stationRequests[stationID];
		if ( (bool)req.timeWindow == true )
			req.timeWindow = req.timeWindow | proc->safetyTimeWindow();
		else
			req.timeWindow = proc->safetyTimeWindow();

		// The second value of the pair describes whether a new entry has been inserted or not
		if ( handle.second ) {
			req.streams.insert(cwids[i]);
			//addStream(cwids[i].networkCode(), cwids[i].stationCode(), cwids[i].locationCode(), cwids[i].channelCode());
			_report << "       + stream request = " << streamIDs[i] << std::endl;
		}

		handle.first->second.push_back(proc);
	}

	_report << "     + " << proc->type() << std::endl;

	return 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmpTool::printReport() {
	SEISCOMP_LOG(_processingInfoChannel, "%s%s", _report.str().c_str(),
	             _result.str().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmpTool::feed(Seiscomp::DataModel::Pick *pick) {
	if ( isAgencyIDAllowed(objectAgencyID(pick)) )
		_cache.feed(pick);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmpTool::feed(Seiscomp::DataModel::Amplitude *amp) {
	if ( isAgencyIDAllowed(objectAgencyID(amp)) )
		_pickAmplitudes.insert(AmplitudeMap::value_type(amp->pickID(), amp));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmpTool::removedFromCache(Seiscomp::DataModel::PublicObject *po) {
	size_t amplCount = _pickAmplitudes.erase(po->publicID());
	SEISCOMP_DEBUG("Removed object %s from cache, removed %lu amplitudes",
	               po->publicID().c_str(), (unsigned long)amplCount);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t AmpTool::loadAmplitudes(const std::string &pickID,
                               Seiscomp::DataModel::Amplitude *ignoreAmp) {
	if ( !_fetchMissingAmplitudes ) return 0;
	// Load all amplitudes from the database
	size_t cnt = 0;
	DatabaseIterator it = query()->getAmplitudesForPick(pickID);
	for ( ; *it; ++it ) {
		AmplitudePtr staAmp = Amplitude::Cast(*it);
		if ( staAmp && staAmp != ignoreAmp ) {
			++cnt;
			feed(staAmp.get());
		}
	}

	return cnt;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmpTool::AmplitudeRange AmpTool::getAmplitudes(const std::string &pickID) {
	AmplitudeRange range = _pickAmplitudes.equal_range(pickID);

	// No amplitude stored yet?
	if ( range.first == range.second ) {
		loadAmplitudes(pickID);

		// Reload the amplitudes from local cache
		range = _pickAmplitudes.equal_range(pickID);
	}

	return range;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmpTool::hasAmplitude(const AmplitudeRange &range,
                           const std::string &type) const {
	for ( AmplitudeMap::iterator it = range.first; it != range.second; ++it )
		if ( it->second->type() == type ) return true;

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmpTool::createDummyAmplitude(const AmplitudeProcessor *proc) {
	AmplitudePtr amp = Amplitude::Create();
	amp->setType(proc->type());
	amp->setPickID(proc->referencingPickID());
	// NOTE: Don't call feed here to bypass the agency check
	_pickAmplitudes.insert(AmplitudeMap::value_type(amp->pickID(), amp));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmpTool::emitAmplitude(const AmplitudeProcessor *proc,
                            const AmplitudeProcessor::Result &res) {
	if ( _dumpRecords && !_originID.empty() )
		proc->writeData();

	AmplitudePtr amp = Amplitude::Create();
	CreationInfo ci;
	if ( amp == NULL ) {
		SEISCOMP_LOG(_errorChannel, "Failed to create Amplitude %s for %s", proc->type().c_str(), res.record->streamID().c_str());
		return;
	}

	amp->setAmplitude(
		RealQuantity(
			res.amplitude.value, Core::None,
			res.amplitude.lowerUncertainty, res.amplitude.upperUncertainty,
			Core::None
		)
	);

	if ( res.period > 0 ) amp->setPeriod(RealQuantity(res.period));
	if ( res.snr >= 0 ) amp->setSnr(res.snr);
	amp->setType(proc->type());
	amp->setUnit(proc->unit());
	amp->setTimeWindow(
		DataModel::TimeWindow(res.time.reference, res.time.begin, res.time.end)
	);

	if ( res.component <= WaveformProcessor::SecondHorizontal )
		amp->setWaveformID(
			WaveformStreamID(
				res.record->networkCode(), res.record->stationCode(),
				res.record->locationCode(), proc->streamConfig((WaveformProcessor::Component)res.component).code(), ""
			)
		);
	else
		amp->setWaveformID(
			WaveformStreamID(
				res.record->networkCode(), res.record->stationCode(),
				res.record->locationCode(), res.record->channelCode().substr(0,2), ""
			)
		);

	amp->setPickID(proc->referencingPickID());

	Time now = Time::GMT();
	ci.setAgencyID(agencyID());
	ci.setAuthor(author());
	ci.setCreationTime(now);
	amp->setCreationInfo(ci);

	logObject(_outputAmps, now);

	if ( connection() && !_testMode ) {
		EventParameters ep;
		Notifier::Enable();
		ep.add(amp.get());
		Notifier::Disable();

		NotifierMessagePtr nmsg = Notifier::GetMessage();
		if ( nmsg )
			connection()->send(nmsg.get());
	}
	else if ( !_originID.empty() || _testMode )
		cerr << *amp << endl;

	// Store the amplitude for pickID
	feed(amp.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmpTool::storeRecord(Record *rec) {
	if ( _firstRecord ) {
		SEISCOMP_INFO("Data request: got first record, set timeout to 2 seconds");
		_noDataTimer.restart();
		_acquisitionTimeout = 2;
		_firstRecord = false;
	}

	// This flag is resetted by handleTimeout each second
	_hasRecordsReceived = true;
	handleRecord(rec);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmpTool::handleRecord(Record *rec) {
	Seiscomp::RecordPtr tmp(rec);

	std::string streamID = rec->streamID();

	ProcessorMap::iterator slot_it = _processors.find(streamID);
	if ( slot_it == _processors.end() ) return;

	for ( ProcessorSlot::iterator it = slot_it->second.begin(); it != slot_it->second.end(); ) {
		(*it)->feed(rec);
		if ( (*it)->status() == WaveformProcessor::InProgress ) {
			// processor still needs some time (progress = (*it)->statusValue())
			++it;
		}
		else if ( (*it)->status() == WaveformProcessor::Finished ) {
			_result << "   + " << (*it)->type() << ", " << slot_it->first.c_str() << std::endl;
			if ( (*it)->noiseOffset() )
				_result << "     + noiseOffset = " << *(*it)->noiseOffset() << std::endl;
			else
				_result << "     - noiseOffset" << std::endl;

			if ( (*it)->noiseAmplitude() )
				_result << "     + noiseAmplitude = " << *(*it)->noiseAmplitude() << std::endl;
			else
				_result << "     - noiseAmplitude" << std::endl;
			// processor finished successfully
			it = slot_it->second.erase(it);
		}
		else if ( (*it)->isFinished() ) {
			_result << "   - " << (*it)->type() << ", " << slot_it->first.c_str() << " ("
			        << (*it)->status().toString()
			        << ")" << std::endl;
			createDummyAmplitude(it->get());
			it = slot_it->second.erase(it);
		}
		else
			++it;
	}

	if ( slot_it->second.empty() )
		_processors.erase(slot_it);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmpTool::handleTimeout() {
	//SEISCOMP_INFO("[data acquisition monitor] checking timeouts");
	// Check for data acquisition timeout
	if ( !_hasRecordsReceived ) {
		if ( _noDataTimer.elapsed().seconds() >= _acquisitionTimeout ) {
			boost::mutex::scoped_lock l(_acquisitionMutex);
			if ( recordStream() ) {
				SEISCOMP_INFO("[data acquisition monitor] timeout reached: closing stream");
				recordStream()->close();
			}
		}
	}
	else
		_noDataTimer.restart();

	_hasRecordsReceived = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmpTool::acquisitionFinished() {
	_timer.stop();
	SEISCOMP_INFO("Stopped timeout monitor");

	{
		boost::mutex::scoped_lock l(_acquisitionMutex);
		closeStream();
	}

	SEISCOMP_INFO("Closed stream");

	_report << " + Data request: finished" << std::endl;

	for ( ProcessorMap::iterator slot_it = _processors.begin();
	      slot_it != _processors.end(); ++slot_it ) {
		for ( ProcessorSlot::iterator it = slot_it->second.begin();
		      it != slot_it->second.end(); ++it ) {
			_result << "   - " << (*it)->type() << ", " << slot_it->first.c_str()
			        << " (" << (*it)->status().toString()
			        << ", " << (*it)->statusValue() << "%)" << std::endl;

			_result << "     - TimeWindow: " << (*it)->safetyTimeWindow().startTime().toString("%F %T") << ", "
			        << (*it)->safetyTimeWindow().endTime().toString("%F %T") << std::endl;

			(*it)->close();
		}
	}

	_processors.clear();

	double seconds = (double)_acquisitionTimer.elapsed();
	SEISCOMP_INFO("Acquisition took %.2f seconds", seconds);

	printReport();

	_report.str(std::string());
	_result.str(std::string());
}
