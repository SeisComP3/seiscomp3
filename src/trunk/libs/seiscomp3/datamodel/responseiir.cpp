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
#include <seiscomp3/datamodel/responseiir.h>
#include <seiscomp3/datamodel/inventory.h>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(ResponseIIR, PublicObject, "ResponseIIR");


ResponseIIR::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("name", "string", false, false, true, false, false, false, NULL, &ResponseIIR::setName, &ResponseIIR::name));
	addProperty(Core::simpleProperty("type", "string", false, false, false, false, false, false, NULL, &ResponseIIR::setType, &ResponseIIR::type));
	addProperty(Core::simpleProperty("gain", "float", false, false, false, false, true, false, NULL, &ResponseIIR::setGain, &ResponseIIR::gain));
	addProperty(Core::simpleProperty("gainFrequency", "float", false, false, false, false, true, false, NULL, &ResponseIIR::setGainFrequency, &ResponseIIR::gainFrequency));
	addProperty(Core::simpleProperty("decimationFactor", "int", false, false, false, false, true, false, NULL, &ResponseIIR::setDecimationFactor, &ResponseIIR::decimationFactor));
	addProperty(Core::simpleProperty("delay", "float", false, false, false, false, true, false, NULL, &ResponseIIR::setDelay, &ResponseIIR::delay));
	addProperty(Core::simpleProperty("correction", "float", false, false, false, false, true, false, NULL, &ResponseIIR::setCorrection, &ResponseIIR::correction));
	addProperty(Core::simpleProperty("numberOfNumerators", "int", false, false, false, false, true, false, NULL, &ResponseIIR::setNumberOfNumerators, &ResponseIIR::numberOfNumerators));
	addProperty(Core::simpleProperty("numberOfDenominators", "int", false, false, false, false, true, false, NULL, &ResponseIIR::setNumberOfDenominators, &ResponseIIR::numberOfDenominators));
	addProperty(objectProperty<RealArray>("numerators", "RealArray", false, false, true, &ResponseIIR::setNumerators, &ResponseIIR::numerators));
	addProperty(objectProperty<RealArray>("denominators", "RealArray", false, false, true, &ResponseIIR::setDenominators, &ResponseIIR::denominators));
	addProperty(objectProperty<Blob>("remark", "Blob", false, false, true, &ResponseIIR::setRemark, &ResponseIIR::remark));
}


IMPLEMENT_METAOBJECT(ResponseIIR)


ResponseIIRIndex::ResponseIIRIndex() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseIIRIndex::ResponseIIRIndex(const std::string& name_) {
	name = name_;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseIIRIndex::ResponseIIRIndex(const ResponseIIRIndex& idx) {
	name = idx.name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseIIRIndex::operator==(const ResponseIIRIndex& idx) const {
	return name == idx.name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseIIRIndex::operator!=(const ResponseIIRIndex& idx) const {
	return !operator==(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseIIR::ResponseIIR() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseIIR::ResponseIIR(const ResponseIIR& other)
 : PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseIIR::ResponseIIR(const std::string& publicID)
 : PublicObject(publicID) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseIIR::~ResponseIIR() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseIIR* ResponseIIR::Create() {
	ResponseIIR* object = new ResponseIIR();
	return static_cast<ResponseIIR*>(GenerateId(object));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseIIR* ResponseIIR::Create(const std::string& publicID) {
	if ( PublicObject::IsRegistrationEnabled() && Find(publicID) != NULL ) {
		SEISCOMP_ERROR(
			"There exists already a PublicObject with Id '%s'",
			publicID.c_str()
		);
		return NULL;
	}

	return new ResponseIIR(publicID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseIIR* ResponseIIR::Find(const std::string& publicID) {
	return ResponseIIR::Cast(PublicObject::Find(publicID));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseIIR::operator==(const ResponseIIR& rhs) const {
	if ( _index != rhs._index ) return false;
	if ( _type != rhs._type ) return false;
	if ( _gain != rhs._gain ) return false;
	if ( _gainFrequency != rhs._gainFrequency ) return false;
	if ( _decimationFactor != rhs._decimationFactor ) return false;
	if ( _delay != rhs._delay ) return false;
	if ( _correction != rhs._correction ) return false;
	if ( _numberOfNumerators != rhs._numberOfNumerators ) return false;
	if ( _numberOfDenominators != rhs._numberOfDenominators ) return false;
	if ( _numerators != rhs._numerators ) return false;
	if ( _denominators != rhs._denominators ) return false;
	if ( _remark != rhs._remark ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseIIR::operator!=(const ResponseIIR& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseIIR::equal(const ResponseIIR& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseIIR::setName(const std::string& name) {
	_index.name = name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ResponseIIR::name() const {
	return _index.name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseIIR::setType(const std::string& type) {
	_type = type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ResponseIIR::type() const {
	return _type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseIIR::setGain(const OPT(double)& gain) {
	_gain = gain;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double ResponseIIR::gain() const {
	if ( _gain )
		return *_gain;
	throw Seiscomp::Core::ValueException("ResponseIIR.gain is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseIIR::setGainFrequency(const OPT(double)& gainFrequency) {
	_gainFrequency = gainFrequency;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double ResponseIIR::gainFrequency() const {
	if ( _gainFrequency )
		return *_gainFrequency;
	throw Seiscomp::Core::ValueException("ResponseIIR.gainFrequency is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseIIR::setDecimationFactor(const OPT(int)& decimationFactor) {
	_decimationFactor = decimationFactor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ResponseIIR::decimationFactor() const {
	if ( _decimationFactor )
		return *_decimationFactor;
	throw Seiscomp::Core::ValueException("ResponseIIR.decimationFactor is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseIIR::setDelay(const OPT(double)& delay) {
	_delay = delay;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double ResponseIIR::delay() const {
	if ( _delay )
		return *_delay;
	throw Seiscomp::Core::ValueException("ResponseIIR.delay is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseIIR::setCorrection(const OPT(double)& correction) {
	_correction = correction;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double ResponseIIR::correction() const {
	if ( _correction )
		return *_correction;
	throw Seiscomp::Core::ValueException("ResponseIIR.correction is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseIIR::setNumberOfNumerators(const OPT(int)& numberOfNumerators) {
	_numberOfNumerators = numberOfNumerators;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ResponseIIR::numberOfNumerators() const {
	if ( _numberOfNumerators )
		return *_numberOfNumerators;
	throw Seiscomp::Core::ValueException("ResponseIIR.numberOfNumerators is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseIIR::setNumberOfDenominators(const OPT(int)& numberOfDenominators) {
	_numberOfDenominators = numberOfDenominators;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ResponseIIR::numberOfDenominators() const {
	if ( _numberOfDenominators )
		return *_numberOfDenominators;
	throw Seiscomp::Core::ValueException("ResponseIIR.numberOfDenominators is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseIIR::setNumerators(const OPT(RealArray)& numerators) {
	_numerators = numerators;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealArray& ResponseIIR::numerators() {
	if ( _numerators )
		return *_numerators;
	throw Seiscomp::Core::ValueException("ResponseIIR.numerators is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealArray& ResponseIIR::numerators() const {
	if ( _numerators )
		return *_numerators;
	throw Seiscomp::Core::ValueException("ResponseIIR.numerators is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseIIR::setDenominators(const OPT(RealArray)& denominators) {
	_denominators = denominators;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealArray& ResponseIIR::denominators() {
	if ( _denominators )
		return *_denominators;
	throw Seiscomp::Core::ValueException("ResponseIIR.denominators is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealArray& ResponseIIR::denominators() const {
	if ( _denominators )
		return *_denominators;
	throw Seiscomp::Core::ValueException("ResponseIIR.denominators is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseIIR::setRemark(const OPT(Blob)& remark) {
	_remark = remark;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Blob& ResponseIIR::remark() {
	if ( _remark )
		return *_remark;
	throw Seiscomp::Core::ValueException("ResponseIIR.remark is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Blob& ResponseIIR::remark() const {
	if ( _remark )
		return *_remark;
	throw Seiscomp::Core::ValueException("ResponseIIR.remark is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ResponseIIRIndex& ResponseIIR::index() const {
	return _index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseIIR::equalIndex(const ResponseIIR* lhs) const {
	if ( lhs == NULL ) return false;
	return lhs->index() == index();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Inventory* ResponseIIR::inventory() const {
	return static_cast<Inventory*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseIIR& ResponseIIR::operator=(const ResponseIIR& other) {
	PublicObject::operator=(other);
	_index = other._index;
	_type = other._type;
	_gain = other._gain;
	_gainFrequency = other._gainFrequency;
	_decimationFactor = other._decimationFactor;
	_delay = other._delay;
	_correction = other._correction;
	_numberOfNumerators = other._numberOfNumerators;
	_numberOfDenominators = other._numberOfDenominators;
	_numerators = other._numerators;
	_denominators = other._denominators;
	_remark = other._remark;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseIIR::assign(Object* other) {
	ResponseIIR* otherResponseIIR = ResponseIIR::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherResponseIIR;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseIIR::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	Inventory* inventory = Inventory::Cast(parent);
	if ( inventory != NULL )
		return inventory->add(this);

	SEISCOMP_ERROR("ResponseIIR::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseIIR::detachFrom(PublicObject* object) {
	if ( object == NULL ) return false;

	// check all possible parents
	Inventory* inventory = Inventory::Cast(object);
	if ( inventory != NULL ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return inventory->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			ResponseIIR* child = inventory->findResponseIIR(publicID());
			if ( child != NULL )
				return inventory->remove(child);
			else {
				SEISCOMP_DEBUG("ResponseIIR::detachFrom(Inventory): responseIIR has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("ResponseIIR::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseIIR::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* ResponseIIR::clone() const {
	ResponseIIR* clonee = new ResponseIIR();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseIIR::updateChild(Object* child) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseIIR::accept(Visitor* visitor) {
	if ( visitor->traversal() == Visitor::TM_TOPDOWN )
		if ( !visitor->visit(this) )
			return;


	if ( visitor->traversal() == Visitor::TM_BOTTOMUP )
		visitor->visit(this);
	else
		visitor->finished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseIIR::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,10>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: ResponseIIR skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	PublicObject::serialize(ar);
	if ( !ar.success() ) return;

	ar & NAMED_OBJECT_HINT("name", _index.name, Archive::INDEX_ATTRIBUTE);
	ar & NAMED_OBJECT_HINT("type", _type, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("gain", _gain, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("gainFrequency", _gainFrequency, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("decimationFactor", _decimationFactor, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("delay", _delay, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("correction", _correction, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("numberOfNumerators", _numberOfNumerators, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("numberOfDenominators", _numberOfDenominators, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("numerators", _numerators, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("denominators", _denominators, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("remark", _remark, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
