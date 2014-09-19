/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <stationxml/channel.h>
#include <stationxml/externaldocument.h>
#include <stationxml/channelepoch.h>
#include <algorithm>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace StationXML {


Channel::MetaObject::MetaObject(const Core::RTTI* rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(Core::simpleProperty("code", "string", false, false, false, false, false, false, NULL, &Channel::setCode, &Channel::code));
	addProperty(Core::simpleProperty("seedCode", "string", false, false, false, false, false, false, NULL, &Channel::setSeedCode, &Channel::seedCode));
	addProperty(Core::simpleProperty("historicalCode", "string", false, false, false, false, false, false, NULL, &Channel::setHistoricalCode, &Channel::historicalCode));
	addProperty(Core::simpleProperty("locationCode", "string", false, false, false, false, false, false, NULL, &Channel::setLocationCode, &Channel::locationCode));
	addProperty(Core::simpleProperty("restricted", "int", false, false, false, false, true, false, NULL, &Channel::setRestricted, &Channel::restricted));
	addProperty(Core::simpleProperty("creationDate", "datetime", false, false, false, false, false, false, NULL, &Channel::setCreationDate, &Channel::creationDate));
	addProperty(Core::simpleProperty("terminationDate", "datetime", false, false, false, false, true, false, NULL, &Channel::setTerminationDate, &Channel::terminationDate));
	addProperty(Core::simpleProperty("dataless", "string", false, false, false, false, false, false, NULL, &Channel::setDataless, &Channel::dataless));
	addProperty(arrayClassProperty<ExternalDocument>("externalReport", "StationXML::ExternalDocument", &Channel::externalReportCount, &Channel::externalReport, static_cast<bool (Channel::*)(ExternalDocument*)>(&Channel::addExternalReport), &Channel::removeExternalReport, static_cast<bool (Channel::*)(ExternalDocument*)>(&Channel::removeExternalReport)));
	addProperty(arrayClassProperty<ChannelEpoch>("epoch", "StationXML::ChannelEpoch", &Channel::epochCount, &Channel::epoch, static_cast<bool (Channel::*)(ChannelEpoch*)>(&Channel::addEpoch), &Channel::removeEpoch, static_cast<bool (Channel::*)(ChannelEpoch*)>(&Channel::removeEpoch)));
}


IMPLEMENT_RTTI(Channel, "StationXML::Channel", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(Channel)
IMPLEMENT_METAOBJECT(Channel)


Channel::Channel() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Channel::Channel(const Channel& other)
 : Core::BaseObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Channel::~Channel() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Channel::operator==(const Channel& rhs) const {
	if ( !(_code == rhs._code) )
		return false;
	if ( !(_seedCode == rhs._seedCode) )
		return false;
	if ( !(_historicalCode == rhs._historicalCode) )
		return false;
	if ( !(_locationCode == rhs._locationCode) )
		return false;
	if ( !(_restricted == rhs._restricted) )
		return false;
	if ( !(_creationDate == rhs._creationDate) )
		return false;
	if ( !(_terminationDate == rhs._terminationDate) )
		return false;
	if ( !(_dataless == rhs._dataless) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setCode(const std::string& code) {
	_code = code;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Channel::code() const {
	return _code;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setSeedCode(const std::string& seedCode) {
	_seedCode = seedCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Channel::seedCode() const {
	return _seedCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setHistoricalCode(const std::string& historicalCode) {
	_historicalCode = historicalCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Channel::historicalCode() const {
	return _historicalCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setLocationCode(const std::string& locationCode) {
	_locationCode = locationCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Channel::locationCode() const {
	return _locationCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setRestricted(const OPT(int)& restricted) {
	_restricted = restricted;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Channel::restricted() const throw(Seiscomp::Core::ValueException) {
	if ( _restricted )
		return *_restricted;
	throw Seiscomp::Core::ValueException("Channel.restricted is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setCreationDate(DateTime creationDate) {
	_creationDate = creationDate;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DateTime Channel::creationDate() const {
	return _creationDate;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setTerminationDate(const OPT(DateTime)& terminationDate) {
	_terminationDate = terminationDate;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DateTime Channel::terminationDate() const throw(Seiscomp::Core::ValueException) {
	if ( _terminationDate )
		return *_terminationDate;
	throw Seiscomp::Core::ValueException("Channel.terminationDate is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setDataless(const std::string& dataless) {
	_dataless = dataless;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Channel::dataless() const {
	return _dataless;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Channel& Channel::operator=(const Channel& other) {
	_code = other._code;
	_seedCode = other._seedCode;
	_historicalCode = other._historicalCode;
	_locationCode = other._locationCode;
	_restricted = other._restricted;
	_creationDate = other._creationDate;
	_terminationDate = other._terminationDate;
	_dataless = other._dataless;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Channel::externalReportCount() const {
	return _externalReports.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ExternalDocument* Channel::externalReport(size_t i) const {
	return _externalReports[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Channel::addExternalReport(ExternalDocument* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_externalReports.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Channel::removeExternalReport(ExternalDocument* obj) {
	if ( obj == NULL )
		return false;

	std::vector<ExternalDocumentPtr>::iterator it;
	it = std::find(_externalReports.begin(), _externalReports.end(), obj);
	// Element has not been found
	if ( it == _externalReports.end() ) {
		SEISCOMP_ERROR("Channel::removeExternalReport(ExternalDocument*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Channel::removeExternalReport(size_t i) {
	// index out of bounds
	if ( i >= _externalReports.size() )
		return false;

	_externalReports.erase(_externalReports.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Channel::epochCount() const {
	return _epochs.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ChannelEpoch* Channel::epoch(size_t i) const {
	return _epochs[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Channel::addEpoch(ChannelEpoch* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_epochs.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Channel::removeEpoch(ChannelEpoch* obj) {
	if ( obj == NULL )
		return false;

	std::vector<ChannelEpochPtr>::iterator it;
	it = std::find(_epochs.begin(), _epochs.end(), obj);
	// Element has not been found
	if ( it == _epochs.end() ) {
		SEISCOMP_ERROR("Channel::removeEpoch(ChannelEpoch*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Channel::removeEpoch(size_t i) {
	// index out of bounds
	if ( i >= _epochs.size() )
		return false;

	_epochs.erase(_epochs.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
