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
#include <seiscomp3/datamodel/strongmotion/strongorigindescription.h>
#include <seiscomp3/datamodel/strongmotion/strongmotionparameters.h>
#include <seiscomp3/datamodel/strongmotion/eventrecordreference.h>
#include <seiscomp3/datamodel/strongmotion/rupture.h>
#include <algorithm>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {
namespace StrongMotion {


IMPLEMENT_SC_CLASS_DERIVED(StrongOriginDescription, PublicObject, "StrongOriginDescription");


StrongOriginDescription::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("originID", "string", false, false, false, true, false, false, NULL, &StrongOriginDescription::setOriginID, &StrongOriginDescription::originID));
	addProperty(Core::simpleProperty("waveformCount", "int", false, false, false, false, true, false, NULL, &StrongOriginDescription::setWaveformCount, &StrongOriginDescription::waveformCount));
	addProperty(objectProperty<CreationInfo>("creationInfo", "CreationInfo", false, false, true, &StrongOriginDescription::setCreationInfo, &StrongOriginDescription::creationInfo));
	addProperty(arrayClassProperty<EventRecordReference>("eventRecordReference", "EventRecordReference", &StrongOriginDescription::eventRecordReferenceCount, &StrongOriginDescription::eventRecordReference, static_cast<bool (StrongOriginDescription::*)(EventRecordReference*)>(&StrongOriginDescription::add), &StrongOriginDescription::removeEventRecordReference, static_cast<bool (StrongOriginDescription::*)(EventRecordReference*)>(&StrongOriginDescription::remove)));
	addProperty(arrayObjectProperty("rupture", "Rupture", &StrongOriginDescription::ruptureCount, &StrongOriginDescription::rupture, static_cast<bool (StrongOriginDescription::*)(Rupture*)>(&StrongOriginDescription::add), &StrongOriginDescription::removeRupture, static_cast<bool (StrongOriginDescription::*)(Rupture*)>(&StrongOriginDescription::remove)));
}


IMPLEMENT_METAOBJECT(StrongOriginDescription)


StrongOriginDescription::StrongOriginDescription() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StrongOriginDescription::StrongOriginDescription(const StrongOriginDescription& other)
 : PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StrongOriginDescription::StrongOriginDescription(const std::string& publicID)
 : PublicObject(publicID) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StrongOriginDescription::~StrongOriginDescription() {
	std::for_each(_eventRecordReferences.begin(), _eventRecordReferences.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&EventRecordReference::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&EventRecordReferencePtr::get)));
	std::for_each(_ruptures.begin(), _ruptures.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Rupture::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&RupturePtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StrongOriginDescription* StrongOriginDescription::Create() {
	StrongOriginDescription* object = new StrongOriginDescription();
	return static_cast<StrongOriginDescription*>(GenerateId(object));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StrongOriginDescription* StrongOriginDescription::Create(const std::string& publicID) {
	if ( Find(publicID) != NULL ) {
		SEISCOMP_ERROR(
			"There exists already a PublicObject with Id '%s'",
			publicID.c_str()
		);
		return NULL;
	}

	return new StrongOriginDescription(publicID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StrongOriginDescription* StrongOriginDescription::Find(const std::string& publicID) {
	return StrongOriginDescription::Cast(PublicObject::Find(publicID));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongOriginDescription::operator==(const StrongOriginDescription& rhs) const {
	if ( _originID != rhs._originID ) return false;
	if ( _waveformCount != rhs._waveformCount ) return false;
	if ( _creationInfo != rhs._creationInfo ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongOriginDescription::operator!=(const StrongOriginDescription& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongOriginDescription::equal(const StrongOriginDescription& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StrongOriginDescription::setOriginID(const std::string& originID) {
	_originID = originID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& StrongOriginDescription::originID() const {
	return _originID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StrongOriginDescription::setWaveformCount(const OPT(int)& waveformCount) {
	_waveformCount = waveformCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StrongOriginDescription::waveformCount() const throw(Seiscomp::Core::ValueException) {
	if ( _waveformCount )
		return *_waveformCount;
	throw Seiscomp::Core::ValueException("StrongOriginDescription.waveformCount is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StrongOriginDescription::setCreationInfo(const OPT(CreationInfo)& creationInfo) {
	_creationInfo = creationInfo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CreationInfo& StrongOriginDescription::creationInfo() throw(Seiscomp::Core::ValueException) {
	if ( _creationInfo )
		return *_creationInfo;
	throw Seiscomp::Core::ValueException("StrongOriginDescription.creationInfo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CreationInfo& StrongOriginDescription::creationInfo() const throw(Seiscomp::Core::ValueException) {
	if ( _creationInfo )
		return *_creationInfo;
	throw Seiscomp::Core::ValueException("StrongOriginDescription.creationInfo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StrongMotionParameters* StrongOriginDescription::strongMotionParameters() const {
	return static_cast<StrongMotionParameters*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StrongOriginDescription& StrongOriginDescription::operator=(const StrongOriginDescription& other) {
	PublicObject::operator=(other);
	_originID = other._originID;
	_waveformCount = other._waveformCount;
	_creationInfo = other._creationInfo;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongOriginDescription::assign(Object* other) {
	StrongOriginDescription* otherStrongOriginDescription = StrongOriginDescription::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherStrongOriginDescription;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongOriginDescription::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	StrongMotionParameters* strongMotionParameters = StrongMotionParameters::Cast(parent);
	if ( strongMotionParameters != NULL )
		return strongMotionParameters->add(this);

	SEISCOMP_ERROR("StrongOriginDescription::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongOriginDescription::detachFrom(PublicObject* object) {
	if ( object == NULL ) return false;

	// check all possible parents
	StrongMotionParameters* strongMotionParameters = StrongMotionParameters::Cast(object);
	if ( strongMotionParameters != NULL ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return strongMotionParameters->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			StrongOriginDescription* child = strongMotionParameters->findStrongOriginDescription(publicID());
			if ( child != NULL )
				return strongMotionParameters->remove(child);
			else {
				SEISCOMP_DEBUG("StrongOriginDescription::detachFrom(StrongMotionParameters): strongOriginDescription has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("StrongOriginDescription::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongOriginDescription::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* StrongOriginDescription::clone() const {
	StrongOriginDescription* clonee = new StrongOriginDescription();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongOriginDescription::updateChild(Object* child) {
	// Do not know how to fetch child of type EventRecordReference without an index

	Rupture* ruptureChild = Rupture::Cast(child);
	if ( ruptureChild != NULL ) {
		Rupture* ruptureElement
			= Rupture::Cast(PublicObject::Find(ruptureChild->publicID()));
		if ( ruptureElement && ruptureElement->parent() == this ) {
			*ruptureElement = *ruptureChild;
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StrongOriginDescription::accept(Visitor* visitor) {
	if ( visitor->traversal() == Visitor::TM_TOPDOWN )
		if ( !visitor->visit(this) )
			return;

	for ( std::vector<EventRecordReferencePtr>::iterator it = _eventRecordReferences.begin(); it != _eventRecordReferences.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<RupturePtr>::iterator it = _ruptures.begin(); it != _ruptures.end(); ++it )
		(*it)->accept(visitor);

	if ( visitor->traversal() == Visitor::TM_BOTTOMUP )
		visitor->visit(this);
	else
		visitor->finished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t StrongOriginDescription::eventRecordReferenceCount() const {
	return _eventRecordReferences.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventRecordReference* StrongOriginDescription::eventRecordReference(size_t i) const {
	return _eventRecordReferences[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventRecordReference* StrongOriginDescription::findEventRecordReference(EventRecordReference* eventRecordReference) const {
	std::vector<EventRecordReferencePtr>::const_iterator it;
	for ( it = _eventRecordReferences.begin(); it != _eventRecordReferences.end(); ++it ) {
		if ( *eventRecordReference == **it )
			return (*it).get();
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongOriginDescription::add(EventRecordReference* eventRecordReference) {
	if ( eventRecordReference == NULL )
		return false;

	// Element has already a parent
	if ( eventRecordReference->parent() != NULL ) {
		SEISCOMP_ERROR("StrongOriginDescription::add(EventRecordReference*) -> element has already a parent");
		return false;
	}

	// Add the element
	_eventRecordReferences.push_back(eventRecordReference);
	eventRecordReference->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		eventRecordReference->accept(&nc);
	}

	// Notify registered observers
	childAdded(eventRecordReference);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongOriginDescription::remove(EventRecordReference* eventRecordReference) {
	if ( eventRecordReference == NULL )
		return false;

	if ( eventRecordReference->parent() != this ) {
		SEISCOMP_ERROR("StrongOriginDescription::remove(EventRecordReference*) -> element has another parent");
		return false;
	}

	std::vector<EventRecordReferencePtr>::iterator it;
	it = std::find(_eventRecordReferences.begin(), _eventRecordReferences.end(), eventRecordReference);
	// Element has not been found
	if ( it == _eventRecordReferences.end() ) {
		SEISCOMP_ERROR("StrongOriginDescription::remove(EventRecordReference*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_eventRecordReferences.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongOriginDescription::removeEventRecordReference(size_t i) {
	// index out of bounds
	if ( i >= _eventRecordReferences.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_eventRecordReferences[i]->accept(&nc);
	}

	_eventRecordReferences[i]->setParent(NULL);
	childRemoved(_eventRecordReferences[i].get());
	
	_eventRecordReferences.erase(_eventRecordReferences.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t StrongOriginDescription::ruptureCount() const {
	return _ruptures.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Rupture* StrongOriginDescription::rupture(size_t i) const {
	return _ruptures[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Rupture* StrongOriginDescription::findRupture(const std::string& publicID) const {
	Rupture* object = Rupture::Cast(PublicObject::Find(publicID));
	if ( object != NULL && object->parent() == this )
		return object;
	
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongOriginDescription::add(Rupture* rupture) {
	if ( rupture == NULL )
		return false;

	// Element has already a parent
	if ( rupture->parent() != NULL ) {
		SEISCOMP_ERROR("StrongOriginDescription::add(Rupture*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		Rupture* ruptureCached = Rupture::Find(rupture->publicID());
		if ( ruptureCached ) {
			if ( ruptureCached->parent() ) {
				if ( ruptureCached->parent() == this )
					SEISCOMP_ERROR("StrongOriginDescription::add(Rupture*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("StrongOriginDescription::add(Rupture*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				rupture = ruptureCached;
		}
	}

	// Add the element
	_ruptures.push_back(rupture);
	rupture->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		rupture->accept(&nc);
	}

	// Notify registered observers
	childAdded(rupture);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongOriginDescription::remove(Rupture* rupture) {
	if ( rupture == NULL )
		return false;

	if ( rupture->parent() != this ) {
		SEISCOMP_ERROR("StrongOriginDescription::remove(Rupture*) -> element has another parent");
		return false;
	}

	std::vector<RupturePtr>::iterator it;
	it = std::find(_ruptures.begin(), _ruptures.end(), rupture);
	// Element has not been found
	if ( it == _ruptures.end() ) {
		SEISCOMP_ERROR("StrongOriginDescription::remove(Rupture*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_ruptures.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongOriginDescription::removeRupture(size_t i) {
	// index out of bounds
	if ( i >= _ruptures.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_ruptures[i]->accept(&nc);
	}

	_ruptures[i]->setParent(NULL);
	childRemoved(_ruptures[i].get());
	
	_ruptures.erase(_ruptures.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StrongOriginDescription::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,7>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: StrongOriginDescription skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	PublicObject::serialize(ar);
	if ( !ar.success() ) return;

	ar & NAMED_OBJECT_HINT("originID", _originID, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("waveformCount", _waveformCount, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("creationInfo", _creationInfo, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("eventRecordReference",
	                       Seiscomp::Core::Generic::containerMember(_eventRecordReferences,
	                       Seiscomp::Core::Generic::bindMemberFunction<EventRecordReference>(static_cast<bool (StrongOriginDescription::*)(EventRecordReference*)>(&StrongOriginDescription::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("rupture",
	                       Seiscomp::Core::Generic::containerMember(_ruptures,
	                       Seiscomp::Core::Generic::bindMemberFunction<Rupture>(static_cast<bool (StrongOriginDescription::*)(Rupture*)>(&StrongOriginDescription::add), this)),
	                       Archive::STATIC_TYPE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
