/***************************************************************************
 * Copyright
 * ---------
 * This file is part of the Virtual Seismologist (VS) software package.
 * VS is free software: you can redistribute it and/or modify it under
 * the terms of the "SED Public License for Seiscomp Contributions"
 *
 * VS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the SED Public License for Seiscomp
 * Contributions for more details.
 *
 * You should have received a copy of the SED Public License for Seiscomp
 * Contributions with VS. If not, you can find it at
 * http://www.seismo.ethz.ch/static/seiscomp_contrib/license.txt
 *
 * Authors of the Software: Jan Becker, Yannik Behr and Stefan Heimers
 * Copyright (C) 2006-2013 by Swiss Seismological Service
 ***************************************************************************/

#include "scvsmag.h"
#include "equations.h"

using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Math;
using namespace Seiscomp::Processing;

namespace {

VsEquations vs;

}

/*!
 \brief set up messaging and some default values at startup
 */
VsMagnitude::VsMagnitude(int argc, char **argv) :
		Client::Application(argc, argv) {
	// Enable loading the inventory at startup
	setDatabaseEnabled(true, true);
	setLoadInventoryEnabled(true);

	// The message group to where messages are going to be sent though other
	// groups can be used as well. E.g. connection->send(msg) sends a message
	// to the primary message group (PICK), but connection->send("LOCATION", msg)
	// sends a message to the passed group name (LOCATION).
	setMessagingEnabled(true);
	setPrimaryMessagingGroup("MAGNITUDE");

	// Listen for events, origins, picks (caching) and
	// envelopes (circular buffer)
	addMessagingSubscription("LOCATION");
	addMessagingSubscription("EVENT");
	addMessagingSubscription("PICK");
	addMessagingSubscription("AMPLITUDE");

	// Setting default values:

	// length of the circular buffer before and after current time
	_backSlots = 6000;
	_headSlots = 60;

	// timewindow for processing data before and after a pick in seconds.
	_twstarttime = 3;
	_twendtime = 30;

	// number seconds after origin time the calculation of vsmagnitudes is stopped
	_eventExpirationTime = 90;

	// choose whether to time the event expiration time with respect to the
	// origin time or the time of the first VS estimate; can be 'ot' or 'ct'
	_expirationTimeReference = "ct";

	// number seconds a stream is not used for following a clipped record
	_clipTimeout = 30;

	// size of the object buffer in seconds
	_timeout = 3600.;
	// default vs30 value
	_vs30default = 910.0;

	// by default don't use site effects
	_siteEffect = false;

	// by default run in realtime mode
	_realtime = true;

	// maximum epicentral distance; -1 means no restriction
	_maxepicdist = -1.0f;
}

VsMagnitude::~VsMagnitude() {
}

/*!
 \brief Create additional command line options.
 */

void VsMagnitude::createCommandLineDescription() {
	commandline().addGroup("Mode");
	commandline().addOption("Mode", "playback",
			"Sets the current time to the latest received envelope timestamp");
	commandline().addOption("Mode", "timeref",
			"Set whether the expiration time is measured with respect to origin time ('ot') or creation time ('ct').",
			&_expirationTimeReference);
	commandline().addGroup("Log");
	commandline().addOption("Log", "processing-log",
			"Set an alternative filename for the processing log-file.", &_proclogfile);
	commandline().addOption("Log", "envelope-log", "Turn on envelope logging.");
}

/*!
 \brief Read the configuration file.
 */
bool VsMagnitude::initConfiguration() {
	SEISCOMP_DEBUG("initConfiguration() started");
	// first call the application's configuration routine
	if ( !Client::Application::initConfiguration() )
		return false;

	// turn site effect correction on or off
	try {
		_siteEffect = configGetBool("vsmag.siteEffect");
		SEISCOMP_INFO(
				"siteEffects turned on in config file (_siteEffect = true)");
	} catch ( ... ) {
		SEISCOMP_INFO(
				"vsmag.siteEffect not given, using default of %s", Core::toString(_siteEffect).c_str());
	}

	// read the filename of a Vs30 value grid file for site effect correction
	if ( _siteEffect ) {
		try {
			_vs30filename = configGetString("vsmag.vs30filename");
		} catch ( ... ) {
			_siteEffect = false;
			SEISCOMP_INFO(
					"vsmag.vs30filename not given; turning of site effect correction.");
		}
	}

	// number seconds after origin time the calculation of VS magnitudes is stopped
	try {
		_eventExpirationTime = configGetInt("vsmag.eventExpirationTime");
		SEISCOMP_INFO(
				"eventExpirationTime configured to %d", _eventExpirationTime);
	} catch ( ... ) {
		SEISCOMP_INFO(
				"vsmag.eventExpirationTime not given, using default of %d", _eventExpirationTime);
	}

	// choose whether to time the event expiration time with respect to the
	// origin time or the time of the first VS estimate
	try {
		_expirationTimeReference = configGetString("vsmag.ExpirationTimeReference");
		SEISCOMP_INFO(
				"eventExpirationTime configured to %s", _expirationTimeReference.c_str());
	} catch ( ... ) {
		SEISCOMP_INFO(
				"vsmag.eventExpirationTime not given, using default of %s", _expirationTimeReference.c_str());
	}

	// number seconds a stream is not used for following a clipped record
	try {
		_clipTimeout = configGetInt("vsmag.clipTimeout");
		SEISCOMP_INFO("clipTimeout configured to %d", _clipTimeout);
	} catch ( ... ) {
		SEISCOMP_INFO(
				"vsmag.clipTimeout not given, using default of %d", _clipTimeout);
	}

	// Timewindow around pick
	try {
		_twstarttime = configGetInt("vsmag.twstarttime");
		SEISCOMP_INFO("twstarttime configured to %d", _twstarttime);
	} catch ( ... ) {
		SEISCOMP_INFO(
				"vsmag.twstarttime not given, using default of %d", _twstarttime);
	}
	try {
		_twendtime = configGetInt("vsmag.twendtime");
		SEISCOMP_INFO("twendtime configured to %d", _twendtime);
	} catch ( ... ) {
		SEISCOMP_INFO(
				"vsmag.twendtime not given, using default of %d", _twendtime);
	}

	// Choose between 'realtime' and 'playback' mode. In 'realtime' mode VS magnitudes
	// are evaluated based on a realtime timer. In 'playback' mode the timing is
	// determined by incoming envelope values.
	try {
		if ( configGetString("vsmag.mode") == "playback" ) {
			_realtime = false;
			SEISCOMP_INFO("mode set to playback (_realtime = false)");
		}
	} catch ( ... ) {
		SEISCOMP_INFO(
				"vsmag.mode not given, using default of %s", Core::toString(_realtime).c_str());
	}

	// length of the timeline ringbuffer in seconds
	try {
		_headSlots = configGetInt("vsmag.headslots");
		if ( _headSlots < 0 ) {
			SEISCOMP_ERROR(
					"vsmag.headslots must not be negative (%d < 0)", _headSlots);
			return false;
		}
	} catch ( ... ) {
		SEISCOMP_INFO(
				"vsmag.headslots not configured, using default: %d", _headSlots);
	}

	try {
		_backSlots = configGetInt("vsmag.backslots");
		if ( _backSlots < 10 ) {
			SEISCOMP_ERROR(
					"vsmag.backslots must not be smaller than 10 (%d < 10)", _backSlots);
			return false;
		}
	} catch ( ... ) {
		SEISCOMP_INFO(
				"vsmag.backslots not configured, using default: %d", _backSlots);
	}

	// If site corrections are turned on give a default Vs30 value that is used if
	// no value can be found in the given Vs30 grid file (s.o.)
	if ( _siteEffect ) {
		try {
			_vs30default = configGetDouble("vsmag.vs30default");
			SEISCOMP_INFO(
					"vsmag.vs30default configured value: %f", _vs30default);
		} catch ( ... ) {
			SEISCOMP_INFO(
					"vsmag.vs30default not configured, using default: %f", _vs30default);
		}
	}

	// Set a maximum epicentral distance past which stations will not contribute
	// to the network magnitude
	try {
		_maxepicdist = configGetDouble("vsmag.maxepicdist");
	} catch ( ... ) {
		SEISCOMP_INFO(
				"vsmag.maxepicdist not configured, using default: %f", _maxepicdist);
	}

	// Set a maximum azimuthal gap
	try {
		_maxazgap = configGetDouble("vsmag.maxazgap");
	} catch ( ... ) {
		SEISCOMP_INFO(
				"vsmag.maxepicdist not configured, using default: %f", _maxepicdist);
	}

	// turn on envelope logging
	try {
		_logenvelopes = configGetBool("vsmag.logenvelopes");
		SEISCOMP_INFO("Envelope logging turned on. (_logenvelopes = true)");
	} catch ( ... ) {
		SEISCOMP_INFO("Logging envelopes is turned off.");
	}

	return true;
}

bool VsMagnitude::validateParameters() {
	// Disable database access if an inventory xml file is given
	// as a command line argument.
	if ( !isInventoryDatabaseEnabled() )
		setDatabaseEnabled(false, false);

	if ( commandline().hasOption("playback") )
		_realtime = false;
	if ( commandline().hasOption("envelope-log") )
		 _logenvelopes = true;

	return Application::validateParameters();
}

/*!
 Method that is run once at startup.
 */
bool VsMagnitude::init() {
	if ( !Client::Application::init() )
		return false;

	enableTimer(1);

	Client::Inventory *inv = Client::Inventory::Instance();
	DataModel::Inventory *inventory = inv->inventory();
	if ( inventory == NULL ) {
		SEISCOMP_ERROR("Inventory not available");
		return false;
	}

	// Read in the Vs30 value file
	if ( _siteEffect ) {
		if ( _vs30filename == "" ) {
			SEISCOMP_ERROR(
					"no Vs30 file name specified, turning off siteEffect");
			_siteEffect = false;
		} else {
			if ( !ch::sed::Vs30Mapping::createInstance(_vs30filename) ) {
				SEISCOMP_ERROR(
						"Error reading Vs30 file %s, turning siteEffect off", _vs30filename.c_str());
				_siteEffect = false;
			}
		}
	}
	if ( !_siteEffect ) {
		SEISCOMP_INFO("siteEffect is turned off");
		if ( _vs30filename != "" ) {
			SEISCOMP_INFO(
					"The Vs30 file %s will be ignored,\n please set vsmag.siteEffect  = true in the config file and make sure the Vs30 file is correct", _vs30filename.c_str());
		}
	}

	// set the timespan of the cache
	_cache.setTimeSpan(TimeSpan(_timeout));

	// get objects from the database if they are not in the cache
	if ( isDatabaseEnabled() )
		_cache.setDatabaseArchive(query());

	// initialize the circular buffer
	_timeline.init(_backSlots, _headSlots, _clipTimeout);

	if ( _realtime ) {
		_currentTime = Core::Time::GMT();
		_timeline.setReferenceTime(_currentTime);
	}

	_creationInfo.setAgencyID(agencyID());
	_creationInfo.setAuthor(author());

	// Log into processing/info to avoid logging the same information into the global info channel
	_processingInfoChannel =
			SEISCOMP_DEF_LOGCHANNEL("processing/info", Logging::LL_INFO);
	if ( commandline().hasOption("processing-log") ){
		_processingInfoFile = new Logging::FileOutput(_proclogfile.c_str());
	} else {
		_processingInfoFile = new Logging::FileRotatorOutput(
				Environment::Instance()->logFile("scvsmag-processing-info").c_str(),
				60 * 60 * 24, 30);
	}
	_processingInfoFile->subscribe(_processingInfoChannel);
	_processingInfoOutput = new Logging::FdOutput(STDERR_FILENO);
	_processingInfoOutput->subscribe(_processingInfoChannel);
	SEISCOMP_LOG(_processingInfoChannel,
			"scvsmag module started at %s", Core::Time::GMT().toString("%FT%T.%fZ").c_str());

	if ( _logenvelopes ){
		_envelopeInfoChannel =
				SEISCOMP_DEF_LOGCHANNEL("envelope/info", Logging::LL_INFO);
		_envelopeInfoFile = new Logging::FileRotatorOutput(
				Environment::Instance()->logFile("envelope-logging-info").c_str(),
				60 * 60 * 24, 30);
		_envelopeInfoFile->subscribe(_envelopeInfoChannel);
	}
	return true;
}

/*!\brief Compute site amplification factors

 Compute site amplification factors by combining Vs30 values with the
 correction table of Borcherdt, 1994.

 \param lat Latitude of the station
 \param lon Longitude of the station
 \param MA whatever...

 */
float VsMagnitude::siteEffect(double lat, double lon, double MA,
		ValueType valueType, SoilClass &soilClass) {
	float corr;
	if ( _siteEffect ) {
		try {
			float _vs30;
			ch::sed::Vs30Mapping * vsmappingP =
					ch::sed::Vs30Mapping::sharedInstance();
			vsmappingP->setVsDefault(ch::sed::Vs30Mapping::TYPE_VS30,
					_vs30default);
			_vs30 = vsmappingP->getVs(ch::sed::Vs30Mapping::TYPE_VS30, lat,
					lon);
			corr = _saflist.getCorr(valueType, _vs30, MA);
			if ( _vs30 > 464 ) {
				soilClass = Rock;
			} else if ( _vs30 <= 464 && _vs30 > 0. ) {
				soilClass = Soil;
			} else {
				SEISCOMP_ERROR("Vs30 value can't be negative.");
			}
		} catch ( ... ) {
			SEISCOMP_ERROR(
					"Failed to get VS30 value for lat: %.2f; lon: %.2f", lat, lon);
			corr = 1.0;
			soilClass = Rock;
		}
	} else {
		corr = 1.0;
		soilClass = Rock;
	}
	return corr;
}

/*!
 * Handle incoming messages from scmaster.
 */
void VsMagnitude::handleMessage(Core::Message* msg) {
	// This causes callbacks (addObject, updateObject) to be called
	// when messages arrive
	Application::handleMessage(msg);

	DataMessage *dm = DataMessage::Cast(msg);
	if ( dm == NULL )
		return;

	bool dirty = false;

	for ( DataMessage::iterator it = dm->begin(); it != dm->end(); ++it ) {
		VS::Envelope *vsenv = VS::Envelope::Cast(it->get());
		if ( vsenv ) {
			string str;

			if ( !_realtime ) {
				if ( !_currentTime.valid()
						|| vsenv->timestamp() > _currentTime ) {
					_currentTime = vsenv->timestamp();
					_timeline.setReferenceTime(_currentTime);
					dirty = true;
				}
			}

			for ( size_t eit = 0; eit < vsenv->envelopeChannelCount(); ++eit ) {
				VS::EnvelopeChannel *chan = vsenv->envelopeChannel(eit);
				const DataModel::WaveformStreamID &wid = chan->waveformID();

				str = "Current time: ";
				str += _currentTime.toString("%FT%T.%3fZ");
				str += "; Envelope: timestamp: ";
				str += vsenv->timestamp().toString("%FT%T.%3fZ");
				str += " waveformID: ";
				str += wid.networkCode();
				str += ".";
				str += wid.stationCode();
				str += ".";
				str += wid.channelCode();
				for ( size_t cit = 0; cit < chan->envelopeValueCount();
						++cit ) {
					VS::EnvelopeValue *eval = chan->envelopeValue(cit);
					str += " ";
					str += eval->type();
					str += ": ";
					str += Core::toString(eval->value());
				}
			}
			if (_logenvelopes)
				SEISCOMP_LOG(_envelopeInfoChannel, "%s", str.c_str());

			if ( !_timeline.feed(vsenv) ) {
				SEISCOMP_WARNING("ignored incoming envelope");
				dirty = false;
			}
		}
	}

	// Only in playback mode call processEvents.
	// TODO: check if this is necessary in realtime
	//if ( !_realtime ) {
	if ( dirty )
		processEvents();
	//}
}

void VsMagnitude::handleTimeout() {
	if ( _realtime ) {
		_currentTime = Core::Time::GMT();
		_timeline.setReferenceTime(_currentTime);
		processEvents();
	}
}

/*!
 * Method called by Application::handleMessage if object already exists.
 */
void VsMagnitude::updateObject(const string &parentID, Object *obj) {
	EventPtr event = Event::Cast(obj);
	if ( event ) {
		SEISCOMP_DEBUG("Event updated");
		// Get the already cached event instance and not the temporary instance
		// in the message
		event = _cache.get<Event>(event->publicID());
		handleEvent(event.get());
		return;
	}
}

/*!
 * Method called by Application::handleMessage if object does not exist.
 ************************************************************************/
void VsMagnitude::addObject(const string &parentID, Object *obj) {
	Pick *pk = Pick::Cast(obj);
	if ( pk ) {
		SEISCOMP_DEBUG("Pick received");
		handlePick(pk);
		return;
	}

	Origin *org = Origin::Cast(obj);
	if ( org ) {
		SEISCOMP_DEBUG("Origin received");
		handleOrigin(org);
		return;
	}

	Event *event = Event::Cast(obj);
	if ( event ) {
		SEISCOMP_DEBUG("Event received");
		handleEvent(event);
		return;
	}
}

/*!
 * Method called by Application::handleMessage if an object is removed.
 ************************************************************************/
void VsMagnitude::removeObject(const std::string &parentID,
		DataModel::Object *obj) {
}

/*!
 * Add picks to the pick cache.
 ************************************************************************/
void VsMagnitude::handlePick(Pick *pk) {
	_cache.feed(pk);
}

/*!
 \brief Add origins to the origin buffer.
 */
void VsMagnitude::handleOrigin(Origin *og) {
	_cache.feed(og);
}

/*!
 \brief Extract information from event object.

 \param event  Seiscomp3 type Event
 */
void VsMagnitude::handleEvent(Event *event) {
	_cache.feed(event);

	double dmax = 0; // dmax distance of the furthest picked station from the epicenter
	double davg = 0; // average distance of all picked stations from the epicenter
	double dsum = 0; // sum of distances
	double dthresh;

	/// get the preferred origin of the event
	OriginPtr org = _cache.get<Origin>(event->preferredOriginID());
	if ( !org ){
		SEISCOMP_WARNING("Object %s not found in cache\nIs the cache size big enough?\n"
							"Have you subscribed to all necessary message groups?",
							event->preferredOriginID().c_str());
		return;
	}
	/// search for the corresponding VsEvent in the cache
	VsEventPtr vsevent;
	VsEvents::iterator it = _events.find(event->publicID());
	if ( it == _events.end() ) {
		/// if not found, create a new VsEvent
		vsevent = new VsEvent;
		if ( _expirationTimeReference == "ct" ){
			vsevent->expirationTime = _currentTime + Core::TimeSpan(_eventExpirationTime, 0);
		} else if ( _expirationTimeReference == "ot" ){
			vsevent->expirationTime = org->time().value() + Core::TimeSpan(_eventExpirationTime, 0);
			if ( !(_currentTime < vsevent->expirationTime) )
				return;
		}
		// set time to track how long it takes to estimate the magnitude
		// and how long it took for the origin to arrive
		vsevent->originArrivalTime = Core::Time::GMT();
		vsevent->originCreationTime = org->creationInfo().creationTime();

		// check whether event has been published already
		EventIDBuffer::iterator cev = _publishedEvents.find(event->publicID());
		if ( cev != _publishedEvents.end() ) {
			SEISCOMP_DEBUG("Event %s has already been published", event->publicID().c_str());
			return;
		}
		vsevent->update = -1;
		vsevent->maxAzGap = _maxazgap;
		/// ...and attach it to the cache (_events)
		_events[event->publicID()] = vsevent;
	}
	/// if found, use the existing one
	else
		vsevent = it->second;

	/// Populate the vsevent with data from the preferred origin of the sc3 event
	vsevent->lat = org->latitude().value();
	vsevent->lon = org->longitude().value();
	vsevent->dep = org->depth().value();
	vsevent->time = org->time().value();

	if ( _expirationTimeReference == "ot" )
		vsevent->expirationTime = org->time().value() + Core::TimeSpan(_eventExpirationTime, 0);

	/// if no arrival then no Mvs, but expiration time is updated
	if ( org->arrivalCount() == 0 ){
		SEISCOMP_DEBUG("Ignoring current preferred origin %s (it has no arrival), but expiration time updated", org->publicID().c_str());
		return;
	}

	/// Generate some statistics for later use in delta-pick quality measure
	Timeline::StationList pickedThresholdStations; // all picked stations at a limited distance from the epicenter
	vsevent->pickedStations.clear();
	SEISCOMP_DEBUG("Number of arrivals in origin %s: %d", org->publicID().c_str(), (int)org->arrivalCount());
	vsevent->stations.clear();
	for ( size_t i = 0; i < org->arrivalCount(); ++i ) {
		Arrival *arr = org->arrival(i);
		PickPtr pick = _cache.get<Pick>(arr->pickID());
		if ( !pick ) {
			SEISCOMP_DEBUG("cache.get<Pick>(\"%s\") failed to return pick", arr->pickID().c_str());
			continue;
		}
		Timeline::StationID id(pick->waveformID().networkCode(),
				pick->waveformID().stationCode());

		// if the station is not yet in the pickedStations set
		if ( vsevent->pickedStations.find(id) == vsevent->pickedStations.end() ) {
			double dist = arr->distance();
			if ( dist > dmax )
				dmax = dist;
			vsevent->pickedStations.insert(id);
			dsum += dist;
		}

		// if Station already used, continue
		if ( vsevent->stations.find(id) != vsevent->stations.end() )
			continue;

		VsTimeWindow &tw = vsevent->stations[id];
		tw.setStartTime(pick->time().value() - Core::TimeSpan(_twstarttime, 0));
		tw.setEndTime(pick->time().value() + Core::TimeSpan(_twendtime, 0));
		tw.setPickTime(pick->time().value());
		// Todo: make sure that at least three seconds of data after the pick
		// are available
	}

	// count the number of arrivals with epicentral distance (in degrees)
	// less than the threshold for use in deltaPick()
	vsevent->pickedStationsCount = vsevent->pickedStations.size();
	davg = dsum / (double) vsevent->pickedStationsCount;
	// calculate threshold
	dthresh = 0.5 * (dmax + davg);
	for ( size_t i = 0; i < org->arrivalCount(); i++ ) {
		Arrival *arr = org->arrival(i);
		PickPtr pick = _cache.get<Pick>(arr->pickID());
		if ( !pick ) {
			SEISCOMP_DEBUG("cache.get<Pick>(\"%s\") failed to return pick", arr->pickID().c_str());
			continue;
		}
		Timeline::StationID id(pick->waveformID().networkCode(),
				pick->waveformID().stationCode());
		if ( pick && arr->distance() < dthresh ) { // if the
			pickedThresholdStations.insert(id);
		}
	}

	vsevent->pickedThresholdStationsCount = pickedThresholdStations.size();

	vsevent->dthresh = dthresh;
	SEISCOMP_DEBUG("dmax; %f, davg: %f, dthresh: %f", dmax, davg, dthresh);

	// Get azimuthal gap
	vsevent->azGap = org->quality().azimuthalGap();
}

namespace {

/*!
 Input structure for final VS magnitude computation.
 This should also hold all values that are not dependent on the input
 magnitude of the grid search. The grid search should use those values to
 not compute values redundantly. And at the end these values (e.g. station
 magnitude) should be used to compute the RMS of the final magnitude.
 */
struct VsInput {
	float ZA, ZV, ZD, HA, HV, HD;
	WaveType PSclass;
	SoilClass SOILclass;
	float lat, lon;
	float mest;
};

}

/*!
 \brief Process all events in the cache
 */
void VsMagnitude::processEvents() {
	_lastProcessingTime = _currentTime;
	VsEvents::iterator it;
	for ( it = _events.begin(); it != _events.end(); ) {
		VsEvent *evt = it->second.get();
		EventPtr event = _cache.get<Event>(it->first);
		if ( event != NULL ) {
			if ( _currentTime < evt->expirationTime ) {
				process(evt, event.get());
				// only send the message / update the database if the event has a VS magnitude
				if ( evt->vsMagnitude ) {
					evt->update++;
					updateVSMagnitude(event.get(), evt);
				}
			} else {
				process(evt, event.get());
				// only send the message / update the database if the event has a VS magnitude
				if ( evt->vsMagnitude )
					updateVSMagnitude(event.get(), evt);
				_events.erase(it++);
				_publishedEvents[event->publicID()] = _currentTime;
				SEISCOMP_LOG(_processingInfoChannel,
						"Processing of event %s is finished.", event->publicID().c_str());
				// erase outdated events
				EventIDBuffer::iterator cev;
				for ( cev = _publishedEvents.begin();
						cev != _publishedEvents.end(); ) {
					if ( cev->second
							< (_currentTime - Core::TimeSpan(_timeout)) ) {
						_publishedEvents.erase(cev++);
						continue;
					}
					++cev;
				}
				continue;
			}
		} else
			SEISCOMP_WARNING("%s: event not found", it->first.c_str());
		++it;
	}
}

/*!
 \brief Process one event

 This is called by processEvents() when iterating over all events in the cache
 */
void VsMagnitude::process(VsEvent *evt, Event *event) {
	if ( evt->stations.empty() )
		return;
	Client::Inventory *inv = Client::Inventory::Instance();

	double stmag;
	double distdg, epicdist, azi1, azi2;
	WaveformStreamID wid;
	ReturnCode ret;
	Timeline::StationList unused;
	evt->allThresholdStationsCount = 0;
	vs.seteqlat(evt->lat);
	vs.seteqlon(evt->lon);

	vector<VsInput> inputs;

	SEISCOMP_LOG(_processingInfoChannel,
			"Start logging for event: %s", event->publicID().c_str());
	SEISCOMP_LOG(_processingInfoChannel, "update number: %d", evt->update);

	OriginPtr org = _cache.get<Origin>(event->preferredOriginID());
		if ( !org ){
			SEISCOMP_WARNING("Object %s not found in cache\nIs the cache size big enough?\n"
								"Have you subscribed to all necessary message groups?",
								event->preferredOriginID().c_str());
			return;
	}

	evt->staMags.clear();
	VsWindows::iterator it;
	for ( it = evt->stations.begin(); it != evt->stations.end(); ++it ) {
		Envelope venv, henv;
		Core::Time vtime, htime;
		string locationCode, channelCode;


		ret = _timeline.maxmimum(it->first, it->second.startTime(),
				it->second.endTime(), it->second.pickTime(), venv, vtime, henv,
				htime, locationCode, channelCode);

		if ( no_data == ret ){
			SEISCOMP_WARNING("No data available for %s.%s.%s", it->first.first.c_str(),
					it->first.second.c_str(),locationCode.c_str());
			unused.insert(it->first);
			continue;
		}

		DataModel::SensorLocation *loc;
		loc = inv->getSensorLocation(it->first.first, it->first.second,
				locationCode, it->second.pickTime());
		if ( loc == NULL ) {
			SEISCOMP_WARNING(
					"%s.%s.%s: sensor location not in inventory: ignoring", it->first.first.c_str(), it->first.second.c_str(), locationCode.c_str());
			continue;
		}

		Math::Geo::delazi(evt->lat, evt->lon, loc->latitude(), loc->longitude(),
				&distdg, &azi1, &azi2);
		epicdist = Math::Geo::deg2km(distdg);

		// if data is clipped or not enough to use it for magnitude
		// computation add the station to the overall count and then continue
		 if ( not_enough_data == ret || clipped_data == ret){
			 SEISCOMP_WARNING("Not enough data available for %s.%s.%s", it->first.first.c_str(),
						it->first.second.c_str(),locationCode.c_str());
			 unused.insert(it->first);
			 continue;
		 }

		// catch remaining errors
		if ( index_error == ret || undefined_problem == ret)
			continue;

		if ( _maxepicdist > 0 ){
			if( epicdist > _maxepicdist )
				continue;
		}

		inputs.resize(inputs.size() + 1);
		VsInput &input = inputs.back();
		input.lat = (float) loc->latitude();
		input.lon = (float) loc->longitude();

		SoilClass soilClass;
		float ca = siteEffect(input.lat, input.lon,
				std::max(venv.values[Acceleration], henv.values[Acceleration]),
				Acceleration, soilClass);
		float cv = siteEffect(input.lat, input.lon,
				std::max(venv.values[Velocity], henv.values[Velocity]),
				Velocity, soilClass);
		float cd = siteEffect(input.lat, input.lon,
				std::max(venv.values[Displacement], henv.values[Displacement]),
				Displacement, soilClass);

		// Convert from m to cm and apply site effect correction
		input.ZA = (float) (venv.values[Acceleration] / ca) * 100;
		input.ZV = (float) (venv.values[Velocity] / cv) * 100;
		input.ZD = (float) (venv.values[Displacement] / cd) * 100;

		input.HA = (float) (henv.values[Acceleration] / ca) * 100;
		input.HV = (float) (henv.values[Velocity] / cv) * 100;
		input.HD = (float) (henv.values[Displacement] / cd) * 100;

		input.PSclass =
				vs.psclass(input.ZA, input.ZV, input.HA, input.HV) == 0 ?
						P_Wave : S_Wave;
		input.SOILclass = soilClass;

		input.mest = vs.mest(vs.ground_motion_ratio(input.ZA, input.ZD),
				input.PSclass);

		// Record single station magnitudes
		Notifier::SetEnabled(true);
		_creationInfo.setCreationTime(Core::Time::GMT()); // was "_currentTime);" before but didn't allow sub-second precision.
		_creationInfo.setModificationTime(Core::None);
		DataModel::StationMagnitudePtr staMag = DataModel::StationMagnitude::Create();
		staMag->setMagnitude(RealQuantity(input.mest));
		staMag->setType("MVS");
		staMag->setCreationInfo(_creationInfo);
		wid.setNetworkCode(it->first.first);
		wid.setStationCode(it->first.second);
		wid.setLocationCode(locationCode);
		wid.setChannelCode(channelCode);
		staMag->setWaveformID(wid);
		org->add(staMag.get());
		evt->staMags.push_back(staMag);
		Notifier::SetEnabled(false);

		// Logging
		string resultstr;
		ostringstream out;
		out.precision(2);
		out.setf(ios::fixed, ios::floatfield);
		out << "Sensor: " << it->first.first << "." << locationCode << ".";
		out << it->first.second << "." << channelCode << "; ";
		out << "Wavetype: " << std::string(input.PSclass.toString()) << "; ";
		out << "Soil class: " << std::string(input.SOILclass.toString())
				<< "; ";
		out << "Magnitude: " << input.mest;
		resultstr = out.str();
		out.str("");
		SEISCOMP_LOG(_processingInfoChannel, "%s", resultstr.c_str());
		out.precision(2);
		out.setf(ios::fixed, ios::floatfield);
		out << "station lat: " << input.lat << "; station lon: " << input.lon;
		out << "; epicentral distance: " << epicdist << ";";
		resultstr = out.str();
		out.str("");
		SEISCOMP_LOG(_processingInfoChannel, "%s", resultstr.c_str());
		out.precision(2);
		out.setf(ios::scientific, ios::floatfield);
		out << "PGA(Z): " << input.ZA / 100. << "; PGV(Z): " << input.ZV / 100.;
		out << "; PGD(Z): " << input.ZD / 100.;
		resultstr = out.str();
		out.str("");
		SEISCOMP_LOG(_processingInfoChannel, "%s", resultstr.c_str());
		out << "PGA(H): " << input.HA / 100. << "; PGV(H): " << input.HV / 100.;
		out << "; PGD(H): " << input.HD / 100.;
		resultstr = out.str();
		SEISCOMP_LOG(_processingInfoChannel, "%s", resultstr.c_str());
	}

	if ( inputs.empty() ) {
		SEISCOMP_LOG(_processingInfoChannel,
				"End logging for event: %s", event->publicID().c_str());
		return;
	}

	// Grid search
	float mag = 0.5f;
	float minL = -1.0f;
	float minMag = mag;

	while ( mag <= 9.0f ) {
		float L = 0.0f;

		for ( size_t i = 0; i < inputs.size(); ++i ) {
			const VsInput &input = inputs[i];

			// Likelihood
			vs.setmag(mag);
			L += vs.likelihood(input.ZA, input.ZV, input.ZD, input.HA, input.HV,
					input.HD, input.PSclass, input.SOILclass, input.lat,
					input.lon);
		}

		if ( minL < 0 || minL > L ) {
			minL = L;
			minMag = mag;
		}

		mag += 0.01f;
	}

	// calculate the median of all station magnitudes
	size_t size = inputs.size();
	double *mestarray = new double[size];

	for ( size_t i = 0; i < size; ++i ) {
		const VsInput &input = inputs[i];
		mestarray[i] = input.mest;
	}

	nth_element(mestarray, mestarray + size / 2, mestarray + size);
	stmag = mestarray[size / 2];
	delete[] mestarray;

	// TODO: Define errors
	evt->vsMagnitude = minMag;
	evt->vsStationCount = inputs.size();


	if ( _timeline.pollbuffer(evt->lat, evt->lon,evt->dthresh,evt->allThresholdStationsCount) != no_problem){
		SEISCOMP_WARNING("Problems in the buffer polling function.");
		return;
	}


	// Use quality control functions to decide if the event is valid
	evt->isValid = false;
	double deltamag;
	double deltapick;
	if ( isEventValid(stmag, evt, evt->likelihood, deltamag, deltapick) ) {
		evt->isValid = true;
	}

	// logging
	string resultstr;
	ostringstream out;
	out.precision(2);
	out.setf(ios::fixed, ios::floatfield);
	out << "VS-mag: " << minMag << "; median single-station-mag: " << stmag;
	out << "; lat: " << evt->lat << "; lon: " << evt->lon;
	out << "; depth : " << evt->dep << " km";
	resultstr = out.str();
	out.str("");
	SEISCOMP_LOG(_processingInfoChannel, "%s", resultstr.c_str());

	out << "creation time: " << _currentTime.toString("%FT%T.%2fZ");
	out << "; origin time: " << evt->time.toString("%FT%T.%2fZ");
	Core::TimeSpan difftime = _currentTime - evt->time;
	Core::Time now = Core::Time::GMT();
	Core::TimeSpan difftime_oa = now - evt->originArrivalTime;
	Core::TimeSpan difftime_ct = now - evt->originCreationTime;
	out << "; t-diff: " << difftime.length();
	out.precision(3);
	out << "; time since origin arrival: " << difftime_oa.length();
	out << "; time since origin creation: " << difftime_ct.length();
	resultstr = out.str();
	out.str("");
	SEISCOMP_LOG(_processingInfoChannel, "%s", resultstr.c_str());

	out << "# picked stations: " << evt->pickedStationsCount; // all stations with picks
	out << "; # envelope streams: " << _timeline.StreamCount(); // all stations with envelope streams
	resultstr = out.str();
	out.str("");
	SEISCOMP_LOG(_processingInfoChannel, "%s", resultstr.c_str());

	// distance threshold for delta-pick quality criteria
	out.precision(2);
	out << "Distance threshold (dt): " << Math::Geo::deg2km(evt->dthresh)
			<< " km";
	out << "; # picked stations < dt: " << evt->pickedThresholdStationsCount;
	out << "; # envelope streams < dt: " << evt->allThresholdStationsCount;
	resultstr = out.str();
	out.str("");
	SEISCOMP_LOG(_processingInfoChannel, "%s", resultstr.c_str());

	if (evt->pickedStationsCount > evt->vsStationCount){
		out << "Stations not used for VS-mag: ";
		// find picked stations that don't contribute to the VS magnitude
		Timeline::StationList &sl = evt->pickedStations;
		for (Timeline::StationList::iterator it=sl.begin(); it!=sl.end(); ++it){
			if ( evt->stations.find(*it) == evt->stations.end() || unused.find(*it) != unused.end()) {
				out << (*it).first << '.' << (*it).second << ' ';
			}
		}
		resultstr = out.str();
		out.str("");
		SEISCOMP_LOG(_processingInfoChannel, "%s", resultstr.c_str());
	}

	out.precision(3);
	out << "Magnitude check: " << deltamag << "; Arrivals check: " << deltapick;
	out << "; Azimuthal gap: " << evt->azGap;
	resultstr = out.str();
	out.str("");
	SEISCOMP_LOG(_processingInfoChannel, "%s", resultstr.c_str());

	out.precision(2);
	out << "likelihood: " << evt->likelihood;
	resultstr = out.str();
	out.str("");
	SEISCOMP_LOG(_processingInfoChannel, "%s", resultstr.c_str());

	SEISCOMP_LOG(_processingInfoChannel,
			"End logging for event: %s", event->publicID().c_str());
}

/*!
 \brief  Save the VS Magnitude in the preferred origin of the event

 Update the preferred origin object of the event and send a message
 to the master so the magnitude can be saved to the database.

 \param event The sc3 event to which the magnitude belongs
 \param mag   The magnitude to be added to the event
 \param stacnt Station Count (number of stations used to calculate the magnitude)
 */
void VsMagnitude::updateVSMagnitude(Event *event, VsEvent *vsevt) {
	SEISCOMP_DEBUG(
			"%s: %s: VS = %.4f", event->publicID().c_str(), _currentTime.toString("%FT%T.%fZ").c_str(), *vsevt->vsMagnitude);
	SEISCOMP_DEBUG("Update number is %d", vsevt->update);

	OriginPtr org = _cache.get<Origin>(event->preferredOriginID());
	if ( org == NULL ) {
		SEISCOMP_WARNING("Object %s not found in cache\nIs the cache size big enough?\n"
							"Have you subscribed to all necessary message groups?",
							event->preferredOriginID().c_str());
		return;
	}

	Notifier::SetEnabled(true);
	_creationInfo.setCreationTime(Core::Time::GMT()); // was "_currentTime);" before but didn't allow sub-second precision.
	_creationInfo.setModificationTime(Core::None);
	_creationInfo.setVersion(Core::toString(vsevt->update));
	MagnitudePtr nmag = Magnitude::Create();
	nmag->setMagnitude(RealQuantity(*vsevt->vsMagnitude));
	nmag->setType("MVS");
	nmag->setStationCount(vsevt->vsStationCount);
	nmag->setCreationInfo(_creationInfo);
	org->add(nmag.get());
	for (StaMagArray::iterator it = vsevt->staMags.begin(); it != vsevt->staMags.end(); ++it) {
			const DataModel::StationMagnitude *staMag = (*it).get();
			nmag->add(new StationMagnitudeContribution(staMag->publicID(),staMag->magnitude().value()- *vsevt->vsMagnitude,1.0));
	}
	vsevt->staMags.clear();

	/// set a comment containing the update number
	/// if the update numbers of two successive comments for the
	/// same event are identical signals the logging script that
	/// the event processing has finished
	setComments(nmag.get(), "update", vsevt->update);

	/// set the likelihood of the estimate as a comment
	setComments(nmag.get(), "likelihood", vsevt->likelihood);

	/// send the message containing the vs magnitude and the
	/// two comments to scmaster
	Core::MessagePtr msg = Notifier::GetMessage();
	if ( connection() && msg )
		connection()->send(msg.get());
	Notifier::SetEnabled(false);
}

template<typename T>
bool VsMagnitude::setComments(Magnitude *mag, const std::string id,
		const T value) {
	for ( size_t i = 0; i < mag->commentCount(); ++i ) {
		CommentPtr cmnt = mag->comment(i);
		if ( cmnt->id() != id )
			continue;
		cmnt->setText(Core::toString(value));
		try {
			cmnt->creationInfo().setModificationTime(_currentTime);
		} catch ( ... ) {
			CreationInfo ci;
			ci.setModificationTime(_currentTime);
			cmnt->setCreationInfo(ci);

		}
		cmnt->update();
		return true;
	}
	// Create a new one
	CommentPtr cmt = new Comment();
	cmt->setId(id);
	cmt->setText(Core::toString(value));
	CreationInfo ci;
	ci.setCreationTime(Core::Time::GMT()); // was "_currentTime);" before but didn't allow sub-second precision.
	cmt->setCreationInfo(ci);

	if ( !mag->add(cmt.get()) ) {
		return false;
	}
	return true;
}
