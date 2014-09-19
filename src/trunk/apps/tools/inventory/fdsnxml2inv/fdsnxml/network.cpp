/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <fdsnxml/network.h>
#include <fdsnxml/station.h>
#include <algorithm>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace FDSNXML {


Network::MetaObject::MetaObject(const Core::RTTI *rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(objectProperty<CounterType>("totalNumberOfStations", "FDSNXML::CounterType", false, true, &Network::setTotalNumberOfStations, &Network::totalNumberOfStations));
	addProperty(objectProperty<CounterType>("selectedNumberStations", "FDSNXML::CounterType", false, true, &Network::setSelectedNumberStations, &Network::selectedNumberStations));
	addProperty(arrayClassProperty<Station>("station", "FDSNXML::Station", &Network::stationCount, &Network::station, static_cast<bool (Network::*)(Station*)>(&Network::addStation), &Network::removeStation, static_cast<bool (Network::*)(Station*)>(&Network::removeStation)));
}


IMPLEMENT_RTTI(Network, "FDSNXML::Network", BaseNode)
IMPLEMENT_RTTI_METHODS(Network)
IMPLEMENT_METAOBJECT_DERIVED(Network, BaseNode)


Network::Network() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Network::Network(const Network &other)
 : BaseNode() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Network::~Network() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::operator==(const Network &rhs) const {
	if ( !(_totalNumberOfStations == rhs._totalNumberOfStations) )
		return false;
	if ( !(_selectedNumberStations == rhs._selectedNumberStations) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Network::setTotalNumberOfStations(const OPT(CounterType)& totalNumberOfStations) {
	_totalNumberOfStations = totalNumberOfStations;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CounterType& Network::totalNumberOfStations() throw(Seiscomp::Core::ValueException) {
	if ( _totalNumberOfStations )
		return *_totalNumberOfStations;
	throw Seiscomp::Core::ValueException("Network.totalNumberOfStations is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CounterType& Network::totalNumberOfStations() const throw(Seiscomp::Core::ValueException) {
	if ( _totalNumberOfStations )
		return *_totalNumberOfStations;
	throw Seiscomp::Core::ValueException("Network.totalNumberOfStations is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Network::setSelectedNumberStations(const OPT(CounterType)& selectedNumberStations) {
	_selectedNumberStations = selectedNumberStations;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CounterType& Network::selectedNumberStations() throw(Seiscomp::Core::ValueException) {
	if ( _selectedNumberStations )
		return *_selectedNumberStations;
	throw Seiscomp::Core::ValueException("Network.selectedNumberStations is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CounterType& Network::selectedNumberStations() const throw(Seiscomp::Core::ValueException) {
	if ( _selectedNumberStations )
		return *_selectedNumberStations;
	throw Seiscomp::Core::ValueException("Network.selectedNumberStations is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Network& Network::operator=(const Network &other) {
	BaseNode::operator=(other);
	_totalNumberOfStations = other._totalNumberOfStations;
	_selectedNumberStations = other._selectedNumberStations;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Network::stationCount() const {
	return _stations.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Station* Network::station(size_t i) const {
	return _stations[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::addStation(Station *obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_stations.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::removeStation(Station *obj) {
	if ( obj == NULL )
		return false;

	std::vector<StationPtr>::iterator it;
	it = std::find(_stations.begin(), _stations.end(), obj);
	// Element has not been found
	if ( it == _stations.end() ) {
		SEISCOMP_ERROR("Network::removeStation(Station*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::removeStation(size_t i) {
	// index out of bounds
	if ( i >= _stations.size() )
		return false;

	_stations.erase(_stations.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
