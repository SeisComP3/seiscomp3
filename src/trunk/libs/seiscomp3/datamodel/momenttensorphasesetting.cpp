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
#include <seiscomp3/datamodel/momenttensorphasesetting.h>
#include <seiscomp3/datamodel/momenttensor.h>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(MomentTensorPhaseSetting, Object, "MomentTensorPhaseSetting");


MomentTensorPhaseSetting::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("code", "string", false, false, true, false, false, false, NULL, &MomentTensorPhaseSetting::setCode, &MomentTensorPhaseSetting::code));
	addProperty(Core::simpleProperty("lowerPeriod", "float", false, false, false, false, false, false, NULL, &MomentTensorPhaseSetting::setLowerPeriod, &MomentTensorPhaseSetting::lowerPeriod));
	addProperty(Core::simpleProperty("upperPeriod", "float", false, false, false, false, false, false, NULL, &MomentTensorPhaseSetting::setUpperPeriod, &MomentTensorPhaseSetting::upperPeriod));
	addProperty(Core::simpleProperty("minimumSNR", "float", false, false, false, false, true, false, NULL, &MomentTensorPhaseSetting::setMinimumSNR, &MomentTensorPhaseSetting::minimumSNR));
	addProperty(Core::simpleProperty("maximumTimeShift", "float", false, false, false, false, true, false, NULL, &MomentTensorPhaseSetting::setMaximumTimeShift, &MomentTensorPhaseSetting::maximumTimeShift));
}


IMPLEMENT_METAOBJECT(MomentTensorPhaseSetting)


MomentTensorPhaseSettingIndex::MomentTensorPhaseSettingIndex() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorPhaseSettingIndex::MomentTensorPhaseSettingIndex(const std::string& code_) {
	code = code_;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorPhaseSettingIndex::MomentTensorPhaseSettingIndex(const MomentTensorPhaseSettingIndex& idx) {
	code = idx.code;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorPhaseSettingIndex::operator==(const MomentTensorPhaseSettingIndex& idx) const {
	return code == idx.code;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorPhaseSettingIndex::operator!=(const MomentTensorPhaseSettingIndex& idx) const {
	return !operator==(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorPhaseSetting::MomentTensorPhaseSetting() {
	_lowerPeriod = 0;
	_upperPeriod = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorPhaseSetting::MomentTensorPhaseSetting(const MomentTensorPhaseSetting& other)
 : Object() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorPhaseSetting::MomentTensorPhaseSetting(const std::string& code)
{
	 _index.code = code;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorPhaseSetting::MomentTensorPhaseSetting(const std::string& code,
                                                   double lowerPeriod,
                                                   double upperPeriod,
                                                   const OPT(double)& minimumSNR,
                                                   const OPT(double)& maximumTimeShift)
 : _lowerPeriod(lowerPeriod),
   _upperPeriod(upperPeriod),
   _minimumSNR(minimumSNR),
   _maximumTimeShift(maximumTimeShift) {
	_index.code = code;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorPhaseSetting::~MomentTensorPhaseSetting() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorPhaseSetting::operator==(const MomentTensorPhaseSetting& rhs) const {
	if ( _index != rhs._index ) return false;
	if ( _lowerPeriod != rhs._lowerPeriod ) return false;
	if ( _upperPeriod != rhs._upperPeriod ) return false;
	if ( _minimumSNR != rhs._minimumSNR ) return false;
	if ( _maximumTimeShift != rhs._maximumTimeShift ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorPhaseSetting::operator!=(const MomentTensorPhaseSetting& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorPhaseSetting::equal(const MomentTensorPhaseSetting& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorPhaseSetting::setCode(const std::string& code) {
	_index.code = code;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& MomentTensorPhaseSetting::code() const {
	return _index.code;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorPhaseSetting::setLowerPeriod(double lowerPeriod) {
	_lowerPeriod = lowerPeriod;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MomentTensorPhaseSetting::lowerPeriod() const {
	return _lowerPeriod;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorPhaseSetting::setUpperPeriod(double upperPeriod) {
	_upperPeriod = upperPeriod;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MomentTensorPhaseSetting::upperPeriod() const {
	return _upperPeriod;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorPhaseSetting::setMinimumSNR(const OPT(double)& minimumSNR) {
	_minimumSNR = minimumSNR;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MomentTensorPhaseSetting::minimumSNR() const throw(Seiscomp::Core::ValueException) {
	if ( _minimumSNR )
		return *_minimumSNR;
	throw Seiscomp::Core::ValueException("MomentTensorPhaseSetting.minimumSNR is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorPhaseSetting::setMaximumTimeShift(const OPT(double)& maximumTimeShift) {
	_maximumTimeShift = maximumTimeShift;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MomentTensorPhaseSetting::maximumTimeShift() const throw(Seiscomp::Core::ValueException) {
	if ( _maximumTimeShift )
		return *_maximumTimeShift;
	throw Seiscomp::Core::ValueException("MomentTensorPhaseSetting.maximumTimeShift is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const MomentTensorPhaseSettingIndex& MomentTensorPhaseSetting::index() const {
	return _index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorPhaseSetting::equalIndex(const MomentTensorPhaseSetting* lhs) const {
	if ( lhs == NULL ) return false;
	return lhs->index() == index();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensor* MomentTensorPhaseSetting::momentTensor() const {
	return static_cast<MomentTensor*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorPhaseSetting& MomentTensorPhaseSetting::operator=(const MomentTensorPhaseSetting& other) {
	_index = other._index;
	_lowerPeriod = other._lowerPeriod;
	_upperPeriod = other._upperPeriod;
	_minimumSNR = other._minimumSNR;
	_maximumTimeShift = other._maximumTimeShift;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorPhaseSetting::assign(Object* other) {
	MomentTensorPhaseSetting* otherMomentTensorPhaseSetting = MomentTensorPhaseSetting::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherMomentTensorPhaseSetting;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorPhaseSetting::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	MomentTensor* momentTensor = MomentTensor::Cast(parent);
	if ( momentTensor != NULL )
		return momentTensor->add(this);

	SEISCOMP_ERROR("MomentTensorPhaseSetting::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorPhaseSetting::detachFrom(PublicObject* object) {
	if ( object == NULL ) return false;

	// check all possible parents
	MomentTensor* momentTensor = MomentTensor::Cast(object);
	if ( momentTensor != NULL ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return momentTensor->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			MomentTensorPhaseSetting* child = momentTensor->momentTensorPhaseSetting(index());
			if ( child != NULL )
				return momentTensor->remove(child);
			else {
				SEISCOMP_DEBUG("MomentTensorPhaseSetting::detachFrom(MomentTensor): momentTensorPhaseSetting has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("MomentTensorPhaseSetting::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorPhaseSetting::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* MomentTensorPhaseSetting::clone() const {
	MomentTensorPhaseSetting* clonee = new MomentTensorPhaseSetting();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorPhaseSetting::accept(Visitor* visitor) {
	visitor->visit(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorPhaseSetting::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,7>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: MomentTensorPhaseSetting skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	ar & NAMED_OBJECT_HINT("code", _index.code, Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	ar & NAMED_OBJECT_HINT("lowerPeriod", _lowerPeriod, Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("upperPeriod", _upperPeriod, Archive::XML_MANDATORY);
	ar & NAMED_OBJECT("minimumSNR", _minimumSNR);
	ar & NAMED_OBJECT("maximumTimeShift", _maximumTimeShift);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
