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


#define SEISCOMP_COMPONENT DataModel
#include <seiscomp3/datamodel/arclinkrequest.h>
#include <seiscomp3/datamodel/arclinklog.h>
#include <algorithm>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(ArclinkRequest, PublicObject, "ArclinkRequest");


ArclinkRequest::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("requestID", "string", false, false, true, false, false, false, NULL, &ArclinkRequest::setRequestID, &ArclinkRequest::requestID));
	addProperty(Core::simpleProperty("userID", "string", false, false, true, false, false, false, NULL, &ArclinkRequest::setUserID, &ArclinkRequest::userID));
	addProperty(Core::simpleProperty("userIP", "string", false, false, false, false, false, false, NULL, &ArclinkRequest::setUserIP, &ArclinkRequest::userIP));
	addProperty(Core::simpleProperty("clientID", "string", false, false, false, false, false, false, NULL, &ArclinkRequest::setClientID, &ArclinkRequest::clientID));
	addProperty(Core::simpleProperty("clientIP", "string", false, false, false, false, false, false, NULL, &ArclinkRequest::setClientIP, &ArclinkRequest::clientIP));
	addProperty(Core::simpleProperty("type", "string", false, false, false, false, false, false, NULL, &ArclinkRequest::setType, &ArclinkRequest::type));
	addProperty(Core::simpleProperty("created", "datetime", false, false, true, false, false, false, NULL, &ArclinkRequest::setCreated, &ArclinkRequest::created));
	addProperty(Core::simpleProperty("status", "string", false, false, false, false, false, false, NULL, &ArclinkRequest::setStatus, &ArclinkRequest::status));
	addProperty(Core::simpleProperty("message", "string", false, false, false, false, false, false, NULL, &ArclinkRequest::setMessage, &ArclinkRequest::message));
	addProperty(Core::simpleProperty("label", "string", false, false, false, false, false, false, NULL, &ArclinkRequest::setLabel, &ArclinkRequest::label));
	addProperty(Core::simpleProperty("header", "string", false, false, false, false, false, false, NULL, &ArclinkRequest::setHeader, &ArclinkRequest::header));
	addProperty(objectProperty<ArclinkRequestSummary>("summary", "ArclinkRequestSummary", false, false, true, &ArclinkRequest::setSummary, &ArclinkRequest::summary));
	addProperty(arrayClassProperty<ArclinkStatusLine>("statusLine", "ArclinkStatusLine", &ArclinkRequest::arclinkStatusLineCount, &ArclinkRequest::arclinkStatusLine, static_cast<bool (ArclinkRequest::*)(ArclinkStatusLine*)>(&ArclinkRequest::add), &ArclinkRequest::removeArclinkStatusLine, static_cast<bool (ArclinkRequest::*)(ArclinkStatusLine*)>(&ArclinkRequest::remove)));
	addProperty(arrayClassProperty<ArclinkRequestLine>("requestLine", "ArclinkRequestLine", &ArclinkRequest::arclinkRequestLineCount, &ArclinkRequest::arclinkRequestLine, static_cast<bool (ArclinkRequest::*)(ArclinkRequestLine*)>(&ArclinkRequest::add), &ArclinkRequest::removeArclinkRequestLine, static_cast<bool (ArclinkRequest::*)(ArclinkRequestLine*)>(&ArclinkRequest::remove)));
}


IMPLEMENT_METAOBJECT(ArclinkRequest)


ArclinkRequestIndex::ArclinkRequestIndex() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequestIndex::ArclinkRequestIndex(Seiscomp::Core::Time created_,
                                         const std::string& requestID_,
                                         const std::string& userID_) {
	created = created_;
	requestID = requestID_;
	userID = userID_;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequestIndex::ArclinkRequestIndex(const ArclinkRequestIndex& idx) {
	created = idx.created;
	requestID = idx.requestID;
	userID = idx.userID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequestIndex::operator==(const ArclinkRequestIndex& idx) const {
	return created == idx.created &&
	       requestID == idx.requestID &&
	       userID == idx.userID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequestIndex::operator!=(const ArclinkRequestIndex& idx) const {
	return !operator==(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequest::ArclinkRequest() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequest::ArclinkRequest(const ArclinkRequest& other)
: PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequest::ArclinkRequest(const std::string& publicID)
: PublicObject(publicID) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequest::~ArclinkRequest() {
	std::for_each(_arclinkStatusLines.begin(), _arclinkStatusLines.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&ArclinkStatusLine::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&ArclinkStatusLinePtr::get)));
	std::for_each(_arclinkRequestLines.begin(), _arclinkRequestLines.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&ArclinkRequestLine::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&ArclinkRequestLinePtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequest* ArclinkRequest::Create() {
	ArclinkRequest* object = new ArclinkRequest();
	return static_cast<ArclinkRequest*>(GenerateId(object));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequest* ArclinkRequest::Create(const std::string& publicID) {
	if ( PublicObject::IsRegistrationEnabled() && Find(publicID) != NULL ) {
		SEISCOMP_ERROR(
			"There exists already a PublicObject with Id '%s'",
			publicID.c_str()
		);
		return NULL;
	}

	return new ArclinkRequest(publicID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequest* ArclinkRequest::Find(const std::string& publicID) {
	return ArclinkRequest::Cast(PublicObject::Find(publicID));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequest::operator==(const ArclinkRequest& rhs) const {
	if ( _index != rhs._index ) return false;
	if ( _userIP != rhs._userIP ) return false;
	if ( _clientID != rhs._clientID ) return false;
	if ( _clientIP != rhs._clientIP ) return false;
	if ( _type != rhs._type ) return false;
	if ( _status != rhs._status ) return false;
	if ( _message != rhs._message ) return false;
	if ( _label != rhs._label ) return false;
	if ( _header != rhs._header ) return false;
	if ( _summary != rhs._summary ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequest::operator!=(const ArclinkRequest& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequest::equal(const ArclinkRequest& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkRequest::setRequestID(const std::string& requestID) {
	_index.requestID = requestID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ArclinkRequest::requestID() const {
	return _index.requestID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkRequest::setUserID(const std::string& userID) {
	_index.userID = userID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ArclinkRequest::userID() const {
	return _index.userID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkRequest::setUserIP(const std::string& userIP) {
	_userIP = userIP;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ArclinkRequest::userIP() const {
	return _userIP;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkRequest::setClientID(const std::string& clientID) {
	_clientID = clientID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ArclinkRequest::clientID() const {
	return _clientID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkRequest::setClientIP(const std::string& clientIP) {
	_clientIP = clientIP;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ArclinkRequest::clientIP() const {
	return _clientIP;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkRequest::setType(const std::string& type) {
	_type = type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ArclinkRequest::type() const {
	return _type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkRequest::setCreated(Seiscomp::Core::Time created) {
	_index.created = created;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time ArclinkRequest::created() const {
	return _index.created;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkRequest::setStatus(const std::string& status) {
	_status = status;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ArclinkRequest::status() const {
	return _status;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkRequest::setMessage(const std::string& message) {
	_message = message;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ArclinkRequest::message() const {
	return _message;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkRequest::setLabel(const std::string& label) {
	_label = label;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ArclinkRequest::label() const {
	return _label;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkRequest::setHeader(const std::string& header) {
	_header = header;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ArclinkRequest::header() const {
	return _header;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkRequest::setSummary(const OPT(ArclinkRequestSummary)& summary) {
	_summary = summary;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequestSummary& ArclinkRequest::summary() {
	if ( _summary )
		return *_summary;
	throw Seiscomp::Core::ValueException("ArclinkRequest.summary is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ArclinkRequestSummary& ArclinkRequest::summary() const {
	if ( _summary )
		return *_summary;
	throw Seiscomp::Core::ValueException("ArclinkRequest.summary is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ArclinkRequestIndex& ArclinkRequest::index() const {
	return _index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequest::equalIndex(const ArclinkRequest* lhs) const {
	if ( lhs == NULL ) return false;
	return lhs->index() == index();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkLog* ArclinkRequest::arclinkLog() const {
	return static_cast<ArclinkLog*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequest& ArclinkRequest::operator=(const ArclinkRequest& other) {
	PublicObject::operator=(other);
	_index = other._index;
	_userIP = other._userIP;
	_clientID = other._clientID;
	_clientIP = other._clientIP;
	_type = other._type;
	_status = other._status;
	_message = other._message;
	_label = other._label;
	_header = other._header;
	_summary = other._summary;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequest::assign(Object* other) {
	ArclinkRequest* otherArclinkRequest = ArclinkRequest::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherArclinkRequest;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequest::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	ArclinkLog* arclinkLog = ArclinkLog::Cast(parent);
	if ( arclinkLog != NULL )
		return arclinkLog->add(this);

	SEISCOMP_ERROR("ArclinkRequest::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequest::detachFrom(PublicObject* object) {
	if ( object == NULL ) return false;

	// check all possible parents
	ArclinkLog* arclinkLog = ArclinkLog::Cast(object);
	if ( arclinkLog != NULL ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return arclinkLog->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			ArclinkRequest* child = arclinkLog->findArclinkRequest(publicID());
			if ( child != NULL )
				return arclinkLog->remove(child);
			else {
				SEISCOMP_DEBUG("ArclinkRequest::detachFrom(ArclinkLog): arclinkRequest has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("ArclinkRequest::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequest::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* ArclinkRequest::clone() const {
	ArclinkRequest* clonee = new ArclinkRequest();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequest::updateChild(Object* child) {
	ArclinkStatusLine* arclinkStatusLineChild = ArclinkStatusLine::Cast(child);
	if ( arclinkStatusLineChild != NULL ) {
		ArclinkStatusLine* arclinkStatusLineElement = arclinkStatusLine(arclinkStatusLineChild->index());
		if ( arclinkStatusLineElement != NULL ) {
			*arclinkStatusLineElement = *arclinkStatusLineChild;
			return true;
		}
		return false;
	}

	ArclinkRequestLine* arclinkRequestLineChild = ArclinkRequestLine::Cast(child);
	if ( arclinkRequestLineChild != NULL ) {
		ArclinkRequestLine* arclinkRequestLineElement = arclinkRequestLine(arclinkRequestLineChild->index());
		if ( arclinkRequestLineElement != NULL ) {
			*arclinkRequestLineElement = *arclinkRequestLineChild;
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkRequest::accept(Visitor* visitor) {
	if ( visitor->traversal() == Visitor::TM_TOPDOWN )
		if ( !visitor->visit(this) )
			return;

	for ( std::vector<ArclinkStatusLinePtr>::iterator it = _arclinkStatusLines.begin(); it != _arclinkStatusLines.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<ArclinkRequestLinePtr>::iterator it = _arclinkRequestLines.begin(); it != _arclinkRequestLines.end(); ++it )
		(*it)->accept(visitor);

	if ( visitor->traversal() == Visitor::TM_BOTTOMUP )
		visitor->visit(this);
	else
		visitor->finished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t ArclinkRequest::arclinkStatusLineCount() const {
	return _arclinkStatusLines.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkStatusLine* ArclinkRequest::arclinkStatusLine(size_t i) const {
	return _arclinkStatusLines[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkStatusLine* ArclinkRequest::arclinkStatusLine(const ArclinkStatusLineIndex& i) const {
	for ( std::vector<ArclinkStatusLinePtr>::const_iterator it = _arclinkStatusLines.begin(); it != _arclinkStatusLines.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequest::add(ArclinkStatusLine* arclinkStatusLine) {
	if ( arclinkStatusLine == NULL )
		return false;

	// Element has already a parent
	if ( arclinkStatusLine->parent() != NULL ) {
		SEISCOMP_ERROR("ArclinkRequest::add(ArclinkStatusLine*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<ArclinkStatusLinePtr>::iterator it = _arclinkStatusLines.begin(); it != _arclinkStatusLines.end(); ++it ) {
		if ( (*it)->index() == arclinkStatusLine->index() ) {
			SEISCOMP_ERROR("ArclinkRequest::add(ArclinkStatusLine*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_arclinkStatusLines.push_back(arclinkStatusLine);
	arclinkStatusLine->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		arclinkStatusLine->accept(&nc);
	}

	// Notify registered observers
	childAdded(arclinkStatusLine);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequest::remove(ArclinkStatusLine* arclinkStatusLine) {
	if ( arclinkStatusLine == NULL )
		return false;

	if ( arclinkStatusLine->parent() != this ) {
		SEISCOMP_ERROR("ArclinkRequest::remove(ArclinkStatusLine*) -> element has another parent");
		return false;
	}

	std::vector<ArclinkStatusLinePtr>::iterator it;
	it = std::find(_arclinkStatusLines.begin(), _arclinkStatusLines.end(), arclinkStatusLine);
	// Element has not been found
	if ( it == _arclinkStatusLines.end() ) {
		SEISCOMP_ERROR("ArclinkRequest::remove(ArclinkStatusLine*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_arclinkStatusLines.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequest::removeArclinkStatusLine(size_t i) {
	// index out of bounds
	if ( i >= _arclinkStatusLines.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_arclinkStatusLines[i]->accept(&nc);
	}

	_arclinkStatusLines[i]->setParent(NULL);
	childRemoved(_arclinkStatusLines[i].get());
	
	_arclinkStatusLines.erase(_arclinkStatusLines.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequest::removeArclinkStatusLine(const ArclinkStatusLineIndex& i) {
	ArclinkStatusLine* object = arclinkStatusLine(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t ArclinkRequest::arclinkRequestLineCount() const {
	return _arclinkRequestLines.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequestLine* ArclinkRequest::arclinkRequestLine(size_t i) const {
	return _arclinkRequestLines[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequestLine* ArclinkRequest::arclinkRequestLine(const ArclinkRequestLineIndex& i) const {
	for ( std::vector<ArclinkRequestLinePtr>::const_iterator it = _arclinkRequestLines.begin(); it != _arclinkRequestLines.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequest::add(ArclinkRequestLine* arclinkRequestLine) {
	if ( arclinkRequestLine == NULL )
		return false;

	// Element has already a parent
	if ( arclinkRequestLine->parent() != NULL ) {
		SEISCOMP_ERROR("ArclinkRequest::add(ArclinkRequestLine*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<ArclinkRequestLinePtr>::iterator it = _arclinkRequestLines.begin(); it != _arclinkRequestLines.end(); ++it ) {
		if ( (*it)->index() == arclinkRequestLine->index() ) {
			SEISCOMP_ERROR("ArclinkRequest::add(ArclinkRequestLine*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_arclinkRequestLines.push_back(arclinkRequestLine);
	arclinkRequestLine->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		arclinkRequestLine->accept(&nc);
	}

	// Notify registered observers
	childAdded(arclinkRequestLine);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequest::remove(ArclinkRequestLine* arclinkRequestLine) {
	if ( arclinkRequestLine == NULL )
		return false;

	if ( arclinkRequestLine->parent() != this ) {
		SEISCOMP_ERROR("ArclinkRequest::remove(ArclinkRequestLine*) -> element has another parent");
		return false;
	}

	std::vector<ArclinkRequestLinePtr>::iterator it;
	it = std::find(_arclinkRequestLines.begin(), _arclinkRequestLines.end(), arclinkRequestLine);
	// Element has not been found
	if ( it == _arclinkRequestLines.end() ) {
		SEISCOMP_ERROR("ArclinkRequest::remove(ArclinkRequestLine*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_arclinkRequestLines.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequest::removeArclinkRequestLine(size_t i) {
	// index out of bounds
	if ( i >= _arclinkRequestLines.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_arclinkRequestLines[i]->accept(&nc);
	}

	_arclinkRequestLines[i]->setParent(NULL);
	childRemoved(_arclinkRequestLines[i].get());
	
	_arclinkRequestLines.erase(_arclinkRequestLines.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkRequest::removeArclinkRequestLine(const ArclinkRequestLineIndex& i) {
	ArclinkRequestLine* object = arclinkRequestLine(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkRequest::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,11>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: ArclinkRequest skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	PublicObject::serialize(ar);
	if ( !ar.success() ) return;

	ar & NAMED_OBJECT_HINT("requestID", _index.requestID, Archive::XML_ELEMENT | Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	ar & NAMED_OBJECT_HINT("userID", _index.userID, Archive::XML_ELEMENT | Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	ar & NAMED_OBJECT_HINT("userIP", _userIP, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("clientID", _clientID, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("clientIP", _clientIP, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("type", _type, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("created", _index.created, Archive::XML_ELEMENT | Archive::SPLIT_TIME | Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	ar & NAMED_OBJECT_HINT("status", _status, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("message", _message, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("label", _label, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("header", _header, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("summary", _summary, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("statusLine",
	                       Seiscomp::Core::Generic::containerMember(_arclinkStatusLines,
	                       Seiscomp::Core::Generic::bindMemberFunction<ArclinkStatusLine>(static_cast<bool (ArclinkRequest::*)(ArclinkStatusLine*)>(&ArclinkRequest::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("requestLine",
	                       Seiscomp::Core::Generic::containerMember(_arclinkRequestLines,
	                       Seiscomp::Core::Generic::bindMemberFunction<ArclinkRequestLine>(static_cast<bool (ArclinkRequest::*)(ArclinkRequestLine*)>(&ArclinkRequest::add), this)),
	                       Archive::STATIC_TYPE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
