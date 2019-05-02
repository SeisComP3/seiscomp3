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
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/compositetime.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <algorithm>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(Origin, PublicObject, "Origin");


namespace {
static Seiscomp::Core::MetaEnumImpl<OriginDepthType> metaOriginDepthType;
static Seiscomp::Core::MetaEnumImpl<OriginType> metaOriginType;
static Seiscomp::Core::MetaEnumImpl<EvaluationMode> metaEvaluationMode;
static Seiscomp::Core::MetaEnumImpl<EvaluationStatus> metaEvaluationStatus;
}


Origin::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(objectProperty<TimeQuantity>("time", "TimeQuantity", false, false, false, &Origin::setTime, &Origin::time));
	addProperty(objectProperty<RealQuantity>("latitude", "RealQuantity", false, false, false, &Origin::setLatitude, &Origin::latitude));
	addProperty(objectProperty<RealQuantity>("longitude", "RealQuantity", false, false, false, &Origin::setLongitude, &Origin::longitude));
	addProperty(objectProperty<RealQuantity>("depth", "RealQuantity", false, false, true, &Origin::setDepth, &Origin::depth));
	addProperty(enumProperty("depthType", "OriginDepthType", false, true, &metaOriginDepthType, &Origin::setDepthType, &Origin::depthType));
	addProperty(Core::simpleProperty("timeFixed", "boolean", false, false, false, false, true, false, NULL, &Origin::setTimeFixed, &Origin::timeFixed));
	addProperty(Core::simpleProperty("epicenterFixed", "boolean", false, false, false, false, true, false, NULL, &Origin::setEpicenterFixed, &Origin::epicenterFixed));
	addProperty(Core::simpleProperty("referenceSystemID", "string", false, false, false, false, false, false, NULL, &Origin::setReferenceSystemID, &Origin::referenceSystemID));
	addProperty(Core::simpleProperty("methodID", "string", false, false, false, false, false, false, NULL, &Origin::setMethodID, &Origin::methodID));
	addProperty(Core::simpleProperty("earthModelID", "string", false, false, false, false, false, false, NULL, &Origin::setEarthModelID, &Origin::earthModelID));
	addProperty(objectProperty<OriginQuality>("quality", "OriginQuality", false, false, true, &Origin::setQuality, &Origin::quality));
	addProperty(objectProperty<OriginUncertainty>("uncertainty", "OriginUncertainty", false, false, true, &Origin::setUncertainty, &Origin::uncertainty));
	addProperty(enumProperty("type", "OriginType", false, true, &metaOriginType, &Origin::setType, &Origin::type));
	addProperty(enumProperty("evaluationMode", "EvaluationMode", false, true, &metaEvaluationMode, &Origin::setEvaluationMode, &Origin::evaluationMode));
	addProperty(enumProperty("evaluationStatus", "EvaluationStatus", false, true, &metaEvaluationStatus, &Origin::setEvaluationStatus, &Origin::evaluationStatus));
	addProperty(objectProperty<CreationInfo>("creationInfo", "CreationInfo", false, false, true, &Origin::setCreationInfo, &Origin::creationInfo));
	addProperty(arrayClassProperty<Comment>("comment", "Comment", &Origin::commentCount, &Origin::comment, static_cast<bool (Origin::*)(Comment*)>(&Origin::add), &Origin::removeComment, static_cast<bool (Origin::*)(Comment*)>(&Origin::remove)));
	addProperty(arrayClassProperty<CompositeTime>("compositeTime", "CompositeTime", &Origin::compositeTimeCount, &Origin::compositeTime, static_cast<bool (Origin::*)(CompositeTime*)>(&Origin::add), &Origin::removeCompositeTime, static_cast<bool (Origin::*)(CompositeTime*)>(&Origin::remove)));
	addProperty(arrayClassProperty<Arrival>("arrival", "Arrival", &Origin::arrivalCount, &Origin::arrival, static_cast<bool (Origin::*)(Arrival*)>(&Origin::add), &Origin::removeArrival, static_cast<bool (Origin::*)(Arrival*)>(&Origin::remove)));
	addProperty(arrayObjectProperty("stationMagnitude", "StationMagnitude", &Origin::stationMagnitudeCount, &Origin::stationMagnitude, static_cast<bool (Origin::*)(StationMagnitude*)>(&Origin::add), &Origin::removeStationMagnitude, static_cast<bool (Origin::*)(StationMagnitude*)>(&Origin::remove)));
	addProperty(arrayObjectProperty("magnitude", "Magnitude", &Origin::magnitudeCount, &Origin::magnitude, static_cast<bool (Origin::*)(Magnitude*)>(&Origin::add), &Origin::removeMagnitude, static_cast<bool (Origin::*)(Magnitude*)>(&Origin::remove)));
}


IMPLEMENT_METAOBJECT(Origin)


Origin::Origin() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin::Origin(const Origin& other)
: PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin::Origin(const std::string& publicID)
: PublicObject(publicID) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin::~Origin() {
	std::for_each(_comments.begin(), _comments.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Comment::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&CommentPtr::get)));
	std::for_each(_compositeTimes.begin(), _compositeTimes.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&CompositeTime::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&CompositeTimePtr::get)));
	std::for_each(_arrivals.begin(), _arrivals.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Arrival::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&ArrivalPtr::get)));
	std::for_each(_stationMagnitudes.begin(), _stationMagnitudes.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&StationMagnitude::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&StationMagnitudePtr::get)));
	std::for_each(_magnitudes.begin(), _magnitudes.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Magnitude::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&MagnitudePtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin* Origin::Create() {
	Origin* object = new Origin();
	return static_cast<Origin*>(GenerateId(object));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin* Origin::Create(const std::string& publicID) {
	if ( PublicObject::IsRegistrationEnabled() && Find(publicID) != NULL ) {
		SEISCOMP_ERROR(
			"There exists already a PublicObject with Id '%s'",
			publicID.c_str()
		);
		return NULL;
	}

	return new Origin(publicID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin* Origin::Find(const std::string& publicID) {
	return Origin::Cast(PublicObject::Find(publicID));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::operator==(const Origin& rhs) const {
	if ( _time != rhs._time ) return false;
	if ( _latitude != rhs._latitude ) return false;
	if ( _longitude != rhs._longitude ) return false;
	if ( _depth != rhs._depth ) return false;
	if ( _depthType != rhs._depthType ) return false;
	if ( _timeFixed != rhs._timeFixed ) return false;
	if ( _epicenterFixed != rhs._epicenterFixed ) return false;
	if ( _referenceSystemID != rhs._referenceSystemID ) return false;
	if ( _methodID != rhs._methodID ) return false;
	if ( _earthModelID != rhs._earthModelID ) return false;
	if ( _quality != rhs._quality ) return false;
	if ( _uncertainty != rhs._uncertainty ) return false;
	if ( _type != rhs._type ) return false;
	if ( _evaluationMode != rhs._evaluationMode ) return false;
	if ( _evaluationStatus != rhs._evaluationStatus ) return false;
	if ( _creationInfo != rhs._creationInfo ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::operator!=(const Origin& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::equal(const Origin& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Origin::setTime(const TimeQuantity& time) {
	_time = time;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeQuantity& Origin::time() {
	return _time;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const TimeQuantity& Origin::time() const {
	return _time;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Origin::setLatitude(const RealQuantity& latitude) {
	_latitude = latitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& Origin::latitude() {
	return _latitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& Origin::latitude() const {
	return _latitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Origin::setLongitude(const RealQuantity& longitude) {
	_longitude = longitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& Origin::longitude() {
	return _longitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& Origin::longitude() const {
	return _longitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Origin::setDepth(const OPT(RealQuantity)& depth) {
	_depth = depth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& Origin::depth() {
	if ( _depth )
		return *_depth;
	throw Seiscomp::Core::ValueException("Origin.depth is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& Origin::depth() const {
	if ( _depth )
		return *_depth;
	throw Seiscomp::Core::ValueException("Origin.depth is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Origin::setDepthType(const OPT(OriginDepthType)& depthType) {
	_depthType = depthType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginDepthType Origin::depthType() const {
	if ( _depthType )
		return *_depthType;
	throw Seiscomp::Core::ValueException("Origin.depthType is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Origin::setTimeFixed(const OPT(bool)& timeFixed) {
	_timeFixed = timeFixed;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::timeFixed() const {
	if ( _timeFixed )
		return *_timeFixed;
	throw Seiscomp::Core::ValueException("Origin.timeFixed is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Origin::setEpicenterFixed(const OPT(bool)& epicenterFixed) {
	_epicenterFixed = epicenterFixed;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::epicenterFixed() const {
	if ( _epicenterFixed )
		return *_epicenterFixed;
	throw Seiscomp::Core::ValueException("Origin.epicenterFixed is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Origin::setReferenceSystemID(const std::string& referenceSystemID) {
	_referenceSystemID = referenceSystemID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Origin::referenceSystemID() const {
	return _referenceSystemID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Origin::setMethodID(const std::string& methodID) {
	_methodID = methodID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Origin::methodID() const {
	return _methodID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Origin::setEarthModelID(const std::string& earthModelID) {
	_earthModelID = earthModelID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Origin::earthModelID() const {
	return _earthModelID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Origin::setQuality(const OPT(OriginQuality)& quality) {
	_quality = quality;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginQuality& Origin::quality() {
	if ( _quality )
		return *_quality;
	throw Seiscomp::Core::ValueException("Origin.quality is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const OriginQuality& Origin::quality() const {
	if ( _quality )
		return *_quality;
	throw Seiscomp::Core::ValueException("Origin.quality is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Origin::setUncertainty(const OPT(OriginUncertainty)& uncertainty) {
	_uncertainty = uncertainty;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginUncertainty& Origin::uncertainty() {
	if ( _uncertainty )
		return *_uncertainty;
	throw Seiscomp::Core::ValueException("Origin.uncertainty is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const OriginUncertainty& Origin::uncertainty() const {
	if ( _uncertainty )
		return *_uncertainty;
	throw Seiscomp::Core::ValueException("Origin.uncertainty is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Origin::setType(const OPT(OriginType)& type) {
	_type = type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginType Origin::type() const {
	if ( _type )
		return *_type;
	throw Seiscomp::Core::ValueException("Origin.type is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Origin::setEvaluationMode(const OPT(EvaluationMode)& evaluationMode) {
	_evaluationMode = evaluationMode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EvaluationMode Origin::evaluationMode() const {
	if ( _evaluationMode )
		return *_evaluationMode;
	throw Seiscomp::Core::ValueException("Origin.evaluationMode is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Origin::setEvaluationStatus(const OPT(EvaluationStatus)& evaluationStatus) {
	_evaluationStatus = evaluationStatus;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EvaluationStatus Origin::evaluationStatus() const {
	if ( _evaluationStatus )
		return *_evaluationStatus;
	throw Seiscomp::Core::ValueException("Origin.evaluationStatus is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Origin::setCreationInfo(const OPT(CreationInfo)& creationInfo) {
	_creationInfo = creationInfo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CreationInfo& Origin::creationInfo() {
	if ( _creationInfo )
		return *_creationInfo;
	throw Seiscomp::Core::ValueException("Origin.creationInfo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CreationInfo& Origin::creationInfo() const {
	if ( _creationInfo )
		return *_creationInfo;
	throw Seiscomp::Core::ValueException("Origin.creationInfo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventParameters* Origin::eventParameters() const {
	return static_cast<EventParameters*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin& Origin::operator=(const Origin& other) {
	PublicObject::operator=(other);
	_time = other._time;
	_latitude = other._latitude;
	_longitude = other._longitude;
	_depth = other._depth;
	_depthType = other._depthType;
	_timeFixed = other._timeFixed;
	_epicenterFixed = other._epicenterFixed;
	_referenceSystemID = other._referenceSystemID;
	_methodID = other._methodID;
	_earthModelID = other._earthModelID;
	_quality = other._quality;
	_uncertainty = other._uncertainty;
	_type = other._type;
	_evaluationMode = other._evaluationMode;
	_evaluationStatus = other._evaluationStatus;
	_creationInfo = other._creationInfo;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::assign(Object* other) {
	Origin* otherOrigin = Origin::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherOrigin;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	EventParameters* eventParameters = EventParameters::Cast(parent);
	if ( eventParameters != NULL )
		return eventParameters->add(this);

	SEISCOMP_ERROR("Origin::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::detachFrom(PublicObject* object) {
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
			Origin* child = eventParameters->findOrigin(publicID());
			if ( child != NULL )
				return eventParameters->remove(child);
			else {
				SEISCOMP_DEBUG("Origin::detachFrom(EventParameters): origin has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("Origin::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* Origin::clone() const {
	Origin* clonee = new Origin();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::updateChild(Object* child) {
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

	// Do not know how to fetch child of type CompositeTime without an index

	Arrival* arrivalChild = Arrival::Cast(child);
	if ( arrivalChild != NULL ) {
		Arrival* arrivalElement = arrival(arrivalChild->index());
		if ( arrivalElement != NULL ) {
			*arrivalElement = *arrivalChild;
			arrivalElement->update();
			return true;
		}
		return false;
	}

	StationMagnitude* stationMagnitudeChild = StationMagnitude::Cast(child);
	if ( stationMagnitudeChild != NULL ) {
		StationMagnitude* stationMagnitudeElement
			= StationMagnitude::Cast(PublicObject::Find(stationMagnitudeChild->publicID()));
		if ( stationMagnitudeElement && stationMagnitudeElement->parent() == this ) {
			*stationMagnitudeElement = *stationMagnitudeChild;
			stationMagnitudeElement->update();
			return true;
		}
		return false;
	}

	Magnitude* magnitudeChild = Magnitude::Cast(child);
	if ( magnitudeChild != NULL ) {
		Magnitude* magnitudeElement
			= Magnitude::Cast(PublicObject::Find(magnitudeChild->publicID()));
		if ( magnitudeElement && magnitudeElement->parent() == this ) {
			*magnitudeElement = *magnitudeChild;
			magnitudeElement->update();
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Origin::accept(Visitor* visitor) {
	if ( visitor->traversal() == Visitor::TM_TOPDOWN )
		if ( !visitor->visit(this) )
			return;

	for ( std::vector<CommentPtr>::iterator it = _comments.begin(); it != _comments.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<CompositeTimePtr>::iterator it = _compositeTimes.begin(); it != _compositeTimes.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<ArrivalPtr>::iterator it = _arrivals.begin(); it != _arrivals.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<StationMagnitudePtr>::iterator it = _stationMagnitudes.begin(); it != _stationMagnitudes.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<MagnitudePtr>::iterator it = _magnitudes.begin(); it != _magnitudes.end(); ++it )
		(*it)->accept(visitor);

	if ( visitor->traversal() == Visitor::TM_BOTTOMUP )
		visitor->visit(this);
	else
		visitor->finished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Origin::commentCount() const {
	return _comments.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Comment* Origin::comment(size_t i) const {
	return _comments[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Comment* Origin::comment(const CommentIndex& i) const {
	for ( std::vector<CommentPtr>::const_iterator it = _comments.begin(); it != _comments.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::add(Comment* comment) {
	if ( comment == NULL )
		return false;

	// Element has already a parent
	if ( comment->parent() != NULL ) {
		SEISCOMP_ERROR("Origin::add(Comment*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<CommentPtr>::iterator it = _comments.begin(); it != _comments.end(); ++it ) {
		if ( (*it)->index() == comment->index() ) {
			SEISCOMP_ERROR("Origin::add(Comment*) -> an element with the same index has been added already");
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
bool Origin::remove(Comment* comment) {
	if ( comment == NULL )
		return false;

	if ( comment->parent() != this ) {
		SEISCOMP_ERROR("Origin::remove(Comment*) -> element has another parent");
		return false;
	}

	std::vector<CommentPtr>::iterator it;
	it = std::find(_comments.begin(), _comments.end(), comment);
	// Element has not been found
	if ( it == _comments.end() ) {
		SEISCOMP_ERROR("Origin::remove(Comment*) -> child object has not been found although the parent pointer matches???");
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
bool Origin::removeComment(size_t i) {
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
bool Origin::removeComment(const CommentIndex& i) {
	Comment* object = comment(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Origin::compositeTimeCount() const {
	return _compositeTimes.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CompositeTime* Origin::compositeTime(size_t i) const {
	return _compositeTimes[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CompositeTime* Origin::findCompositeTime(CompositeTime* compositeTime) const {
	std::vector<CompositeTimePtr>::const_iterator it;
	for ( it = _compositeTimes.begin(); it != _compositeTimes.end(); ++it ) {
		if ( *compositeTime == **it )
			return (*it).get();
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::add(CompositeTime* compositeTime) {
	if ( compositeTime == NULL )
		return false;

	// Element has already a parent
	if ( compositeTime->parent() != NULL ) {
		SEISCOMP_ERROR("Origin::add(CompositeTime*) -> element has already a parent");
		return false;
	}

	// Add the element
	_compositeTimes.push_back(compositeTime);
	compositeTime->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		compositeTime->accept(&nc);
	}

	// Notify registered observers
	childAdded(compositeTime);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::remove(CompositeTime* compositeTime) {
	if ( compositeTime == NULL )
		return false;

	if ( compositeTime->parent() != this ) {
		SEISCOMP_ERROR("Origin::remove(CompositeTime*) -> element has another parent");
		return false;
	}

	std::vector<CompositeTimePtr>::iterator it;
	it = std::find(_compositeTimes.begin(), _compositeTimes.end(), compositeTime);
	// Element has not been found
	if ( it == _compositeTimes.end() ) {
		SEISCOMP_ERROR("Origin::remove(CompositeTime*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_compositeTimes.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::removeCompositeTime(size_t i) {
	// index out of bounds
	if ( i >= _compositeTimes.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_compositeTimes[i]->accept(&nc);
	}

	_compositeTimes[i]->setParent(NULL);
	childRemoved(_compositeTimes[i].get());
	
	_compositeTimes.erase(_compositeTimes.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Origin::arrivalCount() const {
	return _arrivals.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Arrival* Origin::arrival(size_t i) const {
	return _arrivals[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Arrival* Origin::arrival(const ArrivalIndex& i) const {
	for ( std::vector<ArrivalPtr>::const_iterator it = _arrivals.begin(); it != _arrivals.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::add(Arrival* arrival) {
	if ( arrival == NULL )
		return false;

	// Element has already a parent
	if ( arrival->parent() != NULL ) {
		SEISCOMP_ERROR("Origin::add(Arrival*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<ArrivalPtr>::iterator it = _arrivals.begin(); it != _arrivals.end(); ++it ) {
		if ( (*it)->index() == arrival->index() ) {
			SEISCOMP_ERROR("Origin::add(Arrival*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_arrivals.push_back(arrival);
	arrival->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		arrival->accept(&nc);
	}

	// Notify registered observers
	childAdded(arrival);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::remove(Arrival* arrival) {
	if ( arrival == NULL )
		return false;

	if ( arrival->parent() != this ) {
		SEISCOMP_ERROR("Origin::remove(Arrival*) -> element has another parent");
		return false;
	}

	std::vector<ArrivalPtr>::iterator it;
	it = std::find(_arrivals.begin(), _arrivals.end(), arrival);
	// Element has not been found
	if ( it == _arrivals.end() ) {
		SEISCOMP_ERROR("Origin::remove(Arrival*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_arrivals.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::removeArrival(size_t i) {
	// index out of bounds
	if ( i >= _arrivals.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_arrivals[i]->accept(&nc);
	}

	_arrivals[i]->setParent(NULL);
	childRemoved(_arrivals[i].get());
	
	_arrivals.erase(_arrivals.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::removeArrival(const ArrivalIndex& i) {
	Arrival* object = arrival(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Origin::stationMagnitudeCount() const {
	return _stationMagnitudes.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StationMagnitude* Origin::stationMagnitude(size_t i) const {
	return _stationMagnitudes[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StationMagnitude* Origin::findStationMagnitude(const std::string& publicID) const {
	for ( std::vector<StationMagnitudePtr>::const_iterator it = _stationMagnitudes.begin(); it != _stationMagnitudes.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::add(StationMagnitude* stationMagnitude) {
	if ( stationMagnitude == NULL )
		return false;

	// Element has already a parent
	if ( stationMagnitude->parent() != NULL ) {
		SEISCOMP_ERROR("Origin::add(StationMagnitude*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		StationMagnitude* stationMagnitudeCached = StationMagnitude::Find(stationMagnitude->publicID());
		if ( stationMagnitudeCached ) {
			if ( stationMagnitudeCached->parent() ) {
				if ( stationMagnitudeCached->parent() == this )
					SEISCOMP_ERROR("Origin::add(StationMagnitude*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("Origin::add(StationMagnitude*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				stationMagnitude = stationMagnitudeCached;
		}
	}

	// Add the element
	_stationMagnitudes.push_back(stationMagnitude);
	stationMagnitude->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		stationMagnitude->accept(&nc);
	}

	// Notify registered observers
	childAdded(stationMagnitude);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::remove(StationMagnitude* stationMagnitude) {
	if ( stationMagnitude == NULL )
		return false;

	if ( stationMagnitude->parent() != this ) {
		SEISCOMP_ERROR("Origin::remove(StationMagnitude*) -> element has another parent");
		return false;
	}

	std::vector<StationMagnitudePtr>::iterator it;
	it = std::find(_stationMagnitudes.begin(), _stationMagnitudes.end(), stationMagnitude);
	// Element has not been found
	if ( it == _stationMagnitudes.end() ) {
		SEISCOMP_ERROR("Origin::remove(StationMagnitude*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_stationMagnitudes.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::removeStationMagnitude(size_t i) {
	// index out of bounds
	if ( i >= _stationMagnitudes.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_stationMagnitudes[i]->accept(&nc);
	}

	_stationMagnitudes[i]->setParent(NULL);
	childRemoved(_stationMagnitudes[i].get());
	
	_stationMagnitudes.erase(_stationMagnitudes.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Origin::magnitudeCount() const {
	return _magnitudes.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Magnitude* Origin::magnitude(size_t i) const {
	return _magnitudes[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Magnitude* Origin::findMagnitude(const std::string& publicID) const {
	for ( std::vector<MagnitudePtr>::const_iterator it = _magnitudes.begin(); it != _magnitudes.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::add(Magnitude* magnitude) {
	if ( magnitude == NULL )
		return false;

	// Element has already a parent
	if ( magnitude->parent() != NULL ) {
		SEISCOMP_ERROR("Origin::add(Magnitude*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		Magnitude* magnitudeCached = Magnitude::Find(magnitude->publicID());
		if ( magnitudeCached ) {
			if ( magnitudeCached->parent() ) {
				if ( magnitudeCached->parent() == this )
					SEISCOMP_ERROR("Origin::add(Magnitude*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("Origin::add(Magnitude*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				magnitude = magnitudeCached;
		}
	}

	// Add the element
	_magnitudes.push_back(magnitude);
	magnitude->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		magnitude->accept(&nc);
	}

	// Notify registered observers
	childAdded(magnitude);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::remove(Magnitude* magnitude) {
	if ( magnitude == NULL )
		return false;

	if ( magnitude->parent() != this ) {
		SEISCOMP_ERROR("Origin::remove(Magnitude*) -> element has another parent");
		return false;
	}

	std::vector<MagnitudePtr>::iterator it;
	it = std::find(_magnitudes.begin(), _magnitudes.end(), magnitude);
	// Element has not been found
	if ( it == _magnitudes.end() ) {
		SEISCOMP_ERROR("Origin::remove(Magnitude*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_magnitudes.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Origin::removeMagnitude(size_t i) {
	// index out of bounds
	if ( i >= _magnitudes.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_magnitudes[i]->accept(&nc);
	}

	_magnitudes[i]->setParent(NULL);
	childRemoved(_magnitudes[i].get());
	
	_magnitudes.erase(_magnitudes.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Origin::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,11>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: Origin skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	PublicObject::serialize(ar);
	if ( !ar.success() ) return;

	ar & NAMED_OBJECT_HINT("time", _time, Archive::STATIC_TYPE | Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("latitude", _latitude, Archive::STATIC_TYPE | Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("longitude", _longitude, Archive::STATIC_TYPE | Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("depth", _depth, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("depthType", _depthType, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("timeFixed", _timeFixed, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("epicenterFixed", _epicenterFixed, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("referenceSystemID", _referenceSystemID, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("methodID", _methodID, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("earthModelID", _earthModelID, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("quality", _quality, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("uncertainty", _uncertainty, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("type", _type, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("evaluationMode", _evaluationMode, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("evaluationStatus", _evaluationStatus, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("creationInfo", _creationInfo, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("comment",
	                       Seiscomp::Core::Generic::containerMember(_comments,
	                       Seiscomp::Core::Generic::bindMemberFunction<Comment>(static_cast<bool (Origin::*)(Comment*)>(&Origin::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("compositeTime",
	                       Seiscomp::Core::Generic::containerMember(_compositeTimes,
	                       Seiscomp::Core::Generic::bindMemberFunction<CompositeTime>(static_cast<bool (Origin::*)(CompositeTime*)>(&Origin::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("arrival",
	                       Seiscomp::Core::Generic::containerMember(_arrivals,
	                       Seiscomp::Core::Generic::bindMemberFunction<Arrival>(static_cast<bool (Origin::*)(Arrival*)>(&Origin::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("stationMagnitude",
	                       Seiscomp::Core::Generic::containerMember(_stationMagnitudes,
	                       Seiscomp::Core::Generic::bindMemberFunction<StationMagnitude>(static_cast<bool (Origin::*)(StationMagnitude*)>(&Origin::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("magnitude",
	                       Seiscomp::Core::Generic::containerMember(_magnitudes,
	                       Seiscomp::Core::Generic::bindMemberFunction<Magnitude>(static_cast<bool (Origin::*)(Magnitude*)>(&Origin::add), this)),
	                       Archive::STATIC_TYPE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
