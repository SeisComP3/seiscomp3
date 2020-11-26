/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <fdsnxml/basenode.h>
#include <fdsnxml/identifier.h>
#include <fdsnxml/comment.h>
#include <fdsnxml/dataavailability.h>
#include <algorithm>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace FDSNXML {


namespace {

static Seiscomp::Core::MetaEnumImpl<RestrictedStatusType> metaRestrictedStatusType;

}


BaseNode::MetaObject::MetaObject(const Core::RTTI *rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(Core::simpleProperty("description", "string", false, false, false, false, false, false, NULL, &BaseNode::setDescription, &BaseNode::description));
	addProperty(Core::simpleProperty("code", "string", false, false, false, false, false, false, NULL, &BaseNode::setCode, &BaseNode::code));
	addProperty(Core::simpleProperty("startDate", "datetime", false, false, false, false, true, false, NULL, &BaseNode::setStartDate, &BaseNode::startDate));
	addProperty(Core::simpleProperty("endDate", "datetime", false, false, false, false, true, false, NULL, &BaseNode::setEndDate, &BaseNode::endDate));
	addProperty(Core::simpleProperty("sourceID", "string", false, false, false, false, false, false, NULL, &BaseNode::setSourceID, &BaseNode::sourceID));
	addProperty(enumProperty("restrictedStatus", "RestrictedStatusType", false, true, &metaRestrictedStatusType, &BaseNode::setRestrictedStatus, &BaseNode::restrictedStatus));
	addProperty(Core::simpleProperty("alternateCode", "string", false, false, false, false, false, false, NULL, &BaseNode::setAlternateCode, &BaseNode::alternateCode));
	addProperty(Core::simpleProperty("historicCode", "string", false, false, false, false, false, false, NULL, &BaseNode::setHistoricCode, &BaseNode::historicCode));
	addProperty(arrayClassProperty<Identifier>("identifier", "FDSNXML::Identifier", &BaseNode::identifierCount, &BaseNode::identifier, static_cast<bool (BaseNode::*)(Identifier*)>(&BaseNode::addIdentifier), &BaseNode::removeIdentifier, static_cast<bool (BaseNode::*)(Identifier*)>(&BaseNode::removeIdentifier)));
	addProperty(arrayClassProperty<Comment>("comment", "FDSNXML::Comment", &BaseNode::commentCount, &BaseNode::comment, static_cast<bool (BaseNode::*)(Comment*)>(&BaseNode::addComment), &BaseNode::removeComment, static_cast<bool (BaseNode::*)(Comment*)>(&BaseNode::removeComment)));
	addProperty(arrayClassProperty<DataAvailability>("dataAvailability", "FDSNXML::DataAvailability", &BaseNode::dataAvailabilityCount, &BaseNode::dataAvailability, static_cast<bool (BaseNode::*)(DataAvailability*)>(&BaseNode::addDataAvailability), &BaseNode::removeDataAvailability, static_cast<bool (BaseNode::*)(DataAvailability*)>(&BaseNode::removeDataAvailability)));
}


IMPLEMENT_RTTI(BaseNode, "FDSNXML::BaseNode", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(BaseNode)
IMPLEMENT_METAOBJECT(BaseNode)


BaseNode::BaseNode() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BaseNode::BaseNode(const BaseNode &other)
 : Core::BaseObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BaseNode::~BaseNode() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BaseNode::operator==(const BaseNode &rhs) const {
	if ( !(_description == rhs._description) )
		return false;
	if ( !(_code == rhs._code) )
		return false;
	if ( !(_startDate == rhs._startDate) )
		return false;
	if ( !(_endDate == rhs._endDate) )
		return false;
	if ( !(_sourceID == rhs._sourceID) )
		return false;
	if ( !(_restrictedStatus == rhs._restrictedStatus) )
		return false;
	if ( !(_alternateCode == rhs._alternateCode) )
		return false;
	if ( !(_historicCode == rhs._historicCode) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BaseNode::setDescription(const std::string& description) {
	_description = description;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& BaseNode::description() const {
	return _description;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BaseNode::setCode(const std::string& code) {
	_code = code;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& BaseNode::code() const {
	return _code;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BaseNode::setStartDate(const OPT(DateTime)& startDate) {
	_startDate = startDate;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DateTime BaseNode::startDate() const {
	if ( _startDate )
		return *_startDate;
	throw Seiscomp::Core::ValueException("BaseNode.startDate is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BaseNode::setEndDate(const OPT(DateTime)& endDate) {
	_endDate = endDate;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DateTime BaseNode::endDate() const {
	if ( _endDate )
		return *_endDate;
	throw Seiscomp::Core::ValueException("BaseNode.endDate is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BaseNode::setSourceID(const std::string& sourceID) {
	_sourceID = sourceID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& BaseNode::sourceID() const {
	return _sourceID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BaseNode::setRestrictedStatus(const OPT(RestrictedStatusType)& restrictedStatus) {
	_restrictedStatus = restrictedStatus;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RestrictedStatusType BaseNode::restrictedStatus() const {
	if ( _restrictedStatus )
		return *_restrictedStatus;
	throw Seiscomp::Core::ValueException("BaseNode.restrictedStatus is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BaseNode::setAlternateCode(const std::string& alternateCode) {
	_alternateCode = alternateCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& BaseNode::alternateCode() const {
	return _alternateCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BaseNode::setHistoricCode(const std::string& historicCode) {
	_historicCode = historicCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& BaseNode::historicCode() const {
	return _historicCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BaseNode& BaseNode::operator=(const BaseNode &other) {
	_description = other._description;
	_code = other._code;
	_startDate = other._startDate;
	_endDate = other._endDate;
	_sourceID = other._sourceID;
	_restrictedStatus = other._restrictedStatus;
	_alternateCode = other._alternateCode;
	_historicCode = other._historicCode;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t BaseNode::identifierCount() const {
	return _identifiers.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Identifier* BaseNode::identifier(size_t i) const {
	return _identifiers[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BaseNode::addIdentifier(Identifier *obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_identifiers.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BaseNode::removeIdentifier(Identifier *obj) {
	if ( obj == NULL )
		return false;

	std::vector<IdentifierPtr>::iterator it;
	it = std::find(_identifiers.begin(), _identifiers.end(), obj);
	// Element has not been found
	if ( it == _identifiers.end() ) {
		SEISCOMP_ERROR("BaseNode::removeIdentifier(Identifier*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BaseNode::removeIdentifier(size_t i) {
	// index out of bounds
	if ( i >= _identifiers.size() )
		return false;

	_identifiers.erase(_identifiers.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t BaseNode::commentCount() const {
	return _comments.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Comment* BaseNode::comment(size_t i) const {
	return _comments[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BaseNode::addComment(Comment *obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_comments.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BaseNode::removeComment(Comment *obj) {
	if ( obj == NULL )
		return false;

	std::vector<CommentPtr>::iterator it;
	it = std::find(_comments.begin(), _comments.end(), obj);
	// Element has not been found
	if ( it == _comments.end() ) {
		SEISCOMP_ERROR("BaseNode::removeComment(Comment*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BaseNode::removeComment(size_t i) {
	// index out of bounds
	if ( i >= _comments.size() )
		return false;

	_comments.erase(_comments.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t BaseNode::dataAvailabilityCount() const {
	return _dataAvailabilitys.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataAvailability* BaseNode::dataAvailability(size_t i) const {
	return _dataAvailabilitys[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BaseNode::addDataAvailability(DataAvailability *obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_dataAvailabilitys.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BaseNode::removeDataAvailability(DataAvailability *obj) {
	if ( obj == NULL )
		return false;

	std::vector<DataAvailabilityPtr>::iterator it;
	it = std::find(_dataAvailabilitys.begin(), _dataAvailabilitys.end(), obj);
	// Element has not been found
	if ( it == _dataAvailabilitys.end() ) {
		SEISCOMP_ERROR("BaseNode::removeDataAvailability(DataAvailability*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BaseNode::removeDataAvailability(size_t i) {
	// index out of bounds
	if ( i >= _dataAvailabilitys.size() )
		return false;

	_dataAvailabilitys.erase(_dataAvailabilitys.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
