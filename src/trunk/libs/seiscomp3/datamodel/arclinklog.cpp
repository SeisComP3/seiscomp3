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

// This file was created by a source code generator.
// Do not modify the contents. Change the definition and run the generator
// again!


#define SEISCOMP_COMPONENT DataModel
#include <seiscomp3/datamodel/arclinklog.h>
#include <algorithm>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(ArclinkLog, PublicObject, "ArclinkLog");


ArclinkLog::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(arrayObjectProperty("arclinkRequest", "ArclinkRequest", &ArclinkLog::arclinkRequestCount, &ArclinkLog::arclinkRequest, static_cast<bool (ArclinkLog::*)(ArclinkRequest*)>(&ArclinkLog::add), &ArclinkLog::removeArclinkRequest, static_cast<bool (ArclinkLog::*)(ArclinkRequest*)>(&ArclinkLog::remove)));
	addProperty(arrayObjectProperty("arclinkUser", "ArclinkUser", &ArclinkLog::arclinkUserCount, &ArclinkLog::arclinkUser, static_cast<bool (ArclinkLog::*)(ArclinkUser*)>(&ArclinkLog::add), &ArclinkLog::removeArclinkUser, static_cast<bool (ArclinkLog::*)(ArclinkUser*)>(&ArclinkLog::remove)));
}


IMPLEMENT_METAOBJECT(ArclinkLog)


ArclinkLog::ArclinkLog() : PublicObject("ArclinkLog") {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkLog::ArclinkLog(const ArclinkLog& other)
 : PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkLog::~ArclinkLog() {
	std::for_each(_arclinkRequests.begin(), _arclinkRequests.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&ArclinkRequest::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&ArclinkRequestPtr::get)));
	std::for_each(_arclinkUsers.begin(), _arclinkUsers.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&ArclinkUser::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&ArclinkUserPtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::operator==(const ArclinkLog& rhs) const {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::operator!=(const ArclinkLog& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::equal(const ArclinkLog& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkLog& ArclinkLog::operator=(const ArclinkLog& other) {
	PublicObject::operator=(other);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::assign(Object* other) {
	ArclinkLog* otherArclinkLog = ArclinkLog::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherArclinkLog;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::attachTo(PublicObject* parent) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::detachFrom(PublicObject* object) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::detach() {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* ArclinkLog::clone() const {
	ArclinkLog* clonee = new ArclinkLog();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::updateChild(Object* child) {
	ArclinkRequest* arclinkRequestChild = ArclinkRequest::Cast(child);
	if ( arclinkRequestChild != NULL ) {
		ArclinkRequest* arclinkRequestElement
			= ArclinkRequest::Cast(PublicObject::Find(arclinkRequestChild->publicID()));
		if ( arclinkRequestElement && arclinkRequestElement->parent() == this ) {
			*arclinkRequestElement = *arclinkRequestChild;
			return true;
		}
		return false;
	}

	ArclinkUser* arclinkUserChild = ArclinkUser::Cast(child);
	if ( arclinkUserChild != NULL ) {
		ArclinkUser* arclinkUserElement
			= ArclinkUser::Cast(PublicObject::Find(arclinkUserChild->publicID()));
		if ( arclinkUserElement && arclinkUserElement->parent() == this ) {
			*arclinkUserElement = *arclinkUserChild;
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkLog::accept(Visitor* visitor) {
	for ( std::vector<ArclinkRequestPtr>::iterator it = _arclinkRequests.begin(); it != _arclinkRequests.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<ArclinkUserPtr>::iterator it = _arclinkUsers.begin(); it != _arclinkUsers.end(); ++it )
		(*it)->accept(visitor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t ArclinkLog::arclinkRequestCount() const {
	return _arclinkRequests.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequest* ArclinkLog::arclinkRequest(size_t i) const {
	return _arclinkRequests[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequest* ArclinkLog::arclinkRequest(const ArclinkRequestIndex& i) const {
	for ( std::vector<ArclinkRequestPtr>::const_iterator it = _arclinkRequests.begin(); it != _arclinkRequests.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequest* ArclinkLog::findArclinkRequest(const std::string& publicID) const {
	for ( std::vector<ArclinkRequestPtr>::const_iterator it = _arclinkRequests.begin(); it != _arclinkRequests.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::add(ArclinkRequest* arclinkRequest) {
	if ( arclinkRequest == NULL )
		return false;

	// Element has already a parent
	if ( arclinkRequest->parent() != NULL ) {
		SEISCOMP_ERROR("ArclinkLog::add(ArclinkRequest*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		ArclinkRequest* arclinkRequestCached = ArclinkRequest::Find(arclinkRequest->publicID());
		if ( arclinkRequestCached ) {
			if ( arclinkRequestCached->parent() ) {
				if ( arclinkRequestCached->parent() == this )
					SEISCOMP_ERROR("ArclinkLog::add(ArclinkRequest*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("ArclinkLog::add(ArclinkRequest*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				arclinkRequest = arclinkRequestCached;
		}
	}

	// Add the element
	_arclinkRequests.push_back(arclinkRequest);
	arclinkRequest->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		arclinkRequest->accept(&nc);
	}

	// Notify registered observers
	childAdded(arclinkRequest);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::remove(ArclinkRequest* arclinkRequest) {
	if ( arclinkRequest == NULL )
		return false;

	if ( arclinkRequest->parent() != this ) {
		SEISCOMP_ERROR("ArclinkLog::remove(ArclinkRequest*) -> element has another parent");
		return false;
	}

	std::vector<ArclinkRequestPtr>::iterator it;
	it = std::find(_arclinkRequests.begin(), _arclinkRequests.end(), arclinkRequest);
	// Element has not been found
	if ( it == _arclinkRequests.end() ) {
		SEISCOMP_ERROR("ArclinkLog::remove(ArclinkRequest*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_arclinkRequests.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::removeArclinkRequest(size_t i) {
	// index out of bounds
	if ( i >= _arclinkRequests.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_arclinkRequests[i]->accept(&nc);
	}

	_arclinkRequests[i]->setParent(NULL);
	childRemoved(_arclinkRequests[i].get());
	
	_arclinkRequests.erase(_arclinkRequests.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::removeArclinkRequest(const ArclinkRequestIndex& i) {
	ArclinkRequest* object = arclinkRequest(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t ArclinkLog::arclinkUserCount() const {
	return _arclinkUsers.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkUser* ArclinkLog::arclinkUser(size_t i) const {
	return _arclinkUsers[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkUser* ArclinkLog::arclinkUser(const ArclinkUserIndex& i) const {
	for ( std::vector<ArclinkUserPtr>::const_iterator it = _arclinkUsers.begin(); it != _arclinkUsers.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkUser* ArclinkLog::findArclinkUser(const std::string& publicID) const {
	for ( std::vector<ArclinkUserPtr>::const_iterator it = _arclinkUsers.begin(); it != _arclinkUsers.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::add(ArclinkUser* arclinkUser) {
	if ( arclinkUser == NULL )
		return false;

	// Element has already a parent
	if ( arclinkUser->parent() != NULL ) {
		SEISCOMP_ERROR("ArclinkLog::add(ArclinkUser*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		ArclinkUser* arclinkUserCached = ArclinkUser::Find(arclinkUser->publicID());
		if ( arclinkUserCached ) {
			if ( arclinkUserCached->parent() ) {
				if ( arclinkUserCached->parent() == this )
					SEISCOMP_ERROR("ArclinkLog::add(ArclinkUser*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("ArclinkLog::add(ArclinkUser*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				arclinkUser = arclinkUserCached;
		}
	}

	// Add the element
	_arclinkUsers.push_back(arclinkUser);
	arclinkUser->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		arclinkUser->accept(&nc);
	}

	// Notify registered observers
	childAdded(arclinkUser);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::remove(ArclinkUser* arclinkUser) {
	if ( arclinkUser == NULL )
		return false;

	if ( arclinkUser->parent() != this ) {
		SEISCOMP_ERROR("ArclinkLog::remove(ArclinkUser*) -> element has another parent");
		return false;
	}

	std::vector<ArclinkUserPtr>::iterator it;
	it = std::find(_arclinkUsers.begin(), _arclinkUsers.end(), arclinkUser);
	// Element has not been found
	if ( it == _arclinkUsers.end() ) {
		SEISCOMP_ERROR("ArclinkLog::remove(ArclinkUser*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_arclinkUsers.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::removeArclinkUser(size_t i) {
	// index out of bounds
	if ( i >= _arclinkUsers.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_arclinkUsers[i]->accept(&nc);
	}

	_arclinkUsers[i]->setParent(NULL);
	childRemoved(_arclinkUsers[i].get());
	
	_arclinkUsers.erase(_arclinkUsers.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::removeArclinkUser(const ArclinkUserIndex& i) {
	ArclinkUser* object = arclinkUser(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkLog::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,8>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: ArclinkLog skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("arclinkRequest",
	                       Seiscomp::Core::Generic::containerMember(_arclinkRequests,
	                       Seiscomp::Core::Generic::bindMemberFunction<ArclinkRequest>(static_cast<bool (ArclinkLog::*)(ArclinkRequest*)>(&ArclinkLog::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("arclinkUser",
	                       Seiscomp::Core::Generic::containerMember(_arclinkUsers,
	                       Seiscomp::Core::Generic::bindMemberFunction<ArclinkUser>(static_cast<bool (ArclinkLog::*)(ArclinkUser*)>(&ArclinkLog::add), this)),
	                       Archive::STATIC_TYPE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
