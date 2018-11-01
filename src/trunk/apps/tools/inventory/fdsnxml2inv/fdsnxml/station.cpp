/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <fdsnxml/station.h>
#include <fdsnxml/equipment.h>
#include <fdsnxml/operator.h>
#include <fdsnxml/externalreference.h>
#include <fdsnxml/channel.h>
#include <algorithm>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace FDSNXML {


Station::MetaObject::MetaObject(const Core::RTTI *rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(objectProperty<LatitudeType>("latitude", "FDSNXML::LatitudeType", false, false, &Station::setLatitude, &Station::latitude));
	addProperty(objectProperty<LongitudeType>("longitude", "FDSNXML::LongitudeType", false, false, &Station::setLongitude, &Station::longitude));
	addProperty(objectProperty<DistanceType>("elevation", "FDSNXML::DistanceType", false, false, &Station::setElevation, &Station::elevation));
	addProperty(objectProperty<Site>("site", "FDSNXML::Site", false, false, &Station::setSite, &Station::site));
	addProperty(Core::simpleProperty("vault", "string", false, false, false, false, false, false, NULL, &Station::setVault, &Station::vault));
	addProperty(Core::simpleProperty("geology", "string", false, false, false, false, false, false, NULL, &Station::setGeology, &Station::geology));
	addProperty(Core::simpleProperty("creationDate", "datetime", false, false, false, false, false, false, NULL, &Station::setCreationDate, &Station::creationDate));
	addProperty(Core::simpleProperty("terminationDate", "datetime", false, false, false, false, true, false, NULL, &Station::setTerminationDate, &Station::terminationDate));
	addProperty(objectProperty<CounterType>("totalNumberChannels", "FDSNXML::CounterType", false, true, &Station::setTotalNumberChannels, &Station::totalNumberChannels));
	addProperty(objectProperty<CounterType>("selectedNumberChannels", "FDSNXML::CounterType", false, true, &Station::setSelectedNumberChannels, &Station::selectedNumberChannels));
	addProperty(arrayClassProperty<Equipment>("equipment", "FDSNXML::Equipment", &Station::equipmentCount, &Station::equipment, static_cast<bool (Station::*)(Equipment*)>(&Station::addEquipment), &Station::removeEquipment, static_cast<bool (Station::*)(Equipment*)>(&Station::removeEquipment)));
	addProperty(arrayClassProperty<Operator>("operators", "FDSNXML::Operator", &Station::operatorsCount, &Station::operators, static_cast<bool (Station::*)(Operator*)>(&Station::addOperators), &Station::removeOperators, static_cast<bool (Station::*)(Operator*)>(&Station::removeOperators)));
	addProperty(arrayClassProperty<ExternalReference>("externalReference", "FDSNXML::ExternalReference", &Station::externalReferenceCount, &Station::externalReference, static_cast<bool (Station::*)(ExternalReference*)>(&Station::addExternalReference), &Station::removeExternalReference, static_cast<bool (Station::*)(ExternalReference*)>(&Station::removeExternalReference)));
	addProperty(arrayClassProperty<Channel>("channel", "FDSNXML::Channel", &Station::channelCount, &Station::channel, static_cast<bool (Station::*)(Channel*)>(&Station::addChannel), &Station::removeChannel, static_cast<bool (Station::*)(Channel*)>(&Station::removeChannel)));
}


IMPLEMENT_RTTI(Station, "FDSNXML::Station", BaseNode)
IMPLEMENT_RTTI_METHODS(Station)
IMPLEMENT_METAOBJECT_DERIVED(Station, BaseNode)


Station::Station() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Station::Station(const Station &other)
 : BaseNode() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Station::~Station() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Station::operator==(const Station &rhs) const {
	if ( !(_latitude == rhs._latitude) )
		return false;
	if ( !(_longitude == rhs._longitude) )
		return false;
	if ( !(_elevation == rhs._elevation) )
		return false;
	if ( !(_site == rhs._site) )
		return false;
	if ( !(_vault == rhs._vault) )
		return false;
	if ( !(_geology == rhs._geology) )
		return false;
	if ( !(_creationDate == rhs._creationDate) )
		return false;
	if ( !(_terminationDate == rhs._terminationDate) )
		return false;
	if ( !(_totalNumberChannels == rhs._totalNumberChannels) )
		return false;
	if ( !(_selectedNumberChannels == rhs._selectedNumberChannels) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Station::setLatitude(const LatitudeType& latitude) {
	_latitude = latitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LatitudeType& Station::latitude() {
	return _latitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const LatitudeType& Station::latitude() const {
	return _latitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Station::setLongitude(const LongitudeType& longitude) {
	_longitude = longitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LongitudeType& Station::longitude() {
	return _longitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const LongitudeType& Station::longitude() const {
	return _longitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Station::setElevation(const DistanceType& elevation) {
	_elevation = elevation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DistanceType& Station::elevation() {
	return _elevation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DistanceType& Station::elevation() const {
	return _elevation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Station::setSite(const Site& site) {
	_site = site;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Site& Station::site() {
	return _site;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Site& Station::site() const {
	return _site;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Station::setVault(const std::string& vault) {
	_vault = vault;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Station::vault() const {
	return _vault;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Station::setGeology(const std::string& geology) {
	_geology = geology;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Station::geology() const {
	return _geology;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Station::setCreationDate(DateTime creationDate) {
	_creationDate = creationDate;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DateTime Station::creationDate() const {
	return _creationDate;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Station::setTerminationDate(const OPT(DateTime)& terminationDate) {
	_terminationDate = terminationDate;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DateTime Station::terminationDate() const {
	if ( _terminationDate )
		return *_terminationDate;
	throw Seiscomp::Core::ValueException("Station.terminationDate is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Station::setTotalNumberChannels(const OPT(CounterType)& totalNumberChannels) {
	_totalNumberChannels = totalNumberChannels;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CounterType& Station::totalNumberChannels() {
	if ( _totalNumberChannels )
		return *_totalNumberChannels;
	throw Seiscomp::Core::ValueException("Station.totalNumberChannels is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CounterType& Station::totalNumberChannels() const {
	if ( _totalNumberChannels )
		return *_totalNumberChannels;
	throw Seiscomp::Core::ValueException("Station.totalNumberChannels is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Station::setSelectedNumberChannels(const OPT(CounterType)& selectedNumberChannels) {
	_selectedNumberChannels = selectedNumberChannels;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CounterType& Station::selectedNumberChannels() {
	if ( _selectedNumberChannels )
		return *_selectedNumberChannels;
	throw Seiscomp::Core::ValueException("Station.selectedNumberChannels is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CounterType& Station::selectedNumberChannels() const {
	if ( _selectedNumberChannels )
		return *_selectedNumberChannels;
	throw Seiscomp::Core::ValueException("Station.selectedNumberChannels is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Station& Station::operator=(const Station &other) {
	BaseNode::operator=(other);
	_latitude = other._latitude;
	_longitude = other._longitude;
	_elevation = other._elevation;
	_site = other._site;
	_vault = other._vault;
	_geology = other._geology;
	_creationDate = other._creationDate;
	_terminationDate = other._terminationDate;
	_totalNumberChannels = other._totalNumberChannels;
	_selectedNumberChannels = other._selectedNumberChannels;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Station::equipmentCount() const {
	return _equipments.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Equipment* Station::equipment(size_t i) const {
	return _equipments[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Station::addEquipment(Equipment *obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_equipments.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Station::removeEquipment(Equipment *obj) {
	if ( obj == NULL )
		return false;

	std::vector<EquipmentPtr>::iterator it;
	it = std::find(_equipments.begin(), _equipments.end(), obj);
	// Element has not been found
	if ( it == _equipments.end() ) {
		SEISCOMP_ERROR("Station::removeEquipment(Equipment*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Station::removeEquipment(size_t i) {
	// index out of bounds
	if ( i >= _equipments.size() )
		return false;

	_equipments.erase(_equipments.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Station::operatorsCount() const {
	return _operatorss.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Operator* Station::operators(size_t i) const {
	return _operatorss[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Station::addOperators(Operator *obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_operatorss.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Station::removeOperators(Operator *obj) {
	if ( obj == NULL )
		return false;

	std::vector<OperatorPtr>::iterator it;
	it = std::find(_operatorss.begin(), _operatorss.end(), obj);
	// Element has not been found
	if ( it == _operatorss.end() ) {
		SEISCOMP_ERROR("Station::removeOperators(Operator*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Station::removeOperators(size_t i) {
	// index out of bounds
	if ( i >= _operatorss.size() )
		return false;

	_operatorss.erase(_operatorss.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Station::externalReferenceCount() const {
	return _externalReferences.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ExternalReference* Station::externalReference(size_t i) const {
	return _externalReferences[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Station::addExternalReference(ExternalReference *obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_externalReferences.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Station::removeExternalReference(ExternalReference *obj) {
	if ( obj == NULL )
		return false;

	std::vector<ExternalReferencePtr>::iterator it;
	it = std::find(_externalReferences.begin(), _externalReferences.end(), obj);
	// Element has not been found
	if ( it == _externalReferences.end() ) {
		SEISCOMP_ERROR("Station::removeExternalReference(ExternalReference*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Station::removeExternalReference(size_t i) {
	// index out of bounds
	if ( i >= _externalReferences.size() )
		return false;

	_externalReferences.erase(_externalReferences.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Station::channelCount() const {
	return _channels.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Channel* Station::channel(size_t i) const {
	return _channels[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Station::addChannel(Channel *obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_channels.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Station::removeChannel(Channel *obj) {
	if ( obj == NULL )
		return false;

	std::vector<ChannelPtr>::iterator it;
	it = std::find(_channels.begin(), _channels.end(), obj);
	// Element has not been found
	if ( it == _channels.end() ) {
		SEISCOMP_ERROR("Station::removeChannel(Channel*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Station::removeChannel(size_t i) {
	// index out of bounds
	if ( i >= _channels.size() )
		return false;

	_channels.erase(_channels.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
