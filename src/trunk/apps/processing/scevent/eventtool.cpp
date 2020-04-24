/***************************************************************************
 *   Copyright (C) by GFZ Potsdam and gempa GmbH                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#include "eventtool.h"
#include "util.h"

#include <seiscomp3/logging/filerotator.h>
#include <seiscomp3/logging/channel.h>

#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/focalmechanism.h>
#include <seiscomp3/datamodel/momenttensor.h>
#include <seiscomp3/datamodel/eventdescription.h>
#include <seiscomp3/datamodel/inventory.h>
#include <seiscomp3/datamodel/journalentry.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/utils.h>

#include <seiscomp3/io/archive/xmlarchive.h>

#include <seiscomp3/communication/systemmessages.h>
#include <seiscomp3/core/genericmessage.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/math/geo.h>

#include <boost/bind.hpp>

using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::Client;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Private;

#define DELAY_CHECK_INTERVAL 1
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

void makeUpper(std::string &src) {
	for ( size_t i = 0; i < src.size(); ++i )
		src[i] = toupper(src[i]);
}

const char *PRIORITY_TOKENS[] = {
	"AGENCY", "AUTHOR", "MODE", "STATUS", "METHOD",
	"PHASES", "PHASES_AUTOMATIC",
	"RMS", "RMS_AUTOMATIC",
	"TIME", "TIME_AUTOMATIC", "SCORE"
};


// Global region class defining a rectangular region
// by latmin, lonmin, latmax, lonmax.
DEFINE_SMARTPOINTER(GlobalRegion);
class GlobalRegion : public Client::Config::Region {
	public:
		GlobalRegion() {}

		bool init(const Seiscomp::Config::Config &config, const std::string &prefix) {
			vector<double> region;
			try { region = config.getDoubles(prefix + "rect"); }
			catch ( ... ) {
				return false;
			}

			// Parse region
			if ( region.size() != 4 ) {
				SEISCOMP_ERROR("%srect: expected 4 values in region definition, got %d",
							   prefix.c_str(), (int)region.size());
				return false;
			}

			latMin = region[0];
			lonMin = region[1];
			latMax = region[2];
			lonMax = region[3];

			return true;
		}

		bool isInside(double lat, double lon) const {
			double len, dist;

			if ( lat < latMin || lat > latMax ) return false;

			len = lonMax - lonMin;
			if ( len < 0 )
				len += 360.0;

			dist = lon - lonMin;
			if ( dist < 0 )
				dist += 360.0;

			return dist <= len;
		}

		double latMin, lonMin;
		double latMax, lonMax;
};


DEFINE_SMARTPOINTER(ClearCacheRequestMessage);

/**
 * \brief Message for requesting a clearing of the cache
 * This message type requests a response from a peer. 
 */
class SC_SYSTEM_CLIENT_API ClearCacheRequestMessage : public Seiscomp::Core::Message {
	DECLARE_SC_CLASS(ClearCacheRequestMessage);
	DECLARE_SERIALIZATION;
	
	public:
		//! Constructor
		ClearCacheRequestMessage() {}

		//! Implemented interface from Message
		virtual bool empty() const  { return false; }
};

void ClearCacheRequestMessage::serialize(Archive &) {}

IMPLEMENT_SC_CLASS_DERIVED(
	ClearCacheRequestMessage, Message, "clear_cache_request_message"
);

DEFINE_SMARTPOINTER(ClearCacheResponseMessage);

/**
 * \brief Message to respond to a clear cache request
 */
class SC_SYSTEM_CLIENT_API ClearCacheResponseMessage : public Seiscomp::Core::Message {
	DECLARE_SC_CLASS(ClearCacheResponseMessage);
	DECLARE_SERIALIZATION;
	
	public:
		//! Constructor
		ClearCacheResponseMessage() {}

		//! Implemented interface from Message
		virtual bool empty() const  { return false; }
};

void ClearCacheResponseMessage::serialize(Archive &) {}

IMPLEMENT_SC_CLASS_DERIVED(
	ClearCacheResponseMessage, Message, "clear_cache_response_message"
);


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventTool::EventTool(int argc, char **argv) : Application(argc, argv) {
	_fExpiry = 1.0; // one hour cache initially

	setAutoApplyNotifierEnabled(true);
	setInterpretNotifierEnabled(true);

	setLoadRegionsEnabled(true);

	setPrimaryMessagingGroup("EVENT");

	addMessagingSubscription("LOCATION");
	addMessagingSubscription("MAGNITUDE");
	addMessagingSubscription("FOCMECH");
	addMessagingSubscription("EVENT");

	_cache.setPopCallback(boost::bind(&EventTool::removedFromCache, this, _1));

	_infoChannel = SEISCOMP_DEF_LOGCHANNEL("processing/info", Logging::LL_INFO);
	_infoOutput = new Logging::FileRotatorOutput(Environment::Instance()->logFile("scevent-processing-info").c_str(),
	                                             60*60*24, 30);
	_infoOutput->subscribe(_infoChannel);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventTool::~EventTool() {
	delete _infoChannel;
	delete _infoOutput;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTool::createCommandLineDescription() {
	Application::createCommandLineDescription();

	commandline().addOption("Messaging", "test", "Test mode, no messages are sent");
	commandline().addOption("Messaging", "clear-cache", "Send a clear cache message and quit");
	commandline().addOption("Database", "db-disable", "Do not use the database at all");
	commandline().addOption("Generic", "expiry,x", "Time span in hours after which objects expire", &_fExpiry, true);
	commandline().addOption("Generic", "origin-id,O", "Origin ID to associate (test only, no event updates)", &_originID, true);
	commandline().addOption("Generic", "event-id,E", "Event ID to update preferred objects (test only, no event updates)", &_eventID, true);

	commandline().addGroup("Input");
	commandline().addOption("Input", "ep", "Event parameters XML file for offline processing of all contained origins", &_epFile);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventTool::validateParameters() {
	_testMode = commandline().hasOption("test");

	_sendClearCache = commandline().hasOption("clear-cache");

	if ( commandline().hasOption("db-disable") )
		setDatabaseEnabled(false, false);

	if ( _originID != "" || _eventID != "" )
		setMessagingEnabled(false);

	// For offline processing messaging is disabled and database
	if ( !_epFile.empty() ) {
		setMessagingEnabled(false);
		setDatabaseEnabled(false, false);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventTool::initConfiguration() {
	if ( !Application::initConfiguration() )
		return false;

	try { _config.minStationMagnitudes = configGetInt("eventAssociation.minimumMagnitudes"); } catch (...) {}
	try { _config.minMatchingPicks = configGetInt("eventAssociation.minimumMatchingArrivals"); } catch (...) {}
	try { _config.maxMatchingPicksTimeDiff = configGetDouble("eventAssociation.maximumMatchingArrivalTimeDiff"); } catch (...) {}
	try { _config.matchingPicksTimeDiffAND = configGetBool("eventAssociation.compareAllArrivalTimes"); } catch (...) {}
	try { _config.matchingLooseAssociatedPicks = configGetBool("eventAssociation.allowLooseAssociatedArrivals"); } catch (...) {}
	try { _config.minAutomaticArrivals = configGetInt("eventAssociation.minimumDefiningPhases"); } catch (...) {}
	try { _config.minAutomaticScore = configGetDouble("eventAssociation.minimumScore"); } catch (...) {}

	Config::RegionFilter regionFilter;
	GlobalRegionPtr region = new GlobalRegion;
	if ( region->init(configuration(), "eventAssociation.region.") ) {
		SEISCOMP_INFO("Region check activated");
		regionFilter.region = region;
	}

	try { regionFilter.minDepth = configGetDouble("eventAssociation.region.minDepth"); } catch ( ... ) {}
	try { regionFilter.maxDepth = configGetDouble("eventAssociation.region.maxDepth"); } catch ( ... ) {}

	_config.regionFilter.push_back(regionFilter);

	try { _config.eventTimeBefore = TimeSpan(configGetDouble("eventAssociation.eventTimeBefore")); } catch (...) {}
	try { _config.eventTimeAfter = TimeSpan(configGetDouble("eventAssociation.eventTimeAfter")); } catch (...) {}
	try { _config.maxTimeDiff = TimeSpan(configGetDouble("eventAssociation.maximumTimeSpan")); } catch (...) {}
	try { _config.maxDist = configGetDouble("eventAssociation.maximumDistance"); } catch (...) {}

	try { _config.minMwCount = configGetInt("eventAssociation.minMwCount"); } catch (...) {}

	try { _config.mbOverMwCount = configGetInt("eventAssociation.mbOverMwCount"); } catch (...) {}
	try { _config.mbOverMwValue = configGetDouble("eventAssociation.mbOverMwValue"); } catch (...) {}

	try { _config.eventIDPrefix = configGetString("eventIDPrefix"); } catch (...) {}
	try { _config.eventIDPattern = configGetString("eventIDPattern"); } catch (...) {}

	try { _config.updatePreferredSolutionAfterMerge = configGetBool("eventAssociation.updatePreferredAfterMerge"); } catch (...) {}
	try { _config.enableFallbackPreferredMagnitude = configGetBool("eventAssociation.enableFallbackMagnitude"); } catch (...) {}
	try { _config.magTypes = configGetStrings("eventAssociation.magTypes"); } catch (...) {}
	try { _config.agencies = configGetStrings("eventAssociation.agencies"); } catch (...) {}
	try { _config.authors = configGetStrings("eventAssociation.authors"); } catch (...) {}
	try { _config.methods = configGetStrings("eventAssociation.methods"); } catch (...) {}
	try { _config.score = configGetString("eventAssociation.score"); } catch (...) {}
	try { _config.priorities = configGetStrings("eventAssociation.priorities"); } catch (...) {}

	for ( Config::StringList::iterator it = _config.priorities.begin();
	      it != _config.priorities.end(); ++it ) {
		bool validToken = false;
		makeUpper(*it);
		for ( unsigned int t = 0; t < sizeof(PRIORITY_TOKENS) / sizeof(char*); ++t ) {
			if ( *it == PRIORITY_TOKENS[t] ) {
				validToken = true;
				break;
			}
		}

		if ( !validToken ) {
			SEISCOMP_ERROR("Unexpected token in eventAssociation.priorities: %s", it->c_str());
			return false;
		}

		// SCORE requires a score method to be set up.
		if ( *it == "SCORE" && _config.score.empty() ) {
			SEISCOMP_ERROR("eventAssociation.priorities defines SCORE but no score method is set up.");
			return false;
		}
	}

	try { _config.delayTimeSpan = configGetInt("eventAssociation.delayTimeSpan"); } catch (...) {}
	try { _config.delayFilter.agencyID = configGetString("eventAssociation.delayFilter.agencyID"); } catch (...) {}
	try { _config.delayFilter.author = configGetString("eventAssociation.delayFilter.author"); } catch (...) {}
	try {
		DataModel::EvaluationMode mode;
		string strMode = configGetString("eventAssociation.delayFilter.evaluationMode");
		if ( !mode.fromString(strMode.c_str()) ) {
			SEISCOMP_ERROR("eventAssociation.delayFilter.evaluationMode: invalid mode");
			return false;
		}

		_config.delayFilter.evaluationMode = mode;
	} catch (...) {}

	try { _config.delayPrefFocMech = configGetInt("eventAssociation.delayPrefFocMech"); } catch (...) {}
	try { _config.ignoreMTDerivedOrigins = configGetBool("eventAssociation.ignoreFMDerivedOrigins"); } catch (...) {}
	try { _config.setAutoEventTypeNotExisting = configGetBool("eventAssociation.declareFakeEventForRejectedOrigin"); } catch (...) {}

	try {
		Config::StringList blIDs = configGetStrings("processing.blacklist.eventIDs");
		_config.blacklistIDs.insert(blIDs.begin(), blIDs.end());
	}
	catch ( ... ) {}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventTool::init() {
	_config.eventTimeBefore = TimeSpan(30*60);
	_config.eventTimeAfter = TimeSpan(30*60);

	_config.eventIDPrefix = "gfz";
	_config.eventIDPattern = "%p%Y%04c";
	_config.minAutomaticArrivals = 10;
	_config.minAutomaticScore = Core::None;
	_config.minStationMagnitudes = 4;
	_config.minMatchingPicks = 3;
	_config.maxMatchingPicksTimeDiff = -1;
	_config.matchingLooseAssociatedPicks = false;
	_config.maxTimeDiff = Core::TimeSpan(60.);
	_config.maxDist = 5.0;
	_config.minMwCount = 8;

	_config.mbOverMwCount = 30;
	_config.mbOverMwValue = 6.0;

	_config.enableFallbackPreferredMagnitude = false;

	_config.magTypes.push_back("mBc");
	_config.magTypes.push_back("Mw(mB)");
	_config.magTypes.push_back("Mwp");
	_config.magTypes.push_back("ML");
	_config.magTypes.push_back("MLh");
	_config.magTypes.push_back("MLv");
	_config.magTypes.push_back("mb");

	_config.updatePreferredSolutionAfterMerge = false;
	_config.delayTimeSpan = 0;
	_config.delayPrefFocMech = 0;
	_config.ignoreMTDerivedOrigins = true;
	_config.setAutoEventTypeNotExisting = false;

	if ( !Application::init() ) return false;

	if ( !_config.score.empty() || _config.minAutomaticScore ) {
		if ( _config.score.empty() ) {
			SEISCOMP_ERROR("No score processor configured, eventAssociation.score is empty or not set");
			return false;
		}

		_score = ScoreProcessorFactory::Create(_config.score.c_str());
		if ( !_score ) {
			SEISCOMP_ERROR("Score method '%s' is not available. Is the correct plugin loaded?",
			               _config.score.c_str());
			return false;
		}

		if ( !_score->setup(configuration()) ) {
			SEISCOMP_ERROR("Score '%s' failed to initialize", _config.score.c_str());
			return false;
		}
	}

	_inputOrigin = addInputObjectLog("origin");
	_inputMagnitude = addInputObjectLog("magnitude");
	_inputFocalMechanism = addInputObjectLog("focmech");
	_inputMomentTensor = addInputObjectLog("mt");
	_inputOriginRef = addInputObjectLog("originref");
	_inputFMRef = addInputObjectLog("focmechref");
	_inputEvent = addInputObjectLog("event");
	_inputJournal = addInputObjectLog("journal");
	_outputEvent = addOutputObjectLog("event", primaryMessagingGroup());
	_outputOriginRef = addOutputObjectLog("originref", primaryMessagingGroup());
	_outputFMRef = addOutputObjectLog("focmechref", primaryMessagingGroup());

	if ( _config.delayTimeSpan > 0 || _config.delayPrefFocMech > 0 )
		enableTimer(DELAY_CHECK_INTERVAL);

	_cache.setTimeSpan(TimeSpan(_fExpiry*3600.));
	_cache.setDatabaseArchive(query());

	_ep = new EventParameters;
	_journal = new Journaling;

	EventProcessorFactory::ServiceNames *services;
	services = EventProcessorFactory::Services();

	if ( services ) {
		EventProcessorFactory::ServiceNames::iterator it;

		for ( it = services->begin(); it != services->end(); ++it ) {
			EventProcessorPtr proc = EventProcessorFactory::Create(it->c_str());
			if ( proc ) {
				if ( !proc->setup(configuration()) ) {
					SEISCOMP_WARNING("Event processor '%s' failed to initialize: skipping",
					                 it->c_str());
					continue;
				}

				SEISCOMP_INFO("Processor '%s' added", it->c_str());
				_processors[*it] = proc;
			}
		}

		delete services;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventTool::run() {

	if ( _sendClearCache ) {
		SEISCOMP_DEBUG("Sending clear cache request");
		ClearCacheRequestMessage cc_msg;
		connection()->send(&cc_msg);
		SEISCOMP_DEBUG("Waiting for clear cache response message...");
		while ( ! isExitRequested() ) { // sigkill or ctrl+c
			int error;
			Message *msg = connection()->readMessage(true, Communication::Connection::READ_ALL, NULL, &error);
			if ( ClearCacheResponseMessage::Cast(msg) ) {
				SEISCOMP_DEBUG("Clear cache response message received.");
				return true;
			}
		}
		return false;
	}

	if ( !_epFile.empty() ) {
		IO::XMLArchive ar;
		if ( !ar.open(_epFile.c_str()) ) {
			SEISCOMP_ERROR("Failed to open %s", _epFile.c_str());
			return false;
		}

		_ep = NULL;
		ar >> _ep;
		ar.close();

		if ( !_ep ) {
			SEISCOMP_ERROR("No event parameters found in %s", _epFile.c_str());
			return false;
		}

		for ( size_t i = 0; i < _ep->eventCount(); ++i ) {
			EventPtr evt = _ep->event(i);
			EventInformationPtr info = new EventInformation(&_cache, &_config, query(), evt);

			// Loading the references does not make sense here, because
			// the event has been just added
			cacheEvent(info);
		}

		for ( size_t i = 0; i < _ep->originCount(); ++i ) {
			EventInformationPtr info;

			OriginPtr org = _ep->origin(i);
			info = findAssociatedEvent(org.get());
			if ( info ) {
				SEISCOMP_DEBUG("Origin %s already associated with event %s",
				               org->publicID().c_str(),
				               info->event->publicID().c_str());
				continue;
			}

			SEISCOMP_INFO("Processing origin %s", org->publicID().c_str());

			info = associateOrigin(org.get(), true);
			if ( !info ) continue;

			updatePreferredOrigin(info.get());

		}

		ar.create("-");
		ar.setFormattedOutput(true);
		ar << _ep;
		ar.close();

		cerr << _ep->eventCount() << " events found" << endl;
		return true;
	}

	if ( !_originID.empty() ) {
		if ( !query() ) {
			std::cerr << "No database connection available" << std::endl;
			return false;
		}

		OriginPtr origin = Origin::Cast(query()->getObject(Origin::TypeInfo(), _originID));
		if ( !origin ) {
			std::cout << "Origin " << _originID << " has not been found, exiting" << std::endl;
			return true;
		}

		query()->loadArrivals(origin.get());

		EventInformationPtr info = associateOrigin(origin.get(), true);
		if ( !info ) {
			std::cout << "Origin " << _originID << " has not been associated to any event (already associated?)" << std::endl;
			return true;
		}

		std::cout << "Origin " << _originID << " has been associated to event " << info->event->publicID() << std::endl;
		updatePreferredOrigin(info.get());

		return true;
	}

	if ( !_eventID.empty() ) {
		EventInformationPtr info = new EventInformation(&_cache, &_config, query(), _eventID);
		if ( !info->event ) {
			std::cout << "Event " << _eventID << " not found" << std::endl;
			return false;
		}

		info->loadAssocations(query());
		updatePreferredOrigin(info.get());
		return true;
	}

	return Application::run();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTool::handleMessage(Core::Message *msg) {
	_adds.clear();
	_updates.clear();
	_realUpdates.clear();
	_originBlackList.clear();

	Application::handleMessage(msg);

	ClearCacheRequestMessage *cc_msg = ClearCacheRequestMessage::Cast(msg);
	if ( cc_msg ) {
		SEISCOMP_DEBUG("Received clear cache request");
		_cache.clear();
		SEISCOMP_DEBUG("Sending clear cache response");
		ClearCacheResponseMessage cc_resp;
		connection()->send(&cc_resp);
	}

	SEISCOMP_DEBUG("Work on TODO list");
	for ( TodoList::iterator it = _adds.begin(); it != _adds.end(); ++it ) {
		SEISCOMP_DEBUG("Check ID %s", (*it)->publicID().c_str());
		OriginPtr org = Origin::Cast(it->get());
		if ( org ) {
			if ( _originBlackList.find(org->publicID()) == _originBlackList.end() ) {
				SEISCOMP_DEBUG("* work on new origin %s (%ld, %d/%lu)",
				               org->publicID().c_str(), (long int)org.get(),
				               definingPhaseCount(org.get()), (unsigned long)org->arrivalCount());
				associateOriginCheckDelay(org.get());
			}
			else
				SEISCOMP_DEBUG("* skipped new origin %s: blacklisted", org->publicID().c_str());

			continue;
		}

		FocalMechanismPtr fm = FocalMechanism::Cast(it->get());
		if ( fm ) {
			SEISCOMP_DEBUG("* work on new focalmechanism %s",
			               fm->publicID().c_str());
			associateFocalMechanism(fm.get());

			continue;
		}

		SEISCOMP_DEBUG("* unhandled object of class %s", (*it)->className());
	}

	for ( TodoList::iterator it = _updates.begin(); it != _updates.end(); ++it ) {
		// Has this object already been added in the previous step or delayed?
		bool delayed = false;
		for ( DelayBuffer::reverse_iterator dit = _delayBuffer.rbegin();
		      dit != _delayBuffer.rend(); ++dit ) {
			if ( dit->obj == it->get() ) {
				delayed = true;
				break;
			}
		}

		if ( !delayed && (_adds.find(*it) == _adds.end()) ) {
			OriginPtr org = Origin::Cast(it->get());
			if ( org ) {
				if ( _originBlackList.find(org->publicID()) == _originBlackList.end() )
					updatedOrigin(org.get(), Magnitude::Cast(it->triggered.get()),
					              _realUpdates.find(*it) != _realUpdates.end());
				else
					SEISCOMP_DEBUG("* skipped origin %s: blacklisted", org->publicID().c_str());

				continue;
			}

			FocalMechanismPtr fm = FocalMechanism::Cast(it->get());
			if ( fm ) {
				updatedFocalMechanism(fm.get());
				continue;
			}
		}
	}

	// Clean up event cache
	cleanUpEventCache();

	NotifierMessagePtr nmsg = Notifier::GetMessage(true);
	if ( nmsg ) {
		SEISCOMP_DEBUG("%d notifier available", (int)nmsg->size());
		if ( !_testMode ) connection()->send(nmsg.get());
	}
	else
		SEISCOMP_DEBUG("No notifier available");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTool::handleTimeout() {
	// First pass: decrease delay time and try to associate
	for ( DelayBuffer::iterator it = _delayBuffer.begin();
	      it != _delayBuffer.end(); ) {
		it->timeout -= DELAY_CHECK_INTERVAL;
		if ( it->timeout <= -DELAY_CHECK_INTERVAL ) {
			OriginPtr org = Origin::Cast(it->obj);
			if ( org ) {
				SEISCOMP_LOG(_infoChannel, "Processing delayed origin %s",
				             org->publicID().c_str());
				bool createdEvent;
				associateOrigin(org.get(), true, &createdEvent);
				if ( createdEvent ) {
					// In case an event was created based on a delayed origin
					// then we immediately release this information. All other
					// origins will be associated in a row without intermediate
					// messages.
					NotifierMessagePtr nmsg = Notifier::GetMessage(true);
					if ( nmsg ) {
						SEISCOMP_DEBUG("%d notifier available", (int)nmsg->size());
						if ( !_testMode ) connection()->send(nmsg.get());
					}
				}
			}
			else {
				FocalMechanismPtr fm = FocalMechanism::Cast(it->obj);
				if ( fm ) {
					SEISCOMP_LOG(_infoChannel, "Processing delayed focalmechanism %s",
					             fm->publicID().c_str());
					associateFocalMechanism(fm.get());
				}
			}

			it = _delayBuffer.erase(it);
		}
		else
			++it;
	}

	// Second pass: flush pending origins (if possible)
	for ( DelayBuffer::iterator it = _delayBuffer.begin();
	      it != _delayBuffer.end(); ) {
		OriginPtr org = Origin::Cast(it->obj);
		EventInformationPtr info;
		if ( org ) {
			SEISCOMP_LOG(_infoChannel, "Processing delayed origin %s (no event "
			             "creation before %i s)", org->publicID().c_str(),
			             it->timeout + DELAY_CHECK_INTERVAL);
			info = associateOrigin(org.get(), false);
		}
		else {
			FocalMechanismPtr fm = FocalMechanism::Cast(it->obj);
			if ( fm ) {
				SEISCOMP_LOG(_infoChannel, "Processing delayed focalmechanism %s",
				             fm->publicID().c_str());
				info = associateFocalMechanism(fm.get());
			}
		}

		// Has been associated
		if ( info )
			it = _delayBuffer.erase(it);
		else
			++it;
	}

	// Third pass: check pending event updates
	for ( DelayEventBuffer::iterator it = _delayEventBuffer.begin();
	      it != _delayEventBuffer.end(); ) {
		it->timeout -= DELAY_CHECK_INTERVAL;
		if ( it->timeout <= -DELAY_CHECK_INTERVAL ) {
			if ( it->reason == SetPreferredFM ) {
				SEISCOMP_LOG(_infoChannel, "Handling pending event update for %s", it->id.c_str());
				EventInformationPtr info = cachedEvent(it->id);
				if ( !info ) {
					info = new EventInformation(&_cache, &_config, query(), it->id);
					if ( !info->event ) {
						SEISCOMP_ERROR("event %s not found", it->id.c_str());
						SEISCOMP_LOG(_infoChannel, "Skipped delayed preferred FM update, "
						             "event %s not found in database", it->id.c_str());
					}
					else {
						info->loadAssocations(query());
						cacheEvent(info);
						Notifier::Enable();
						updatePreferredFocalMechanism(info.get());
						Notifier::Disable();
					}
				}
				else {
					Notifier::Enable();
					updatePreferredFocalMechanism(info.get());
					Notifier::Disable();
				}
			}

			it = _delayEventBuffer.erase(it);
		}
		else
			++it;
	}


	// Clean up event cache
	cleanUpEventCache();

	NotifierMessagePtr nmsg = Notifier::GetMessage(true);
	if ( nmsg ) {
		SEISCOMP_DEBUG("%d notifier available", (int)nmsg->size());
		if ( !_testMode ) connection()->send(nmsg.get());
	}
	else
		SEISCOMP_DEBUG("No notifier available");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTool::cleanUpEventCache() {
	EventMap::iterator it;
	for ( it = _events.begin(); it != _events.end(); ) {
		if ( it->second->aboutToBeRemoved ) {
			SEISCOMP_DEBUG("... remove event %s from cache",
			               it->second->event->publicID().c_str());
			_events.erase(it++);
		}
		else
			++it;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventTool::hasDelayedEvent(const std::string &publicID,
                                DelayReason reason) const {
	DelayEventBuffer::const_iterator it;
	for ( it = _delayEventBuffer.begin(); it != _delayEventBuffer.end(); ++it ) {
		if ( publicID == it->id && reason == it->reason )
			return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTool::addObject(const string &parentID, Object* object) {
	OriginPtr org = Origin::Cast(object);
	if ( org ) {
		logObject(_inputOrigin, Core::Time::GMT());
		SEISCOMP_DEBUG("* queued new origin %s (%ld, %d/%lu)",
		              org->publicID().c_str(), (long int)org.get(),
		              definingPhaseCount(org.get()), (unsigned long)org->arrivalCount());
		SEISCOMP_LOG(_infoChannel, "Received new origin %s", org->publicID().c_str());
		_adds.insert(TodoEntry(org));
		return;
	}

	Magnitude *mag = Magnitude::Cast(object);
	if ( mag ) {
		logObject(_inputMagnitude, Core::Time::GMT());
		org = _cache.get<Origin>(parentID);
		if ( org ) {
			if ( org != mag->origin() )
				org->add(mag);

			SEISCOMP_LOG(_infoChannel, "Received new magnitude %s (%s %.2f)",
			             mag->publicID().c_str(), mag->type().c_str(), mag->magnitude().value());

			_updates.insert(TodoEntry(org));
		}
		return;
	}

	FocalMechanismPtr fm = FocalMechanism::Cast(object);
	if ( fm ) {
		logObject(_inputFocalMechanism, Core::Time::GMT());
		SEISCOMP_DEBUG("* queued new focalmechanism %s (%ld)",
		               fm->publicID().c_str(), (long int)fm.get());
		SEISCOMP_LOG(_infoChannel, "Received new focalmechanism %s", fm->publicID().c_str());
		_adds.insert(TodoEntry(fm));
		return;
	}

	MomentTensor *mt = MomentTensor::Cast(object);
	if ( mt ) {
		logObject(_inputMomentTensor, Core::Time::GMT());
		fm = _cache.get<FocalMechanism>(parentID);
		if ( fm ) {
			if ( fm != mt->focalMechanism() )
				fm->add(mt);

			SEISCOMP_LOG(_infoChannel, "Received new momenttensor %s",
			             mt->publicID().c_str());

			if ( _config.ignoreMTDerivedOrigins ) {
				// Blacklist the derived originID to prevent event
				// association.
				_originBlackList.insert(mt->derivedOriginID());
			}
		}
		return;
	}


	OriginReference *ref = OriginReference::Cast(object);
	if ( ref && !ref->originID().empty() ) {
		logObject(_inputOriginRef, Core::Time::GMT());
		SEISCOMP_LOG(_infoChannel, "Received new origin reference %s for event %s",
		             ref->originID().c_str(), parentID.c_str());

		EventInformationPtr info = cachedEvent(parentID);
		if ( !info ) {
			info = new EventInformation(&_cache, &_config, query(), parentID);
			if ( !info->event ) {
				SEISCOMP_ERROR("event %s for OriginReference not found", parentID.c_str());
				SEISCOMP_LOG(_infoChannel, " - skipped, event %s not found in database", parentID.c_str());
				return;
			}

			info->loadAssocations(query());
			cacheEvent(info);
		}

		org = _cache.get<Origin>(ref->originID());

		TodoList::iterator it = _adds.find(TodoEntry(org));
		// If this origin has to be associated in this turn
		if ( it != _adds.end() ) {
			// Remove the origin from the association list
			_adds.erase(it);
			// Add it to the origin updates (not triggered by a magnitude change)
			_realUpdates.insert(TodoEntry(org));
			SEISCOMP_DEBUG("* removed new origin %s from queue because of preset association", (*it)->publicID().c_str());
		}

		_updates.insert(TodoEntry(org));

		return;
	}

	FocalMechanismReference *fm_ref = FocalMechanismReference::Cast(object);
	if ( fm_ref && !fm_ref->focalMechanismID().empty() ) {
		logObject(_inputFMRef, Core::Time::GMT());
		SEISCOMP_LOG(_infoChannel, "Received new focalmechanism reference %s for event %s",
		             fm_ref->focalMechanismID().c_str(), parentID.c_str());

		EventInformationPtr info = cachedEvent(parentID);
		if ( !info ) {
			info = new EventInformation(&_cache, &_config, query(), parentID);
			if ( !info->event ) {
				SEISCOMP_ERROR("event %s for ForcalMechanismReference not found", parentID.c_str());
				SEISCOMP_LOG(_infoChannel, " - skipped, event %s not found in database", parentID.c_str());
				return;
			}

			info->loadAssocations(query());
			cacheEvent(info);
		}

		fm = _cache.get<FocalMechanism>(fm_ref->focalMechanismID());

		TodoList::iterator it = _adds.find(TodoEntry(fm));
		// If this origin has to be associated in this turn
		if ( it != _adds.end() ) {
			// Remove the focalmechanism from the association list
			_adds.erase(it);
			// Add it to the focalmechanism updates (not triggered by a magnitude change)
			_realUpdates.insert(TodoEntry(fm));
			SEISCOMP_DEBUG("* removed new focalmechanism %s from queue because of preset association", (*it)->publicID().c_str());
		}

		_updates.insert(TodoEntry(fm));

		return;
	}

	EventPtr evt = Event::Cast(object);
	if ( evt ) {
		logObject(_inputEvent, Core::Time::GMT());
		SEISCOMP_LOG(_infoChannel, "Received new event %s", evt->publicID().c_str());
		EventInformationPtr info = cachedEvent(evt->publicID());
		if ( !info ) {
			info = new EventInformation(&_cache, &_config, query(), evt);
			// Loading the references does not make sense here, because
			// the event has been just added
			cacheEvent(info);
		}
		return;
	}

	JournalEntryPtr journalEntry = JournalEntry::Cast(object);
	if ( journalEntry ) {
		logObject(_inputJournal, Core::Time::GMT());
		SEISCOMP_LOG(_infoChannel,
		             "Received new journal entry from %s for object %s",
		             journalEntry->sender().c_str(), journalEntry->objectID().c_str());
		if ( handleJournalEntry(journalEntry.get()) )
			return;
	}

	// We are not interested in anything else than the parsed
	// objects
	if ( object->parent() == _ep.get() || object->parent() == _journal.get() )
		object->detach();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTool::updateObject(const std::string &parentID, Object* object) {
	OriginPtr org = Origin::Cast(object);
	if ( org ) {
		logObject(_inputOrigin, Core::Time::GMT());
		if ( !org->registered() )
			org = Origin::Find(org->publicID());
		_updates.insert(TodoEntry(org));
		_realUpdates.insert(TodoEntry(org));
		SEISCOMP_DEBUG("* queued updated origin %s (%d/%lu)",
		               org->publicID().c_str(),
		               definingPhaseCount(org.get()), (unsigned long)org->arrivalCount());
		SEISCOMP_LOG(_infoChannel, "Received updated origin %s", org->publicID().c_str());
		return;
	}

	FocalMechanismPtr fm = FocalMechanism::Cast(object);
	if ( fm ) {
		logObject(_inputFocalMechanism, Core::Time::GMT());
		if ( !fm->registered() )
			fm = FocalMechanism::Find(fm->publicID());
		_updates.insert(TodoEntry(fm));
		_realUpdates.insert(TodoEntry(fm));
		SEISCOMP_DEBUG("* queued updated focalmechanism %s",
		               fm->publicID().c_str());
		SEISCOMP_LOG(_infoChannel, "Received updated focalmechanism %s", fm->publicID().c_str());
		return;
	}

	MagnitudePtr mag = Magnitude::Cast(object);
	if ( mag ) {
		logObject(_inputMagnitude, Core::Time::GMT());
		if ( !mag->registered() )
			mag = Magnitude::Find(mag->publicID());
		SEISCOMP_LOG(_infoChannel, "Received updated magnitude %s (%s %.2f)",
		             mag->publicID().c_str(), mag->type().c_str(), mag->magnitude().value());
		org = _cache.get<Origin>(parentID);
		if ( org )
			_updates.insert(TodoEntry(org, mag));
		return;
	}

	EventPtr evt = Event::Cast(object);
	if ( evt ) {
		logObject(_inputEvent, Core::Time::GMT());
		if ( !evt->registered() )
			evt = Event::Find(evt->publicID());
		SEISCOMP_LOG(_infoChannel, "Received updated event %s", evt->publicID().c_str());
		EventInformationPtr info = cachedEvent(evt->publicID());
		if ( !info ) {
			info = new EventInformation(&_cache, &_config, query(), evt);
			info->loadAssocations(query());
			cacheEvent(info);
		}

		// NOTE: What to do with an event update?
		// What we can do is to compare the cached event by the updated
		// one and resent it if other attributes than preferredOriginID
		// and preferredMagnitudeID has changed (e.g. status)
		// That does not avoid race conditions but minimizes them with
		// the current implementation.
		// For now it's getting cached unless it has been done already.
		return;
	}

	if ( object->parent() == _ep.get() || object->parent() == _journal.get() )
		object->detach();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTool::removeObject(const string &parentID, Object* object) {
	OriginReference *ref = OriginReference::Cast(object);
	if ( ref ) {
		SEISCOMP_DEBUG("%s: origin reference '%s' removed "
		               "from outside", parentID.c_str(), ref->originID().c_str());
		SEISCOMP_LOG(_infoChannel, "%s: origin reference '%s' removed "
		             "from outside", parentID.c_str(), ref->originID().c_str());

		EventInformationPtr info = cachedEvent(parentID);
		if ( !info ) {
			info = new EventInformation(&_cache, &_config, query(), parentID);
			info->loadAssocations(query());
			if ( !info->event ) {
				SEISCOMP_ERROR("event %s for OriginReference not found", parentID.c_str());
				return;
			}
			cacheEvent(info);
		}
		else if ( !info->event ) {
			info->dirtyPickSet = true;
			SEISCOMP_ERROR("event %s for OriginReference not found", parentID.c_str());
			return;
		}

		info->dirtyPickSet = true;

		if ( info->event->originReferenceCount() == 0 ) {
			SEISCOMP_DEBUG("%s: last origin reference removed, remove event",
			               parentID.c_str());
			SEISCOMP_LOG(_infoChannel, "%s: last origin reference removed, removing event",
			             parentID.c_str());

			Notifier::SetEnabled(true);
			info->event->detach();
			removeCachedEvent(info->event->publicID());
			_cache.remove(info->event.get());
			Notifier::SetEnabled(false);

			return;
		}

		if ( info->event->preferredOriginID() == ref->originID() ) {
			SEISCOMP_DEBUG("%s: removed origin reference was the preferred origin, set to NULL",
			               parentID.c_str());
			// Reset preferred information
			info->event->setPreferredOriginID("");
			info->event->setPreferredMagnitudeID("");
			info->preferredOrigin = NULL;
			info->preferredMagnitude = NULL;
			Notifier::Enable();
			// Select the preferred origin again among all remaining origins
			updatePreferredOrigin(info.get());
			Notifier::Disable();
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

DataModel::JournalEntryPtr
createEntry(const std::string &id, const std::string &proc,
            const std::string &param) {
	DataModel::JournalEntryPtr e = new DataModel::JournalEntry;
	e->setObjectID(id);
	e->setAction(proc);
	e->setParameters(param);
	e->setCreated(Core::Time::GMT());
	return e;
}

}

bool EventTool::handleJournalEntry(DataModel::JournalEntry *entry) {
	JournalEntryPtr response;

	static const std::string Failed = "Failed";
	static const std::string OK = "OK";

	// Not event specific journals
	if ( entry->action() == "EvNewEvent" ) {
		SEISCOMP_INFO("Handling journal entry for origin %s: %s(%s)",
		              entry->objectID().c_str(), entry->action().c_str(),
		              entry->parameters().c_str());

		EventPtr e = getEventForOrigin(entry->objectID());
		if ( e )
			response = createEntry(entry->objectID(), entry->action() + Failed, ":origin already associated:");
		else {
			OriginPtr origin = _cache.get<Origin>(entry->objectID());
			if ( !origin ) {
				SEISCOMP_INFO("origin %s for JournalEntry not found", entry->objectID().c_str());
				return false;
			}

			if ( !origin->magnitudeCount() && query() ) {
				SEISCOMP_DEBUG("... loading magnitudes for origin %s", origin->publicID().c_str());
				query()->loadMagnitudes(origin.get());
			}

			EventInformationPtr info = createEvent(origin.get());
			if ( info ) {
				SEISCOMP_INFO("%s: created", info->event->publicID().c_str());
				SEISCOMP_LOG(_infoChannel, "Origin %s created a new event %s",
				             origin->publicID().c_str(), info->event->publicID().c_str());

				Notifier::Enable();
				if ( !info->associate(origin.get()) ) {
					SEISCOMP_ERROR("Association of origin %s to event %s failed",
					               origin->publicID().c_str(), info->event->publicID().c_str());
					SEISCOMP_LOG(_infoChannel, "Failed to associate origin %s to event %s",
					             origin->publicID().c_str(), info->event->publicID().c_str());
				}
				else {
					logObject(_outputOriginRef, Time::GMT());
					SEISCOMP_INFO("%s: associated origin %s", info->event->publicID().c_str(),
					              origin->publicID().c_str());
					SEISCOMP_LOG(_infoChannel, "Origin %s associated to event %s",
					             origin->publicID().c_str(), info->event->publicID().c_str());
					choosePreferred(info.get(), origin.get(), NULL, true);
				}

				Notifier::Disable();

				response = createEntry(info->event->publicID(), entry->action() + OK, "created by command");
				info->addJournalEntry(response.get());
				response->setSender(name() + "@" + Core::getHostname());
				Notifier::Enable();
				Notifier::Create(_journal->publicID(), OP_ADD, response.get());
				Notifier::Disable();

				response = createEntry(entry->objectID(), entry->action() + OK, string("associated to event ") + info->event->publicID());
			}
			else
				response = createEntry(entry->objectID(), entry->action() + Failed, ":running out of eventIDs:");
		}

		if ( response ) {
			response->setSender(name() + "@" + Core::getHostname());
			Notifier::Enable();
			Notifier::Create(_journal->publicID(), OP_ADD, response.get());
			Notifier::Disable();
		}

		return true;
	}


	SEISCOMP_INFO("Handling journal entry for event %s: %s(%s)",
	              entry->objectID().c_str(), entry->action().c_str(),
	              entry->parameters().c_str());

	EventInformationPtr info = cachedEvent(entry->objectID());

	if ( !info ) {
		// No chached information yet -> load it and cache it
		info = new EventInformation(&_cache, &_config, query(), entry->objectID());
		if ( !info->event ) {
			SEISCOMP_INFO("event %s for JournalEntry not found", entry->objectID().c_str());
			return false;
		}

		info->loadAssocations(query());
		cacheEvent(info);
	}
	else
		info->addJournalEntry(entry);

	if ( entry->action() == "EvPrefMagType" ) {
		SEISCOMP_DEBUG("...set preferred magnitude type");
		response = createEntry(entry->objectID(), entry->action() + OK, !info->constraints.preferredMagnitudeType.empty()?info->constraints.preferredMagnitudeType:":automatic:");

		if ( info->preferredOrigin && !info->preferredOrigin->magnitudeCount() && query() ) {
			SEISCOMP_DEBUG("... loading magnitudes for origin %s", info->preferredOrigin->publicID().c_str());
			query()->loadMagnitudes(info->preferredOrigin.get());
		}

		// Choose the new preferred magnitude
		Notifier::Enable();
		choosePreferred(info.get(), info->preferredOrigin.get(), NULL);
		Notifier::Disable();
	}
	else if ( entry->action() == "EvPrefOrgID" ) {
		SEISCOMP_DEBUG("...set preferred origin by ID");

		// Release fixed origin and choose the best one automatically
		if ( info->constraints.preferredOriginID.empty() ) {
			response = createEntry(entry->objectID(), entry->action() + OK, ":automatic:");

			Notifier::Enable();
			updatePreferredOrigin(info.get());
			Notifier::Disable();
		}
		else {
			if ( info->event->originReference(info->constraints.preferredOriginID) == NULL ) {
				response = createEntry(entry->objectID(), entry->action() + Failed, ":unreferenced:");
			}
			else {
				OriginPtr org = _cache.get<Origin>(info->constraints.preferredOriginID);
				if ( org ) {
					if ( !org->magnitudeCount() && query() ) {
						SEISCOMP_DEBUG("... loading magnitudes for origin %s", org->publicID().c_str());
						query()->loadMagnitudes(org.get());
					}

					response = createEntry(entry->objectID(), entry->action() + OK, info->constraints.preferredOriginID);
					Notifier::Enable();
					choosePreferred(info.get(), org.get(), NULL);
					Notifier::Disable();
				}
				else {
					response = createEntry(entry->objectID(), entry->action() + Failed, ":not available:");
				}
			}
		}
	}
	else if ( entry->action() == "EvPrefOrgEvalMode" ) {
		SEISCOMP_DEBUG("...set preferred origin by evaluation mode");

		DataModel::EvaluationMode em;
		if ( !em.fromString(entry->parameters().c_str()) && !entry->parameters().empty() ) {
			// If a mode is requested but an invalid mode identifier has been submitted
			response = createEntry(entry->objectID(), entry->action() + Failed, string(":mode '") + entry->parameters() + "' unknown:");
		}
		else {
			// Release fixed origin mode and choose the best one automatically
			if ( !info->constraints.preferredOriginEvaluationMode )
				response = createEntry(entry->objectID(), entry->action() + OK, ":automatic:");
			else
				response = createEntry(entry->objectID(), entry->action() + OK, entry->parameters());

			Notifier::Enable();
			updatePreferredOrigin(info.get());
			Notifier::Disable();
		}
	}
	else if ( entry->action() == "EvPrefOrgAutomatic" ) {
		response = createEntry(entry->objectID(), entry->action() + OK, ":automatic mode:");

		Notifier::Enable();
		updatePreferredOrigin(info.get());
		Notifier::Disable();
	}
	else if ( entry->action() == "EvType" ) {
		SEISCOMP_DEBUG("...set event type");

		OPT(EventType) et;
		EventType newEt;

		if ( !entry->parameters().empty() && !newEt.fromString(entry->parameters()) ) {
			response = createEntry(entry->objectID(), entry->action() + Failed, ":invalid type:");
		}
		else {
			try { et = info->event->type(); }
			catch ( ... ) {}

			if ( !et && entry->parameters().empty() ) {
				response = createEntry(entry->objectID(), entry->action() + Failed, ":not modified:");
			}
			else if ( et && !entry->parameters().empty() && *et == newEt ) {
				response = createEntry(entry->objectID(), entry->action() + Failed, ":not modified:");
			}
			else {
				if ( !entry->parameters().empty() ) {
					info->event->setType(newEt);
					response = createEntry(entry->objectID(), entry->action() + OK, entry->parameters());
				}
				else {
					info->event->setType(None);
					response = createEntry(entry->objectID(), entry->action() + OK, ":unset:");
				}

				Notifier::Enable();
				updateEvent(info->event.get());
				Notifier::Disable();
			}
		}
	}
	else if ( entry->action() == "EvTypeCertainty" ) {
		SEISCOMP_DEBUG("...set event type certainty");

		OPT(EventTypeCertainty) etc;
		EventTypeCertainty newEtc;

		if ( !entry->parameters().empty() && !newEtc.fromString(entry->parameters()) ) {
			response = createEntry(entry->objectID(), entry->action() + Failed, ":invalid type certainty:");
		}
		else {
			try { etc = info->event->typeCertainty(); }
			catch ( ... ) {}

			if ( !etc && entry->parameters().empty() ) {
				response = createEntry(entry->objectID(), entry->action() + Failed, ":not modified:");
			}
			else if ( etc && !entry->parameters().empty() && *etc == newEtc ) {
				response = createEntry(entry->objectID(), entry->action() + Failed, ":not modified:");
			}
			else {
				if ( !entry->parameters().empty() ) {
					info->event->setTypeCertainty(newEtc);
					response = createEntry(entry->objectID(), entry->action() + OK, entry->parameters());
				}
				else {
					info->event->setTypeCertainty(None);
					response = createEntry(entry->objectID(), entry->action() + OK, ":unset:");
				}
				Notifier::Enable();
				updateEvent(info->event.get());
				Notifier::Disable();
			}
		}
	}
	else if ( entry->action() == "EvName" ) {
		SEISCOMP_DEBUG("...set event name");

		string error;
		Notifier::Enable();
		if ( info->setEventName(entry, error) )
			response = createEntry(entry->objectID(), entry->action() + OK, entry->parameters());
		else
			response = createEntry(entry->objectID(), entry->action() + Failed, error);
		Notifier::Disable();
	}
	else if ( entry->action() == "EvOpComment" ) {
		SEISCOMP_DEBUG("...set event operator's comment");

		string error;
		Notifier::Enable();
		if ( info->setEventOpComment(entry, error) )
			response = createEntry(entry->objectID(), entry->action() + OK, entry->parameters());
		else
			response = createEntry(entry->objectID(), entry->action() + Failed, error);
		Notifier::Disable();
	}
	else if ( entry->action() == "EvPrefFocMecID" ) {
		SEISCOMP_DEBUG("...set event preferred focal mechanism");

		if ( entry->parameters().empty() ) {
			info->event->setPreferredFocalMechanismID("");
			response = createEntry(entry->objectID(), entry->action() + OK, ":unset:");
		}
		else {
			if ( info->event->focalMechanismReference(info->constraints.preferredFocalMechanismID) == NULL ) {
				response = createEntry(entry->objectID(), entry->action() + Failed, ":unreferenced:");
			}
			else {
				FocalMechanismPtr fm = _cache.get<FocalMechanism>(info->constraints.preferredFocalMechanismID);
				if ( fm ) {
					if ( !fm->momentTensorCount() && query() ) {
						SEISCOMP_DEBUG("... loading moment tensors for focal mechanism %s", fm->publicID().c_str());
						query()->loadMomentTensors(fm.get());
					}

					response = createEntry(entry->objectID(), entry->action() + OK, info->constraints.preferredFocalMechanismID);
					Notifier::Enable();
					choosePreferred(info.get(), fm.get());
					Notifier::Disable();
				}
				else {
					response = createEntry(entry->objectID(), entry->action() + Failed, ":not available:");
				}
			}
		}
		Notifier::Enable();
		updateEvent(info->event.get());
		Notifier::Disable();
	}
	else if ( entry->action() == "EvPrefMw" ) {
		SEISCOMP_DEBUG("...set Mw from focal mechanism as preferred magnitude");
		if ( entry->parameters().empty() ) {
			response = createEntry(entry->objectID(), entry->action() + Failed, ":empty parameter (true or false expected):");
		}
		else {
			if ( entry->parameters() == "true" || entry->parameters() == "false" ) {
				response = createEntry(entry->objectID(), entry->action() + OK, entry->parameters());

				Notifier::Enable();
				choosePreferred(info.get(), info->preferredOrigin.get(), NULL);
				Notifier::Disable();
			}
			else
				response = createEntry(entry->objectID(), entry->action() + Failed, ":true or false expected:");
		}
	}
	// Merge event in  parameters into event in objectID. The source
	// event is deleted afterwards.
	else if ( entry->action() == "EvMerge" ) {
		SEISCOMP_DEBUG("...merge event '%s'", entry->parameters().c_str());

		if ( entry->parameters().empty() )
			response = createEntry(entry->objectID(), entry->action() + Failed, ":empty source event id:");
		else if ( info->event->publicID() == entry->parameters() ) {
			response = createEntry(entry->objectID(), entry->action() + Failed, ":source and target are equal:");
		}
		else {
			EventInformationPtr sourceInfo = cachedEvent(entry->parameters());
			if ( !sourceInfo ) {
				sourceInfo = new EventInformation(&_cache, &_config, query(), entry->parameters());
				if ( !sourceInfo->event ) {
					SEISCOMP_ERROR("source event %s for merge not found", entry->parameters().c_str());
					SEISCOMP_LOG(_infoChannel, " - skipped, source event %s for merge not found in database", entry->parameters().c_str());
					sourceInfo = NULL;
				}
				else {
					sourceInfo->loadAssocations(query());
					cacheEvent(sourceInfo);
				}
			}

			if ( !sourceInfo )
				response = createEntry(entry->objectID(), entry->action() + Failed, ":source event not found:");
			else {
				Notifier::Enable();
				// Do the merge
				if ( mergeEvents(info.get(), sourceInfo.get()) ) {
					JournalEntryPtr srcResponse;
					srcResponse = createEntry(sourceInfo->event->publicID(), "EvDeleteOK", string("merged into ") + info->event->publicID());
					if ( srcResponse ) {
						sourceInfo->addJournalEntry(srcResponse.get());
						srcResponse->setSender(name() + "@" + Core::getHostname());
						Notifier::Enable();
						Notifier::Create(_journal->publicID(), OP_ADD, srcResponse.get());
						Notifier::Disable();
					}

					response = createEntry(entry->objectID(), entry->action() + OK, sourceInfo->event->publicID());
				}
				else
					response = createEntry(entry->objectID(), entry->action() + Failed, ":internal error:");
				Notifier::Disable();
			}
		}
	}
	// Move an origin to this event. If it is already associated to another
	// event, remove this reference
	else if ( entry->action() == "EvGrabOrg") {
		SEISCOMP_DEBUG("...grab origin '%s'", entry->parameters().c_str());
		OriginPtr org = _cache.get<Origin>(entry->parameters());
		list<string> fmIDsToMove;

		if ( !org )
			response = createEntry(entry->objectID(), entry->action() + Failed, ":origin not found:");
		else {
			EventPtr e = getEventForOrigin(org->publicID());
			if ( e ) {
				EventInformationPtr sourceInfo = cachedEvent(e->publicID());
				if ( !sourceInfo ) {
					sourceInfo = new EventInformation(&_cache, &_config, query(), e);
					sourceInfo->loadAssocations(query());
					cacheEvent(sourceInfo);
				}

				if ( sourceInfo->event->originReferenceCount() < 2 ) {
					response = createEntry(entry->objectID(), entry->action() + Failed, ":last origin cannot be removed:");
				}
				else {
					Notifier::Enable();
					sourceInfo->event->removeOriginReference(org->publicID());

					// Remove all focal mechanism references that
					// used this origin as trigger
					bool updatedPrefFM = false;
					for ( size_t i = 0; i < sourceInfo->event->focalMechanismReferenceCount(); ) {
						FocalMechanismPtr fm = _cache.get<FocalMechanism>(sourceInfo->event->focalMechanismReference(i)->focalMechanismID());
						if ( !fm ) { ++i; continue; }
						if ( fm->triggeringOriginID() == org->publicID() ) {
							fmIDsToMove.push_back(fm->publicID());
							sourceInfo->event->removeFocalMechanismReference(i);
							if ( fm->publicID() == sourceInfo->event->preferredFocalMechanismID() )
								updatedPrefFM = true;
						}
						else
							++i;
					}

					if ( sourceInfo->event->preferredOriginID() == org->publicID() ) {
						SEISCOMP_DEBUG("%s: removed origin reference was the preferred origin, set to NULL",
						               sourceInfo->event->publicID().c_str());
						// Reset preferred information
						sourceInfo->event->setPreferredOriginID("");
						sourceInfo->event->setPreferredMagnitudeID("");
						sourceInfo->preferredOrigin = NULL;
						sourceInfo->preferredMagnitude = NULL;
						// Select the preferred origin again among all remaining origins
						updatePreferredOrigin(sourceInfo.get());
					}

					if ( updatedPrefFM ) {
						SEISCOMP_DEBUG("%s: removed focal mechanism reference was the preferred focal mechanism, set to NULL",
						               sourceInfo->event->publicID().c_str());
						sourceInfo->event->setPreferredFocalMechanismID("");
						sourceInfo->preferredFocalMechanism = NULL;
						updatePreferredFocalMechanism(sourceInfo.get());
					}

					Notifier::Disable();

					e = NULL;
				}
			}

			// If the event is still a valid pointer an error occured while
			// removing the reference
			if ( !e ) {
				Notifier::Enable();
				info->associate(org.get());
				logObject(_outputOriginRef, Time::GMT());
				// Associate focal mechanism references
				list<string>::iterator it;
				for ( it = fmIDsToMove.begin(); it != fmIDsToMove.end(); ++it ) {
					logObject(_outputFMRef, Time::GMT());
					info->event->add(new FocalMechanismReference(*it));
				}

				updatePreferredOrigin(info.get());
				updatePreferredFocalMechanism(info.get());

				Notifier::Disable();

				response = createEntry(entry->objectID(), entry->action() + OK, org->publicID());
			}
		}
	}
	// Remove an origin reference from an event and create a new event for
	// this origin
	else if ( entry->action() == "EvSplitOrg" ) {
		SEISCOMP_DEBUG("...split origin '%s' and create a new event", entry->parameters().c_str());
		OriginPtr org = _cache.get<Origin>(entry->parameters());
		list<string> fmIDsToMove;

		if ( !org )
			response = createEntry(entry->objectID(), entry->action() + Failed, ":origin not found:");
		else {
			if ( info->event->originReference(org->publicID()) == NULL )
				response = createEntry(entry->objectID(), entry->action() + Failed, ":origin not associated:");
			else {
				if ( info->event->originReferenceCount() < 2 )
					response = createEntry(entry->objectID(), entry->action() + Failed, ":last origin cannot be removed:");
				else {
					EventInformationPtr newInfo = createEvent(org.get());
					JournalEntryPtr newResponse;

					if ( newInfo ) {
						// Remove origin reference
						Notifier::SetEnabled(true);
						info->event->removeOriginReference(org->publicID());

						// Remove all focal mechanism references that
						// used this origin as trigger
						bool updatedPrefFM = false;
						for ( size_t i = 0; i < info->event->focalMechanismReferenceCount(); ) {
							FocalMechanismPtr fm = _cache.get<FocalMechanism>(info->event->focalMechanismReference(i)->focalMechanismID());
							if ( !fm ) { ++i; continue; }
							if ( fm->triggeringOriginID() == org->publicID() ) {
								SEISCOMP_DEBUG("...scheduled focal mechanism %s for split",
								               fm->publicID().c_str());
								fmIDsToMove.push_back(fm->publicID());
								info->event->removeFocalMechanismReference(i);
								if ( fm->publicID() == info->event->preferredFocalMechanismID() )
									updatedPrefFM = true;
							}
							else
								++i;
						}
						Notifier::Enable();

						if ( info->event->preferredOriginID() == org->publicID() ) {
							SEISCOMP_DEBUG("%s: removed origin reference was the preferred origin, set to NULL",
							               info->event->publicID().c_str());
							// Reset preferred information
							info->event->setPreferredOriginID("");
							info->event->setPreferredMagnitudeID("");
							info->preferredOrigin = NULL;
							info->preferredMagnitude = NULL;
							// Select the preferred origin again among all remaining origins
							updatePreferredOrigin(info.get());

						}

						if ( updatedPrefFM ) {
							SEISCOMP_DEBUG("%s: removed focal mechanism reference was the preferred focal mechanism, set to NULL",
							               info->event->publicID().c_str());
							info->event->setPreferredFocalMechanismID("");
							info->preferredFocalMechanism = NULL;
							updatePreferredFocalMechanism(info.get());
						}

						Notifier::Disable();

						response = createEntry(entry->objectID(), entry->action() + OK, org->publicID() + " removed by command");

						SEISCOMP_INFO("%s: created", newInfo->event->publicID().c_str());
						SEISCOMP_LOG(_infoChannel, "Origin %s created a new event %s",
						             org->publicID().c_str(), newInfo->event->publicID().c_str());

						Notifier::Enable();
						newInfo->associate(org.get());
						logObject(_outputOriginRef, Time::GMT());
						// Associate focal mechanism references
						list<string>::iterator it;
						for ( it = fmIDsToMove.begin(); it != fmIDsToMove.end(); ++it ) {
							SEISCOMP_INFO("%s: associated focal mechanism %s", newInfo->event->publicID().c_str(),
							              it->c_str());
							newInfo->event->add(new FocalMechanismReference(*it));
							logObject(_outputFMRef, Time::GMT());
						}
						SEISCOMP_INFO("%s: associated origin %s", newInfo->event->publicID().c_str(),
						              org->publicID().c_str());
						SEISCOMP_LOG(_infoChannel, "Origin %s associated to event %s",
						             org->publicID().c_str(), newInfo->event->publicID().c_str());

						if ( !org->magnitudeCount() && query() ) {
							SEISCOMP_DEBUG("... loading magnitudes for origin %s", org->publicID().c_str());
							query()->loadMagnitudes(org.get());
						}

						choosePreferred(newInfo.get(), org.get(), NULL, true);
						updatePreferredFocalMechanism(newInfo.get());
						Notifier::Disable();

						newResponse = createEntry(newInfo->event->publicID(), "EvNewEventOK", "created by command");
						if ( newResponse ) {
							newInfo->addJournalEntry(newResponse.get());
							newResponse->setSender(name() + "@" + Core::getHostname());
							Notifier::Enable();
							Notifier::Create(_journal->publicID(), OP_ADD, newResponse.get());
							Notifier::Disable();
						}
					}
					else
						response = createEntry(entry->objectID(), entry->action() + Failed, ":running out of eventIDs:");
				}
			}
		}
	}
	// Is this command ours by starting with Ev?
	else if ( !entry->action().empty() && (entry->action().compare(0, 2,"Ev") == 0) ) {
		// Make sure we don't process result journal entries ending on either
		// OK or Failed
		if ( ( entry->action().size() >= Failed.size() + 2 &&
		       std::equal(Failed.rbegin(), Failed.rend(), entry->action().rbegin() ) ) ||
		     ( entry->action().size() >= OK.size() + 2 &&
		       std::equal(OK.rbegin(), OK.rend(), entry->action().rbegin() ) ) )
			SEISCOMP_ERROR("Ignoring already processed journal entry from %s: %s(%s)",
			               entry->sender().c_str(), entry->action().c_str(),
			               entry->parameters().c_str());
		else
			response = createEntry(entry->objectID(), entry->action() + Failed, ":unknown command:");
	}

	if ( response ) {
		info->addJournalEntry(response.get());
		response->setSender(name() + "@" + Core::getHostname());
		Notifier::Enable();
		Notifier::Create(_journal->publicID(), OP_ADD, response.get());
		Notifier::Disable();
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventInformationPtr EventTool::associateOriginCheckDelay(DataModel::Origin *origin) {
	if ( _config.delayTimeSpan > 0 ) {
		SEISCOMP_LOG(_infoChannel, "Checking delay filter for origin %s",
		             origin->publicID().c_str());

		EvaluationMode mode = AUTOMATIC;
		try {
			mode = origin->evaluationMode();
		}
		catch ( ValueException& ) {}

		if ( _config.delayFilter.agencyID &&
		     objectAgencyID(origin) != *_config.delayFilter.agencyID ) {
			SEISCOMP_LOG(_infoChannel, " * agency does not match (%s != %s), process immediately",
			             objectAgencyID(origin).c_str(), (*_config.delayFilter.agencyID).c_str());
			return associateOrigin(origin, true);
		}

		if ( _config.delayFilter.author &&
		     objectAuthor(origin) != *_config.delayFilter.author ) {
			SEISCOMP_LOG(_infoChannel, " * author does not match (%s != %s), process immediately",
			             objectAuthor(origin).c_str(), (*_config.delayFilter.author).c_str());
			return associateOrigin(origin, true);
		}

		if ( _config.delayFilter.evaluationMode &&
		     mode != *_config.delayFilter.evaluationMode ) {
			SEISCOMP_LOG(_infoChannel, " * evaluationMode does not match (%s != %s), process immediately",
			             mode.toString(), (*_config.delayFilter.evaluationMode).toString());
			return associateOrigin(origin, true);
		}

		// Filter to delay the origin passes
		SEISCOMP_LOG(_infoChannel, "Origin %s delayed for %i s",
		             origin->publicID().c_str(), _config.delayTimeSpan);
		_delayBuffer.push_back(DelayedObject(origin, _config.delayTimeSpan));

		return NULL;
	}

	return associateOrigin(origin, true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventInformationPtr EventTool::associateOrigin(Seiscomp::DataModel::Origin *origin,
                                               bool allowEventCreation,
                                               bool *createdEvent) {
	if ( createdEvent ) *createdEvent = false;

	// Default origin status
	EvaluationMode status = AUTOMATIC;
	try {
		status = origin->evaluationMode();
	}
	catch ( Core::ValueException &e ) {
	}

	// Find a matching (cached) event for this origin
	EventInformationPtr info = findMatchingEvent(origin);
	if ( !info ) {
		Core::Time startTime = origin->time().value() - _config.eventTimeBefore;
		Core::Time endTime = origin->time().value() + _config.eventTimeAfter;
		MatchResult bestResult = Nothing;

		SEISCOMP_DEBUG("... search for origin's %s event in database", origin->publicID().c_str());

		if ( query() ) {
			// Look for events in a certain timewindow around the origintime
			DatabaseIterator it = query()->getEvents(startTime, endTime);

			std::vector<EventPtr> fetchedEvents;
			for ( ; *it; ++it ) {
				EventPtr e = Event::Cast(*it);
				assert(e != NULL);
				// Is this event already cached and associated with an information
				// object?
				if ( isEventCached(e->publicID()) ) continue;

				fetchedEvents.push_back(e);
			}

			for ( size_t i = 0; i < fetchedEvents.size(); ++i ) {
				// Load the eventinformation for this event
				EventInformationPtr tmp = new EventInformation(&_cache, &_config, query(), fetchedEvents[i]);
				if ( tmp->valid() ) {
					tmp->loadAssocations(query());
					MatchResult res = compare(tmp.get(), origin);
					if ( res > bestResult ) {
						bestResult = res;
						info = tmp;
					}
				}
			}
		}

		if ( info ) {
			SEISCOMP_LOG(_infoChannel, "Found matching event %s for origin %s (code: %d)",
			             info->event->publicID().c_str(), origin->publicID().c_str(), bestResult);
			SEISCOMP_DEBUG("... found best matching event %s (code: %d)", info->event->publicID().c_str(), bestResult);
			cacheEvent(info);
			if ( info->event->originReference(origin->publicID()) != NULL ) {
				SEISCOMP_DEBUG("... origin already associated to event %s", info->event->publicID().c_str());
				SEISCOMP_LOG(_infoChannel, "Origin %s skipped: already associated to event %s",
				             origin->publicID().c_str(), info->event->publicID().c_str());
				info = NULL;
			}
		}
		// Create a new event
		else {
			if ( !allowEventCreation ) return NULL;

			if ( isAgencyIDBlocked(objectAgencyID(origin)) ) {
				SEISCOMP_LOG(_infoChannel, "Origin %s skipped: agencyID blocked and is not allowed to create a new event",
				             origin->publicID().c_str());
				return NULL;
			}

			if ( status == AUTOMATIC ) {
				if ( _config.minAutomaticScore ) {
					double score = _score->evaluate(origin);
					if ( score < *_config.minAutomaticScore ) {
						SEISCOMP_DEBUG("... rejecting automatic origin %s (score: %f < %f)",
						               origin->publicID().c_str(),
						               score, *_config.minAutomaticScore);
						SEISCOMP_LOG(_infoChannel,
						             "Origin %s skipped: score too low (%f < %f) to create a new event",
						             origin->publicID().c_str(),
						             score, *_config.minAutomaticScore);
						return NULL;
					}
				}
				else if ( definingPhaseCount(origin) < int(_config.minAutomaticArrivals) ) {
					SEISCOMP_DEBUG("... rejecting automatic origin %s (phaseCount: %d < %zu)",
					               origin->publicID().c_str(),
					               definingPhaseCount(origin),
					               _config.minAutomaticArrivals);
					SEISCOMP_LOG(_infoChannel,
					             "Origin %s skipped: phaseCount too low (%d < %zu) to create a new event",
					             origin->publicID().c_str(),
					             definingPhaseCount(origin),
					             _config.minAutomaticArrivals);
					return NULL;
				}
			}

			if ( !checkRegionFilter(_config.regionFilter, origin) )
				return NULL;

			info = createEvent(origin);
			if ( info ) {
				if ( createdEvent ) *createdEvent = true;
				SEISCOMP_INFO("%s: created", info->event->publicID().c_str());
				SEISCOMP_LOG(_infoChannel, "Origin %s created a new event %s",
				             origin->publicID().c_str(), info->event->publicID().c_str());
			}
		}
	}
	else {
		SEISCOMP_DEBUG("... found cached event information %s for origin", info->event->publicID().c_str());
		SEISCOMP_LOG(_infoChannel, "Found matching event %s for origin %s",
			         info->event->publicID().c_str(), origin->publicID().c_str());
	}

	if ( info ) {
		// Found an event => so associate the origin
		Notifier::Enable();
		if ( !info->associate(origin) ) {
			SEISCOMP_ERROR("Association of origin %s to event %s failed",
			               origin->publicID().c_str(), info->event->publicID().c_str());
			SEISCOMP_LOG(_infoChannel, "Failed to associate origin %s to event %s",
			             origin->publicID().c_str(), info->event->publicID().c_str());
		}
		else {
			logObject(_outputOriginRef, Time::GMT());
			SEISCOMP_INFO("%s: associated origin %s", info->event->publicID().c_str(),
			              origin->publicID().c_str());
			SEISCOMP_LOG(_infoChannel, "Origin %s associated to event %s",
			             origin->publicID().c_str(), info->event->publicID().c_str());
			choosePreferred(info.get(), origin, NULL, true);
		}

		Notifier::Disable();
	}

	_cache.feed(origin);

	return info;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventTool::checkRegionFilter(const Config::RegionFilters &fs, const Origin *origin) {
	Config::RegionFilters::const_iterator it;
	for ( it = fs.begin(); it != fs.end(); ++it ) {
		const Config::RegionFilter &f = *it;
		if ( f.region ) {
			try {
				if ( !f.region->isInside(origin->latitude(),
				                         origin->longitude()) ) {
					SEISCOMP_DEBUG("... rejecting automatic origin %s: region "
					               "criterion not met",
					               origin->publicID().c_str());
					SEISCOMP_LOG(_infoChannel, "Origin %s skipped: region criteria not met",
					             origin->publicID().c_str());
					return false;
				}
			}
			catch ( ValueException &exc ) {
				SEISCOMP_DEBUG("...region check exception: %s", exc.what());
				SEISCOMP_LOG(_infoChannel, "Origin %s skipped: region check exception: %s",
					         origin->publicID().c_str(), exc.what());
				return false;
			}
		}

		if ( f.minDepth ) {
			try {
				if ( origin->depth().value() < *f.minDepth ) {
					SEISCOMP_DEBUG("... rejecting automatic origin %s: min depth "
					               "criterion not met",
					               origin->publicID().c_str());
					SEISCOMP_LOG(_infoChannel, "Origin %s skipped: min depth criteria not met",
					             origin->publicID().c_str());
					return false;
				}
			}
			catch ( ... ) {}
		}

		if ( f.maxDepth ) {
			try {
				if ( origin->depth().value() > *f.maxDepth ) {
					SEISCOMP_DEBUG("... rejecting automatic origin %s: max depth "
					               "criterion not met",
					               origin->publicID().c_str());
					SEISCOMP_LOG(_infoChannel, "Origin %s skipped: max depth criteria not met",
					             origin->publicID().c_str());
					return false;
				}
			}
			catch ( ... ) {}
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTool::updatedOrigin(DataModel::Origin *org,
                              DataModel::Magnitude *mag, bool realOriginUpdate) {
	// Get the cached origin, not the one sent in this message
	// If there is no cached origin the same instance is passed back
	Origin *origin = Origin::Find(org->publicID());
	if ( origin != NULL ) {
		if ( origin != org )
			*origin = *org;
	}
	else
		origin = org;

	EventInformationPtr info = findAssociatedEvent(origin);
	if ( !info ) {
		EventPtr e;
		if ( query() ) {
			SEISCOMP_DEBUG("... search for origin's %s event in database", origin->publicID().c_str());
			e = getEventForOrigin(origin->publicID());
		}

		if ( !e ) {
			SEISCOMP_DEBUG("... updated origin %s has not been associated yet, doing this",
			               origin->publicID().c_str());
			associateOrigin(origin, true);
			return;
		}

		info = new EventInformation(&_cache, &_config, query(), e);
		info->loadAssocations(query());
		cacheEvent(info);
	}
	else {
		SEISCOMP_DEBUG("... found cached event information %s for origin", info->event->publicID().c_str());
	}

	if ( !origin->arrivalCount() && query() ) {
		SEISCOMP_DEBUG("... loading arrivals for origin %s", origin->publicID().c_str());
		query()->loadArrivals(origin);
	}

	if ( !origin->magnitudeCount() && query() ) {
		SEISCOMP_DEBUG("... loading magnitudes for origin %s", origin->publicID().c_str());
		query()->loadMagnitudes(origin);
	}

	// Cache this origin
	_cache.feed(origin);

	Notifier::Enable();
	if ( realOriginUpdate &&
	     info->event->preferredOriginID() == origin->publicID() )
		updatePreferredOrigin(info.get());
	else
		choosePreferred(info.get(), origin, mag, realOriginUpdate);
	Notifier::Disable();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventInformationPtr EventTool::associateFocalMechanismCheckDelay(DataModel::FocalMechanism *fm) {
	if ( _config.delayTimeSpan > 0 ) {
		EvaluationMode mode = AUTOMATIC;
		try {
			mode = fm->evaluationMode();
		}
		catch ( ValueException& ) {}

		if ( _config.delayFilter.agencyID &&
		     objectAgencyID(fm) != *_config.delayFilter.agencyID )
			return associateFocalMechanism(fm);

		if ( _config.delayFilter.author &&
		     objectAuthor(fm) != *_config.delayFilter.author )
			return associateFocalMechanism(fm);

		if ( _config.delayFilter.evaluationMode &&
		     mode != *_config.delayFilter.evaluationMode )
			return associateFocalMechanism(fm);

		// Filter to delay the origin passes
		SEISCOMP_LOG(_infoChannel, "FocalMechanism %s delayed", fm->publicID().c_str());
		_delayBuffer.push_back(DelayedObject(fm, _config.delayTimeSpan));

		return NULL;
	}

	return associateFocalMechanism(fm);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventInformationPtr EventTool::associateFocalMechanism(FocalMechanism *fm) {
	EventInformationPtr info;

	// Default origin status
	EvaluationMode status = AUTOMATIC;
	try {
		status = fm->evaluationMode();
	}
	catch ( Core::ValueException &e ) {
	}

	OriginPtr triggeringOrigin = _cache.get<Origin>(fm->triggeringOriginID());
	if ( triggeringOrigin == NULL ) {
		SEISCOMP_DEBUG("... triggering origin %s not found, skipping event association",
		               fm->triggeringOriginID().c_str());
		return info;
	}

	info = findAssociatedEvent(triggeringOrigin.get());
	if ( !info ) {
		EventPtr e;
		if ( query() ) {
			SEISCOMP_DEBUG("... search for triggering origin's %s event in database", triggeringOrigin->publicID().c_str());
			e = getEventForOrigin(triggeringOrigin->publicID());
		}

		if ( !e ) {
			SEISCOMP_DEBUG("... triggering origin %s has not been associated yet, skipping focalmechanism",
			               triggeringOrigin->publicID().c_str());
			return NULL;
		}

		info = new EventInformation(&_cache, &_config, query(), e);
		info->loadAssocations(query());
		cacheEvent(info);
	}
	else {
		SEISCOMP_DEBUG("... found cached event information %s for focalmechanism", info->event->publicID().c_str());
	}

	if ( !fm->momentTensorCount() && query() ) {
		SEISCOMP_DEBUG("... loading moment tensor for focalmechanism %s", fm->publicID().c_str());
		query()->loadMomentTensors(fm);
	}

	// Cache this origin
	_cache.feed(fm);

	Notifier::Enable();
	if ( !info->associate(fm) ) {
		SEISCOMP_ERROR("Association of focalmechanism %s to event %s failed",
		               fm->publicID().c_str(), info->event->publicID().c_str());
		SEISCOMP_LOG(_infoChannel, "Failed to associate focalmechanism %s to event %s",
		             fm->publicID().c_str(), info->event->publicID().c_str());
	}
	else {
		logObject(_outputFMRef, Time::GMT());
		SEISCOMP_INFO("%s: associated focalmechanism %s", info->event->publicID().c_str(),
		              fm->publicID().c_str());
		SEISCOMP_LOG(_infoChannel, "FocalMechanism %s associated to event %s",
		             fm->publicID().c_str(), info->event->publicID().c_str());
		choosePreferred(info.get(), fm);
	}
	Notifier::Disable();

	return info;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTool::updatedFocalMechanism(FocalMechanism *focalMechanism) {
	// Get the cached focal mechanism, not the one sent in this message
	// If there is no cached origin the same instance is passed back
	FocalMechanism *fm = FocalMechanism::Find(focalMechanism->publicID());
	if ( fm != NULL ) {
		if ( focalMechanism != fm )
			*fm = *focalMechanism;
	}
	else
		fm = focalMechanism;

	EventInformationPtr info = findAssociatedEvent(fm);
	if ( !info ) {
		EventPtr e;
		if ( query() ) {
			SEISCOMP_DEBUG("... search for focal mechanisms's %s event in database", fm->publicID().c_str());
			e = getEventForFocalMechanism(fm->publicID());
		}

		if ( !e ) {
			SEISCOMP_DEBUG("... updated focal mechanism %s has not been associated yet, doing this",
			               fm->publicID().c_str());
			associateFocalMechanism(fm);
			return;
		}

		info = new EventInformation(&_cache, &_config, query(), e);
		info->loadAssocations(query());
		cacheEvent(info);
	}
	else {
		SEISCOMP_DEBUG("... found cached event information %s for focalmechanism", info->event->publicID().c_str());
	}

	if ( !fm->momentTensorCount() && query() ) {
		SEISCOMP_DEBUG("... loading moment tensor for focalmechanism %s", fm->publicID().c_str());
		query()->loadMomentTensors(fm);
	}

	// Cache this origin
	_cache.feed(fm);

	Notifier::Enable();
	choosePreferred(info.get(), fm);
	Notifier::Disable();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventTool::MatchResult EventTool::compare(EventInformation *info,
                                          Seiscomp::DataModel::Origin *origin) {
	size_t matchingPicks = info->matchingPicks(query(), origin);

	MatchResult result = Nothing;

	if ( matchingPicks >= _config.minMatchingPicks )
		result = Picks;

	if ( _config.maxMatchingPicksTimeDiff >= 0 )
		SEISCOMP_DEBUG("... compare pick times with threshold %.2fs",
		               _config.maxMatchingPicksTimeDiff);
	SEISCOMP_DEBUG("... matching picks of %s and %s = %lu/%lu, need at least %lu",
	               origin->publicID().c_str(), info->event->publicID().c_str(),
	               (unsigned long)matchingPicks, (unsigned long)origin->arrivalCount(),
	               (unsigned long)_config.minMatchingPicks);
	SEISCOMP_LOG(_infoChannel, "... matching picks of %s and %s = %lu/%lu, need at least %lu",
	             origin->publicID().c_str(), info->event->publicID().c_str(),
	             (unsigned long)matchingPicks, (unsigned long)origin->arrivalCount(),
	             (unsigned long)_config.minMatchingPicks);

	if ( !info->preferredOrigin )
		return Nothing;

	double dist, azi1, azi2;

	Math::Geo::delazi(origin->latitude().value(), origin->longitude().value(),
	                  info->preferredOrigin->latitude().value(),
	                  info->preferredOrigin->longitude().value(),
	                  &dist, &azi1, &azi2);

	// Dist out of range
	SEISCOMP_DEBUG("... distance of %s to %s = %f, max = %f",
	               origin->publicID().c_str(), info->event->publicID().c_str(),
	               dist, _config.maxDist);
	SEISCOMP_LOG(_infoChannel, "... distance of %s to %s = %f, max = %f",
	             origin->publicID().c_str(), info->event->publicID().c_str(),
	             dist, _config.maxDist);
	if ( dist <= _config.maxDist ) {
		TimeSpan diffTime = info->preferredOrigin->time().value() - origin->time().value();

		SEISCOMP_DEBUG("... time diff of %s to %s = %.2fs, max = %.2f",
		               origin->publicID().c_str(), info->event->publicID().c_str(),
		               (double)diffTime, (double)_config.maxTimeDiff);
		SEISCOMP_LOG(_infoChannel, "... time diff of %s to %s = %f, max = %f",
		             origin->publicID().c_str(), info->event->publicID().c_str(),
		             (double)diffTime, (double)_config.maxTimeDiff);

		if ( diffTime.abs() <= _config.maxTimeDiff ) {

			if ( result == Picks )
				result = PicksAndLocation;
			else
				result = Location;

		}
	}

	switch ( result ) {
		case Nothing:
			SEISCOMP_DEBUG("... no match for %s and %s",
			               origin->publicID().c_str(), info->event->publicID().c_str());
			SEISCOMP_LOG(_infoChannel, "... no match for %s and %s",
			             origin->publicID().c_str(), info->event->publicID().c_str());
			break;
		case Location:
			SEISCOMP_DEBUG("... time/location matches for %s and %s",
			               origin->publicID().c_str(), info->event->publicID().c_str());
			SEISCOMP_LOG(_infoChannel, "... time/location matches for %s and %s",
			             origin->publicID().c_str(), info->event->publicID().c_str());
			break;
		case Picks:
			SEISCOMP_DEBUG("... matching picks for %s and %s",
			               origin->publicID().c_str(), info->event->publicID().c_str());
			SEISCOMP_LOG(_infoChannel, "... matching picks for %s and %s",
			             origin->publicID().c_str(), info->event->publicID().c_str());
			break;
		case PicksAndLocation:
			SEISCOMP_DEBUG("... matching picks and time/location for %s and %s",
			               origin->publicID().c_str(), info->event->publicID().c_str());
			SEISCOMP_LOG(_infoChannel, "... matching picks and time/location for %s and %s",
			             origin->publicID().c_str(), info->event->publicID().c_str());
			break;
	}

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventInformationPtr EventTool::createEvent(Origin *origin) {
	string eventID = allocateEventID(query(), origin, _config.eventIDPrefix,
	                                 _config.eventIDPattern, &_config.blacklistIDs);

	if ( eventID.empty() ) {
		SEISCOMP_ERROR("Unable to allocate a new eventID, skipping origin %s\n"
		               "Hint: increase event slots with eventIDPattern parameter",
		               origin->publicID().c_str());
		SEISCOMP_LOG(_infoChannel, "Event created failed: unable to allocate a new EventID");
		return NULL;
	}
	else {
		if ( Event::Find(eventID) != NULL ) {
			SEISCOMP_ERROR("Unable to allocate a new eventID, skipping origin %s\n"
			               "Hint: increase event slots with eventIDPattern parameter",
			               origin->publicID().c_str());
			SEISCOMP_LOG(_infoChannel, "Event created failed: unable to allocate a new EventID");
			return NULL;
		}

		Time now = Time::GMT();
		logObject(_outputEvent, now);

		Notifier::Enable();

		EventInformationPtr info = new EventInformation(&_cache, &_config);
		info->event = new Event(eventID);

		CreationInfo ci;
		ci.setAgencyID(agencyID());
		ci.setAuthor(author());
		ci.setCreationTime(now);

		info->event->setCreationInfo(ci);
		info->created = true;

		cacheEvent(info);

		Notifier::Disable();

		return info;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventInformationPtr EventTool::findMatchingEvent(Origin *origin) {
	MatchResult bestResult = Nothing;
	EventInformationPtr bestInfo = NULL;
	EventMap::iterator it;

	for ( it = _events.begin(); it != _events.end(); ++it ) {
		MatchResult res = compare(it->second.get(), origin);
		if ( res > bestResult ) {
			bestResult = res;
			bestInfo = it->second;
		}
	}

	if ( bestInfo ) {
		SEISCOMP_DEBUG("... found best matching cached event %s (code: %d)",
		               bestInfo->event->publicID().c_str(), bestResult);
		SEISCOMP_LOG(_infoChannel, "... found best matching cached event %s (code: %d)",
		             bestInfo->event->publicID().c_str(), bestResult);
	}

	return bestInfo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventInformationPtr EventTool::findAssociatedEvent(DataModel::Origin *origin) {
	EventMap::iterator it;

	for ( it = _events.begin(); it != _events.end(); ++it ) {
		if ( it->second->event->originReference(origin->publicID()) != NULL ) {
			SEISCOMP_DEBUG("... feeding cache with event %s",
			               it->second->event->publicID().c_str());
			_cache.feed(it->second->event.get());
			return it->second;
		}
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventInformationPtr EventTool::findAssociatedEvent(DataModel::FocalMechanism *fm) {
	EventMap::iterator it;

	for ( it = _events.begin(); it != _events.end(); ++it ) {
		if ( it->second->event->focalMechanismReference(fm->publicID()) != NULL ) {
			SEISCOMP_DEBUG("... feeding cache with event %s",
			               it->second->event->publicID().c_str());
			_cache.feed(it->second->event.get());
			return it->second;
		}
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Magnitude *EventTool::preferredMagnitude(Origin *origin) {
	int goodCount = 0;
	int goodPriority = 0;
	int fallbackCount = 0;
	int fallbackPriority = 0;
	Magnitude *goodMag = NULL;
	Magnitude *fallbackMag = NULL;

	int mbcount = 0;
	double mbval = 0.0;
	for ( size_t i = 0; i < origin->magnitudeCount(); ++i ) {
		Magnitude *mag = origin->magnitude(i);
		if ( isAgencyIDBlocked(objectAgencyID(mag)) ) continue;
		if ( mag->type() == "mb" ) {
			if ( mag->magnitude().value() > mbval ) {
				mbval = mag->magnitude().value();
				mbcount = stationCount(mag);
			}
		}
	}

	for ( size_t i = 0; i < origin->magnitudeCount(); ++i ) {
		try {
			Magnitude *mag = origin->magnitude(i);
			if ( isAgencyIDBlocked(objectAgencyID(mag)) ) continue;

			int priority = goodness(mag, mbcount, mbval, _config);
			if ( priority <= 0 )
				continue;

			if ( isMw(mag) ) {
				if ( (stationCount(mag) > goodCount)
				  || ((stationCount(mag) == goodCount) && (priority > goodPriority)) ) {
					goodPriority = priority;
					goodCount = stationCount(mag);
					goodMag = mag;
				}
			}
			else if ( (stationCount(mag) > fallbackCount)
			       || ((stationCount(mag) == fallbackCount) && (priority > fallbackPriority)) ) {
				fallbackPriority = priority;
				fallbackCount = stationCount(mag);
				fallbackMag = mag;
			}
		}
		catch ( ValueException& ) {
			continue;
		}
	}

	if ( goodMag )
		return goodMag;

	if ( !fallbackMag && _config.enableFallbackPreferredMagnitude ) {
		// Find the network magnitude with the most station magnitudes according the
		// magnitude priority and set this magnitude preferred
		fallbackCount = 0;
		fallbackPriority = 0;

		for ( size_t i = 0; i < origin->magnitudeCount(); ++i ) {
			Magnitude *mag = origin->magnitude(i);
			if ( isAgencyIDBlocked(objectAgencyID(mag)) ) continue;

			int prio = magnitudePriority(mag->type(), _config);
			if ( (stationCount(mag) > fallbackCount)
			  || (stationCount(mag) == fallbackCount && prio > fallbackPriority ) ) {
				fallbackPriority = prio;
				fallbackCount = stationCount(mag);
				fallbackMag = mag;
			}
		}
	}

	return fallbackMag;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Event *EventTool::getEventForOrigin(const std::string &originID) {
	EventMap::iterator it;
	for ( it = _events.begin(); it != _events.end(); ++it ) {
		Event *evt = it->second->event.get();
		if ( !evt ) continue;
		if ( evt->originReference(originID) != NULL ) return evt;
	}

	return Event::Cast(query()->getEvent(originID));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Event *EventTool::getEventForFocalMechanism(const std::string &fmID) {
	EventMap::iterator it;
	for ( it = _events.begin(); it != _events.end(); ++it ) {
		Event *evt = it->second->event.get();
		if ( !evt ) continue;
		if ( evt->focalMechanismReference(fmID) != NULL ) return evt;
	}

	return Event::Cast(query()->getEventForFocalMechanism(fmID));
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTool::cacheEvent(EventInformationPtr info) {
	SEISCOMP_DEBUG("... caching event %s",
	               info->event->publicID().c_str());

	// Cache the complete event information
	_events[info->event->publicID()] = info;
	// Set the clean-up flag to false
	info->aboutToBeRemoved = false;
	// Add the event to the EventParameters
	if ( info->event->eventParameters() == NULL )
		_ep->add(info->event.get());
	// Feed event into cache
	_cache.feed(info->event.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventTool::isEventCached(const string &eventID) const {
	return _events.find(eventID) != _events.end();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventInformationPtr EventTool::cachedEvent(const std::string &eventID) {
	EventMap::const_iterator it = _events.find(eventID);
	if ( it == _events.end() ) return NULL;

	// If scheduled for removal, reset and cache it again
	if ( it->second->aboutToBeRemoved ) {
		// Reset removal flag
		it->second->aboutToBeRemoved = false;
		// Add it again to event parameters
		_ep->add(it->second->event.get());
		// Feed the cache again
		_cache.feed(it->second->event.get());
	}

	return it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventTool::removeCachedEvent(const std::string &eventID) {
	EventMap::iterator it = _events.find(eventID);
	if ( it != _events.end() ) {
		_events.erase(it);
		return true;
	}
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTool::choosePreferred(EventInformation *info, Origin *origin,
                                DataModel::Magnitude *triggeredMag,
                                bool realOriginUpdate) {
	Magnitude *mag = NULL;
	MagnitudePtr momentMag;

	SEISCOMP_DEBUG("%s: [choose preferred origin/magnitude(%s)]",
	               info->event->publicID().c_str(),
	               origin->publicID().c_str());

	// Is originID blacklisted, e.g. a derived origin of a moment tensor
	if ( _originBlackList.find(origin->publicID()) != _originBlackList.end() ) {
		SEISCOMP_DEBUG("%s: skip setting preferredOriginID, origin %s is blacklisted",
		               info->event->publicID().c_str(), origin->publicID().c_str());
		SEISCOMP_LOG(_infoChannel, "Origin %s cannot be set preferred: blacklisted",
		             origin->publicID().c_str());
		return;
	}

	if ( info->constraints.fixMw && info->preferredFocalMechanism ) {
		for ( size_t i = 0; i < info->preferredFocalMechanism->momentTensorCount(); ++i ) {
			MomentTensor *mt = info->preferredFocalMechanism->momentTensor(i);
			momentMag = _cache.get<Magnitude>(mt->momentMagnitudeID());
			if ( momentMag == NULL ) {
				SEISCOMP_WARNING("Moment magnitude with id '%s' not found",
				                 mt->momentMagnitudeID().c_str());
				continue;
			}

			mag = momentMag.get();
			SEISCOMP_DEBUG("... found preferred Mw %s", mag->publicID().c_str());

			// Just take the first moment tensor object
			break;
		}
	}

	// Special magnitude type requested?
	if ( mag == NULL && !info->constraints.preferredMagnitudeType.empty() ) {
		SEISCOMP_DEBUG("... requested preferred magnitude type is %s",
		               info->constraints.preferredMagnitudeType.c_str());
		for ( size_t i = 0; i < origin->magnitudeCount(); ++i ) {
			Magnitude *nm = origin->magnitude(i);
			if ( nm->type() == info->constraints.preferredMagnitudeType ) {
				SEISCOMP_DEBUG("... found magnitude %s with requested type",
				               nm->publicID().c_str());
				mag = nm;
				break;
			}
		}
	}

	// No magnitude found -> go on using the normal procedure
	if ( mag == NULL ) {
		SEISCOMP_DEBUG("... looking for preferred magnitude in origin %s",
		               origin->publicID().c_str());
		mag = preferredMagnitude(origin);
	}

	bool update = false;

	if ( !info->preferredOrigin ) {
		if ( isAgencyIDAllowed(objectAgencyID(origin)) || info->constraints.fixOrigin(origin) ) {
			info->event->setPreferredOriginID(origin->publicID());

			SEISCOMP_INFO("%s: set first preferredOriginID to %s",
			              info->event->publicID().c_str(), origin->publicID().c_str());
			SEISCOMP_LOG(_infoChannel, "Origin %s has been set preferred in event %s",
			             origin->publicID().c_str(), info->event->publicID().c_str());

			updateRegionName(info->event.get(), origin);

			info->preferredOrigin = origin;
			update = true;
		}
		else {
			if ( !isAgencyIDAllowed(objectAgencyID(origin)) ) {
				SEISCOMP_DEBUG("%s: skip setting first preferredOriginID, agencyID '%s' is blocked",
				               info->event->publicID().c_str(), objectAgencyID(origin).c_str());
				SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred: agencyID %s is blocked",
				             origin->publicID().c_str(), objectAgencyID(origin).c_str());
			}
			else {
				SEISCOMP_DEBUG("%s: skip setting first preferredOriginID, preferredOriginID is fixed to '%s'",
				               info->event->publicID().c_str(), info->constraints.preferredOriginID.c_str());
				SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred: preferredOriginID is fixed to %s",
				             origin->publicID().c_str(), info->constraints.preferredOriginID.c_str());
			}
			return;
		}

		if ( mag ) {
			info->event->setPreferredMagnitudeID(mag->publicID());
			info->preferredMagnitude = mag;
			SEISCOMP_INFO("%s: set first preferredMagnitudeID to %s",
			              info->event->publicID().c_str(), mag->publicID().c_str());
			SEISCOMP_LOG(_infoChannel, "Magnitude %s has been set preferred in event %s",
			             mag->publicID().c_str(), info->event->publicID().c_str());
			update = true;
		}
	}
	else if ( info->preferredOrigin->publicID() != origin->publicID() ) {
		SEISCOMP_DEBUG("... checking whether origin %s can become preferred",
		               origin->publicID().c_str());

		update = true;

		// Fixed origin => check if the passed origin is the fixed one
		if ( info->constraints.fixedOrigin() ) {
			if ( !info->constraints.fixOrigin(origin) ) {
				SEISCOMP_DEBUG("... skipping potential preferred origin, origin '%s' is fixed",
				               info->constraints.preferredOriginID.c_str());
				SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred in event %s: origin %s is fixed",
				             origin->publicID().c_str(), info->event->publicID().c_str(),
				             info->constraints.preferredOriginID.c_str());
				return;
			}
			else {
				SEISCOMP_LOG(_infoChannel, "Origin %s is fixed as preferred origin",
				             origin->publicID().c_str());
				SEISCOMP_DEBUG("... incoming origin is fixed");
			}
		}
		// No fixed origin => select it using the automatic rules
		else {

			if ( isAgencyIDBlocked(objectAgencyID(origin)) ) {
				SEISCOMP_DEBUG("... skipping potential preferred origin, agencyID '%s' is blocked",
				               objectAgencyID(origin).c_str());
				SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred in event %s: agencyID %s is blocked",
				             origin->publicID().c_str(), info->event->publicID().c_str(),
				             objectAgencyID(origin).c_str());
				return;
			}

			if ( !_config.priorities.empty() ) {
				bool allowBadMagnitude = false;

				// Run through the priority list and check the values
				for ( Config::StringList::iterator it = _config.priorities.begin();
				      it != _config.priorities.end(); ++it ) {
					if ( *it == "AGENCY" ) {
						int originAgencyPriority = agencyPriority(objectAgencyID(origin), _config);
						int preferredOriginAgencyPriority = agencyPriority(objectAgencyID(info->preferredOrigin.get()), _config);

						if ( originAgencyPriority < preferredOriginAgencyPriority ) {
							SEISCOMP_DEBUG("... skipping potential preferred origin, priority of agencyID '%s' is too low",
							               objectAgencyID(origin).c_str());
							SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred in event %s: priority of agencyID %s is too low",
							             origin->publicID().c_str(), info->event->publicID().c_str(),
							             objectAgencyID(origin).c_str());
							return;
						}
						// Found origin with higher agency priority
						else if ( originAgencyPriority > preferredOriginAgencyPriority ) {
							SEISCOMP_DEBUG("... agencyID '%s' overrides current agencyID '%s'",
							               objectAgencyID(origin).c_str(), objectAgencyID(info->preferredOrigin.get()).c_str());
							SEISCOMP_LOG(_infoChannel, "Origin %s: agencyID '%s' overrides agencyID '%s'",
							             origin->publicID().c_str(), objectAgencyID(origin).c_str(), objectAgencyID(info->preferredOrigin.get()).c_str());

							allowBadMagnitude = true;
							break;
						}
					}
					else if ( *it == "AUTHOR" ) {
						int originAuthorPriority = authorPriority(objectAuthor(origin), _config);
						int preferredOriginAuthorPriority = authorPriority(objectAuthor(info->preferredOrigin.get()), _config);

						if ( originAuthorPriority < preferredOriginAuthorPriority ) {
							SEISCOMP_DEBUG("... skipping potential preferred origin, priority of author '%s' is too low",
							               objectAuthor(origin).c_str());
							SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred in event %s: priority of author %s is too low",
							             origin->publicID().c_str(), info->event->publicID().c_str(),
							             objectAuthor(origin).c_str());
							return;
						}
						// Found origin with higher author priority
						else if ( originAuthorPriority > preferredOriginAuthorPriority ) {
							SEISCOMP_DEBUG("... author '%s' overrides current author '%s'",
							               objectAuthor(origin).c_str(), objectAuthor(info->preferredOrigin.get()).c_str());
							SEISCOMP_LOG(_infoChannel, "Origin %s: author '%s' overrides author '%s'",
							             origin->publicID().c_str(), objectAuthor(origin).c_str(), objectAuthor(info->preferredOrigin.get()).c_str());
							break;
						}
					}
					else if ( *it == "MODE" ) {
						int originPriority = modePriority(origin);
						int preferredOriginPriority = modePriority(info->preferredOrigin.get());

						if ( info->constraints.fixOriginMode(info->preferredOrigin.get()) ) {
							SEISCOMP_LOG(_infoChannel, "Origin %s: has priority %d vs %d",
							             origin->publicID().c_str(), originPriority, preferredOriginPriority);

							// Set back the evalmode to automatic if a higher priority
							// origin has been send (but not triggered by a magnitude change only)
							if ( realOriginUpdate && (originPriority > preferredOriginPriority) ) {
								SEISCOMP_LOG(_infoChannel, "Origin %s has higher priority: releasing EvPrefOrgEvalMode",
								             origin->publicID().c_str());
								JournalEntryPtr entry = new JournalEntry;
								entry->setObjectID(info->event->publicID());
								entry->setAction("EvPrefOrgEvalMode");
								entry->setParameters("");
								entry->setSender(name() + "@" + Core::getHostname());
								entry->setCreated(Core::Time::GMT());
								Notifier::Create(_journal->publicID(), OP_ADD, entry.get());
								info->addJournalEntry(entry.get());
							}
							else
								preferredOriginPriority = ORIGIN_PRIORITY_MAX;
						}

						if ( info->constraints.fixOriginMode(origin) )
							originPriority = ORIGIN_PRIORITY_MAX;

						if ( originPriority < preferredOriginPriority ) {
							SEISCOMP_DEBUG("... skipping potential preferred origin (%d < %d)",
							               originPriority, preferredOriginPriority);
							SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred in event %s: priority too low (%d < %d)",
							             origin->publicID().c_str(), info->event->publicID().c_str(),
							             originPriority, preferredOriginPriority);
							return;
						}
						// Found origin with higher status priority
						else if ( originPriority > preferredOriginPriority ) {
							SEISCOMP_LOG(_infoChannel, "Origin %s: status priority %d overrides status priority %d",
							             origin->publicID().c_str(), originPriority, preferredOriginPriority);
							break;
						}
					}
					else if ( *it == "STATUS" ) {
						int originPriority = priority(origin);
						int preferredOriginPriority = priority(info->preferredOrigin.get());

						if ( info->constraints.fixOriginMode(info->preferredOrigin.get()) ) {
							SEISCOMP_LOG(_infoChannel, "Origin %s: has priority %d vs %d",
							             origin->publicID().c_str(), originPriority, preferredOriginPriority);

							// Set back the evalmode to automatic if a higher priority
							// origin has been send (but not triggered by a magnitude change only)
							if ( realOriginUpdate && (originPriority > preferredOriginPriority) ) {
								SEISCOMP_LOG(_infoChannel, "Origin %s has higher priority: releasing EvPrefOrgEvalMode",
								             origin->publicID().c_str());
								JournalEntryPtr entry = new JournalEntry;
								entry->setObjectID(info->event->publicID());
								entry->setAction("EvPrefOrgEvalMode");
								entry->setParameters("");
								entry->setSender(name() + "@" + Core::getHostname());
								entry->setCreated(Core::Time::GMT());
								Notifier::Create(_journal->publicID(), OP_ADD, entry.get());
								info->addJournalEntry(entry.get());
							}
							else
								preferredOriginPriority = ORIGIN_PRIORITY_MAX;
						}

						if ( info->constraints.fixOriginMode(origin) )
							originPriority = ORIGIN_PRIORITY_MAX;

						if ( originPriority < preferredOriginPriority ) {
							SEISCOMP_DEBUG("... skipping potential preferred origin (%d < %d)",
							               originPriority, preferredOriginPriority);
							SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred in event %s: priority too low (%d < %d)",
							             origin->publicID().c_str(), info->event->publicID().c_str(),
							             originPriority, preferredOriginPriority);
							return;
						}
						// Found origin with higher status priority
						else if ( originPriority > preferredOriginPriority ) {
							SEISCOMP_LOG(_infoChannel, "Origin %s: status priority %d overrides status priority %d",
							             origin->publicID().c_str(), originPriority, preferredOriginPriority);
							break;
						}
					}
					else if ( *it == "METHOD" ) {
						int originMethodPriority = methodPriority(origin->methodID(), _config);
						int preferredOriginMethodPriority = methodPriority(info->preferredOrigin->methodID(), _config);

						if ( originMethodPriority < preferredOriginMethodPriority ) {
							SEISCOMP_DEBUG("... skipping potential preferred origin, priority of method '%s' is too low",
							               origin->methodID().c_str());
							SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred in event %s: priority of method %s is too low",
							             origin->publicID().c_str(), info->event->publicID().c_str(),
							             origin->methodID().c_str());
							return;
						}
						// Found origin with higher method priority
						else if ( originMethodPriority > preferredOriginMethodPriority ) {
							SEISCOMP_DEBUG("... methodID '%s' overrides current methodID '%s'",
							               origin->methodID().c_str(), info->preferredOrigin->methodID().c_str());
							SEISCOMP_LOG(_infoChannel, "Origin %s: methodID '%s' overrides methodID '%s'",
							             origin->publicID().c_str(), origin->methodID().c_str(),
							             info->preferredOrigin->methodID().c_str());
							break;
						}
					}
					else if ( *it == "PHASES" ) {
						int originPhaseCount = definingPhaseCount(origin);
						int preferredOriginPhaseCount = definingPhaseCount(info->preferredOrigin.get());
						if ( originPhaseCount < preferredOriginPhaseCount ) {
							SEISCOMP_DEBUG("... skipping potential preferred automatic origin, phaseCount too low");
							SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred in event %s: phaseCount too low (%d < %d)",
							             origin->publicID().c_str(), info->event->publicID().c_str(),
							             originPhaseCount, preferredOriginPhaseCount);
							return;
						}
						else if ( originPhaseCount > preferredOriginPhaseCount ) {
							SEISCOMP_LOG(_infoChannel, "Origin %s: phaseCount %d overrides phaseCount %d",
							             origin->publicID().c_str(), originPhaseCount,
							             preferredOriginPhaseCount);
							break;
						}
					}
					else if ( *it == "PHASES_AUTOMATIC" ) {
						EvaluationMode status = AUTOMATIC;
						try {
							status = origin->evaluationMode();
						}
						catch ( ValueException& ) {}

						// Make a noop in case of non automatic origins
						if ( status != AUTOMATIC ) continue;

						int originPhaseCount = definingPhaseCount(origin);
						int preferredOriginPhaseCount = definingPhaseCount(info->preferredOrigin.get());
						if ( originPhaseCount < preferredOriginPhaseCount ) {
							SEISCOMP_DEBUG("... skipping potential preferred automatic origin, phaseCount too low");
							SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred in event %s: phaseCount too low (%d < %d)",
							             origin->publicID().c_str(), info->event->publicID().c_str(),
							             originPhaseCount, preferredOriginPhaseCount);
							return;
						}
						else if ( originPhaseCount > preferredOriginPhaseCount ) {
							SEISCOMP_LOG(_infoChannel, "Origin %s: phaseCount (automatic) %d overrides phaseCount %d",
							             origin->publicID().c_str(), originPhaseCount,
							             preferredOriginPhaseCount);
							break;
						}
					}
					else if ( *it == "RMS" ) {
						double originRMS = rms(origin);
						double preferredOriginRMS = rms(info->preferredOrigin.get());

						// Both RMS attributes are set: check priority
						if ( (originRMS >= 0) && (preferredOriginRMS >= 0) && (originRMS > preferredOriginRMS) ) {
							SEISCOMP_DEBUG("... skipping potential preferred automatic origin, RMS too high");
							SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred in event %s: RMS too high (%.1f > %.1f",
							             origin->publicID().c_str(), info->event->publicID().c_str(),
							             originRMS, preferredOriginRMS);
							return;
						}
						// Both RMS attribute are set: check priority
						else if ( (originRMS >= 0) && (preferredOriginRMS >= 0) && (originRMS < preferredOriginRMS) ) {
							SEISCOMP_LOG(_infoChannel, "Origin %s: RMS %.1f overrides RMS %.1f",
							             origin->publicID().c_str(), originRMS,
							             preferredOriginRMS);
							break;
						}
						// Incoming RMS is not set but preferred origin has RMS: skip incoming
						else if ( (originRMS < 0) && (preferredOriginRMS >= 0) ) {
							SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred in event %s: RMS not set",
							             origin->publicID().c_str(), info->event->publicID().c_str());
							return;
						}
						// Incoming RMS is set but preferred origin has no RMS: prioritize incoming
						else if ( (originRMS >= 0) && (preferredOriginRMS < 0) ) {
							SEISCOMP_LOG(_infoChannel, "Origin %s: RMS %.1f overrides current unset preferred RMS",
							             origin->publicID().c_str(), originRMS);
							break;
						}
					}
					else if ( *it == "RMS_AUTOMATIC" ) {
						EvaluationMode status = AUTOMATIC;
						try {
							status = origin->evaluationMode();
						}
						catch ( ValueException& ) {}

						// Make a noop in case of non automatic origins
						if ( status != AUTOMATIC ) continue;

						double originRMS = rms(origin);
						double preferredOriginRMS = rms(info->preferredOrigin.get());

						// Both RMS attributes are set: check priority
						if ( (originRMS >= 0) && (preferredOriginRMS >= 0) && (originRMS > preferredOriginRMS) ) {
							SEISCOMP_DEBUG("... skipping potential preferred automatic origin, RMS too high");
							SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred in event %s: RMS too high (%.1f > %.1f",
							             origin->publicID().c_str(), info->event->publicID().c_str(),
							             originRMS, preferredOriginRMS);
							return;
						}
						// Both RMS attribute are set: check priority
						else if ( (originRMS >= 0) && (preferredOriginRMS >= 0) && (originRMS < preferredOriginRMS) ) {
							SEISCOMP_LOG(_infoChannel, "Origin %s: RMS %.1f overrides RMS %.1f",
							             origin->publicID().c_str(), originRMS,
							             preferredOriginRMS);
							break;
						}
						// Incoming RMS is not set but preferred origin has RMS: skip incoming
						else if ( (originRMS < 0) && (preferredOriginRMS >= 0) ) {
							SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred in event %s: RMS not set",
							             origin->publicID().c_str(), info->event->publicID().c_str());
							return;
						}
						// Incoming RMS is set but preferred origin has no RMS: prioritize incoming
						else if ( (originRMS >= 0) && (preferredOriginRMS < 0) ) {
							SEISCOMP_LOG(_infoChannel, "Origin %s: RMS %.1f overrides current unset preferred RMS",
							             origin->publicID().c_str(), originRMS);
							break;
						}
					}
					else if ( *it == "TIME" ) {
						Core::Time originCreationTime = created(origin);
						Core::Time preferredOriginCreationTime = created(info->preferredOrigin.get());
						if ( originCreationTime < preferredOriginCreationTime ) {
							SEISCOMP_DEBUG("... skipping potential preferred origin, there is a better one created later");
							return;
						}
						else if ( originCreationTime > preferredOriginCreationTime ) {
							SEISCOMP_LOG(_infoChannel, "Origin %s: %s is more recent than %s",
							             origin->publicID().c_str(), originCreationTime.iso().c_str(),
							             preferredOriginCreationTime.iso().c_str());
							break;
						}
					}
					else if ( *it == "TIME_AUTOMATIC" ) {
						EvaluationMode status = AUTOMATIC;
						try {
							status = origin->evaluationMode();
						}
						catch ( ValueException& ) {}

						// Make a noop in case of non automatic origins
						if ( status != AUTOMATIC ) continue;

						Core::Time originCreationTime = created(origin);
						Core::Time preferredOriginCreationTime = created(info->preferredOrigin.get());
						if ( originCreationTime < preferredOriginCreationTime ) {
							SEISCOMP_DEBUG("... skipping potential preferred origin, there is a better one created later");
							return;
						}
						else if ( originCreationTime > preferredOriginCreationTime ) {
							SEISCOMP_LOG(_infoChannel, "Origin %s: %s (automatic) is more recent than %s",
							             origin->publicID().c_str(), originCreationTime.iso().c_str(),
							             preferredOriginCreationTime.iso().c_str());
							break;
						}
					}
					else if ( *it == "SCORE" ) {
						double score = _score->evaluate(origin);
						double preferredScore = _score->evaluate(info->preferredOrigin.get());
						if ( score < preferredScore ) {
							SEISCOMP_DEBUG("... skipping potential preferred origin, there is one with higher score: %f > %f",
							               preferredScore, score);
							return;
						}
						else if ( score > preferredScore ) {
							SEISCOMP_LOG(_infoChannel, "Origin %s: score of %f is larger than %f",
							             origin->publicID().c_str(), score,
							             preferredScore);
							break;
						}
					}
				}

				// Agency priority is a special case and an origin can become preferred without
				// a preferred magnitude if the agencyID has higher priority
				if ( !allowBadMagnitude ) {
					// The current origin has no preferred magnitude yet but
					// the event already has => ignore the origin for now
					if ( info->preferredMagnitude && !mag ) {
						SEISCOMP_DEBUG("... skipping potential preferred origin, no preferred magnitude");
						SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred in event %s: no preferrable magnitude",
						             origin->publicID().c_str(), info->event->publicID().c_str());
						return;
					}
				}
			}
			else {
				int originAgencyPriority = agencyPriority(objectAgencyID(origin), _config);
				int preferredOriginAgencyPriority = agencyPriority(objectAgencyID(info->preferredOrigin.get()), _config);

				if ( originAgencyPriority < preferredOriginAgencyPriority ) {
					SEISCOMP_DEBUG("... skipping potential preferred origin, priority of agencyID '%s' is too low",
					               objectAgencyID(origin).c_str());
					SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred in event %s: priority of agencyID %s is too low",
					             origin->publicID().c_str(), info->event->publicID().c_str(),
					             objectAgencyID(origin).c_str());
					return;
				}

				// Same agency priorities -> compare origin priority
				if ( originAgencyPriority == preferredOriginAgencyPriority ) {
					int originPriority = priority(origin);
					int preferredOriginPriority = priority(info->preferredOrigin.get());

					if ( info->constraints.fixOriginMode(info->preferredOrigin.get()) ) {
						SEISCOMP_LOG(_infoChannel, "Origin %s: has priority %d vs %d",
						             origin->publicID().c_str(), originPriority, preferredOriginPriority);

						// Set back the evalmode to automatic if a higher priority
						// origin has been send (but not triggered by a magnitude change only)
						if ( realOriginUpdate && (originPriority > preferredOriginPriority) ) {
							SEISCOMP_LOG(_infoChannel, "Origin %s has higher priority: releasing EvPrefOrgEvalMode",
							             origin->publicID().c_str());
							JournalEntryPtr entry = new JournalEntry;
							entry->setObjectID(info->event->publicID());
							entry->setAction("EvPrefOrgEvalMode");
							entry->setParameters("");
							entry->setSender(name() + "@" + Core::getHostname());
							entry->setCreated(Core::Time::GMT());
							Notifier::Create(_journal->publicID(), OP_ADD, entry.get());
							info->addJournalEntry(entry.get());
						}
						else
							preferredOriginPriority = ORIGIN_PRIORITY_MAX;
					}

					if ( info->constraints.fixOriginMode(origin) )
						originPriority = ORIGIN_PRIORITY_MAX;

					if ( originPriority < preferredOriginPriority ) {
						SEISCOMP_DEBUG("... skipping potential preferred origin (%d < %d)",
						               originPriority, preferredOriginPriority);
						SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred in event %s: priority too low (%d < %d)",
						             origin->publicID().c_str(), info->event->publicID().c_str(),
						             originPriority, preferredOriginPriority);
						return;
					}

					// The current origin has no preferred magnitude yet but
					// the event already has => ignore the origin for now
					if ( info->preferredMagnitude && !mag ) {
						SEISCOMP_DEBUG("... skipping potential preferred origin, no preferred magnitude");
						SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred in event %s: no preferrable magnitude",
						             origin->publicID().c_str(), info->event->publicID().c_str());
						return;
					}

					if ( originPriority == preferredOriginPriority ) {
						EvaluationMode status = AUTOMATIC;
						try {
							status = origin->evaluationMode();
						}
						catch ( ValueException& ) {}

						if ( status == AUTOMATIC ) {
							SEISCOMP_DEBUG("... same priority and mode is AUTOMATIC");

							originPriority = definingPhaseCount(origin);
							preferredOriginPriority = definingPhaseCount(info->preferredOrigin.get());
							if ( originPriority < preferredOriginPriority ) {
								SEISCOMP_DEBUG("... skipping potential preferred automatic origin, phaseCount too low");
								SEISCOMP_LOG(_infoChannel, "Origin %s has not been set preferred in event %s: priorities are equal (%d) but phaseCount too low (%d < %d)",
								             origin->publicID().c_str(), info->event->publicID().c_str(), originPriority,
								             definingPhaseCount(origin), definingPhaseCount(info->preferredOrigin.get()));
								return;
							}

							/*
							if ( created(origin) < created(info->preferredOrigin.get()) ) {
								SEISCOMP_DEBUG("... skipping potential preferred origin, there is a better one created later");
								return;
							}
							*/
						}

						if ( originPriority == preferredOriginPriority ) {
							if ( created(origin) < created(info->preferredOrigin.get()) ) {
								SEISCOMP_DEBUG("... skipping potential preferred origin, there is a better one created later");
								SEISCOMP_LOG(_infoChannel, "Origin %s: skipped potential preferred origin, there is a better one created later",
								             origin->publicID().c_str());
								return;
							}
						}
					}
					else {
						SEISCOMP_DEBUG("... priority %d overrides current prioriy %d",
						               originPriority, preferredOriginPriority);
						SEISCOMP_LOG(_infoChannel, "Origin %s: priority %d overrides priority %d",
						             origin->publicID().c_str(), originPriority, preferredOriginPriority);
					}
				}
				else {
					SEISCOMP_DEBUG("... agencyID '%s' overrides current agencyID '%s'",
					               objectAgencyID(origin).c_str(), objectAgencyID(info->preferredOrigin.get()).c_str());
					SEISCOMP_LOG(_infoChannel, "Origin %s: agencyID '%s' overrides agencyID '%s'",
					             origin->publicID().c_str(), objectAgencyID(origin).c_str(), objectAgencyID(info->preferredOrigin.get()).c_str());
				}
			}
		}

		info->event->setPreferredOriginID(origin->publicID());

		SEISCOMP_INFO("%s: set preferredOriginID to %s",
		              info->event->publicID().c_str(), origin->publicID().c_str());
		SEISCOMP_LOG(_infoChannel, "Origin %s has been set preferred in event %s",
		             origin->publicID().c_str(), info->event->publicID().c_str());

		updateRegionName(info->event.get(), origin);

		info->preferredOrigin = origin;

		if ( mag ) {
			if ( info->event->preferredMagnitudeID() != mag->publicID() ) {
				update = true;

				info->event->setPreferredMagnitudeID(mag->publicID());
				info->preferredMagnitude = mag;
				SEISCOMP_INFO("%s: set preferredMagnitudeID to %s",
				              info->event->publicID().c_str(), mag->publicID().c_str());
				SEISCOMP_LOG(_infoChannel, "Magnitude %s has been set preferred in event %s",
				             mag->publicID().c_str(), info->event->publicID().c_str());
			}
		}
		else {
			if ( !info->event->preferredMagnitudeID().empty() ) {
				update = true;

				info->event->setPreferredMagnitudeID("");
				info->preferredMagnitude = mag;
				SEISCOMP_INFO("%s: set preferredMagnitudeID to ''",
				              info->event->publicID().c_str());
				SEISCOMP_LOG(_infoChannel, "Set empty preferredMagnitudeID in event %s",
				             info->event->publicID().c_str());
			}
		}
	}
	else if ( mag && info->event->preferredMagnitudeID() != mag->publicID() ) {
		info->event->setPreferredMagnitudeID(mag->publicID());
		info->preferredMagnitude = mag;
		SEISCOMP_INFO("%s: set preferredMagnitudeID to %s",
		              info->event->publicID().c_str(), mag->publicID().c_str());
		SEISCOMP_INFO("%s: set preferredMagnitudeID to %s",
		              info->event->publicID().c_str(), mag->publicID().c_str());
		update = true;
	}
	else {
		SEISCOMP_INFO("%s: nothing to do", info->event->publicID().c_str());
	}

	bool callProcessors = true;

	// If only the magnitude changed, call updated processors
	if ( !update && triggeredMag && !realOriginUpdate &&
	     triggeredMag->publicID() == info->event->preferredMagnitudeID() ) {
		// Call registered processors
		EventProcessors::iterator it;
		for ( it = _processors.begin(); it != _processors.end(); ++it ) {
			if ( it->second->process(info->event.get()) )
				update = true;
		}

		// Processors have been called already
		callProcessors = false;
	}

	// If an preferred origin is set and the event type has not been fixed
	// manually set it to 'not existing' if the preferred origin is rejected.
	if ( _config.setAutoEventTypeNotExisting &&
	     info->preferredOrigin &&
	     !info->constraints.fixType ) {
		bool isRejected = false;
		bool notExistingEvent = false;

		try {
			if ( info->preferredOrigin->evaluationStatus() == REJECTED )
				isRejected = true;
		}
		catch ( ... ) {}

		try {
			notExistingEvent = info->event->type() == NOT_EXISTING;
		}
		catch ( ... ) {}

		if ( isRejected ) {
			SEISCOMP_INFO("%s: preferred origin is rejected",
			              info->event->publicID().c_str());

			// User has manually fixed an origin, don't touch the event type
			if ( !info->constraints.fixedOrigin() && !notExistingEvent ) {
				SEISCOMP_INFO("%s: set type to 'not existing' since preferred origin is rejected",
				              info->event->publicID().c_str());
				SEISCOMP_LOG(_infoChannel, "Set type to 'not existing' since preferred origin is rejected in event %s",
				             info->event->publicID().c_str());
				info->event->setType(EventType(NOT_EXISTING));
				update = true;
			}
		}
		else {
			// User has manually fixed an origin, don't touch the event type
			// Preferred origin is not rejected, remove the event type if it is
			// set to 'not existing'
			if ( !info->constraints.fixedOrigin() && notExistingEvent ) {
				SEISCOMP_INFO("%s: remove type since preferred origin changed to not rejected",
				              info->event->publicID().c_str());
				SEISCOMP_LOG(_infoChannel, "Remove type since preferred origin is changed from rejected in event %s",
				             info->event->publicID().c_str());
				info->event->setType(None);
				update = true;
			}
		}
	}

	if ( update ) {
		SEISCOMP_DEBUG("%s: update (created: %d, notifiers enabled: %d)",
		               info->event->publicID().c_str(), info->created,
		               Notifier::IsEnabled());

		if ( !info->created )
			updateEvent(info->event.get(), callProcessors);
		else {
			info->created = false;

			// Call registered processors
			EventProcessors::iterator it;
			for ( it = _processors.begin(); it != _processors.end(); ++it )
				it->second->process(info->event.get());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTool::choosePreferred(EventInformation *info, DataModel::FocalMechanism *fm) {
	SEISCOMP_DEBUG("%s: [choose preferred focalmechanism(%s)]",
	               info->event->publicID().c_str(),
	               fm->publicID().c_str());

	if ( _config.delayPrefFocMech > 0 ) {
		if ( !info->preferredOrigin ) {
			SEISCOMP_DEBUG("No preferred origins set for event %s, cannot compute delay time, ignoring focal mechanism %s",
			               info->event->publicID().c_str(), fm->publicID().c_str());
			SEISCOMP_LOG(_infoChannel, "No preferred origins set for event %s, cannot compute delay time, ignoring focal mechanism %s",
			             info->event->publicID().c_str(), fm->publicID().c_str());
			return;
		}

		Core::Time minTime = info->preferredOrigin->time().value() + Core::TimeSpan(_config.delayPrefFocMech,0);
		Core::Time now = Core::Time::GMT();

		//SEISCOMP_LOG(_infoChannel, "Time to reach to set focal mechanism preferred is %s, now is %s",
		//             minTime.toString("%FT%T").c_str(), now.toString("%FT%T").c_str());

		if ( minTime > now ) {
			int secs = (minTime - now).seconds() + 1;
			SEISCOMP_DEBUG("Setting preferred focal mechanism needs "
			               "to be delayed by config, still %d secs to go for event %s",
			               secs, info->event->publicID().c_str());
			SEISCOMP_LOG(_infoChannel, "Setting preferred focal mechanism needs "
			             "to be delayed by config, still %d secs to go for event %s",
			             secs, info->event->publicID().c_str());

			if ( !hasDelayedEvent(info->event->publicID(), SetPreferredFM) )
				_delayEventBuffer.push_back(DelayedEventUpdate(info->event->publicID(), secs, SetPreferredFM));

			return;
		}
	}

	bool update = false;

	if ( !info->preferredFocalMechanism ) {
		if ( isAgencyIDAllowed(objectAgencyID(fm)) || info->constraints.fixFocalMechanism(fm) ) {
			if ( isRejected(fm) ) {
				SEISCOMP_DEBUG("%s: skip setting first preferredFocalMechanismID, '%s' is rejected",
				               info->event->publicID().c_str(), fm->publicID().c_str());
				SEISCOMP_LOG(_infoChannel, "FocalMechanism %s has not been set preferred: status is REJECTED",
				             fm->publicID().c_str());
				return;
			}

			info->event->setPreferredFocalMechanismID(fm->publicID());

			info->preferredFocalMechanism = fm;
			SEISCOMP_INFO("%s: set first preferredFocalMechanismID to %s",
			              info->event->publicID().c_str(), fm->publicID().c_str());
			SEISCOMP_LOG(_infoChannel, "FocalMechanism %s has been set preferred in event %s",
			             fm->publicID().c_str(), info->event->publicID().c_str());
			update = true;
		}
		else {
			if ( !isAgencyIDAllowed(objectAgencyID(fm)) ) {
				SEISCOMP_DEBUG("%s: skip setting first preferredFocalMechanismID, agencyID '%s' is blocked",
				               info->event->publicID().c_str(), objectAgencyID(fm).c_str());
				SEISCOMP_LOG(_infoChannel, "FocalMechanism %s has not been set preferred: agencyID %s is blocked",
				             fm->publicID().c_str(), objectAgencyID(fm).c_str());
			}
			else {
				SEISCOMP_DEBUG("%s: skip setting first preferredFocalMechanismID, preferredFocalMechanismID is fixed to '%s'",
				               info->event->publicID().c_str(), info->constraints.preferredFocalMechanismID.c_str());
				SEISCOMP_LOG(_infoChannel, "FocalMechanism %s has not been set preferred: preferredFocalMechanismID is fixed to %s",
				             fm->publicID().c_str(), info->constraints.preferredFocalMechanismID.c_str());
			}
			return;
		}
	}
	else if ( info->preferredFocalMechanism->publicID() != fm->publicID() ) {
		SEISCOMP_DEBUG("... checking whether focalmechanism %s can become preferred",
		               fm->publicID().c_str());

		if ( isRejected(fm) ) {
			SEISCOMP_DEBUG("... skipping potential preferred focalmechanism, status is REJECTED");
			SEISCOMP_LOG(_infoChannel, "FocalMechanism %s has not been set preferred in event %s: status is REJECTED",
			             fm->publicID().c_str(), info->event->publicID().c_str());
			return;
		}

		// Fixed focalmechanism => check if the passed focalmechanism is the fixed one
		if ( info->constraints.fixedFocalMechanism() ) {
			if ( !info->constraints.fixFocalMechanism(fm) ) {
				SEISCOMP_DEBUG("... skipping potential preferred focalmechanism, focalmechanism '%s' is fixed",
				               info->constraints.preferredFocalMechanismID.c_str());
				SEISCOMP_LOG(_infoChannel, "FocalMechanism %s has not been set preferred in event %s: focalmechanism %s is fixed",
				             fm->publicID().c_str(), info->event->publicID().c_str(),
				             info->constraints.preferredFocalMechanismID.c_str());
				return;
			}
		}
		// No fixed focalmechanism => select it using the automatic rules
		else {
			if ( isAgencyIDBlocked(objectAgencyID(fm)) ) {
				SEISCOMP_DEBUG("... skipping potential preferred focalmechanism, agencyID '%s' is blocked",
				               objectAgencyID(fm).c_str());
				SEISCOMP_LOG(_infoChannel, "FocalMechanism %s has not been set preferred in event %s: agencyID %s is blocked",
				             fm->publicID().c_str(), info->event->publicID().c_str(),
				             objectAgencyID(fm).c_str());
				return;
			}

			if ( !_config.priorities.empty() ) {
				// Run through the priority list and check the values
				for ( Config::StringList::iterator it = _config.priorities.begin();
				      it != _config.priorities.end(); ++it ) {
					if ( *it == "AGENCY" ) {
						int fmAgencyPriority = agencyPriority(objectAgencyID(fm), _config);
						int preferredFMAgencyPriority = agencyPriority(objectAgencyID(info->preferredFocalMechanism.get()), _config);

						if ( fmAgencyPriority < preferredFMAgencyPriority ) {
							SEISCOMP_DEBUG("... skipping potential preferred focalmechanism, priority of agencyID '%s' is too low",
							               objectAgencyID(fm).c_str());
							SEISCOMP_LOG(_infoChannel, "FocalMechanism %s has not been set preferred in event %s: priority of agencyID %s is too low",
							             fm->publicID().c_str(), info->event->publicID().c_str(),
							             objectAgencyID(fm).c_str());
							return;
						}
						// Found origin with higher agency priority
						else if ( fmAgencyPriority > preferredFMAgencyPriority ) {
							SEISCOMP_DEBUG("... agencyID '%s' overrides current agencyID '%s'",
							               objectAgencyID(fm).c_str(), objectAgencyID(info->preferredFocalMechanism.get()).c_str());
							SEISCOMP_LOG(_infoChannel, "FocalMechanism %s: agencyID '%s' overrides agencyID '%s'",
							             fm->publicID().c_str(), objectAgencyID(fm).c_str(), objectAgencyID(info->preferredFocalMechanism.get()).c_str());
							break;
						}
					}
					else if ( *it == "AUTHOR" ) {
						int fmAuthorPriority = authorPriority(objectAuthor(fm), _config);
						int preferredFMAuthorPriority = authorPriority(objectAuthor(info->preferredFocalMechanism.get()), _config);

						if ( fmAuthorPriority < preferredFMAuthorPriority ) {
							SEISCOMP_DEBUG("... skipping potential preferred focalmechanism, priority of author '%s' is too low",
							               objectAuthor(fm).c_str());
							SEISCOMP_LOG(_infoChannel, "FocalMechanism %s has not been set preferred in event %s: priority of author %s is too low",
							             fm->publicID().c_str(), info->event->publicID().c_str(),
							             objectAuthor(fm).c_str());
							return;
						}
						// Found focalmechanism with higher author priority
						else if ( fmAuthorPriority > preferredFMAuthorPriority ) {
							SEISCOMP_DEBUG("... author '%s' overrides current author '%s'",
							               objectAuthor(fm).c_str(), objectAuthor(info->preferredFocalMechanism.get()).c_str());
							SEISCOMP_LOG(_infoChannel, "FocalMechanism %s: author '%s' overrides author '%s'",
							             fm->publicID().c_str(), objectAuthor(fm).c_str(), objectAuthor(info->preferredFocalMechanism.get()).c_str());
							break;
						}
					}
					else if ( *it == "MODE" ) {
						int fmPriority = modePriority(fm);
						int preferredFMPriority = modePriority(info->preferredFocalMechanism.get());

						if ( info->constraints.fixFocalMechanismMode(info->preferredFocalMechanism.get()) ) {
							SEISCOMP_LOG(_infoChannel, "FocalMechanism %s: has priority %d vs %d",
							             fm->publicID().c_str(), fmPriority, preferredFMPriority);
							// Set back the evalmode to automatic if a higher priority
							// origin has been send (but not triggered by a magnitude change only)
							if ( fmPriority > preferredFMPriority ) {
								/*
								SEISCOMP_LOG(_infoChannel, "FocalMechanism %s has higher priority: releasing EvPrefOrgEvalMode",
								             origin->publicID().c_str());
								JournalEntryPtr entry = new JournalEntry;
								entry->setObjectID(info->event->publicID());
								entry->setAction("EvPrefOrgEvalMode");
								entry->setParameters("");
								entry->setSender(name() + "@" + Core::getHostname());
								entry->setCreated(Core::Time::GMT());
								Notifier::Create(_journal->publicID(), OP_ADD, entry.get());
								info->addJournalEntry(entry.get());
								*/
							}
							else
								preferredFMPriority = FOCALMECHANISM_PRIORITY_MAX;
						}

						if ( info->constraints.fixFocalMechanismMode(fm) )
							fmPriority = FOCALMECHANISM_PRIORITY_MAX;

						if ( fmPriority < preferredFMPriority ) {
							SEISCOMP_DEBUG("... skipping potential preferred focalmechanism (%d < %d)",
							               fmPriority, preferredFMPriority);
							SEISCOMP_LOG(_infoChannel, "FocalMechanism %s has not been set preferred in event %s: priority too low (%d < %d)",
							             fm->publicID().c_str(), info->event->publicID().c_str(),
							             fmPriority, preferredFMPriority);
							return;
						}
						// Found origin with higher status priority
						else if ( fmPriority > preferredFMPriority ) {
							SEISCOMP_LOG(_infoChannel, "FocalMechanism %s: status priority %d overrides status priority %d",
							             fm->publicID().c_str(), fmPriority, preferredFMPriority);
							break;
						}
					}
					else if ( *it == "STATUS" ) {
						int fmPriority = priority(fm);
						int preferredFMPriority = priority(info->preferredFocalMechanism.get());

						if ( info->constraints.fixFocalMechanismMode(info->preferredFocalMechanism.get()) ) {
							SEISCOMP_LOG(_infoChannel, "FocalMechanism %s: has priority %d vs %d",
							             fm->publicID().c_str(), fmPriority, preferredFMPriority);
							// Set back the evalmode to automatic if a higher priority
							// origin has been send (but not triggered by a magnitude change only)
							if ( fmPriority > preferredFMPriority ) {
								/*
								SEISCOMP_LOG(_infoChannel, "FocalMechanism %s has higher priority: releasing EvPrefOrgEvalMode",
								             origin->publicID().c_str());
								JournalEntryPtr entry = new JournalEntry;
								entry->setObjectID(info->event->publicID());
								entry->setAction("EvPrefOrgEvalMode");
								entry->setParameters("");
								entry->setSender(name() + "@" + Core::getHostname());
								entry->setCreated(Core::Time::GMT());
								Notifier::Create(_journal->publicID(), OP_ADD, entry.get());
								info->addJournalEntry(entry.get());
								*/
							}
							else
								preferredFMPriority = FOCALMECHANISM_PRIORITY_MAX;
						}

						if ( info->constraints.fixFocalMechanismMode(fm) )
							fmPriority = FOCALMECHANISM_PRIORITY_MAX;

						if ( fmPriority < preferredFMPriority ) {
							SEISCOMP_DEBUG("... skipping potential preferred focalmechanism (%d < %d)",
							               fmPriority, preferredFMPriority);
							SEISCOMP_LOG(_infoChannel, "FocalMechanism %s has not been set preferred in event %s: priority too low (%d < %d)",
							             fm->publicID().c_str(), info->event->publicID().c_str(),
							             fmPriority, preferredFMPriority);
							return;
						}
						// Found origin with higher status priority
						else if ( fmPriority > preferredFMPriority ) {
							SEISCOMP_LOG(_infoChannel, "FocalMechanism %s: status priority %d overrides status priority %d",
							             fm->publicID().c_str(), fmPriority, preferredFMPriority);
							break;
						}
					}
					else if ( *it == "METHOD" ) {
						int fmMethodPriority = methodPriority(fm->methodID(), _config);
						int preferredFMMethodPriority = methodPriority(info->preferredFocalMechanism->methodID(), _config);

						if ( fmMethodPriority < preferredFMMethodPriority ) {
							SEISCOMP_DEBUG("... skipping potential preferred focalMechanism, priority of method '%s' is too low",
							               fm->methodID().c_str());
							SEISCOMP_LOG(_infoChannel, "FocalMechanism %s has not been set preferred in event %s: priority of method %s is too low",
							             fm->publicID().c_str(), info->event->publicID().c_str(),
							             fm->methodID().c_str());
							return;
						}
						// Found origin with higher method priority
						else if ( fmMethodPriority > preferredFMMethodPriority ) {
							SEISCOMP_DEBUG("... methodID '%s' overrides current methodID '%s'",
							               fm->methodID().c_str(), info->preferredFocalMechanism->methodID().c_str());
							SEISCOMP_LOG(_infoChannel, "FocalMechanism %s: methodID '%s' overrides methodID '%s'",
							             fm->publicID().c_str(), fm->methodID().c_str(),
							             info->preferredFocalMechanism->methodID().c_str());
							break;
						}
					}
					else if ( *it == "TIME" ) {
						Core::Time fmCreationTime = created(fm);
						Core::Time preferredFMCreationTime = created(info->preferredFocalMechanism.get());
						if ( fmCreationTime < preferredFMCreationTime ) {
							SEISCOMP_DEBUG("... skipping potential preferred focalmechanism, there is a better one created later");
							return;
						}
						else if ( fmCreationTime > preferredFMCreationTime ) {
							SEISCOMP_LOG(_infoChannel, "FocalMechanism %s: %s is more recent than %s",
							             fm->publicID().c_str(), fmCreationTime.iso().c_str(),
							             preferredFMCreationTime.iso().c_str());
							break;
						}
					}
					else if ( *it == "TIME_AUTOMATIC" ) {
						EvaluationMode status = AUTOMATIC;
						try {
							status = fm->evaluationMode();
						}
						catch ( ValueException& ) {}

						// Make a noop in case of non automatic focalmechanisms
						if ( status != AUTOMATIC ) continue;

						Core::Time fmCreationTime = created(fm);
						Core::Time preferredFMCreationTime = created(info->preferredFocalMechanism.get());
						if ( fmCreationTime < preferredFMCreationTime ) {
							SEISCOMP_DEBUG("... skipping potential preferred focalmechanism, there is a better one created later");
							return;
						}
						else if ( fmCreationTime > preferredFMCreationTime ) {
							SEISCOMP_LOG(_infoChannel, "FocalMechanism %s: %s (automatic) is more recent than %s",
							             fm->publicID().c_str(), fmCreationTime.iso().c_str(),
							             preferredFMCreationTime.iso().c_str());
							break;
						}
					}
					else if ( *it == "SCORE" ) {
						double score = _score->evaluate(fm);
						double preferredScore = _score->evaluate(info->preferredFocalMechanism.get());
						if ( score < preferredScore ) {
							SEISCOMP_DEBUG("... skipping potential preferred focalmechanism, there is one with higher score: %f > %f",
							               preferredScore, score);
							return;
						}
						else if ( score > preferredScore ) {
							SEISCOMP_LOG(_infoChannel, "FocalMechanism %s: score of %f is larger than %f",
							             fm->publicID().c_str(), score,
							             preferredScore);
							break;
						}
					}
				}
			}
			else {
				int fmAgencyPriority = agencyPriority(objectAgencyID(fm), _config);
				int preferredFMAgencyPriority = agencyPriority(objectAgencyID(info->preferredFocalMechanism.get()), _config);

				if ( fmAgencyPriority < preferredFMAgencyPriority ) {
					SEISCOMP_DEBUG("... skipping potential preferred focalmechanism, priority of agencyID '%s' is too low",
					               objectAgencyID(fm).c_str());
					SEISCOMP_LOG(_infoChannel, "FocalMechanism %s has not been set preferred in event %s: priority of agencyID %s is too low",
					             fm->publicID().c_str(), info->event->publicID().c_str(),
					             objectAgencyID(fm).c_str());
					return;
				}

				// Same agency priorities -> compare fm priority
				if ( fmAgencyPriority == preferredFMAgencyPriority ) {
					int fmPriority = priority(fm);
					int preferredFMPriority = priority(info->preferredFocalMechanism.get());

					if ( info->constraints.fixFocalMechanismMode(info->preferredFocalMechanism.get()) ) {
						SEISCOMP_LOG(_infoChannel, "FocalMechanism %s: has priority %d vs %d",
						             fm->publicID().c_str(), fmPriority, preferredFMPriority);

						// Set back the evalmode to automatic if a higher priority
						// focalmechanism has been send
						if ( fmPriority > preferredFMPriority ) {
							/*
							SEISCOMP_LOG(_infoChannel, "Origin %s has higher priority: releasing EvPrefOrgEvalMode",
							             origin->publicID().c_str());
							JournalEntryPtr entry = new JournalEntry;
							entry->setObjectID(info->event->publicID());
							entry->setAction("EvPrefOrgEvalMode");
							entry->setParameters("");
							entry->setSender(name() + "@" + Core::getHostname());
							entry->setCreated(Core::Time::GMT());
							Notifier::Create(_journal->publicID(), OP_ADD, entry.get());
							info->addJournalEntry(entry.get());
							*/
						}
						else
							preferredFMPriority = FOCALMECHANISM_PRIORITY_MAX;
					}

					if ( info->constraints.fixFocalMechanismMode(fm) )
						fmPriority = FOCALMECHANISM_PRIORITY_MAX;

					if ( fmPriority < preferredFMPriority ) {
						SEISCOMP_DEBUG("... skipping potential preferred focalmechanism (%d < %d)",
						               fmPriority, preferredFMPriority);
						SEISCOMP_LOG(_infoChannel, "FocalMechanism %s has not been set preferred in event %s: priority too low (%d < %d)",
						             fm->publicID().c_str(), info->event->publicID().c_str(),
						             fmPriority, preferredFMPriority);
						return;
					}

					if ( fmPriority == preferredFMPriority ) {
						EvaluationMode status = AUTOMATIC;
						try {
							status = fm->evaluationMode();
						}
						catch ( ValueException& ) {}

						if ( status == AUTOMATIC ) {
							SEISCOMP_DEBUG("Same priority and mode is AUTOMATIC");

							if ( created(fm) < created(info->preferredFocalMechanism.get()) ) {
								SEISCOMP_DEBUG("... skipping potential preferred focalmechanism, there is a better one created later");
								return;
							}
						}
					}
				}
				else {
					SEISCOMP_DEBUG("... agencyID '%s' overrides current agencyID '%s'",
					               objectAgencyID(fm).c_str(), objectAgencyID(info->preferredFocalMechanism.get()).c_str());
					SEISCOMP_LOG(_infoChannel, "FocalMechanism %s: agencyID '%s' overrides agencyID '%s'",
					             fm->publicID().c_str(), objectAgencyID(fm).c_str(), objectAgencyID(info->preferredFocalMechanism.get()).c_str());
				}
			}
		}

		info->event->setPreferredFocalMechanismID(fm->publicID());

		info->preferredFocalMechanism = fm;
		SEISCOMP_INFO("%s: set preferredFocalMechanismID to %s",
		              info->event->publicID().c_str(), fm->publicID().c_str());
		SEISCOMP_LOG(_infoChannel, "FocalMechanism %s has been set preferred in event %s",
		             fm->publicID().c_str(), info->event->publicID().c_str());

		update = true;
	}
	else {
		SEISCOMP_INFO("%s: nothing to do", info->event->publicID().c_str());
	}

	if ( update ) {
		// Update preferred magnitude based on new focal mechanism if
		// Mw is fixed
		if ( info->constraints.fixMw )
			choosePreferred(info, info->preferredOrigin.get(), NULL);

		if ( !info->created )
			updateEvent(info->event.get());
		else {
			info->created = false;

			// Call registered processors
			EventProcessors::iterator it;
			for ( it = _processors.begin(); it != _processors.end(); ++it )
				it->second->process(info->event.get());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTool::updatePreferredOrigin(EventInformation *info) {
	if ( !info->event ) return;

	for ( size_t i = 0; i < info->event->originReferenceCount(); ++i ) {
		OriginPtr org = _cache.get<Origin>(info->event->originReference(i)->originID());
		if ( !org ) continue;
		if ( !org->magnitudeCount() && query() ) {
			SEISCOMP_DEBUG("... loading magnitudes for origin %s", org->publicID().c_str());
			query()->loadMagnitudes(org.get());
		}
		choosePreferred(info, org.get(), NULL);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTool::updatePreferredFocalMechanism(EventInformation *info) {
	if ( !info->event ) return;

	for ( size_t i = 0; i < info->event->focalMechanismReferenceCount(); ++i ) {
		FocalMechanismPtr fm = _cache.get<FocalMechanism>(info->event->focalMechanismReference(i)->focalMechanismID());
		if ( !fm ) continue;
		if ( !fm->momentTensorCount() && query() ) {
			SEISCOMP_DEBUG("... loading moment tensor for focalmechanism %s", fm->publicID().c_str());
			query()->loadMomentTensors(fm.get());
		}
		choosePreferred(info, fm.get());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventTool::mergeEvents(EventInformation *target, EventInformation *source) {
	Event *sourceEvent = source->event.get();
	if ( sourceEvent == NULL ) return false;

	Event *targetEvent = target->event.get();
	if ( targetEvent == NULL ) return false;

	if ( sourceEvent->originReferenceCount() == 0 )
		query()->loadOriginReferences(sourceEvent);

	if ( sourceEvent->focalMechanismReferenceCount() == 0 )
		query()->loadFocalMechanismReferences(sourceEvent);

	while ( sourceEvent->originReferenceCount() > 0 ) {
		OriginReferencePtr ref = sourceEvent->originReference(0);
		sourceEvent->removeOriginReference(0);

		OriginPtr org = _cache.get<Origin>(ref->originID());

		if ( !org ) {
			SEISCOMP_WARNING("%s: referenced origin %s not found",
			                 sourceEvent->publicID().c_str(),
			                 ref->originID().c_str());

			if ( targetEvent->originReference(ref->originID()) != NULL ) {
				SEISCOMP_WARNING("... origin %s already associated to event %s",
				                 ref->originID().c_str(),
				                 targetEvent->publicID().c_str());
			}
			else {
				SEISCOMP_DEBUG("%s: added origin reference %s due to merge request",
				               targetEvent->publicID().c_str(),
				               ref->originID().c_str());
				SEISCOMP_LOG(_infoChannel, "OriginReference %s added to event %s due to merge request",
				             ref->originID().c_str(), targetEvent->publicID().c_str());
				OriginReferencePtr newRef = new OriginReference(*ref);
				targetEvent->add(newRef.get());
			}

			continue;
		}

		if ( !target->associate(org.get()) ) {
			SEISCOMP_WARNING("%s: associating origin %s failed",
			                 targetEvent->publicID().c_str(),
			                 org->publicID().c_str());
			continue;
		}

		SEISCOMP_INFO("%s: associated origin %s due to merge request",
		              targetEvent->publicID().c_str(),
		              org->publicID().c_str());

		if ( !org->magnitudeCount() && query() ) {
			SEISCOMP_DEBUG("... loading magnitudes for origin %s", org->publicID().c_str());
			query()->loadMagnitudes(org.get());
		}

		// Update the preferred origin if configured to do so
		if ( _config.updatePreferredSolutionAfterMerge )
			choosePreferred(target, org.get(), NULL);
	}

	while ( sourceEvent->focalMechanismReferenceCount() > 0 ) {
		FocalMechanismReferencePtr ref = sourceEvent->focalMechanismReference(0);
		sourceEvent->removeFocalMechanismReference(0);

		FocalMechanismPtr fm = _cache.get<FocalMechanism>(ref->focalMechanismID());

		if ( !fm ) {
			SEISCOMP_WARNING("%s: referenced focal mechanism %s not found",
			                 sourceEvent->publicID().c_str(),
			                 ref->focalMechanismID().c_str());

			if ( targetEvent->focalMechanismReference(ref->focalMechanismID()) != NULL ) {
				SEISCOMP_WARNING("... focal mechanism %s already associated to event %s",
				                 ref->focalMechanismID().c_str(),
				                 targetEvent->publicID().c_str());
			}
			else {
				SEISCOMP_DEBUG("%s: added focal mechanism reference %s due to merge request",
				               targetEvent->publicID().c_str(),
				               ref->focalMechanismID().c_str());
				SEISCOMP_LOG(_infoChannel, "FocalMechanismReference %s added to event %s due to merge request",
				             ref->focalMechanismID().c_str(), targetEvent->publicID().c_str());
				FocalMechanismReferencePtr newRef = new FocalMechanismReference(*ref);
				targetEvent->add(newRef.get());
			}

			continue;
		}

		if ( !target->associate(fm.get()) ) {
			SEISCOMP_WARNING("%s: associating focal mechanism %s failed",
			                 targetEvent->publicID().c_str(),
			                 fm->publicID().c_str());
			continue;
		}

		SEISCOMP_INFO("%s: associated focal mechanism %s due to merge request",
		              targetEvent->publicID().c_str(),
		              fm->publicID().c_str());

		if ( !fm->momentTensorCount() && query() ) {
			SEISCOMP_DEBUG("... loading moment tensor for focalmechanism %s", fm->publicID().c_str());
			query()->loadMomentTensors(fm.get());
		}

		// Update the preferred focalfechanism
		if ( _config.updatePreferredSolutionAfterMerge )
			choosePreferred(target, fm.get());
	}

	// Remove source event
	SEISCOMP_INFO("%s: deleted", sourceEvent->publicID().c_str());
	SEISCOMP_LOG(_infoChannel, "Delete event %s after merging",
	             sourceEvent->publicID().c_str());

	sourceEvent->detach();
	_cache.remove(sourceEvent);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTool::removedFromCache(Seiscomp::DataModel::PublicObject *po) {
	bool saveState = DataModel::Notifier::IsEnabled();
	DataModel::Notifier::Disable();

	EventMap::iterator it = _events.find(po->publicID());
	if ( it != _events.end() ) {
		// Remove EventInfo item for uncached event
		// Don't erase the element but mark it. While iterating over the
		// event cache and performing check and cache updates an event can
		// be removed from the cache which leads to crashes. We clean up
		// the removed events after the work has been done.
		it->second->aboutToBeRemoved = true;
		SEISCOMP_DEBUG("... mark event %s to be removed from cache", po->publicID().c_str());
	}

	// Only allow to detach objects from the EP instance if it hasn't been read
	// from a file
	if ( _epFile.empty() )
		po->detach();

	DataModel::Notifier::SetEnabled(saveState);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTool::updateEvent(DataModel::Event *ev, bool callProcessors) {
	Core::Time now = Core::Time::GMT();
	// Set the modification to current time
	try {
		ev->creationInfo().setModificationTime(now);
	}
	catch ( ... ) {
		DataModel::CreationInfo ci;
		ci.setModificationTime(now);
		ev->setCreationInfo(ci);
	}

	logObject(_outputEvent, now);

	if ( ev->parent() == NULL ) {

	}

	// Flag the event as updated to be sent around
	ev->update();

	if ( callProcessors ) {
		// Call registered processors
		EventProcessors::iterator it;
		for ( it = _processors.begin(); it != _processors.end(); ++it )
			it->second->process(ev);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTool::updateRegionName(DataModel::Event *ev, DataModel::Origin *org) {
	std::string reg = org != NULL?region(org):"";
	EventDescription *ed = eventRegionDescription(ev);
	if ( ed != NULL ) {
		if ( ed->text() != reg ) {
			SEISCOMP_INFO("%s: updating region name to '%s'",
			              ev->publicID().c_str(), reg.c_str());
			SEISCOMP_LOG(_infoChannel, "Event %s region name updated: %s",
			             ev->publicID().c_str(), reg.c_str());
			ed->setText(reg);
			ed->update();
		}
	}
	else {
		EventDescriptionPtr ed = new EventDescription(reg, REGION_NAME);
		ev->add(ed.get());
		SEISCOMP_INFO("%s: adding region name '%s'",
		              ev->publicID().c_str(), reg.c_str());
		SEISCOMP_LOG(_infoChannel, "Event %s got new region name: %s",
		             ev->publicID().c_str(), reg.c_str());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
