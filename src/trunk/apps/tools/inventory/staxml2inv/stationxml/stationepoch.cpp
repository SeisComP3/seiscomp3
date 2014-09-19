/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <stationxml/stationepoch.h>
#include <stationxml/equipment.h>
#include <stationxml/operator.h>
#include <stationxml/externaldocument.h>
#include <stationxml/channel.h>
#include <algorithm>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace StationXML {


StationEpoch::MetaObject::MetaObject(const Core::RTTI* rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(objectProperty<LatType>("latitude", "StationXML::LatType", false, false, &StationEpoch::setLatitude, &StationEpoch::latitude));
	addProperty(objectProperty<LonType>("longitude", "StationXML::LonType", false, false, &StationEpoch::setLongitude, &StationEpoch::longitude));
	addProperty(objectProperty<DistanceType>("elevation", "StationXML::DistanceType", false, false, &StationEpoch::setElevation, &StationEpoch::elevation));
	addProperty(objectProperty<Site>("site", "StationXML::Site", false, false, &StationEpoch::setSite, &StationEpoch::site));
	addProperty(Core::simpleProperty("vault", "string", false, false, false, false, false, false, NULL, &StationEpoch::setVault, &StationEpoch::vault));
	addProperty(Core::simpleProperty("geology", "string", false, false, false, false, false, false, NULL, &StationEpoch::setGeology, &StationEpoch::geology));
	addProperty(Core::simpleProperty("structure", "string", false, false, false, false, false, false, NULL, &StationEpoch::setStructure, &StationEpoch::structure));
	addProperty(Core::simpleProperty("creationDate", "datetime", false, false, false, false, false, false, NULL, &StationEpoch::setCreationDate, &StationEpoch::creationDate));
	addProperty(Core::simpleProperty("terminationDate", "datetime", false, false, false, false, true, false, NULL, &StationEpoch::setTerminationDate, &StationEpoch::terminationDate));
	addProperty(Core::simpleProperty("numberRecorders", "int", false, false, false, false, true, false, NULL, &StationEpoch::setNumberRecorders, &StationEpoch::numberRecorders));
	addProperty(Core::simpleProperty("totalNumberChannels", "int", false, false, false, false, true, false, NULL, &StationEpoch::setTotalNumberChannels, &StationEpoch::totalNumberChannels));
	addProperty(Core::simpleProperty("selectedNumberChannels", "int", false, false, false, false, true, false, NULL, &StationEpoch::setSelectedNumberChannels, &StationEpoch::selectedNumberChannels));
	addProperty(arrayClassProperty<Equipment>("equipment", "StationXML::Equipment", &StationEpoch::equipmentCount, &StationEpoch::equipment, static_cast<bool (StationEpoch::*)(Equipment*)>(&StationEpoch::addEquipment), &StationEpoch::removeEquipment, static_cast<bool (StationEpoch::*)(Equipment*)>(&StationEpoch::removeEquipment)));
	addProperty(arrayClassProperty<Operator>("operators", "StationXML::Operator", &StationEpoch::operatorsCount, &StationEpoch::operators, static_cast<bool (StationEpoch::*)(Operator*)>(&StationEpoch::addOperators), &StationEpoch::removeOperators, static_cast<bool (StationEpoch::*)(Operator*)>(&StationEpoch::removeOperators)));
	addProperty(arrayClassProperty<ExternalDocument>("externalReport", "StationXML::ExternalDocument", &StationEpoch::externalReportCount, &StationEpoch::externalReport, static_cast<bool (StationEpoch::*)(ExternalDocument*)>(&StationEpoch::addExternalReport), &StationEpoch::removeExternalReport, static_cast<bool (StationEpoch::*)(ExternalDocument*)>(&StationEpoch::removeExternalReport)));
	addProperty(arrayClassProperty<Channel>("channel", "StationXML::Channel", &StationEpoch::channelCount, &StationEpoch::channel, static_cast<bool (StationEpoch::*)(Channel*)>(&StationEpoch::addChannel), &StationEpoch::removeChannel, static_cast<bool (StationEpoch::*)(Channel*)>(&StationEpoch::removeChannel)));
}


IMPLEMENT_RTTI(StationEpoch, "StationXML::StationEpoch", Epoch)
IMPLEMENT_RTTI_METHODS(StationEpoch)
IMPLEMENT_METAOBJECT_DERIVED(StationEpoch, Epoch)


StationEpoch::StationEpoch() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StationEpoch::StationEpoch(const StationEpoch& other)
 : Epoch() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StationEpoch::~StationEpoch() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationEpoch::operator==(const StationEpoch& rhs) const {
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
	if ( !(_structure == rhs._structure) )
		return false;
	if ( !(_creationDate == rhs._creationDate) )
		return false;
	if ( !(_terminationDate == rhs._terminationDate) )
		return false;
	if ( !(_numberRecorders == rhs._numberRecorders) )
		return false;
	if ( !(_totalNumberChannels == rhs._totalNumberChannels) )
		return false;
	if ( !(_selectedNumberChannels == rhs._selectedNumberChannels) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationEpoch::setLatitude(const LatType& latitude) {
	_latitude = latitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LatType& StationEpoch::latitude() {
	return _latitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const LatType& StationEpoch::latitude() const {
	return _latitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationEpoch::setLongitude(const LonType& longitude) {
	_longitude = longitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LonType& StationEpoch::longitude() {
	return _longitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const LonType& StationEpoch::longitude() const {
	return _longitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationEpoch::setElevation(const DistanceType& elevation) {
	_elevation = elevation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DistanceType& StationEpoch::elevation() {
	return _elevation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DistanceType& StationEpoch::elevation() const {
	return _elevation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationEpoch::setSite(const Site& site) {
	_site = site;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Site& StationEpoch::site() {
	return _site;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Site& StationEpoch::site() const {
	return _site;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationEpoch::setVault(const std::string& vault) {
	_vault = vault;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& StationEpoch::vault() const {
	return _vault;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationEpoch::setGeology(const std::string& geology) {
	_geology = geology;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& StationEpoch::geology() const {
	return _geology;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationEpoch::setStructure(const std::string& structure) {
	_structure = structure;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& StationEpoch::structure() const {
	return _structure;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationEpoch::setCreationDate(DateTime creationDate) {
	_creationDate = creationDate;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DateTime StationEpoch::creationDate() const {
	return _creationDate;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationEpoch::setTerminationDate(const OPT(DateTime)& terminationDate) {
	_terminationDate = terminationDate;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DateTime StationEpoch::terminationDate() const throw(Seiscomp::Core::ValueException) {
	if ( _terminationDate )
		return *_terminationDate;
	throw Seiscomp::Core::ValueException("StationEpoch.terminationDate is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationEpoch::setNumberRecorders(const OPT(int)& numberRecorders) {
	_numberRecorders = numberRecorders;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StationEpoch::numberRecorders() const throw(Seiscomp::Core::ValueException) {
	if ( _numberRecorders )
		return *_numberRecorders;
	throw Seiscomp::Core::ValueException("StationEpoch.numberRecorders is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationEpoch::setTotalNumberChannels(const OPT(int)& totalNumberChannels) {
	_totalNumberChannels = totalNumberChannels;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StationEpoch::totalNumberChannels() const throw(Seiscomp::Core::ValueException) {
	if ( _totalNumberChannels )
		return *_totalNumberChannels;
	throw Seiscomp::Core::ValueException("StationEpoch.totalNumberChannels is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationEpoch::setSelectedNumberChannels(const OPT(int)& selectedNumberChannels) {
	_selectedNumberChannels = selectedNumberChannels;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StationEpoch::selectedNumberChannels() const throw(Seiscomp::Core::ValueException) {
	if ( _selectedNumberChannels )
		return *_selectedNumberChannels;
	throw Seiscomp::Core::ValueException("StationEpoch.selectedNumberChannels is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StationEpoch& StationEpoch::operator=(const StationEpoch& other) {
	Epoch::operator=(other);
	_latitude = other._latitude;
	_longitude = other._longitude;
	_elevation = other._elevation;
	_site = other._site;
	_vault = other._vault;
	_geology = other._geology;
	_structure = other._structure;
	_creationDate = other._creationDate;
	_terminationDate = other._terminationDate;
	_numberRecorders = other._numberRecorders;
	_totalNumberChannels = other._totalNumberChannels;
	_selectedNumberChannels = other._selectedNumberChannels;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t StationEpoch::equipmentCount() const {
	return _equipments.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Equipment* StationEpoch::equipment(size_t i) const {
	return _equipments[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationEpoch::addEquipment(Equipment* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_equipments.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationEpoch::removeEquipment(Equipment* obj) {
	if ( obj == NULL )
		return false;

	std::vector<EquipmentPtr>::iterator it;
	it = std::find(_equipments.begin(), _equipments.end(), obj);
	// Element has not been found
	if ( it == _equipments.end() ) {
		SEISCOMP_ERROR("StationEpoch::removeEquipment(Equipment*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationEpoch::removeEquipment(size_t i) {
	// index out of bounds
	if ( i >= _equipments.size() )
		return false;

	_equipments.erase(_equipments.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t StationEpoch::operatorsCount() const {
	return _operatorss.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Operator* StationEpoch::operators(size_t i) const {
	return _operatorss[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationEpoch::addOperators(Operator* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_operatorss.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationEpoch::removeOperators(Operator* obj) {
	if ( obj == NULL )
		return false;

	std::vector<OperatorPtr>::iterator it;
	it = std::find(_operatorss.begin(), _operatorss.end(), obj);
	// Element has not been found
	if ( it == _operatorss.end() ) {
		SEISCOMP_ERROR("StationEpoch::removeOperators(Operator*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationEpoch::removeOperators(size_t i) {
	// index out of bounds
	if ( i >= _operatorss.size() )
		return false;

	_operatorss.erase(_operatorss.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t StationEpoch::externalReportCount() const {
	return _externalReports.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ExternalDocument* StationEpoch::externalReport(size_t i) const {
	return _externalReports[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationEpoch::addExternalReport(ExternalDocument* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_externalReports.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationEpoch::removeExternalReport(ExternalDocument* obj) {
	if ( obj == NULL )
		return false;

	std::vector<ExternalDocumentPtr>::iterator it;
	it = std::find(_externalReports.begin(), _externalReports.end(), obj);
	// Element has not been found
	if ( it == _externalReports.end() ) {
		SEISCOMP_ERROR("StationEpoch::removeExternalReport(ExternalDocument*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationEpoch::removeExternalReport(size_t i) {
	// index out of bounds
	if ( i >= _externalReports.size() )
		return false;

	_externalReports.erase(_externalReports.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t StationEpoch::channelCount() const {
	return _channels.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Channel* StationEpoch::channel(size_t i) const {
	return _channels[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationEpoch::addChannel(Channel* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_channels.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationEpoch::removeChannel(Channel* obj) {
	if ( obj == NULL )
		return false;

	std::vector<ChannelPtr>::iterator it;
	it = std::find(_channels.begin(), _channels.end(), obj);
	// Element has not been found
	if ( it == _channels.end() ) {
		SEISCOMP_ERROR("StationEpoch::removeChannel(Channel*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationEpoch::removeChannel(size_t i) {
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
