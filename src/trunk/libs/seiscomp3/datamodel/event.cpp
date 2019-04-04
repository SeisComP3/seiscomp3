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
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <algorithm>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(Event, PublicObject, "Event");


namespace {
static Seiscomp::Core::MetaEnumImpl<EventType> metaEventType;
static Seiscomp::Core::MetaEnumImpl<EventTypeCertainty> metaEventTypeCertainty;
}


Event::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("preferredOriginID", "string", false, false, false, true, false, false, NULL, &Event::setPreferredOriginID, &Event::preferredOriginID));
	addProperty(Core::simpleProperty("preferredMagnitudeID", "string", false, false, false, true, false, false, NULL, &Event::setPreferredMagnitudeID, &Event::preferredMagnitudeID));
	addProperty(Core::simpleProperty("preferredFocalMechanismID", "string", false, false, false, true, false, false, NULL, &Event::setPreferredFocalMechanismID, &Event::preferredFocalMechanismID));
	addProperty(enumProperty("type", "EventType", false, true, &metaEventType, &Event::setType, &Event::type));
	addProperty(enumProperty("typeCertainty", "EventTypeCertainty", false, true, &metaEventTypeCertainty, &Event::setTypeCertainty, &Event::typeCertainty));
	addProperty(objectProperty<CreationInfo>("creationInfo", "CreationInfo", false, false, true, &Event::setCreationInfo, &Event::creationInfo));
	addProperty(arrayClassProperty<EventDescription>("description", "EventDescription", &Event::eventDescriptionCount, &Event::eventDescription, static_cast<bool (Event::*)(EventDescription*)>(&Event::add), &Event::removeEventDescription, static_cast<bool (Event::*)(EventDescription*)>(&Event::remove)));
	addProperty(arrayClassProperty<Comment>("comment", "Comment", &Event::commentCount, &Event::comment, static_cast<bool (Event::*)(Comment*)>(&Event::add), &Event::removeComment, static_cast<bool (Event::*)(Comment*)>(&Event::remove)));
	addProperty(arrayClassProperty<OriginReference>("originReference", "OriginReference", &Event::originReferenceCount, &Event::originReference, static_cast<bool (Event::*)(OriginReference*)>(&Event::add), &Event::removeOriginReference, static_cast<bool (Event::*)(OriginReference*)>(&Event::remove)));
	addProperty(arrayClassProperty<FocalMechanismReference>("focalMechanismReference", "FocalMechanismReference", &Event::focalMechanismReferenceCount, &Event::focalMechanismReference, static_cast<bool (Event::*)(FocalMechanismReference*)>(&Event::add), &Event::removeFocalMechanismReference, static_cast<bool (Event::*)(FocalMechanismReference*)>(&Event::remove)));
}


IMPLEMENT_METAOBJECT(Event)


Event::Event() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Event::Event(const Event& other)
: PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Event::Event(const std::string& publicID)
: PublicObject(publicID) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Event::~Event() {
	std::for_each(_eventDescriptions.begin(), _eventDescriptions.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&EventDescription::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&EventDescriptionPtr::get)));
	std::for_each(_comments.begin(), _comments.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Comment::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&CommentPtr::get)));
	std::for_each(_originReferences.begin(), _originReferences.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&OriginReference::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&OriginReferencePtr::get)));
	std::for_each(_focalMechanismReferences.begin(), _focalMechanismReferences.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&FocalMechanismReference::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&FocalMechanismReferencePtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Event* Event::Create() {
	Event* object = new Event();
	return static_cast<Event*>(GenerateId(object));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Event* Event::Create(const std::string& publicID) {
	if ( PublicObject::IsRegistrationEnabled() && Find(publicID) != NULL ) {
		SEISCOMP_ERROR(
			"There exists already a PublicObject with Id '%s'",
			publicID.c_str()
		);
		return NULL;
	}

	return new Event(publicID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Event* Event::Find(const std::string& publicID) {
	return Event::Cast(PublicObject::Find(publicID));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::operator==(const Event& rhs) const {
	if ( _preferredOriginID != rhs._preferredOriginID ) return false;
	if ( _preferredMagnitudeID != rhs._preferredMagnitudeID ) return false;
	if ( _preferredFocalMechanismID != rhs._preferredFocalMechanismID ) return false;
	if ( _type != rhs._type ) return false;
	if ( _typeCertainty != rhs._typeCertainty ) return false;
	if ( _creationInfo != rhs._creationInfo ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::operator!=(const Event& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::equal(const Event& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Event::setPreferredOriginID(const std::string& preferredOriginID) {
	_preferredOriginID = preferredOriginID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Event::preferredOriginID() const {
	return _preferredOriginID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Event::setPreferredMagnitudeID(const std::string& preferredMagnitudeID) {
	_preferredMagnitudeID = preferredMagnitudeID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Event::preferredMagnitudeID() const {
	return _preferredMagnitudeID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Event::setPreferredFocalMechanismID(const std::string& preferredFocalMechanismID) {
	_preferredFocalMechanismID = preferredFocalMechanismID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Event::preferredFocalMechanismID() const {
	return _preferredFocalMechanismID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Event::setType(const OPT(EventType)& type) {
	_type = type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventType Event::type() const {
	if ( _type )
		return *_type;
	throw Seiscomp::Core::ValueException("Event.type is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Event::setTypeCertainty(const OPT(EventTypeCertainty)& typeCertainty) {
	_typeCertainty = typeCertainty;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventTypeCertainty Event::typeCertainty() const {
	if ( _typeCertainty )
		return *_typeCertainty;
	throw Seiscomp::Core::ValueException("Event.typeCertainty is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Event::setCreationInfo(const OPT(CreationInfo)& creationInfo) {
	_creationInfo = creationInfo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CreationInfo& Event::creationInfo() {
	if ( _creationInfo )
		return *_creationInfo;
	throw Seiscomp::Core::ValueException("Event.creationInfo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CreationInfo& Event::creationInfo() const {
	if ( _creationInfo )
		return *_creationInfo;
	throw Seiscomp::Core::ValueException("Event.creationInfo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventParameters* Event::eventParameters() const {
	return static_cast<EventParameters*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Event& Event::operator=(const Event& other) {
	PublicObject::operator=(other);
	_preferredOriginID = other._preferredOriginID;
	_preferredMagnitudeID = other._preferredMagnitudeID;
	_preferredFocalMechanismID = other._preferredFocalMechanismID;
	_type = other._type;
	_typeCertainty = other._typeCertainty;
	_creationInfo = other._creationInfo;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::assign(Object* other) {
	Event* otherEvent = Event::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherEvent;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	EventParameters* eventParameters = EventParameters::Cast(parent);
	if ( eventParameters != NULL )
		return eventParameters->add(this);

	SEISCOMP_ERROR("Event::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::detachFrom(PublicObject* object) {
	if ( object == NULL ) return false;

	// check all possible parents
	EventParameters* eventParameters = EventParameters::Cast(object);
	if ( eventParameters != NULL ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return eventParameters->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Event* child = eventParameters->findEvent(publicID());
			if ( child != NULL )
				return eventParameters->remove(child);
			else {
				SEISCOMP_DEBUG("Event::detachFrom(EventParameters): event has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("Event::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* Event::clone() const {
	Event* clonee = new Event();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::updateChild(Object* child) {
	EventDescription* eventDescriptionChild = EventDescription::Cast(child);
	if ( eventDescriptionChild != NULL ) {
		EventDescription* eventDescriptionElement = eventDescription(eventDescriptionChild->index());
		if ( eventDescriptionElement != NULL ) {
			*eventDescriptionElement = *eventDescriptionChild;
			eventDescriptionElement->update();
			return true;
		}
		return false;
	}

	Comment* commentChild = Comment::Cast(child);
	if ( commentChild != NULL ) {
		Comment* commentElement = comment(commentChild->index());
		if ( commentElement != NULL ) {
			*commentElement = *commentChild;
			commentElement->update();
			return true;
		}
		return false;
	}

	OriginReference* originReferenceChild = OriginReference::Cast(child);
	if ( originReferenceChild != NULL ) {
		OriginReference* originReferenceElement = originReference(originReferenceChild->index());
		if ( originReferenceElement != NULL ) {
			*originReferenceElement = *originReferenceChild;
			originReferenceElement->update();
			return true;
		}
		return false;
	}

	FocalMechanismReference* focalMechanismReferenceChild = FocalMechanismReference::Cast(child);
	if ( focalMechanismReferenceChild != NULL ) {
		FocalMechanismReference* focalMechanismReferenceElement = focalMechanismReference(focalMechanismReferenceChild->index());
		if ( focalMechanismReferenceElement != NULL ) {
			*focalMechanismReferenceElement = *focalMechanismReferenceChild;
			focalMechanismReferenceElement->update();
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Event::accept(Visitor* visitor) {
	if ( visitor->traversal() == Visitor::TM_TOPDOWN )
		if ( !visitor->visit(this) )
			return;

	for ( std::vector<EventDescriptionPtr>::iterator it = _eventDescriptions.begin(); it != _eventDescriptions.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<CommentPtr>::iterator it = _comments.begin(); it != _comments.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<OriginReferencePtr>::iterator it = _originReferences.begin(); it != _originReferences.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<FocalMechanismReferencePtr>::iterator it = _focalMechanismReferences.begin(); it != _focalMechanismReferences.end(); ++it )
		(*it)->accept(visitor);

	if ( visitor->traversal() == Visitor::TM_BOTTOMUP )
		visitor->visit(this);
	else
		visitor->finished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Event::eventDescriptionCount() const {
	return _eventDescriptions.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventDescription* Event::eventDescription(size_t i) const {
	return _eventDescriptions[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventDescription* Event::eventDescription(const EventDescriptionIndex& i) const {
	for ( std::vector<EventDescriptionPtr>::const_iterator it = _eventDescriptions.begin(); it != _eventDescriptions.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::add(EventDescription* eventDescription) {
	if ( eventDescription == NULL )
		return false;

	// Element has already a parent
	if ( eventDescription->parent() != NULL ) {
		SEISCOMP_ERROR("Event::add(EventDescription*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<EventDescriptionPtr>::iterator it = _eventDescriptions.begin(); it != _eventDescriptions.end(); ++it ) {
		if ( (*it)->index() == eventDescription->index() ) {
			SEISCOMP_ERROR("Event::add(EventDescription*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_eventDescriptions.push_back(eventDescription);
	eventDescription->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		eventDescription->accept(&nc);
	}

	// Notify registered observers
	childAdded(eventDescription);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::remove(EventDescription* eventDescription) {
	if ( eventDescription == NULL )
		return false;

	if ( eventDescription->parent() != this ) {
		SEISCOMP_ERROR("Event::remove(EventDescription*) -> element has another parent");
		return false;
	}

	std::vector<EventDescriptionPtr>::iterator it;
	it = std::find(_eventDescriptions.begin(), _eventDescriptions.end(), eventDescription);
	// Element has not been found
	if ( it == _eventDescriptions.end() ) {
		SEISCOMP_ERROR("Event::remove(EventDescription*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_eventDescriptions.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::removeEventDescription(size_t i) {
	// index out of bounds
	if ( i >= _eventDescriptions.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_eventDescriptions[i]->accept(&nc);
	}

	_eventDescriptions[i]->setParent(NULL);
	childRemoved(_eventDescriptions[i].get());
	
	_eventDescriptions.erase(_eventDescriptions.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::removeEventDescription(const EventDescriptionIndex& i) {
	EventDescription* object = eventDescription(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Event::commentCount() const {
	return _comments.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Comment* Event::comment(size_t i) const {
	return _comments[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Comment* Event::comment(const CommentIndex& i) const {
	for ( std::vector<CommentPtr>::const_iterator it = _comments.begin(); it != _comments.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::add(Comment* comment) {
	if ( comment == NULL )
		return false;

	// Element has already a parent
	if ( comment->parent() != NULL ) {
		SEISCOMP_ERROR("Event::add(Comment*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<CommentPtr>::iterator it = _comments.begin(); it != _comments.end(); ++it ) {
		if ( (*it)->index() == comment->index() ) {
			SEISCOMP_ERROR("Event::add(Comment*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_comments.push_back(comment);
	comment->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		comment->accept(&nc);
	}

	// Notify registered observers
	childAdded(comment);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::remove(Comment* comment) {
	if ( comment == NULL )
		return false;

	if ( comment->parent() != this ) {
		SEISCOMP_ERROR("Event::remove(Comment*) -> element has another parent");
		return false;
	}

	std::vector<CommentPtr>::iterator it;
	it = std::find(_comments.begin(), _comments.end(), comment);
	// Element has not been found
	if ( it == _comments.end() ) {
		SEISCOMP_ERROR("Event::remove(Comment*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_comments.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::removeComment(size_t i) {
	// index out of bounds
	if ( i >= _comments.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_comments[i]->accept(&nc);
	}

	_comments[i]->setParent(NULL);
	childRemoved(_comments[i].get());
	
	_comments.erase(_comments.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::removeComment(const CommentIndex& i) {
	Comment* object = comment(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Event::originReferenceCount() const {
	return _originReferences.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginReference* Event::originReference(size_t i) const {
	return _originReferences[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginReference* Event::originReference(const OriginReferenceIndex& i) const {
	for ( std::vector<OriginReferencePtr>::const_iterator it = _originReferences.begin(); it != _originReferences.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::add(OriginReference* originReference) {
	if ( originReference == NULL )
		return false;

	// Element has already a parent
	if ( originReference->parent() != NULL ) {
		SEISCOMP_ERROR("Event::add(OriginReference*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<OriginReferencePtr>::iterator it = _originReferences.begin(); it != _originReferences.end(); ++it ) {
		if ( (*it)->index() == originReference->index() ) {
			SEISCOMP_ERROR("Event::add(OriginReference*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_originReferences.push_back(originReference);
	originReference->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		originReference->accept(&nc);
	}

	// Notify registered observers
	childAdded(originReference);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::remove(OriginReference* originReference) {
	if ( originReference == NULL )
		return false;

	if ( originReference->parent() != this ) {
		SEISCOMP_ERROR("Event::remove(OriginReference*) -> element has another parent");
		return false;
	}

	std::vector<OriginReferencePtr>::iterator it;
	it = std::find(_originReferences.begin(), _originReferences.end(), originReference);
	// Element has not been found
	if ( it == _originReferences.end() ) {
		SEISCOMP_ERROR("Event::remove(OriginReference*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_originReferences.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::removeOriginReference(size_t i) {
	// index out of bounds
	if ( i >= _originReferences.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_originReferences[i]->accept(&nc);
	}

	_originReferences[i]->setParent(NULL);
	childRemoved(_originReferences[i].get());
	
	_originReferences.erase(_originReferences.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::removeOriginReference(const OriginReferenceIndex& i) {
	OriginReference* object = originReference(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Event::focalMechanismReferenceCount() const {
	return _focalMechanismReferences.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FocalMechanismReference* Event::focalMechanismReference(size_t i) const {
	return _focalMechanismReferences[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FocalMechanismReference* Event::focalMechanismReference(const FocalMechanismReferenceIndex& i) const {
	for ( std::vector<FocalMechanismReferencePtr>::const_iterator it = _focalMechanismReferences.begin(); it != _focalMechanismReferences.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::add(FocalMechanismReference* focalMechanismReference) {
	if ( focalMechanismReference == NULL )
		return false;

	// Element has already a parent
	if ( focalMechanismReference->parent() != NULL ) {
		SEISCOMP_ERROR("Event::add(FocalMechanismReference*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<FocalMechanismReferencePtr>::iterator it = _focalMechanismReferences.begin(); it != _focalMechanismReferences.end(); ++it ) {
		if ( (*it)->index() == focalMechanismReference->index() ) {
			SEISCOMP_ERROR("Event::add(FocalMechanismReference*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_focalMechanismReferences.push_back(focalMechanismReference);
	focalMechanismReference->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		focalMechanismReference->accept(&nc);
	}

	// Notify registered observers
	childAdded(focalMechanismReference);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::remove(FocalMechanismReference* focalMechanismReference) {
	if ( focalMechanismReference == NULL )
		return false;

	if ( focalMechanismReference->parent() != this ) {
		SEISCOMP_ERROR("Event::remove(FocalMechanismReference*) -> element has another parent");
		return false;
	}

	std::vector<FocalMechanismReferencePtr>::iterator it;
	it = std::find(_focalMechanismReferences.begin(), _focalMechanismReferences.end(), focalMechanismReference);
	// Element has not been found
	if ( it == _focalMechanismReferences.end() ) {
		SEISCOMP_ERROR("Event::remove(FocalMechanismReference*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_focalMechanismReferences.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::removeFocalMechanismReference(size_t i) {
	// index out of bounds
	if ( i >= _focalMechanismReferences.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_focalMechanismReferences[i]->accept(&nc);
	}

	_focalMechanismReferences[i]->setParent(NULL);
	childRemoved(_focalMechanismReferences[i].get());
	
	_focalMechanismReferences.erase(_focalMechanismReferences.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Event::removeFocalMechanismReference(const FocalMechanismReferenceIndex& i) {
	FocalMechanismReference* object = focalMechanismReference(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Event::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,11>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: Event skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	PublicObject::serialize(ar);
	if ( !ar.success() ) return;

	ar & NAMED_OBJECT_HINT("preferredOriginID", _preferredOriginID, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("preferredMagnitudeID", _preferredMagnitudeID, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("preferredFocalMechanismID", _preferredFocalMechanismID, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("type", _type, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("typeCertainty", _typeCertainty, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("creationInfo", _creationInfo, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("description",
	                       Seiscomp::Core::Generic::containerMember(_eventDescriptions,
	                       Seiscomp::Core::Generic::bindMemberFunction<EventDescription>(static_cast<bool (Event::*)(EventDescription*)>(&Event::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("comment",
	                       Seiscomp::Core::Generic::containerMember(_comments,
	                       Seiscomp::Core::Generic::bindMemberFunction<Comment>(static_cast<bool (Event::*)(Comment*)>(&Event::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("originReference",
	                       Seiscomp::Core::Generic::containerMember(_originReferences,
	                       Seiscomp::Core::Generic::bindMemberFunction<OriginReference>(static_cast<bool (Event::*)(OriginReference*)>(&Event::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("focalMechanismReference",
	                       Seiscomp::Core::Generic::containerMember(_focalMechanismReferences,
	                       Seiscomp::Core::Generic::bindMemberFunction<FocalMechanismReference>(static_cast<bool (Event::*)(FocalMechanismReference*)>(&Event::add), this)),
	                       Archive::STATIC_TYPE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
