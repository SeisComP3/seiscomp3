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
#include <seiscomp3/datamodel/responsefir.h>
#include <seiscomp3/datamodel/inventory.h>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(ResponseFIR, PublicObject, "ResponseFIR");


ResponseFIR::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("name", "string", false, false, true, false, false, false, NULL, &ResponseFIR::setName, &ResponseFIR::name));
	addProperty(Core::simpleProperty("gain", "float", false, false, false, false, true, false, NULL, &ResponseFIR::setGain, &ResponseFIR::gain));
	addProperty(Core::simpleProperty("gainFrequency", "float", false, false, false, false, true, false, NULL, &ResponseFIR::setGainFrequency, &ResponseFIR::gainFrequency));
	addProperty(Core::simpleProperty("decimationFactor", "int", false, false, false, false, true, false, NULL, &ResponseFIR::setDecimationFactor, &ResponseFIR::decimationFactor));
	addProperty(Core::simpleProperty("delay", "float", false, false, false, false, true, false, NULL, &ResponseFIR::setDelay, &ResponseFIR::delay));
	addProperty(Core::simpleProperty("correction", "float", false, false, false, false, true, false, NULL, &ResponseFIR::setCorrection, &ResponseFIR::correction));
	addProperty(Core::simpleProperty("numberOfCoefficients", "int", false, false, false, false, true, false, NULL, &ResponseFIR::setNumberOfCoefficients, &ResponseFIR::numberOfCoefficients));
	addProperty(Core::simpleProperty("symmetry", "string", false, false, false, false, false, false, NULL, &ResponseFIR::setSymmetry, &ResponseFIR::symmetry));
	addProperty(objectProperty<RealArray>("coefficients", "RealArray", false, false, true, &ResponseFIR::setCoefficients, &ResponseFIR::coefficients));
	addProperty(objectProperty<Blob>("remark", "Blob", false, false, true, &ResponseFIR::setRemark, &ResponseFIR::remark));
}


IMPLEMENT_METAOBJECT(ResponseFIR)


ResponseFIRIndex::ResponseFIRIndex() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFIRIndex::ResponseFIRIndex(const std::string& name_) {
	name = name_;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFIRIndex::ResponseFIRIndex(const ResponseFIRIndex& idx) {
	name = idx.name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseFIRIndex::operator==(const ResponseFIRIndex& idx) const {
	return name == idx.name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseFIRIndex::operator!=(const ResponseFIRIndex& idx) const {
	return !operator==(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFIR::ResponseFIR() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFIR::ResponseFIR(const ResponseFIR& other)
 : PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFIR::ResponseFIR(const std::string& publicID)
 : PublicObject(publicID) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFIR::~ResponseFIR() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFIR* ResponseFIR::Create() {
	ResponseFIR* object = new ResponseFIR();
	return static_cast<ResponseFIR*>(GenerateId(object));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFIR* ResponseFIR::Create(const std::string& publicID) {
	if ( PublicObject::IsRegistrationEnabled() && Find(publicID) != NULL ) {
		SEISCOMP_ERROR(
			"There exists already a PublicObject with Id '%s'",
			publicID.c_str()
		);
		return NULL;
	}

	return new ResponseFIR(publicID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFIR* ResponseFIR::Find(const std::string& publicID) {
	return ResponseFIR::Cast(PublicObject::Find(publicID));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseFIR::operator==(const ResponseFIR& rhs) const {
	if ( _index != rhs._index ) return false;
	if ( _gain != rhs._gain ) return false;
	if ( _gainFrequency != rhs._gainFrequency ) return false;
	if ( _decimationFactor != rhs._decimationFactor ) return false;
	if ( _delay != rhs._delay ) return false;
	if ( _correction != rhs._correction ) return false;
	if ( _numberOfCoefficients != rhs._numberOfCoefficients ) return false;
	if ( _symmetry != rhs._symmetry ) return false;
	if ( _coefficients != rhs._coefficients ) return false;
	if ( _remark != rhs._remark ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseFIR::operator!=(const ResponseFIR& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseFIR::equal(const ResponseFIR& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseFIR::setName(const std::string& name) {
	_index.name = name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ResponseFIR::name() const {
	return _index.name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseFIR::setGain(const OPT(double)& gain) {
	_gain = gain;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double ResponseFIR::gain() const {
	if ( _gain )
		return *_gain;
	throw Seiscomp::Core::ValueException("ResponseFIR.gain is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseFIR::setGainFrequency(const OPT(double)& gainFrequency) {
	_gainFrequency = gainFrequency;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double ResponseFIR::gainFrequency() const {
	if ( _gainFrequency )
		return *_gainFrequency;
	throw Seiscomp::Core::ValueException("ResponseFIR.gainFrequency is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseFIR::setDecimationFactor(const OPT(int)& decimationFactor) {
	_decimationFactor = decimationFactor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ResponseFIR::decimationFactor() const {
	if ( _decimationFactor )
		return *_decimationFactor;
	throw Seiscomp::Core::ValueException("ResponseFIR.decimationFactor is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseFIR::setDelay(const OPT(double)& delay) {
	_delay = delay;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double ResponseFIR::delay() const {
	if ( _delay )
		return *_delay;
	throw Seiscomp::Core::ValueException("ResponseFIR.delay is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseFIR::setCorrection(const OPT(double)& correction) {
	_correction = correction;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double ResponseFIR::correction() const {
	if ( _correction )
		return *_correction;
	throw Seiscomp::Core::ValueException("ResponseFIR.correction is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseFIR::setNumberOfCoefficients(const OPT(int)& numberOfCoefficients) {
	_numberOfCoefficients = numberOfCoefficients;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ResponseFIR::numberOfCoefficients() const {
	if ( _numberOfCoefficients )
		return *_numberOfCoefficients;
	throw Seiscomp::Core::ValueException("ResponseFIR.numberOfCoefficients is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseFIR::setSymmetry(const std::string& symmetry) {
	_symmetry = symmetry;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ResponseFIR::symmetry() const {
	return _symmetry;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseFIR::setCoefficients(const OPT(RealArray)& coefficients) {
	_coefficients = coefficients;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealArray& ResponseFIR::coefficients() {
	if ( _coefficients )
		return *_coefficients;
	throw Seiscomp::Core::ValueException("ResponseFIR.coefficients is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealArray& ResponseFIR::coefficients() const {
	if ( _coefficients )
		return *_coefficients;
	throw Seiscomp::Core::ValueException("ResponseFIR.coefficients is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseFIR::setRemark(const OPT(Blob)& remark) {
	_remark = remark;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Blob& ResponseFIR::remark() {
	if ( _remark )
		return *_remark;
	throw Seiscomp::Core::ValueException("ResponseFIR.remark is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Blob& ResponseFIR::remark() const {
	if ( _remark )
		return *_remark;
	throw Seiscomp::Core::ValueException("ResponseFIR.remark is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ResponseFIRIndex& ResponseFIR::index() const {
	return _index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseFIR::equalIndex(const ResponseFIR* lhs) const {
	if ( lhs == NULL ) return false;
	return lhs->index() == index();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Inventory* ResponseFIR::inventory() const {
	return static_cast<Inventory*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFIR& ResponseFIR::operator=(const ResponseFIR& other) {
	PublicObject::operator=(other);
	_index = other._index;
	_gain = other._gain;
	_gainFrequency = other._gainFrequency;
	_decimationFactor = other._decimationFactor;
	_delay = other._delay;
	_correction = other._correction;
	_numberOfCoefficients = other._numberOfCoefficients;
	_symmetry = other._symmetry;
	_coefficients = other._coefficients;
	_remark = other._remark;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseFIR::assign(Object* other) {
	ResponseFIR* otherResponseFIR = ResponseFIR::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherResponseFIR;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseFIR::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	Inventory* inventory = Inventory::Cast(parent);
	if ( inventory != NULL )
		return inventory->add(this);

	SEISCOMP_ERROR("ResponseFIR::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseFIR::detachFrom(PublicObject* object) {
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
			ResponseFIR* child = inventory->findResponseFIR(publicID());
			if ( child != NULL )
				return inventory->remove(child);
			else {
				SEISCOMP_DEBUG("ResponseFIR::detachFrom(Inventory): responseFIR has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("ResponseFIR::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseFIR::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* ResponseFIR::clone() const {
	ResponseFIR* clonee = new ResponseFIR();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseFIR::updateChild(Object* child) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseFIR::accept(Visitor* visitor) {
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
void ResponseFIR::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,10>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: ResponseFIR skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	PublicObject::serialize(ar);
	if ( !ar.success() ) return;

	ar & NAMED_OBJECT_HINT("name", _index.name, Archive::INDEX_ATTRIBUTE);
	ar & NAMED_OBJECT_HINT("gain", _gain, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("gainFrequency", _gainFrequency, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("decimationFactor", _decimationFactor, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("delay", _delay, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("correction", _correction, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("numberOfCoefficients", _numberOfCoefficients, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("symmetry", _symmetry, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("coefficients", _coefficients, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("remark", _remark, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
