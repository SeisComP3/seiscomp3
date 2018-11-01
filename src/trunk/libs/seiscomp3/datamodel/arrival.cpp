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
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(Arrival, Object, "Arrival");


Arrival::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("pickID", "string", false, false, true, true, false, false, NULL, &Arrival::setPickID, &Arrival::pickID));
	addProperty(objectProperty<Phase>("phase", "Phase", false, false, false, &Arrival::setPhase, &Arrival::phase));
	addProperty(Core::simpleProperty("timeCorrection", "float", false, false, false, false, true, false, NULL, &Arrival::setTimeCorrection, &Arrival::timeCorrection));
	addProperty(Core::simpleProperty("azimuth", "float", false, false, false, false, true, false, NULL, &Arrival::setAzimuth, &Arrival::azimuth));
	addProperty(Core::simpleProperty("distance", "float", false, false, false, false, true, false, NULL, &Arrival::setDistance, &Arrival::distance));
	addProperty(Core::simpleProperty("takeOffAngle", "float", false, false, false, false, true, false, NULL, &Arrival::setTakeOffAngle, &Arrival::takeOffAngle));
	addProperty(Core::simpleProperty("timeResidual", "float", false, false, false, false, true, false, NULL, &Arrival::setTimeResidual, &Arrival::timeResidual));
	addProperty(Core::simpleProperty("horizontalSlownessResidual", "float", false, false, false, false, true, false, NULL, &Arrival::setHorizontalSlownessResidual, &Arrival::horizontalSlownessResidual));
	addProperty(Core::simpleProperty("backazimuthResidual", "float", false, false, false, false, true, false, NULL, &Arrival::setBackazimuthResidual, &Arrival::backazimuthResidual));
	addProperty(Core::simpleProperty("timeUsed", "boolean", false, false, false, false, true, false, NULL, &Arrival::setTimeUsed, &Arrival::timeUsed));
	addProperty(Core::simpleProperty("horizontalSlownessUsed", "boolean", false, false, false, false, true, false, NULL, &Arrival::setHorizontalSlownessUsed, &Arrival::horizontalSlownessUsed));
	addProperty(Core::simpleProperty("backazimuthUsed", "boolean", false, false, false, false, true, false, NULL, &Arrival::setBackazimuthUsed, &Arrival::backazimuthUsed));
	addProperty(Core::simpleProperty("weight", "float", false, false, false, false, true, false, NULL, &Arrival::setWeight, &Arrival::weight));
	addProperty(Core::simpleProperty("earthModelID", "string", false, false, false, false, false, false, NULL, &Arrival::setEarthModelID, &Arrival::earthModelID));
	addProperty(Core::simpleProperty("preliminary", "boolean", false, false, false, false, true, false, NULL, &Arrival::setPreliminary, &Arrival::preliminary));
	addProperty(objectProperty<CreationInfo>("creationInfo", "CreationInfo", false, false, true, &Arrival::setCreationInfo, &Arrival::creationInfo));
}


IMPLEMENT_METAOBJECT(Arrival)


ArrivalIndex::ArrivalIndex() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArrivalIndex::ArrivalIndex(const std::string& pickID_) {
	pickID = pickID_;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArrivalIndex::ArrivalIndex(const ArrivalIndex& idx) {
	pickID = idx.pickID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArrivalIndex::operator==(const ArrivalIndex& idx) const {
	return pickID == idx.pickID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArrivalIndex::operator!=(const ArrivalIndex& idx) const {
	return !operator==(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Arrival::Arrival() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Arrival::Arrival(const Arrival& other)
: Object() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Arrival::~Arrival() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Arrival::operator==(const Arrival& rhs) const {
	if ( _index != rhs._index ) return false;
	if ( _phase != rhs._phase ) return false;
	if ( _timeCorrection != rhs._timeCorrection ) return false;
	if ( _azimuth != rhs._azimuth ) return false;
	if ( _distance != rhs._distance ) return false;
	if ( _takeOffAngle != rhs._takeOffAngle ) return false;
	if ( _timeResidual != rhs._timeResidual ) return false;
	if ( _horizontalSlownessResidual != rhs._horizontalSlownessResidual ) return false;
	if ( _backazimuthResidual != rhs._backazimuthResidual ) return false;
	if ( _timeUsed != rhs._timeUsed ) return false;
	if ( _horizontalSlownessUsed != rhs._horizontalSlownessUsed ) return false;
	if ( _backazimuthUsed != rhs._backazimuthUsed ) return false;
	if ( _weight != rhs._weight ) return false;
	if ( _earthModelID != rhs._earthModelID ) return false;
	if ( _preliminary != rhs._preliminary ) return false;
	if ( _creationInfo != rhs._creationInfo ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Arrival::operator!=(const Arrival& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Arrival::equal(const Arrival& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setPickID(const std::string& pickID) {
	_index.pickID = pickID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Arrival::pickID() const {
	return _index.pickID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setPhase(const Phase& phase) {
	_phase = phase;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Phase& Arrival::phase() {
	return _phase;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Phase& Arrival::phase() const {
	return _phase;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setTimeCorrection(const OPT(double)& timeCorrection) {
	_timeCorrection = timeCorrection;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Arrival::timeCorrection() const {
	if ( _timeCorrection )
		return *_timeCorrection;
	throw Seiscomp::Core::ValueException("Arrival.timeCorrection is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setAzimuth(const OPT(double)& azimuth) {
	_azimuth = azimuth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Arrival::azimuth() const {
	if ( _azimuth )
		return *_azimuth;
	throw Seiscomp::Core::ValueException("Arrival.azimuth is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setDistance(const OPT(double)& distance) {
	_distance = distance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Arrival::distance() const {
	if ( _distance )
		return *_distance;
	throw Seiscomp::Core::ValueException("Arrival.distance is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setTakeOffAngle(const OPT(double)& takeOffAngle) {
	_takeOffAngle = takeOffAngle;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Arrival::takeOffAngle() const {
	if ( _takeOffAngle )
		return *_takeOffAngle;
	throw Seiscomp::Core::ValueException("Arrival.takeOffAngle is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setTimeResidual(const OPT(double)& timeResidual) {
	_timeResidual = timeResidual;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Arrival::timeResidual() const {
	if ( _timeResidual )
		return *_timeResidual;
	throw Seiscomp::Core::ValueException("Arrival.timeResidual is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setHorizontalSlownessResidual(const OPT(double)& horizontalSlownessResidual) {
	_horizontalSlownessResidual = horizontalSlownessResidual;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Arrival::horizontalSlownessResidual() const {
	if ( _horizontalSlownessResidual )
		return *_horizontalSlownessResidual;
	throw Seiscomp::Core::ValueException("Arrival.horizontalSlownessResidual is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setBackazimuthResidual(const OPT(double)& backazimuthResidual) {
	_backazimuthResidual = backazimuthResidual;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Arrival::backazimuthResidual() const {
	if ( _backazimuthResidual )
		return *_backazimuthResidual;
	throw Seiscomp::Core::ValueException("Arrival.backazimuthResidual is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setTimeUsed(const OPT(bool)& timeUsed) {
	_timeUsed = timeUsed;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Arrival::timeUsed() const {
	if ( _timeUsed )
		return *_timeUsed;
	throw Seiscomp::Core::ValueException("Arrival.timeUsed is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setHorizontalSlownessUsed(const OPT(bool)& horizontalSlownessUsed) {
	_horizontalSlownessUsed = horizontalSlownessUsed;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Arrival::horizontalSlownessUsed() const {
	if ( _horizontalSlownessUsed )
		return *_horizontalSlownessUsed;
	throw Seiscomp::Core::ValueException("Arrival.horizontalSlownessUsed is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setBackazimuthUsed(const OPT(bool)& backazimuthUsed) {
	_backazimuthUsed = backazimuthUsed;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Arrival::backazimuthUsed() const {
	if ( _backazimuthUsed )
		return *_backazimuthUsed;
	throw Seiscomp::Core::ValueException("Arrival.backazimuthUsed is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setWeight(const OPT(double)& weight) {
	_weight = weight;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Arrival::weight() const {
	if ( _weight )
		return *_weight;
	throw Seiscomp::Core::ValueException("Arrival.weight is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setEarthModelID(const std::string& earthModelID) {
	_earthModelID = earthModelID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Arrival::earthModelID() const {
	return _earthModelID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setPreliminary(const OPT(bool)& preliminary) {
	_preliminary = preliminary;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Arrival::preliminary() const {
	if ( _preliminary )
		return *_preliminary;
	throw Seiscomp::Core::ValueException("Arrival.preliminary is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setCreationInfo(const OPT(CreationInfo)& creationInfo) {
	_creationInfo = creationInfo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CreationInfo& Arrival::creationInfo() {
	if ( _creationInfo )
		return *_creationInfo;
	throw Seiscomp::Core::ValueException("Arrival.creationInfo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CreationInfo& Arrival::creationInfo() const {
	if ( _creationInfo )
		return *_creationInfo;
	throw Seiscomp::Core::ValueException("Arrival.creationInfo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ArrivalIndex& Arrival::index() const {
	return _index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Arrival::equalIndex(const Arrival* lhs) const {
	if ( lhs == NULL ) return false;
	return lhs->index() == index();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin* Arrival::origin() const {
	return static_cast<Origin*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Arrival& Arrival::operator=(const Arrival& other) {
	_index = other._index;
	_phase = other._phase;
	_timeCorrection = other._timeCorrection;
	_azimuth = other._azimuth;
	_distance = other._distance;
	_takeOffAngle = other._takeOffAngle;
	_timeResidual = other._timeResidual;
	_horizontalSlownessResidual = other._horizontalSlownessResidual;
	_backazimuthResidual = other._backazimuthResidual;
	_timeUsed = other._timeUsed;
	_horizontalSlownessUsed = other._horizontalSlownessUsed;
	_backazimuthUsed = other._backazimuthUsed;
	_weight = other._weight;
	_earthModelID = other._earthModelID;
	_preliminary = other._preliminary;
	_creationInfo = other._creationInfo;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Arrival::assign(Object* other) {
	Arrival* otherArrival = Arrival::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherArrival;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Arrival::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	Origin* origin = Origin::Cast(parent);
	if ( origin != NULL )
		return origin->add(this);

	SEISCOMP_ERROR("Arrival::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Arrival::detachFrom(PublicObject* object) {
	if ( object == NULL ) return false;

	// check all possible parents
	Origin* origin = Origin::Cast(object);
	if ( origin != NULL ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return origin->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Arrival* child = origin->arrival(index());
			if ( child != NULL )
				return origin->remove(child);
			else {
				SEISCOMP_DEBUG("Arrival::detachFrom(Origin): arrival has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("Arrival::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Arrival::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* Arrival::clone() const {
	Arrival* clonee = new Arrival();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::accept(Visitor* visitor) {
	visitor->visit(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,10>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: Arrival skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	ar & NAMED_OBJECT_HINT("pickID", _index.pickID, Archive::XML_ELEMENT | Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	ar & NAMED_OBJECT_HINT("phase", _phase, Archive::STATIC_TYPE | Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("timeCorrection", _timeCorrection, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("azimuth", _azimuth, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("distance", _distance, Archive::XML_ELEMENT);
	if ( ar.supportsVersion<0,6>() ) {
		ar & NAMED_OBJECT_HINT("takeOffAngle", _takeOffAngle, Archive::XML_ELEMENT);
	}
	ar & NAMED_OBJECT_HINT("timeResidual", _timeResidual, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("horizontalSlownessResidual", _horizontalSlownessResidual, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("backazimuthResidual", _backazimuthResidual, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("timeUsed", _timeUsed, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("horizontalSlownessUsed", _horizontalSlownessUsed, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("backazimuthUsed", _backazimuthUsed, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("weight", _weight, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("earthModelID", _earthModelID, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT("preliminary", _preliminary);
	ar & NAMED_OBJECT_HINT("creationInfo", _creationInfo, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
