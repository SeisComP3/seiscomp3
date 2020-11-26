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
#include <seiscomp3/datamodel/strongmotion/eventrecordreference.h>
#include <seiscomp3/datamodel/strongmotion/strongorigindescription.h>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {
namespace StrongMotion {


IMPLEMENT_SC_CLASS_DERIVED(EventRecordReference, Object, "EventRecordReference");


EventRecordReference::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("recordID", "string", false, false, false, true, false, false, NULL, &EventRecordReference::setRecordID, &EventRecordReference::recordID));
	addProperty(objectProperty<RealQuantity>("campbellDistance", "RealQuantity", false, false, true, &EventRecordReference::setCampbellDistance, &EventRecordReference::campbellDistance));
	addProperty(objectProperty<RealQuantity>("ruptureToStationAzimuth", "RealQuantity", false, false, true, &EventRecordReference::setRuptureToStationAzimuth, &EventRecordReference::ruptureToStationAzimuth));
	addProperty(objectProperty<RealQuantity>("ruptureAreaDistance", "RealQuantity", false, false, true, &EventRecordReference::setRuptureAreaDistance, &EventRecordReference::ruptureAreaDistance));
	addProperty(objectProperty<RealQuantity>("JoynerBooreDistance", "RealQuantity", false, false, true, &EventRecordReference::setJoynerBooreDistance, &EventRecordReference::JoynerBooreDistance));
	addProperty(objectProperty<RealQuantity>("closestFaultDistance", "RealQuantity", false, false, true, &EventRecordReference::setClosestFaultDistance, &EventRecordReference::closestFaultDistance));
	addProperty(Core::simpleProperty("preEventLength", "float", false, false, false, false, true, false, NULL, &EventRecordReference::setPreEventLength, &EventRecordReference::preEventLength));
	addProperty(Core::simpleProperty("postEventLength", "float", false, false, false, false, true, false, NULL, &EventRecordReference::setPostEventLength, &EventRecordReference::postEventLength));
}


IMPLEMENT_METAOBJECT(EventRecordReference)


EventRecordReference::EventRecordReference() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventRecordReference::EventRecordReference(const EventRecordReference& other)
 : Object() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventRecordReference::EventRecordReference(const std::string& recordID)
 : _recordID(recordID) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventRecordReference::EventRecordReference(const std::string& recordID,
                                           const OPT(RealQuantity)& campbellDistance,
                                           const OPT(RealQuantity)& ruptureToStationAzimuth,
                                           const OPT(RealQuantity)& ruptureAreaDistance,
                                           const OPT(RealQuantity)& JoynerBooreDistance,
                                           const OPT(RealQuantity)& closestFaultDistance,
                                           const OPT(double)& preEventLength,
                                           const OPT(double)& postEventLength)
 : _recordID(recordID),
   _campbellDistance(campbellDistance),
   _ruptureToStationAzimuth(ruptureToStationAzimuth),
   _ruptureAreaDistance(ruptureAreaDistance),
   _joynerBooreDistance(JoynerBooreDistance),
   _closestFaultDistance(closestFaultDistance),
   _preEventLength(preEventLength),
   _postEventLength(postEventLength) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventRecordReference::~EventRecordReference() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventRecordReference::operator==(const EventRecordReference& rhs) const {
	if ( !(_recordID == rhs._recordID) )
		return false;
	if ( !(_campbellDistance == rhs._campbellDistance) )
		return false;
	if ( !(_ruptureToStationAzimuth == rhs._ruptureToStationAzimuth) )
		return false;
	if ( !(_ruptureAreaDistance == rhs._ruptureAreaDistance) )
		return false;
	if ( !(_joynerBooreDistance == rhs._joynerBooreDistance) )
		return false;
	if ( !(_closestFaultDistance == rhs._closestFaultDistance) )
		return false;
	if ( !(_preEventLength == rhs._preEventLength) )
		return false;
	if ( !(_postEventLength == rhs._postEventLength) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventRecordReference::operator!=(const EventRecordReference& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventRecordReference::equal(const EventRecordReference& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventRecordReference::setRecordID(const std::string& recordID) {
	_recordID = recordID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& EventRecordReference::recordID() const {
	return _recordID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventRecordReference::setCampbellDistance(const OPT(RealQuantity)& campbellDistance) {
	_campbellDistance = campbellDistance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& EventRecordReference::campbellDistance() {
	if ( _campbellDistance )
		return *_campbellDistance;
	throw Seiscomp::Core::ValueException("EventRecordReference.campbellDistance is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& EventRecordReference::campbellDistance() const {
	if ( _campbellDistance )
		return *_campbellDistance;
	throw Seiscomp::Core::ValueException("EventRecordReference.campbellDistance is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventRecordReference::setRuptureToStationAzimuth(const OPT(RealQuantity)& ruptureToStationAzimuth) {
	_ruptureToStationAzimuth = ruptureToStationAzimuth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& EventRecordReference::ruptureToStationAzimuth() {
	if ( _ruptureToStationAzimuth )
		return *_ruptureToStationAzimuth;
	throw Seiscomp::Core::ValueException("EventRecordReference.ruptureToStationAzimuth is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& EventRecordReference::ruptureToStationAzimuth() const {
	if ( _ruptureToStationAzimuth )
		return *_ruptureToStationAzimuth;
	throw Seiscomp::Core::ValueException("EventRecordReference.ruptureToStationAzimuth is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventRecordReference::setRuptureAreaDistance(const OPT(RealQuantity)& ruptureAreaDistance) {
	_ruptureAreaDistance = ruptureAreaDistance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& EventRecordReference::ruptureAreaDistance() {
	if ( _ruptureAreaDistance )
		return *_ruptureAreaDistance;
	throw Seiscomp::Core::ValueException("EventRecordReference.ruptureAreaDistance is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& EventRecordReference::ruptureAreaDistance() const {
	if ( _ruptureAreaDistance )
		return *_ruptureAreaDistance;
	throw Seiscomp::Core::ValueException("EventRecordReference.ruptureAreaDistance is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventRecordReference::setJoynerBooreDistance(const OPT(RealQuantity)& JoynerBooreDistance) {
	_joynerBooreDistance = JoynerBooreDistance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& EventRecordReference::JoynerBooreDistance() {
	if ( _joynerBooreDistance )
		return *_joynerBooreDistance;
	throw Seiscomp::Core::ValueException("EventRecordReference.JoynerBooreDistance is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& EventRecordReference::JoynerBooreDistance() const {
	if ( _joynerBooreDistance )
		return *_joynerBooreDistance;
	throw Seiscomp::Core::ValueException("EventRecordReference.JoynerBooreDistance is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventRecordReference::setClosestFaultDistance(const OPT(RealQuantity)& closestFaultDistance) {
	_closestFaultDistance = closestFaultDistance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& EventRecordReference::closestFaultDistance() {
	if ( _closestFaultDistance )
		return *_closestFaultDistance;
	throw Seiscomp::Core::ValueException("EventRecordReference.closestFaultDistance is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& EventRecordReference::closestFaultDistance() const {
	if ( _closestFaultDistance )
		return *_closestFaultDistance;
	throw Seiscomp::Core::ValueException("EventRecordReference.closestFaultDistance is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventRecordReference::setPreEventLength(const OPT(double)& preEventLength) {
	_preEventLength = preEventLength;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double EventRecordReference::preEventLength() const {
	if ( _preEventLength )
		return *_preEventLength;
	throw Seiscomp::Core::ValueException("EventRecordReference.preEventLength is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventRecordReference::setPostEventLength(const OPT(double)& postEventLength) {
	_postEventLength = postEventLength;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double EventRecordReference::postEventLength() const {
	if ( _postEventLength )
		return *_postEventLength;
	throw Seiscomp::Core::ValueException("EventRecordReference.postEventLength is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StrongOriginDescription* EventRecordReference::strongOriginDescription() const {
	return static_cast<StrongOriginDescription*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventRecordReference& EventRecordReference::operator=(const EventRecordReference& other) {
	_recordID = other._recordID;
	_campbellDistance = other._campbellDistance;
	_ruptureToStationAzimuth = other._ruptureToStationAzimuth;
	_ruptureAreaDistance = other._ruptureAreaDistance;
	_joynerBooreDistance = other._joynerBooreDistance;
	_closestFaultDistance = other._closestFaultDistance;
	_preEventLength = other._preEventLength;
	_postEventLength = other._postEventLength;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventRecordReference::assign(Object* other) {
	EventRecordReference* otherEventRecordReference = EventRecordReference::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherEventRecordReference;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventRecordReference::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	StrongOriginDescription* strongOriginDescription = StrongOriginDescription::Cast(parent);
	if ( strongOriginDescription != NULL )
		return strongOriginDescription->add(this);

	SEISCOMP_ERROR("EventRecordReference::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventRecordReference::detachFrom(PublicObject* object) {
	if ( object == NULL ) return false;

	// check all possible parents
	StrongOriginDescription* strongOriginDescription = StrongOriginDescription::Cast(object);
	if ( strongOriginDescription != NULL ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return strongOriginDescription->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			EventRecordReference* child = strongOriginDescription->findEventRecordReference(this);
			if ( child != NULL )
				return strongOriginDescription->remove(child);
			else {
				SEISCOMP_DEBUG("EventRecordReference::detachFrom(StrongOriginDescription): eventRecordReference has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("EventRecordReference::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventRecordReference::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* EventRecordReference::clone() const {
	EventRecordReference* clonee = new EventRecordReference();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventRecordReference::accept(Visitor* visitor) {
	visitor->visit(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventRecordReference::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,11>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: EventRecordReference skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	ar & NAMED_OBJECT_HINT("recordID", _recordID, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("campbellDistance", _campbellDistance, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("ruptureToStationAzimuth", _ruptureToStationAzimuth, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("ruptureAreaDistance", _ruptureAreaDistance, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("JoynerBooreDistance", _joynerBooreDistance, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("closestFaultDistance", _closestFaultDistance, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("preEventLength", _preEventLength, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("postEventLength", _postEventLength, Archive::XML_ELEMENT);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
