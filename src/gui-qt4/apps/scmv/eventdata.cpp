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


#include "eventdata.h"

#include <iostream>
#include <algorithm>

using namespace Seiscomp;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


template <typename DataType>
bool idObjectComparison(std::string id, DataType object) {
	if ( id != object.id() )
		return false;
	return true;
}



template <typename DataType, typename CollectionType>
void addObject(DataType& data, CollectionType& collection) {
	collection.push_back(data);
}



template <typename DataType, typename CollectionType>
DataType* findObjectWithId(const std::string& id, CollectionType& collection) {
	typename CollectionType::iterator it;
	it = std::find_if(collection.begin(),
	                  collection.end(),
	                  std::bind1st(std::ptr_fun(idObjectComparison<DataType>), id));

	if ( it != collection.end() )
		return &(*it);

	return NULL;
}



template <typename CollectionType>
void removeDataOlderThan(CollectionType& collection, const Core::TimeSpan& objectLifeSpan) {
	typename CollectionType::iterator it = collection.begin();
	for ( ; it != collection.end(); it++ ) {
		Core::Time referenceTime = Core::Time::GMT() - objectLifeSpan;
		if ( it->containerCreationTime() > referenceTime ) break;

		it = collection.erase(it);
	}
}


} // namespace
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventDataRepository::EventDataRepository()
 : _eventDataObjectLifeSpan(0.0) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventDataRepository::EventDataRepository(DataModel::DatabaseArchive* databaseArchive,
                                       const Core::TimeSpan& timeSpan)
 : _objectTimeSpanBuffer(databaseArchive, timeSpan),
   _eventDataObjectLifeSpan(timeSpan) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventDataRepository::event_iterator EventDataRepository::eventsBegin() {
	return _eventDataCollection.begin();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventDataRepository::event_iterator EventDataRepository::eventsEnd() {
	return _eventDataCollection.end();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventDataRepository::const_event_iterator EventDataRepository::eventsBegin() const {
	return _eventDataCollection.begin();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventDataRepository::const_event_iterator EventDataRepository::eventsEnd() const {
	return _eventDataCollection.end();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int EventDataRepository::eventCount() const {
	return _eventDataCollection.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventDataRepository::setDatabaseArchive(Seiscomp::DataModel::DatabaseArchive* databaseArchive) {
	_objectTimeSpanBuffer.setDatabaseArchive(databaseArchive);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventDataRepository::setEventDataLifeSpan(const Seiscomp::Core::TimeSpan& timeSpan) {
	_objectTimeSpanBuffer.setTimeSpan(timeSpan);
	_eventDataObjectLifeSpan = timeSpan;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventDataRepository::addEvent(DataModel::Event* event,
                                   Gui::OriginSymbol* originSymbol,
                                   Gui::TensorSymbol *tensorSymbol,
                                   bool passedFilter) {
	std::string preferredOriginId = event->preferredOriginID();
	DataModel::Origin* preferredOrigin = findOrigin(preferredOriginId);
	Core::Time preferredOriginTime = preferredOrigin->time();

	if ( preferredOriginTime < Core::Time::GMT() - _eventDataObjectLifeSpan ) return false;

	EventDataCollection::iterator it = eventsBegin();
	for ( ; it != eventsEnd(); it++ ) {
		std::string tmpPreferredOriginId = it->object()->preferredOriginID();
		DataModel::Origin* tmpPreferredOrigin = findOrigin(tmpPreferredOriginId);
		Core::Time tmpPreferredOriginTime = tmpPreferredOrigin->time();

		if ( tmpPreferredOriginTime > preferredOriginTime ) break;
	}

	EventData eventData(event, originSymbol, tensorSymbol, passedFilter);
	_eventDataCollection.insert(it, eventData);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventDataRepository::addOrigin(DataModel::Origin* origin) {
	_objectTimeSpanBuffer.feed(origin);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventDataRepository::addFocalMechanism(DataModel::FocalMechanism* fm) {
	_objectTimeSpanBuffer.feed(fm);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventDataRepository::addMagnitude(DataModel::Magnitude* magnitude) {
	_objectTimeSpanBuffer.feed(magnitude);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventDataRepository::addAmplitude(Seiscomp::DataModel::Amplitude* amplitude) {
	_objectTimeSpanBuffer.feed(amplitude);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventDataRepository::addArrival(DataModel::Arrival* arrival) {
	removeDataOlderThan(_arrivalDataCollection, _eventDataObjectLifeSpan);

	ArrivalData arrivalData(arrival);
	addObject(arrivalData, _arrivalDataCollection);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventDataRepository::addPick(DataModel::Pick* arrival) {
	_objectTimeSpanBuffer.feed(arrival);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventData* EventDataRepository::findEvent(const std::string& id) {
	return findObjectWithId<EventData>(id, _eventDataCollection);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventData *EventDataRepository::findNextExpiredEvent() {
	Core::Time maxTime = Core::Time::GMT() - _eventDataObjectLifeSpan;

	EventDataCollection::iterator it;
	for ( it = _eventDataCollection.begin(); it != _eventDataCollection.end(); ++it ) {
		DataModel::Origin *origin = findOrigin(it->object()->preferredOriginID());
		if ( origin == NULL )
			return &*it;

		if ( origin->time().value() < maxTime )
			return &*it;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventData* EventDataRepository::findLatestEvent() {
	if ( !_eventDataCollection.empty() )
		return &_eventDataCollection.back();
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArrivalData* EventDataRepository::findArrivalwithPickId(const std::string& pickId) {
	return findObjectWithId<ArrivalData>(pickId, _arrivalDataCollection);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Origin* EventDataRepository::findOrigin(const std::string& id) {
	DataModel::PublicObject* publicObject = _objectTimeSpanBuffer.find(DataModel::Origin::TypeInfo(), id);
	return DataModel::Origin::Cast(publicObject);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::FocalMechanism* EventDataRepository::findFocalMechanism(const std::string& id) {
	DataModel::PublicObject* publicObject = _objectTimeSpanBuffer.find(DataModel::FocalMechanism::TypeInfo(), id);
	return DataModel::FocalMechanism::Cast(publicObject);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Magnitude* EventDataRepository::findMagnitude(const std::string& id) {
	DataModel::PublicObject* publicObject = _objectTimeSpanBuffer.find(DataModel::Magnitude::TypeInfo(), id);
	return DataModel::Magnitude::Cast(publicObject);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventDataRepository::removeEvent(const std::string& id) {
	EventDataCollection::iterator it;
	it = std::find_if(_eventDataCollection.begin(),
	                  _eventDataCollection.end(),
	                  std::bind1st(std::ptr_fun(idObjectComparison<EventData>), id));

	if ( it != _eventDataCollection.end() )
		_eventDataCollection.erase(it);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
