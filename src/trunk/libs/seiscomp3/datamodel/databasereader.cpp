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


#define SEISCOMP_COMPONENT DatabaseReader
#include <seiscomp3/datamodel/databasereader.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/datamodel/eventparameters_package.h>
#include <seiscomp3/datamodel/config_package.h>
#include <seiscomp3/datamodel/qualitycontrol_package.h>
#include <seiscomp3/datamodel/inventory_package.h>
#include <seiscomp3/datamodel/routing_package.h>
#include <seiscomp3/datamodel/journaling_package.h>
#include <seiscomp3/datamodel/arclinklog_package.h>
#include <seiscomp3/datamodel/dataavailability_package.h>
#include <seiscomp3/datamodel/comment.h>
#include <boost/bind.hpp>

using namespace std;

namespace Seiscomp {
namespace DataModel {


DatabaseReader::DatabaseReader(Seiscomp::IO::DatabaseInterface* dbDriver)
: DatabaseArchive(dbDriver) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseReader::~DatabaseReader() {
	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObject* DatabaseReader::loadObject(const Seiscomp::Core::RTTI& classType,
                                         const std::string& publicID) {
	PublicObject* publicObject = getObject(classType, publicID);
	if ( publicObject == NULL )
		return NULL;

	Pick* pick = Pick::Cast(publicObject);
	if ( pick ) {
		load(pick);
		return pick;
	}

	Amplitude* amplitude = Amplitude::Cast(publicObject);
	if ( amplitude ) {
		load(amplitude);
		return amplitude;
	}

	Reading* reading = Reading::Cast(publicObject);
	if ( reading ) {
		load(reading);
		return reading;
	}

	Origin* origin = Origin::Cast(publicObject);
	if ( origin ) {
		load(origin);
		return origin;
	}

	StationMagnitude* stationMagnitude = StationMagnitude::Cast(publicObject);
	if ( stationMagnitude ) {
		load(stationMagnitude);
		return stationMagnitude;
	}

	Magnitude* magnitude = Magnitude::Cast(publicObject);
	if ( magnitude ) {
		load(magnitude);
		return magnitude;
	}

	FocalMechanism* focalMechanism = FocalMechanism::Cast(publicObject);
	if ( focalMechanism ) {
		load(focalMechanism);
		return focalMechanism;
	}

	MomentTensor* momentTensor = MomentTensor::Cast(publicObject);
	if ( momentTensor ) {
		load(momentTensor);
		return momentTensor;
	}

	MomentTensorStationContribution* momentTensorStationContribution = MomentTensorStationContribution::Cast(publicObject);
	if ( momentTensorStationContribution ) {
		load(momentTensorStationContribution);
		return momentTensorStationContribution;
	}

	Event* event = Event::Cast(publicObject);
	if ( event ) {
		load(event);
		return event;
	}

	ParameterSet* parameterSet = ParameterSet::Cast(publicObject);
	if ( parameterSet ) {
		load(parameterSet);
		return parameterSet;
	}

	Parameter* parameter = Parameter::Cast(publicObject);
	if ( parameter ) {
		load(parameter);
		return parameter;
	}

	ConfigModule* configModule = ConfigModule::Cast(publicObject);
	if ( configModule ) {
		load(configModule);
		return configModule;
	}

	ConfigStation* configStation = ConfigStation::Cast(publicObject);
	if ( configStation ) {
		load(configStation);
		return configStation;
	}

	StationGroup* stationGroup = StationGroup::Cast(publicObject);
	if ( stationGroup ) {
		load(stationGroup);
		return stationGroup;
	}

	AuxDevice* auxDevice = AuxDevice::Cast(publicObject);
	if ( auxDevice ) {
		load(auxDevice);
		return auxDevice;
	}

	Sensor* sensor = Sensor::Cast(publicObject);
	if ( sensor ) {
		load(sensor);
		return sensor;
	}

	Datalogger* datalogger = Datalogger::Cast(publicObject);
	if ( datalogger ) {
		load(datalogger);
		return datalogger;
	}

	Network* network = Network::Cast(publicObject);
	if ( network ) {
		load(network);
		return network;
	}

	Station* station = Station::Cast(publicObject);
	if ( station ) {
		load(station);
		return station;
	}

	SensorLocation* sensorLocation = SensorLocation::Cast(publicObject);
	if ( sensorLocation ) {
		load(sensorLocation);
		return sensorLocation;
	}

	Stream* stream = Stream::Cast(publicObject);
	if ( stream ) {
		load(stream);
		return stream;
	}

	Route* route = Route::Cast(publicObject);
	if ( route ) {
		load(route);
		return route;
	}

	ArclinkRequest* arclinkRequest = ArclinkRequest::Cast(publicObject);
	if ( arclinkRequest ) {
		load(arclinkRequest);
		return arclinkRequest;
	}

	DataExtent* dataExtent = DataExtent::Cast(publicObject);
	if ( dataExtent ) {
		load(dataExtent);
		return dataExtent;
	}

	return publicObject;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventParameters* DatabaseReader::loadEventParameters() {
	if ( !validInterface() ) return NULL;

	EventParameters *eventParameters = new EventParameters;

	load(eventParameters);

	SEISCOMP_DEBUG("objects in cache: %d", getCacheSize());
	
	return eventParameters;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(EventParameters* eventParameters) {
	size_t count = 0;

	count += loadPicks(eventParameters);
	{
		size_t elementCount = eventParameters->pickCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(eventParameters->pick(i));
	}

	count += loadAmplitudes(eventParameters);
	{
		size_t elementCount = eventParameters->amplitudeCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(eventParameters->amplitude(i));
	}

	count += loadReadings(eventParameters);
	{
		size_t elementCount = eventParameters->readingCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(eventParameters->reading(i));
	}

	count += loadOrigins(eventParameters);
	{
		size_t elementCount = eventParameters->originCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(eventParameters->origin(i));
	}

	count += loadFocalMechanisms(eventParameters);
	{
		size_t elementCount = eventParameters->focalMechanismCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(eventParameters->focalMechanism(i));
	}

	count += loadEvents(eventParameters);
	{
		size_t elementCount = eventParameters->eventCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(eventParameters->event(i));
	}

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadPicks(EventParameters* eventParameters) {
	if ( !validInterface() || eventParameters == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(eventParameters, Pick::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			eventParameters->add(Pick::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("EventParameters::add(Pick) -> Pick has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadAmplitudes(EventParameters* eventParameters) {
	if ( !validInterface() || eventParameters == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(eventParameters, Amplitude::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			eventParameters->add(Amplitude::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("EventParameters::add(Amplitude) -> Amplitude has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadReadings(EventParameters* eventParameters) {
	if ( !validInterface() || eventParameters == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(eventParameters, Reading::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			eventParameters->add(Reading::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("EventParameters::add(Reading) -> Reading has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadOrigins(EventParameters* eventParameters) {
	if ( !validInterface() || eventParameters == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(eventParameters, Origin::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			eventParameters->add(Origin::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("EventParameters::add(Origin) -> Origin has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadFocalMechanisms(EventParameters* eventParameters) {
	if ( !validInterface() || eventParameters == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(eventParameters, FocalMechanism::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			eventParameters->add(FocalMechanism::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("EventParameters::add(FocalMechanism) -> FocalMechanism has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadEvents(EventParameters* eventParameters) {
	if ( !validInterface() || eventParameters == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(eventParameters, Event::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			eventParameters->add(Event::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("EventParameters::add(Event) -> Event has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(Pick* pick) {
	size_t count = 0;

	count += loadComments(pick);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadComments(Pick* pick) {
	if ( !validInterface() || pick == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(pick, Comment::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			pick->add(Comment::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Pick::add(Comment) -> Comment has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(Amplitude* amplitude) {
	size_t count = 0;

	count += loadComments(amplitude);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadComments(Amplitude* amplitude) {
	if ( !validInterface() || amplitude == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(amplitude, Comment::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			amplitude->add(Comment::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Amplitude::add(Comment) -> Comment has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(Reading* reading) {
	size_t count = 0;

	count += loadPickReferences(reading);

	count += loadAmplitudeReferences(reading);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadPickReferences(Reading* reading) {
	if ( !validInterface() || reading == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(reading, PickReference::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			reading->add(PickReference::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Reading::add(PickReference) -> PickReference has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadAmplitudeReferences(Reading* reading) {
	if ( !validInterface() || reading == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(reading, AmplitudeReference::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			reading->add(AmplitudeReference::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Reading::add(AmplitudeReference) -> AmplitudeReference has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(Origin* origin) {
	size_t count = 0;

	count += loadComments(origin);

	count += loadCompositeTimes(origin);

	count += loadArrivals(origin);

	count += loadStationMagnitudes(origin);
	{
		size_t elementCount = origin->stationMagnitudeCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(origin->stationMagnitude(i));
	}

	count += loadMagnitudes(origin);
	{
		size_t elementCount = origin->magnitudeCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(origin->magnitude(i));
	}

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadComments(Origin* origin) {
	if ( !validInterface() || origin == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(origin, Comment::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			origin->add(Comment::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Origin::add(Comment) -> Comment has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadCompositeTimes(Origin* origin) {
	if ( !validInterface() || origin == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(origin, CompositeTime::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			origin->add(CompositeTime::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Origin::add(CompositeTime) -> CompositeTime has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadArrivals(Origin* origin) {
	if ( !validInterface() || origin == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(origin, Arrival::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			origin->add(Arrival::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Origin::add(Arrival) -> Arrival has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadStationMagnitudes(Origin* origin) {
	if ( !validInterface() || origin == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(origin, StationMagnitude::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			origin->add(StationMagnitude::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Origin::add(StationMagnitude) -> StationMagnitude has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadMagnitudes(Origin* origin) {
	if ( !validInterface() || origin == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(origin, Magnitude::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			origin->add(Magnitude::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Origin::add(Magnitude) -> Magnitude has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(StationMagnitude* stationMagnitude) {
	size_t count = 0;

	count += loadComments(stationMagnitude);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadComments(StationMagnitude* stationMagnitude) {
	if ( !validInterface() || stationMagnitude == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(stationMagnitude, Comment::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			stationMagnitude->add(Comment::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("StationMagnitude::add(Comment) -> Comment has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(Magnitude* magnitude) {
	size_t count = 0;

	count += loadComments(magnitude);

	count += loadStationMagnitudeContributions(magnitude);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadComments(Magnitude* magnitude) {
	if ( !validInterface() || magnitude == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(magnitude, Comment::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			magnitude->add(Comment::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Magnitude::add(Comment) -> Comment has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadStationMagnitudeContributions(Magnitude* magnitude) {
	if ( !validInterface() || magnitude == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(magnitude, StationMagnitudeContribution::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			magnitude->add(StationMagnitudeContribution::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Magnitude::add(StationMagnitudeContribution) -> StationMagnitudeContribution has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(FocalMechanism* focalMechanism) {
	size_t count = 0;

	count += loadComments(focalMechanism);

	count += loadMomentTensors(focalMechanism);
	{
		size_t elementCount = focalMechanism->momentTensorCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(focalMechanism->momentTensor(i));
	}

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadComments(FocalMechanism* focalMechanism) {
	if ( !validInterface() || focalMechanism == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(focalMechanism, Comment::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			focalMechanism->add(Comment::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("FocalMechanism::add(Comment) -> Comment has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadMomentTensors(FocalMechanism* focalMechanism) {
	if ( !validInterface() || focalMechanism == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(focalMechanism, MomentTensor::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			focalMechanism->add(MomentTensor::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("FocalMechanism::add(MomentTensor) -> MomentTensor has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(MomentTensor* momentTensor) {
	size_t count = 0;

	count += loadComments(momentTensor);

	count += loadDataUseds(momentTensor);

	count += loadMomentTensorPhaseSettings(momentTensor);

	count += loadMomentTensorStationContributions(momentTensor);
	{
		size_t elementCount = momentTensor->momentTensorStationContributionCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(momentTensor->momentTensorStationContribution(i));
	}

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadComments(MomentTensor* momentTensor) {
	if ( !validInterface() || momentTensor == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(momentTensor, Comment::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			momentTensor->add(Comment::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("MomentTensor::add(Comment) -> Comment has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadDataUseds(MomentTensor* momentTensor) {
	if ( !validInterface() || momentTensor == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(momentTensor, DataUsed::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			momentTensor->add(DataUsed::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("MomentTensor::add(DataUsed) -> DataUsed has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadMomentTensorPhaseSettings(MomentTensor* momentTensor) {
	if ( !validInterface() || momentTensor == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(momentTensor, MomentTensorPhaseSetting::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			momentTensor->add(MomentTensorPhaseSetting::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("MomentTensor::add(MomentTensorPhaseSetting) -> MomentTensorPhaseSetting has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadMomentTensorStationContributions(MomentTensor* momentTensor) {
	if ( !validInterface() || momentTensor == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(momentTensor, MomentTensorStationContribution::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			momentTensor->add(MomentTensorStationContribution::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("MomentTensor::add(MomentTensorStationContribution) -> MomentTensorStationContribution has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(MomentTensorStationContribution* momentTensorStationContribution) {
	size_t count = 0;

	count += loadMomentTensorComponentContributions(momentTensorStationContribution);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadMomentTensorComponentContributions(MomentTensorStationContribution* momentTensorStationContribution) {
	if ( !validInterface() || momentTensorStationContribution == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(momentTensorStationContribution, MomentTensorComponentContribution::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			momentTensorStationContribution->add(MomentTensorComponentContribution::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("MomentTensorStationContribution::add(MomentTensorComponentContribution) -> MomentTensorComponentContribution has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(Event* event) {
	size_t count = 0;

	count += loadEventDescriptions(event);

	count += loadComments(event);

	count += loadOriginReferences(event);

	count += loadFocalMechanismReferences(event);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadEventDescriptions(Event* event) {
	if ( !validInterface() || event == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(event, EventDescription::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			event->add(EventDescription::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Event::add(EventDescription) -> EventDescription has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadComments(Event* event) {
	if ( !validInterface() || event == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(event, Comment::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			event->add(Comment::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Event::add(Comment) -> Comment has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadOriginReferences(Event* event) {
	if ( !validInterface() || event == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(event, OriginReference::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			event->add(OriginReference::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Event::add(OriginReference) -> OriginReference has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadFocalMechanismReferences(Event* event) {
	if ( !validInterface() || event == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(event, FocalMechanismReference::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			event->add(FocalMechanismReference::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Event::add(FocalMechanismReference) -> FocalMechanismReference has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Config* DatabaseReader::loadConfig() {
	if ( !validInterface() ) return NULL;

	Config *config = new Config;

	load(config);

	SEISCOMP_DEBUG("objects in cache: %d", getCacheSize());
	
	return config;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(Config* config) {
	size_t count = 0;

	count += loadParameterSets(config);
	{
		size_t elementCount = config->parameterSetCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(config->parameterSet(i));
	}

	count += loadConfigModules(config);
	{
		size_t elementCount = config->configModuleCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(config->configModule(i));
	}

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadParameterSets(Config* config) {
	if ( !validInterface() || config == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(config, ParameterSet::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			config->add(ParameterSet::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Config::add(ParameterSet) -> ParameterSet has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadConfigModules(Config* config) {
	if ( !validInterface() || config == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(config, ConfigModule::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			config->add(ConfigModule::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Config::add(ConfigModule) -> ConfigModule has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(ParameterSet* parameterSet) {
	size_t count = 0;

	count += loadParameters(parameterSet);
	{
		size_t elementCount = parameterSet->parameterCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(parameterSet->parameter(i));
	}

	count += loadComments(parameterSet);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadParameters(ParameterSet* parameterSet) {
	if ( !validInterface() || parameterSet == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(parameterSet, Parameter::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			parameterSet->add(Parameter::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("ParameterSet::add(Parameter) -> Parameter has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadComments(ParameterSet* parameterSet) {
	if ( !validInterface() || parameterSet == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(parameterSet, Comment::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			parameterSet->add(Comment::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("ParameterSet::add(Comment) -> Comment has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(Parameter* parameter) {
	size_t count = 0;

	count += loadComments(parameter);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadComments(Parameter* parameter) {
	if ( !validInterface() || parameter == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(parameter, Comment::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			parameter->add(Comment::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Parameter::add(Comment) -> Comment has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(ConfigModule* configModule) {
	size_t count = 0;

	count += loadConfigStations(configModule);
	{
		size_t elementCount = configModule->configStationCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(configModule->configStation(i));
	}

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadConfigStations(ConfigModule* configModule) {
	if ( !validInterface() || configModule == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(configModule, ConfigStation::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			configModule->add(ConfigStation::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("ConfigModule::add(ConfigStation) -> ConfigStation has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(ConfigStation* configStation) {
	size_t count = 0;

	count += loadSetups(configStation);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadSetups(ConfigStation* configStation) {
	if ( !validInterface() || configStation == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(configStation, Setup::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			configStation->add(Setup::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("ConfigStation::add(Setup) -> Setup has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QualityControl* DatabaseReader::loadQualityControl() {
	if ( !validInterface() ) return NULL;

	QualityControl *qualityControl = new QualityControl;

	load(qualityControl);

	SEISCOMP_DEBUG("objects in cache: %d", getCacheSize());
	
	return qualityControl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(QualityControl* qualityControl) {
	size_t count = 0;

	count += loadQCLogs(qualityControl);

	count += loadWaveformQualitys(qualityControl);

	count += loadOutages(qualityControl);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadQCLogs(QualityControl* qualityControl) {
	if ( !validInterface() || qualityControl == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(qualityControl, QCLog::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			qualityControl->add(QCLog::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("QualityControl::add(QCLog) -> QCLog has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadWaveformQualitys(QualityControl* qualityControl) {
	if ( !validInterface() || qualityControl == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(qualityControl, WaveformQuality::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			qualityControl->add(WaveformQuality::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("QualityControl::add(WaveformQuality) -> WaveformQuality has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadOutages(QualityControl* qualityControl) {
	if ( !validInterface() || qualityControl == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(qualityControl, Outage::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			qualityControl->add(Outage::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("QualityControl::add(Outage) -> Outage has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Inventory* DatabaseReader::loadInventory() {
	if ( !validInterface() ) return NULL;

	Inventory *inventory = new Inventory;

	load(inventory);

	SEISCOMP_DEBUG("objects in cache: %d", getCacheSize());
	
	return inventory;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(Inventory* inventory) {
	size_t count = 0;

	count += loadStationGroups(inventory);
	{
		size_t elementCount = inventory->stationGroupCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(inventory->stationGroup(i));
	}

	count += loadAuxDevices(inventory);
	{
		size_t elementCount = inventory->auxDeviceCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(inventory->auxDevice(i));
	}

	count += loadSensors(inventory);
	{
		size_t elementCount = inventory->sensorCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(inventory->sensor(i));
	}

	count += loadDataloggers(inventory);
	{
		size_t elementCount = inventory->dataloggerCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(inventory->datalogger(i));
	}

	count += loadResponsePAZs(inventory);

	count += loadResponseFIRs(inventory);

	if ( supportsVersion<0,10>() )
		count += loadResponseIIRs(inventory);

	count += loadResponsePolynomials(inventory);

	if ( supportsVersion<0,8>() )
		count += loadResponseFAPs(inventory);

	count += loadNetworks(inventory);
	{
		size_t elementCount = inventory->networkCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(inventory->network(i));
	}

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadStationGroups(Inventory* inventory) {
	if ( !validInterface() || inventory == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(inventory, StationGroup::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			inventory->add(StationGroup::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Inventory::add(StationGroup) -> StationGroup has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadAuxDevices(Inventory* inventory) {
	if ( !validInterface() || inventory == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(inventory, AuxDevice::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			inventory->add(AuxDevice::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Inventory::add(AuxDevice) -> AuxDevice has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadSensors(Inventory* inventory) {
	if ( !validInterface() || inventory == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(inventory, Sensor::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			inventory->add(Sensor::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Inventory::add(Sensor) -> Sensor has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadDataloggers(Inventory* inventory) {
	if ( !validInterface() || inventory == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(inventory, Datalogger::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			inventory->add(Datalogger::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Inventory::add(Datalogger) -> Datalogger has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadResponsePAZs(Inventory* inventory) {
	if ( !validInterface() || inventory == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(inventory, ResponsePAZ::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			inventory->add(ResponsePAZ::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Inventory::add(ResponsePAZ) -> ResponsePAZ has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadResponseFIRs(Inventory* inventory) {
	if ( !validInterface() || inventory == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(inventory, ResponseFIR::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			inventory->add(ResponseFIR::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Inventory::add(ResponseFIR) -> ResponseFIR has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadResponseIIRs(Inventory* inventory) {
	if ( !validInterface() || inventory == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(inventory, ResponseIIR::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			inventory->add(ResponseIIR::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Inventory::add(ResponseIIR) -> ResponseIIR has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadResponsePolynomials(Inventory* inventory) {
	if ( !validInterface() || inventory == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(inventory, ResponsePolynomial::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			inventory->add(ResponsePolynomial::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Inventory::add(ResponsePolynomial) -> ResponsePolynomial has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadResponseFAPs(Inventory* inventory) {
	if ( !validInterface() || inventory == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(inventory, ResponseFAP::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			inventory->add(ResponseFAP::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Inventory::add(ResponseFAP) -> ResponseFAP has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadNetworks(Inventory* inventory) {
	if ( !validInterface() || inventory == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(inventory, Network::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			inventory->add(Network::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Inventory::add(Network) -> Network has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(StationGroup* stationGroup) {
	size_t count = 0;

	count += loadStationReferences(stationGroup);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadStationReferences(StationGroup* stationGroup) {
	if ( !validInterface() || stationGroup == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(stationGroup, StationReference::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			stationGroup->add(StationReference::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("StationGroup::add(StationReference) -> StationReference has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(AuxDevice* auxDevice) {
	size_t count = 0;

	count += loadAuxSources(auxDevice);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadAuxSources(AuxDevice* auxDevice) {
	if ( !validInterface() || auxDevice == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(auxDevice, AuxSource::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			auxDevice->add(AuxSource::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("AuxDevice::add(AuxSource) -> AuxSource has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(Sensor* sensor) {
	size_t count = 0;

	count += loadSensorCalibrations(sensor);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadSensorCalibrations(Sensor* sensor) {
	if ( !validInterface() || sensor == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(sensor, SensorCalibration::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			sensor->add(SensorCalibration::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Sensor::add(SensorCalibration) -> SensorCalibration has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(Datalogger* datalogger) {
	size_t count = 0;

	count += loadDataloggerCalibrations(datalogger);

	count += loadDecimations(datalogger);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadDataloggerCalibrations(Datalogger* datalogger) {
	if ( !validInterface() || datalogger == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(datalogger, DataloggerCalibration::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			datalogger->add(DataloggerCalibration::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Datalogger::add(DataloggerCalibration) -> DataloggerCalibration has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadDecimations(Datalogger* datalogger) {
	if ( !validInterface() || datalogger == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(datalogger, Decimation::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			datalogger->add(Decimation::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Datalogger::add(Decimation) -> Decimation has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(Network* network) {
	size_t count = 0;

	if ( supportsVersion<0,10>() )
		count += loadComments(network);

	count += loadStations(network);
	{
		size_t elementCount = network->stationCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(network->station(i));
	}

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadComments(Network* network) {
	if ( !validInterface() || network == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(network, Comment::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			network->add(Comment::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Network::add(Comment) -> Comment has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadStations(Network* network) {
	if ( !validInterface() || network == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(network, Station::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			network->add(Station::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Network::add(Station) -> Station has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(Station* station) {
	size_t count = 0;

	if ( supportsVersion<0,10>() )
		count += loadComments(station);

	count += loadSensorLocations(station);
	{
		size_t elementCount = station->sensorLocationCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(station->sensorLocation(i));
	}

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadComments(Station* station) {
	if ( !validInterface() || station == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(station, Comment::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			station->add(Comment::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Station::add(Comment) -> Comment has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadSensorLocations(Station* station) {
	if ( !validInterface() || station == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(station, SensorLocation::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			station->add(SensorLocation::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Station::add(SensorLocation) -> SensorLocation has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(SensorLocation* sensorLocation) {
	size_t count = 0;

	if ( supportsVersion<0,10>() )
		count += loadComments(sensorLocation);

	count += loadAuxStreams(sensorLocation);

	count += loadStreams(sensorLocation);
	{
		size_t elementCount = sensorLocation->streamCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(sensorLocation->stream(i));
	}

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadComments(SensorLocation* sensorLocation) {
	if ( !validInterface() || sensorLocation == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(sensorLocation, Comment::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			sensorLocation->add(Comment::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("SensorLocation::add(Comment) -> Comment has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadAuxStreams(SensorLocation* sensorLocation) {
	if ( !validInterface() || sensorLocation == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(sensorLocation, AuxStream::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			sensorLocation->add(AuxStream::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("SensorLocation::add(AuxStream) -> AuxStream has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadStreams(SensorLocation* sensorLocation) {
	if ( !validInterface() || sensorLocation == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(sensorLocation, Stream::TypeInfo(), isLowerVersion<0,10>());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			sensorLocation->add(Stream::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("SensorLocation::add(Stream) -> Stream has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(Stream* stream) {
	size_t count = 0;

	if ( supportsVersion<0,10>() )
		count += loadComments(stream);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadComments(Stream* stream) {
	if ( !validInterface() || stream == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(stream, Comment::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			stream->add(Comment::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Stream::add(Comment) -> Comment has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Routing* DatabaseReader::loadRouting() {
	if ( !validInterface() ) return NULL;

	Routing *routing = new Routing;

	load(routing);

	SEISCOMP_DEBUG("objects in cache: %d", getCacheSize());
	
	return routing;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(Routing* routing) {
	size_t count = 0;

	count += loadRoutes(routing);
	{
		size_t elementCount = routing->routeCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(routing->route(i));
	}

	count += loadAccesss(routing);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadRoutes(Routing* routing) {
	if ( !validInterface() || routing == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(routing, Route::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			routing->add(Route::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Routing::add(Route) -> Route has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadAccesss(Routing* routing) {
	if ( !validInterface() || routing == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(routing, Access::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			routing->add(Access::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Routing::add(Access) -> Access has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(Route* route) {
	size_t count = 0;

	count += loadRouteArclinks(route);

	count += loadRouteSeedlinks(route);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadRouteArclinks(Route* route) {
	if ( !validInterface() || route == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(route, RouteArclink::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			route->add(RouteArclink::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Route::add(RouteArclink) -> RouteArclink has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadRouteSeedlinks(Route* route) {
	if ( !validInterface() || route == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(route, RouteSeedlink::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			route->add(RouteSeedlink::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Route::add(RouteSeedlink) -> RouteSeedlink has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Journaling* DatabaseReader::loadJournaling() {
	if ( !validInterface() ) return NULL;

	Journaling *journaling = new Journaling;

	load(journaling);

	SEISCOMP_DEBUG("objects in cache: %d", getCacheSize());
	
	return journaling;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(Journaling* journaling) {
	size_t count = 0;

	count += loadJournalEntrys(journaling);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadJournalEntrys(Journaling* journaling) {
	if ( !validInterface() || journaling == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(journaling, JournalEntry::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			journaling->add(JournalEntry::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Journaling::add(JournalEntry) -> JournalEntry has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkLog* DatabaseReader::loadArclinkLog() {
	if ( !validInterface() ) return NULL;

	ArclinkLog *arclinkLog = new ArclinkLog;

	load(arclinkLog);

	SEISCOMP_DEBUG("objects in cache: %d", getCacheSize());
	
	return arclinkLog;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(ArclinkLog* arclinkLog) {
	size_t count = 0;

	count += loadArclinkRequests(arclinkLog);
	{
		size_t elementCount = arclinkLog->arclinkRequestCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(arclinkLog->arclinkRequest(i));
	}

	count += loadArclinkUsers(arclinkLog);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadArclinkRequests(ArclinkLog* arclinkLog) {
	if ( !validInterface() || arclinkLog == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(arclinkLog, ArclinkRequest::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			arclinkLog->add(ArclinkRequest::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("ArclinkLog::add(ArclinkRequest) -> ArclinkRequest has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadArclinkUsers(ArclinkLog* arclinkLog) {
	if ( !validInterface() || arclinkLog == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(arclinkLog, ArclinkUser::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			arclinkLog->add(ArclinkUser::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("ArclinkLog::add(ArclinkUser) -> ArclinkUser has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(ArclinkRequest* arclinkRequest) {
	size_t count = 0;

	count += loadArclinkStatusLines(arclinkRequest);

	count += loadArclinkRequestLines(arclinkRequest);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadArclinkStatusLines(ArclinkRequest* arclinkRequest) {
	if ( !validInterface() || arclinkRequest == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(arclinkRequest, ArclinkStatusLine::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			arclinkRequest->add(ArclinkStatusLine::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("ArclinkRequest::add(ArclinkStatusLine) -> ArclinkStatusLine has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadArclinkRequestLines(ArclinkRequest* arclinkRequest) {
	if ( !validInterface() || arclinkRequest == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(arclinkRequest, ArclinkRequestLine::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			arclinkRequest->add(ArclinkRequestLine::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("ArclinkRequest::add(ArclinkRequestLine) -> ArclinkRequestLine has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataAvailability* DatabaseReader::loadDataAvailability() {
	if ( !validInterface() ) return NULL;

	DataAvailability *dataAvailability = new DataAvailability;

	load(dataAvailability);

	SEISCOMP_DEBUG("objects in cache: %d", getCacheSize());
	
	return dataAvailability;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(DataAvailability* dataAvailability) {
	size_t count = 0;

	count += loadDataExtents(dataAvailability);
	{
		size_t elementCount = dataAvailability->dataExtentCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(dataAvailability->dataExtent(i));
	}

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadDataExtents(DataAvailability* dataAvailability) {
	if ( !validInterface() || dataAvailability == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(dataAvailability, DataExtent::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			dataAvailability->add(DataExtent::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("DataAvailability::add(DataExtent) -> DataExtent has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::load(DataExtent* dataExtent) {
	size_t count = 0;

	count += loadDataSegments(dataExtent);

	count += loadDataAttributeExtents(dataExtent);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadDataSegments(DataExtent* dataExtent) {
	if ( !validInterface() || dataExtent == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(dataExtent, DataSegment::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			dataExtent->add(DataSegment::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("DataExtent::add(DataSegment) -> DataSegment has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseReader::loadDataAttributeExtents(DataExtent* dataExtent) {
	if ( !validInterface() || dataExtent == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(dataExtent, DataAttributeExtent::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			dataExtent->add(DataAttributeExtent::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("DataExtent::add(DataAttributeExtent) -> DataAttributeExtent has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
