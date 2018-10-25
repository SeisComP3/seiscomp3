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




#define SEISCOMP_COMPONENT Autoloc
#include <seiscomp3/logging/log.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/utils/files.h>
#include <seiscomp3/core/datamessage.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <algorithm>

#include "app.h"
#include "datamodel.h"
#include "sc3adapters.h"
#include "util.h"


using namespace std;
using namespace Seiscomp::Client;
using namespace Seiscomp::Math;


namespace Seiscomp {

namespace Applications {

namespace Autoloc {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
App::App(int argc, char **argv)
: Application(argc, argv), Autoloc3()
, objectCount(0)
, _inputPicks(NULL)
, _inputAmps(NULL)
, _inputOrgs(NULL)
, _outputOrgs(NULL)
{
	setMessagingEnabled(true);

	setPrimaryMessagingGroup("LOCATION");

	addMessagingSubscription("PICK");
	addMessagingSubscription("AMPLITUDE");
	addMessagingSubscription("LOCATION");

	_config = Autoloc3::config();

	_keepEventsTimeSpan = 86400; // one day
	_wakeUpTimout = 5; // wake up every 5 seconds to check pending operations

	_playbackSpeed = 1;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
App::~App() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::createCommandLineDescription() {
	Client::Application::createCommandLineDescription();

	commandline().addGroup("Mode");
	commandline().addOption("Mode", "test", "Do not send any object");
	commandline().addOption("Mode", "offline", "Do not connect to a messaging server. Instead a station-locations.conf file can be provided. This implies --test and --playback");
	commandline().addOption("Mode", "playback", "Flush origins immediately without delay");
	commandline().addOption("Mode", "xml-playback", "TODO"); // TODO
	commandline().addGroup("Input");
	commandline().addOption("Input", "input,i", "XML input file for --xml-playback",&_inputFileXML, false);
	commandline().addOption("Input", "ep", "Event parameters XML file for offline processing of all contained picks and amplitudes" ,&_inputEPFile, false);

	commandline().addGroup("Settings");
	commandline().addOption("Settings", "station-locations", "The station-locations.conf file to use when in offline mode. If no file is given the database is used.", &_stationLocationFile, false);
	commandline().addOption("Settings", "station-config", "The station.conf file", &_config.staConfFile, false);
	commandline().addOption("Settings", "pick-log", "The pick log file", &_config.pickLogFile, false);
	commandline().addOption("Settings", "grid", "The grid.conf file to use", &_gridConfigFile, false);

	commandline().addOption("Settings", "default-depth", "", &_config.defaultDepth);
	commandline().addOption("Settings", "default-depth-stickiness", "", &_config.defaultDepthStickiness);
	commandline().addOption("Settings", "max-sgap", "", &_config.maxAziGapSecondary);
	commandline().addOption("Settings", "max-rms", "", &_config.maxRMS);
	commandline().addOption("Settings", "max-residual", "", &_config.maxResidualUse);
	commandline().addOption("Settings", "max-station-distance", "Maximum distance of stations to be used", &_config.maxStaDist);
	commandline().addOption("Settings", "max-nucleation-distance-default", "Default maximum distance of stations to be used for nucleating new origins", &_config.defaultMaxNucDist);
	commandline().addOption("Settings", "min-pick-affinity", "", &_config.minPickAffinity);

	commandline().addOption("Settings", "min-phase-count", "Minimum number of picks for an origin to be reported", &_config.minPhaseCount);
	commandline().addOption("Settings", "min-score", "Minimum score for an origin to be reported", &_config.minScore);
	commandline().addOption("Settings", "min-pick-snr", "Minimum SNR for a pick to be processed", &_config.minPickSNR);

	commandline().addOption("Settings", "xxl-enable", "", &_config.xxlEnabled);
	commandline().addOption("Settings", "xxl-min-phase-count", "Minimum number of picks for an XXL origin to be reported", &_config.xxlMinPhaseCount);
	commandline().addOption("Settings", "xxl-min-amplitude", "Flag pick as XXL if BOTH snr and amplitude exceed a threshold", &_config.xxlMinAmplitude);
	commandline().addOption("Settings", "xxl-min-snr", "Flag pick as XXL if BOTH snr and amplitude exceed a threshold", &_config.xxlMinSNR);
	commandline().addOption("Settings", "xxl-max-distance", "", &_config.xxlMaxStaDist);
	commandline().addOption("Settings", "xxl-max-depth", "", &_config.xxlMaxDepth);
	commandline().addOption("Settings", "xxl-dead-time", "", &_config.xxlDeadTime);

	commandline().addOption("Settings", "min-sta-count-ignore-pkp", "Minimum station count for which we ignore PKP phases", &_config.minStaCountIgnorePKP);
	commandline().addOption("Settings", "min-score-bypass-nucleator", "Minimum score at which the nucleator is bypassed", &_config.minScoreBypassNucleator);

	commandline().addOption("Settings", "keep-events-timespan", "The timespan to keep historical events", &_keepEventsTimeSpan);

	commandline().addOption("Settings", "cleanup-interval", "The object cleanup interval in seconds", &_config.cleanupInterval);
	commandline().addOption("Settings", "max-age", "During cleanup all objects older than maxAge (in seconds) are removed (maxAge == 0 => disable cleanup)", &_config.maxAge);

	commandline().addOption("Settings", "wakeup-interval", "The interval in seconds to check pending operations", &_wakeUpTimout);
	commandline().addOption("Settings", "speed", "Set this to greater 1 to increase XML playback speed", &_playbackSpeed);
	commandline().addOption("Settings", "dynamic-pick-threshold-interval", "The interval in seconds in which to check for extraordinarily high pick activity, resulting in a dynamically increased pick threshold", &_config.dynamicPickThresholdInterval);

	commandline().addOption("Settings", "use-manual-picks", "allow use of manual picks for nucleation and location");
	commandline().addOption("Settings", "use-manual-origins", "allow use of manual origins from our own agency");
	commandline().addOption("Settings", "use-imported-origins", "allow use of imported origins from trusted agencies as configured in 'processing.whitelist.agencies'. Imported origins are not relocated and only used for phase association");
//	commandline().addOption("Settings", "resend-imported-origins", "Re-send imported origins after phase association");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::validateParameters() {
	if ( !isInventoryDatabaseEnabled() )
		setDatabaseEnabled(false, false);

	if ( commandline().hasOption("offline") ) {
		_config.offline = true;
		_config.playback = true;
		_config.test = true;
	}
	else
		_config.offline = false;

	if ( !_inputEPFile.empty() ) {
		_config.playback = true;
		_config.offline = true;
	}


	if ( _config.offline ) {
		setMessagingEnabled(false);
		if ( !_stationLocationFile.empty() )
			setDatabaseEnabled(false, false);
	}

	// Load inventory from database only if no station location file was specified.
	if ( ! _stationLocationFile.empty()) {
		setLoadStationsEnabled(false);
		setDatabaseEnabled(false, false);
	}
	else {
		setLoadStationsEnabled(true);
		setDatabaseEnabled(true, true);
	}

	// Maybe we do want to allow sending of origins in offline mode?
	if ( commandline().hasOption("test") )
		_config.test = true;

	if ( commandline().hasOption("playback") )
		_config.playback = true;

	if ( commandline().hasOption("use-manual-picks") )
		_config.useManualPicks = true;

	if ( commandline().hasOption("use-manual-origins") )
		_config.useManualOrigins = true;

	if ( commandline().hasOption("use-imported-origins") )
		_config.useImportedOrigins = true;

	if ( commandline().hasOption("try-default-depth") )
		_config.tryDefaultDepth = true;

	if ( commandline().hasOption("adopt-manual-depth") )
		_config.adoptManualDepth = true;

	_config.maxResidualKeep = 3*_config.maxResidualUse;

	setConfig(_config);

	return Client::Application::validateParameters();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::initConfiguration() {
	if ( !Client::Application::initConfiguration() ) return false;

	try { _config.maxAge = configGetDouble("autoloc.maxAge"); } catch (...) {}

	try { _config.defaultDepth = configGetDouble("locator.defaultDepth"); } catch (...) {}
	try { _config.defaultDepthStickiness = configGetDouble("autoloc.defaultDepthStickiness"); } catch (...) {}
	try { _config.tryDefaultDepth = configGetBool("autoloc.tryDefaultDepth"); } catch (...) {}
	try { _config.adoptManualDepth = configGetBool("autoloc.adoptManualDepth"); } catch (...) {}
	try { _config.minimumDepth = configGetDouble("locator.minimumDepth"); } catch (...) {}

	try { _config.maxAziGapSecondary = configGetDouble("autoloc.maxSGAP"); } catch (...) {}
	try { _config.maxRMS = configGetDouble("autoloc.maxRMS"); } catch (...) {}
	try { _config.maxResidualUse = configGetDouble("autoloc.maxResidual"); } catch (...) {}
	try { _config.maxStaDist = configGetDouble("autoloc.maxStationDistance"); } catch (...) {}
	try { _config.defaultMaxNucDist = configGetDouble("autoloc.defaultMaxNucleationDistance"); } catch (...) {}

	try { _config.xxlEnabled = configGetBool("autoloc.xxl.enable"); } catch (...) {}
	try { _config.xxlMinAmplitude = configGetDouble("autoloc.xxl.minAmplitude"); } catch (...) {
		try {
			// deprecated since 2013-06-26
			_config.xxlMinAmplitude = configGetDouble("autoloc.thresholdXXL");
			SEISCOMP_WARNING("Config parameter autoloc.thresholdXXL is deprecated.  Use autoloc.xxl.minAmplitude instead!");
		} catch (...) {}
	}

	try { _config.xxlMaxStaDist = configGetDouble("autoloc.xxl.maxStationDistance"); } catch (...) {
		try {
			// deprecated since 2013-06-26
			_config.xxlMaxStaDist = configGetDouble("autoloc.maxStationDistanceXXL");
			SEISCOMP_WARNING("Config parameter autoloc.maxStationDistanceXXL is deprecated. Use autoloc.xxl.maxStationDistance instead!");
		} catch (...) {}
	}

	try { _config.xxlMinPhaseCount = configGetInt("autoloc.xxl.minPhaseCount"); } catch (...) {
		try {
			// deprecated since 2013-06-26
			_config.xxlMinPhaseCount = configGetInt("autoloc.minPhaseCountXXL");
			SEISCOMP_WARNING("Config parameter autoloc.minPhaseCountXXL is deprecated. Use autoloc.xxl.minPhaseCount instead!");
		} catch (...) {}
	}

	try { _config.xxlMinSNR = configGetDouble("autoloc.xxl.minSNR"); } catch (...) {}
	try { _config.xxlMaxDepth = configGetDouble("autoloc.xxl.maxDepth"); } catch (...) {}
	try { _config.xxlDeadTime = configGetDouble("autoloc.xxl.deadTime"); } catch (...) {}


	try { _config.minPickSNR = configGetDouble("autoloc.minPickSNR"); } catch (...) {}
	try { _config.minPickAffinity = configGetDouble("autoloc.minPickAffinity"); } catch (...) {}

	try { _config.minPhaseCount = configGetInt("autoloc.minPhaseCount"); } catch (...) {}
	try { _config.minScore = configGetDouble("autoloc.minScore"); } catch (...) {}
	try { _config.minScoreBypassNucleator = configGetDouble("autoloc.minScoreBypassNucleator"); } catch (...) {}

	try { _config.minStaCountIgnorePKP = configGetInt("autoloc.minStaCountIgnorePKP"); } catch (...) {}
	try { _config.reportAllPhases = configGetBool("autoloc.reportAllPhases"); } catch (...) {}
	try { _config.useManualOrigins = configGetBool("autoloc.useManualOrigins"); } catch (...) {}
	try { _config.useImportedOrigins = configGetBool("autoloc.useImportedOrigins"); } catch (...) {}

	try { _config.cleanupInterval = configGetDouble("autoloc.cleanupInterval"); } catch (...) {}
	try { _wakeUpTimout = configGetInt("autoloc.wakeupInterval"); } catch (...) {}
	try { _config.maxRadiusFactor = configGetDouble("autoloc.gridsearch._maxRadiusFactor"); } catch (...) {}

	try { _config.publicationIntervalTimeSlope = configGetDouble("autoloc.publicationIntervalTimeSlope"); } catch ( ... ) {}
	try { _config.publicationIntervalTimeIntercept = configGetDouble("autoloc.publicationIntervalTimeIntercept"); } catch ( ... ) {}
	try { _config.publicationIntervalPickCount = configGetInt("autoloc.publicationIntervalPickCount"); } catch ( ... ) {}

	try { _config.dynamicPickThresholdInterval = configGetDouble("autoloc.dynamicPickThresholdInterval"); } catch ( ... ) {}

	try { _keepEventsTimeSpan = configGetInt("keepEventsTimeSpan"); } catch ( ... ) {}

	try { _gridConfigFile = Environment::Instance()->absolutePath(configGetString("autoloc.grid")); }
	catch (...) { _gridConfigFile = Environment::Instance()->shareDir() + "/scautoloc/grid.conf"; }

	try { _config.staConfFile = Environment::Instance()->absolutePath(configGetString("autoloc.stationConfig")); }
	catch (...) { _config.staConfFile = Environment::Instance()->shareDir() + "/scautoloc/station.conf"; }

	try { _config.pickLogFile = configGetString("autoloc.pickLog"); }
	catch (...) { _config.pickLogFile = ""; }

	try { _amplTypeSNR = configGetString("autoloc.amplTypeSNR"); } catch (...) {}
	try { _amplTypeAbs = configGetString("autoloc.amplTypeAbs"); } catch (...) {}
	try { _stationLocationFile = configGetString("autoloc.stationLocations"); } catch (...) {}
	try { _config.locatorProfile = configGetString("autoloc.locator.profile"); } catch (...) {}

	try { _config.playback = configGetBool("autoloc.playback"); } catch ( ... ) {}
	try { _config.offline = configGetBool("autoloc.offline"); } catch ( ... ) {}
	try { _config.test = configGetBool("autoloc.test"); } catch ( ... ) {}

	_config.pickLogFile = Environment::Instance()->absolutePath(_config.pickLogFile);
	_stationLocationFile = Environment::Instance()->absolutePath(_stationLocationFile);

	// network type
	std::string ntp = "global";
	try { ntp = configGetString("autoloc.networkType"); } catch ( ... ) {}
	if      (ntp=="global") {
		_config.networkType = ::Autoloc::GlobalNetwork;
	}
	else if (ntp=="regional") {
		_config.networkType = ::Autoloc::RegionalNetwork;
	}
	else if (ntp=="local") {
		_config.networkType = ::Autoloc::LocalNetwork;
	}
	else    {
		SEISCOMP_ERROR_S("illegal value '"+ntp+"' for autoloc.networkType");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::init() {

	if ( ! Client::Application::init() ) return false;

	_inputPicks = addInputObjectLog("pick");
	_inputAmps = addInputObjectLog("amplitude");
	_inputOrgs = addInputObjectLog("origin");
	_outputOrgs = addOutputObjectLog("origin", primaryMessagingGroup());

	SEISCOMP_INFO("Starting Autoloc");
	dumpConfig();
	if ( ! setGridFile(_gridConfigFile) )
		return false;

	if ( ! initInventory() )
		return false;

	setPickLogFilePrefix(_config.pickLogFile);

	if ( _config.playback ) {
		if ( _inputEPFile.empty() ) {
			// XML playback, set timer to 1 sec
			SEISCOMP_DEBUG("Playback mode - enable timer of 1 sec");
			enableTimer(1);
		}
	}
	else {
		// Read historical preferred origins in case we missed something
		readHistoricEvents();

		if ( _wakeUpTimout > 0 ) {
			SEISCOMP_DEBUG("Enable timer of %d secs", _wakeUpTimout);
			enableTimer(_wakeUpTimout);
		}
	}
	
	return Autoloc3::init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::initInventory() {
	if ( _stationLocationFile.empty() ) {
		SEISCOMP_DEBUG("Initializing station inventory from DB");
		inventory = Inventory::Instance()->inventory();
		if ( ! inventory ) {
			SEISCOMP_ERROR("no inventory!");
			return false;
		}
	}
	else {
		SEISCOMP_DEBUG_S("Initializing station inventory from file '" + _stationLocationFile + "'");
		inventory = ::Autoloc::Utils::inventoryFromStationLocationFile(_stationLocationFile);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::initOneStation(const DataModel::WaveformStreamID &wfid, const Core::Time &time) {

	bool found = false;
	static std::set<std::string> configuredStreams;
	std::string key = wfid.networkCode() + "." + wfid.stationCode();
	if (configuredStreams.find(key) != configuredStreams.end())
		return false;

	for ( size_t n = 0; n < inventory->networkCount(); ++n ) {
		DataModel::Network *network = inventory->network(n);

		if (network->code() != wfid.networkCode())
			continue;

		try {
			if ( time < network->start() )
				continue;
		}
		catch ( ... ) { }

		try {
			if ( time > network->end() )
				continue;
		}
		catch ( ... ) { }

		for ( size_t s = 0; s < network->stationCount(); ++s ) {
			DataModel::Station *station = network->station(s);

			if (station->code() != wfid.stationCode())
				continue;

			std::string epochStart="unset", epochEnd="unset";

			try {
				if (time < station->start())
					continue;
				epochStart = station->start().toString("%FT%TZ");
			}
			catch ( ... ) { } 

			try {
				if (time > station->end())
					continue;
				epochEnd = station->end().toString("%FT%TZ");
			}
			catch ( ... ) { } 

			SEISCOMP_DEBUG_S("Station "+network->code()+" "+station->code()+
					 "  epoch "+epochStart+" ... "+epochEnd); 

			double elev = 0;
			try { elev = station->elevation(); }
			catch ( ... ) {}
			::Autoloc::Station *sta =
				new ::Autoloc::Station(
					station->code(),
					network->code(),
					station->latitude(),
					station->longitude(),
					elev);

			sta->used = true;
			sta->maxNucDist = _config.defaultMaxNucDist;

			setStation(sta);
			found = true;

			break;
		}
		break;
	}

	if ( ! found) {
		SEISCOMP_WARNING_S(key+" not found in station inventory");
		return false;
	}

	configuredStreams.insert(key);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::readHistoricEvents() {
	if ( _keepEventsTimeSpan <= 0 || ! query() ) return;

	SEISCOMP_DEBUG("readHistoricEvents: reading %d seconds of events", _keepEventsTimeSpan);

	typedef std::list<DataModel::OriginPtr> OriginList;
	typedef std::set<std::string> PickIds;

	// Fetch all historic events out of the database. The endtime is
	// probably in the future but because of timing differences between
	// different computers: safety first!
	Core::Time now = Core::Time::GMT();
	DataModel::DatabaseIterator it =
		query()->getPreferredOrigins(now - Core::TimeSpan(_keepEventsTimeSpan),
		                             now + Core::TimeSpan(_keepEventsTimeSpan), "");

	OriginList preferredOrigins;
	PickIds pickIds;

	// Store all preferred origins
	for ( ; it.get() != NULL; ++it ) {
		DataModel::OriginPtr o = DataModel::Origin::Cast(it.get());
		if ( o ) preferredOrigins.push_back(o);
	}
	it.close();

	// Store all pickIDs of all origins and remove duplicates
	for ( OriginList::iterator it = preferredOrigins.begin();
	      it != preferredOrigins.end(); ++it ) {
		DataModel::OriginPtr origin = *it;
		if ( origin->arrivalCount() == 0 ) {
			query()->loadArrivals(it->get());
			for ( size_t i = 0; i < origin->arrivalCount(); ++i )
				pickIds.insert(origin->arrival(i)->pickID());
		}

		SEISCOMP_DEBUG_S("read historical origin "+origin->publicID());

		// Feed it!
		//feedOrigin(it->get());
	}

	// Read all picks out of the database
	for ( PickIds::iterator it = pickIds.begin();
	      it != pickIds.end(); ++it ) {

		DataModel::ObjectPtr obj = query()->getObject(DataModel::Pick::TypeInfo(), *it);
		if ( !obj ) continue;
		DataModel::PickPtr pick = DataModel::Pick::Cast(obj);
		if ( !pick ) continue;

		// Feed it!
		//feedPick(pick.get());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::runFromXMLFile(const char *fname)
{
	DataModel::EventParametersPtr ep = new DataModel::EventParameters();
	IO::XMLArchive ar;

	if ( ! ar.open(fname)) {
		SEISCOMP_ERROR("unable to open XML playback file '%s'", fname);
		return false;
	}

	ar >> ep;
	SEISCOMP_INFO("finished reading event parameters from XML");
	SEISCOMP_INFO("  number of picks:      %ld", (long int)ep->pickCount());
	SEISCOMP_INFO("  number of amplitudes: %ld", (long int)ep->amplitudeCount());
	SEISCOMP_INFO("  number of origins:    %ld", (long int)ep->originCount());

	typedef std::pair<Core::Time,DataModel::PublicObjectPtr> TimeObject;
	typedef std::vector<TimeObject> TimeObjectVector;

	// retrieval of relevant objects from event parameters
	// and subsequent DSU sort
	TimeObjectVector objs;
	while (ep->pickCount() > 0) {
		DataModel::PickPtr pick = ep->pick(0);
		ep->removePick(0);
		DataModel::PublicObjectPtr o(pick);
		Core::Time t = pick->creationInfo().creationTime();
		objs.push_back(TimeObject(t,o));
	}
	while (ep->amplitudeCount() > 0) {
		DataModel::AmplitudePtr amplitude = ep->amplitude(0);
		ep->removeAmplitude(0);
		DataModel::PublicObjectPtr o(amplitude);
		Core::Time t = amplitude->creationInfo().creationTime();
		objs.push_back(TimeObject(t,o));
	}
	while (ep->originCount() > 0) {
		DataModel::OriginPtr origin = ep->origin(0);
		ep->removeOrigin(0);
		DataModel::PublicObjectPtr o(origin);
		Core::Time t = origin->creationInfo().creationTime();
		objs.push_back(TimeObject(t,o));
	}
	std::sort(objs.begin(),objs.end());
	for (TimeObjectVector::iterator
	     it = objs.begin(); it != objs.end(); ++it) {
		_objects.push(it->second);
	}

	if ( _objects.empty() )
		return false;

	if (_playbackSpeed > 0) {
		SEISCOMP_DEBUG("playback speed factor %g", _playbackSpeed);
	}

	objectsStartTime = playbackStartTime = Core::Time::GMT();
	objectCount = 0;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::runFromEPFile(const char *fname) {
	IO::XMLArchive ar;

	if ( !ar.open(fname)) {
		SEISCOMP_ERROR("unable to open XML file: %s", fname);
		return false;
	}

	ar >> _ep;
	ar.close();

	if ( !_ep ) {
		SEISCOMP_ERROR("No event parameters found: %s", fname);
		return false;
	}

	SEISCOMP_INFO("finished reading event parameters from XML");
	SEISCOMP_INFO("  number of picks:      %ld", (long int)_ep->pickCount());
	SEISCOMP_INFO("  number of amplitudes: %ld", (long int)_ep->amplitudeCount());
	SEISCOMP_INFO("  number of origins:    %ld", (long int)_ep->originCount());

	typedef std::pair<Core::Time,DataModel::PublicObjectPtr> TimeObject;
	typedef std::vector<TimeObject> TimeObjectVector;

	// retrieval of relevant objects from event parameters
	// and subsequent DSU sort
	TimeObjectVector objs;

	for ( size_t i = 0; i < _ep->pickCount(); ++i ) {
		DataModel::PickPtr pick = _ep->pick(i);
		try {
			Core::Time t = pick->creationInfo().creationTime();
			objs.push_back(TimeObject(t, pick));
		}
		catch ( ... ) {
			SEISCOMP_WARNING("Ignore pick %s: no creation time set",
			                 pick->publicID().c_str());
		}
	}

	for ( size_t i = 0; i < _ep->amplitudeCount(); ++i ) {
		DataModel::AmplitudePtr amplitude = _ep->amplitude(i);
		try {
			Core::Time t = amplitude->creationInfo().creationTime();
			objs.push_back(TimeObject(t, amplitude));
		}
		catch ( ... ) {
			SEISCOMP_WARNING("Ignore amplitude %s: no creation time set",
			                 amplitude->publicID().c_str());
		}
	}

	for ( size_t i = 0; i < _ep->originCount(); ++i ) {
		DataModel::OriginPtr origin = _ep->origin(i);
		try {
			Core::Time t = origin->creationInfo().creationTime();
			objs.push_back(TimeObject(t, origin));
		}
		catch ( ... ) {
			SEISCOMP_WARNING("Ignore origin %s: no creation time set",
			                 origin->publicID().c_str());
		}
	}

	std::sort(objs.begin(), objs.end());
	for (TimeObjectVector::iterator
	     it = objs.begin(); it != objs.end(); ++it) {
		_objects.push(it->second);
	}

	while ( !_objects.empty() && !isExitRequested() ) {
		DataModel::PublicObjectPtr o = _objects.front();

		_objects.pop();
		addObject("", o.get());
		++objectCount;
	}

	_flush();

	ar.create("-");
	ar.setFormattedOutput(true);
	ar << _ep;
	ar.close();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::run() {
	if ( !_inputEPFile.empty() )
		return runFromEPFile(_inputEPFile.c_str());

	// normal online mode
	if ( ! Autoloc3::config().offline )
		return Application::run();

	// XML playback: first fill object queue, then run()
	if ( _config.playback && _inputFileXML.size() > 0) {
		runFromXMLFile(_inputFileXML.c_str());
		return Application::run();
	}

/*
	// OBSOLETE not that the XML playback is available
	else if ( ! _exitRequested )
		runFromPickFile(); // pick file read from stdin
*/
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::done() {
	_exitRequested = true;
// FIXME	_flush();
	shutdown();
//	setStations(NULL);
	Application::done();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::handleMessage(Core::Message* msg) {
	// Call the original method to make sure that the
	// interpret callbacks (addObject, updateObject -> see below)
	// will be called
	Application::handleMessage(msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::handleTimeout() {

	if ( !_config.playback || _inputFileXML.empty() ) {
		_flush();
		return;
	}


	// The following is relevant (and executed) only for XML playback.

	while ( ! _objects.empty() && !isExitRequested() ) {

		Core::Time t;
		DataModel::PublicObjectPtr o = _objects.front();

		// retrieve the creationTime...
		if (DataModel::Pick::Cast(o.get()))
			t = DataModel::Pick::Cast(o.get())->creationInfo().creationTime();
		else if (DataModel::Amplitude::Cast(o.get()))
			t = DataModel::Amplitude::Cast(o.get())->creationInfo().creationTime();
		else if (DataModel::Origin::Cast(o.get()))
			t = DataModel::Origin::Cast(o.get())->creationInfo().creationTime();
		else continue;

		// at the first object:
		if (objectCount == 0)
			objectsStartTime = t;

		if (_playbackSpeed > 0) {
			double dt = t - objectsStartTime;
			Core::TimeSpan dp = dt/_playbackSpeed;
			t = playbackStartTime + dp;
			if (Core::Time::GMT() < t)
				break; // until next handleTimeout() call
		} // otherwise no speed limit :)

		_objects.pop();
		addObject("", o.get());
		objectCount++;
	}

	// for an XML playback, we're done once the object queue is empty
	if ( _objects.empty() )
		quit();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::handleAutoShutdown() {
//	SEISCOMP_DEBUG("Autoshutdown: flushing pending results");
// XXX FIXME: The following causes the shutdown to hang.
//	_flush();
	Client::Application::handleAutoShutdown();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



static bool manual(const DataModel::Origin *origin) {
	try {
		switch (origin->evaluationMode()) {
		case DataModel::MANUAL:
			return true;
		default:
			break;
		}
	}
	catch ( Core::ValueException & ) {}
	return false;
}

/*
static bool preliminary(const DataModel::Origin *origin) {
	try {
		switch (origin->evaluationStatus()) {
		case DataModel::PRELIMINARY:
			return true;
		default:
			break;
		}
	}
	catch ( Core::ValueException & ) {}
	return false;
}
*/


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::addObject(const std::string& parentID, DataModel::Object* o) {
	DataModel::PublicObject *po = DataModel::PublicObject::Cast(o);
	if ( po == NULL )
		return;
	// SEISCOMP_DEBUG("adding  %-12s %s", po->className(), po->publicID().c_str());

	DataModel::Pick *pick = DataModel::Pick::Cast(o);
	if ( pick ) {
		logObject(_inputPicks, Core::Time::GMT());
		feed(pick);
		return;
	}

	DataModel::Amplitude *amplitude = DataModel::Amplitude::Cast(o);
	if ( amplitude ) {
		logObject(_inputAmps, Core::Time::GMT());
		feed(amplitude);
		return;
	}

	DataModel::Origin *origin = DataModel::Origin::Cast(o);
	if ( origin ) {
		logObject(_inputOrgs, Core::Time::GMT());
		feed(origin);
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::removeObject(const std::string& parentID, DataModel::Object* o) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::updateObject(const std::string& parentID, DataModel::Object* o) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::feed(DataModel::Pick *sc3pick) {

	const std::string &pickID = sc3pick->publicID();

	if (objectAgencyID(sc3pick) != agencyID()) {
		if ( isAgencyIDBlocked(objectAgencyID(sc3pick)) ) {
			SEISCOMP_INFO_S("Blocked pick from agency '" + objectAgencyID(sc3pick) + "'");
			return false;
		}

		SEISCOMP_INFO("pick '%s' from agency '%s'", pickID.c_str(), objectAgencyID(sc3pick).c_str());

	}

	try {
		if (sc3pick->evaluationMode() == DataModel::MANUAL) {
		}
	}
	catch ( ... ) {
		SEISCOMP_WARNING_S("got pick without status " + sc3pick->publicID());
		sc3pick->setEvaluationMode(DataModel::EvaluationMode(DataModel::AUTOMATIC));
	}

	// configure station if needed
	initOneStation(sc3pick->waveformID(), sc3pick->time().value());

	::Autoloc::PickPtr pick = convertFromSC3(sc3pick);
	if ( ! pick )
		return false;

	if ( ! ::Autoloc::Autoloc3::feed(pick.get()))
		return false;

	if ( _config.offline )
		_flush();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::feed(DataModel::Amplitude *sc3ampl) {

	const std::string &amplID = sc3ampl->publicID();

	if (objectAgencyID(sc3ampl) != agencyID()) {
		if ( isAgencyIDBlocked(objectAgencyID(sc3ampl)) ) {
			SEISCOMP_INFO_S("Blocked amplitude from agency '" + objectAgencyID(sc3ampl) + "'");
			return false;
		}
		SEISCOMP_INFO("ampl '%s' from agency '%s'", amplID.c_str(), objectAgencyID(sc3ampl).c_str());
	}

	const std::string &atype  = sc3ampl->type();
	const std::string &pickID = sc3ampl->pickID();

	if ( atype != _amplTypeAbs && atype != _amplTypeSNR )
		return false;

	::Autoloc::Pick *pick = (::Autoloc::Pick *) Autoloc3::pick(pickID);
	if ( ! pick ) {
		SEISCOMP_WARNING_S("Pick " + pickID + " not found for " + atype + " amplitude");
		return false;
	}

	try {
		// note that for testing it is allowed to use the same amplitude as
		// _amplTypeSNR and _amplTypeAbs  -> no 'else if' here
		if ( atype == _amplTypeSNR )
			pick->snr = sc3ampl->amplitude().value();
		if ( atype == _amplTypeAbs ) {
			pick->amp = sc3ampl->amplitude().value();
			pick->per = (_amplTypeAbs == "mb") ? sc3ampl->period().value() : 1;
		}
	}
	catch ( ... ) {
		return false;
	}

	::Autoloc::Autoloc3::feed(pick);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::feed(DataModel::Origin *sc3origin) {

	if ( ! sc3origin ) {
		SEISCOMP_ERROR("This should never happen: origin=NULL");
		return false;
	}

	SEISCOMP_INFO_S("got origin " + sc3origin->publicID() +
			"   agency: " + objectAgencyID(sc3origin));

	const bool ownOrigin = objectAgencyID(sc3origin) == agencyID();

	if ( ownOrigin ) {
		if ( manual(sc3origin) ) {
			if ( ! _config.useImportedOrigins ) {
				SEISCOMP_INFO_S("Ignored origin from " + objectAgencyID(sc3origin) + " because autoloc.useManualOrigins = false");
				return false;
			}
		}
		else {
			// own origin which is not manual -> ignore
			SEISCOMP_INFO_S("Ignored origin from " + objectAgencyID(sc3origin) + " because not a manual origin");
			return false;
		}
	}
	else {
		// imported origin

		if ( ! _config.useImportedOrigins ) {
			SEISCOMP_INFO_S("Ignored origin from " + objectAgencyID(sc3origin) + " because autoloc.useImportedOrigins = false");
			return false;
		}

		if ( isAgencyIDBlocked(objectAgencyID(sc3origin)) ) {
			SEISCOMP_INFO_S("Ignored origin from " + objectAgencyID(sc3origin) + " due to blocked agency ID");
			return false;
		}
	}

	// now we know that the origin is either
	//  * imported from a trusted external source or
	//  * an internal, manual origin

	// TODO: Vorher konsistente Picks/Arrivals sicher stellen.

	::Autoloc::Origin *origin = convertFromSC3(sc3origin);
	if ( ! origin ) {
		SEISCOMP_ERROR_S("Failed to convert origin " + objectAgencyID(sc3origin));
		return false;
	}

	// mark and log imported origin
	if ( objectAgencyID(sc3origin) == agencyID() ) {
		SEISCOMP_INFO_S("Using origin from agency " + objectAgencyID(sc3origin));
		origin->imported = false;
	}
	else {
		SEISCOMP_INFO_S("Using origin from agency " + objectAgencyID(sc3origin));
		origin->imported = true;
	}

	::Autoloc::Autoloc3::feed(origin);

	if ( _config.offline )
		_flush();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::_report(const ::Autoloc::Origin *origin) {
	Core::Time now = Core::Time::GMT();

	// Log object flow
	logObject(_outputOrgs, now);

	if ( _config.offline || _config.test ) {
		std::string reportStr = ::Autoloc::printDetailed(origin);
		SEISCOMP_INFO("Reporting origin %ld\n%s", origin->id, reportStr.c_str());
		SEISCOMP_INFO ("Origin %ld not sent (test/offline mode)", origin->id);

		if ( _ep ) {
			DataModel::OriginPtr sc3origin = ::Autoloc::convertToSC3(origin, _config.reportAllPhases);
			DataModel::CreationInfo ci;
			ci.setAgencyID(agencyID());
			ci.setAuthor(author());
			ci.setCreationTime(now);
			sc3origin->setCreationInfo(ci);

			_ep->add(sc3origin.get());

			std::cerr << reportStr << std::endl;
		}
		else
			std::cout << reportStr << std::endl;

		return true;
	}

	DataModel::OriginPtr sc3origin = ::Autoloc::convertToSC3(origin, _config.reportAllPhases);
	DataModel::CreationInfo ci;
	ci.setAgencyID(agencyID());
	ci.setAuthor(author());
	ci.setCreationTime(now);
	sc3origin->setCreationInfo(ci);

	DataModel::EventParameters ep;
	bool wasEnabled = DataModel::Notifier::IsEnabled();
	DataModel::Notifier::Enable();
	ep.add(sc3origin.get());
	DataModel::Notifier::SetEnabled(wasEnabled);

	DataModel::NotifierMessagePtr nmsg = DataModel::Notifier::GetMessage(true);
	connection()->send(nmsg.get());

	if (origin->preliminary )
		SEISCOMP_INFO("Sent preliminary origin %ld (heads up)", origin->id);
	else
		SEISCOMP_INFO("Sent origin %ld", origin->id);

	SEISCOMP_INFO_S(::Autoloc::printOrigin(origin, false));

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}

}
