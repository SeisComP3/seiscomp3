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
#include <seiscomp3/datamodel/momenttensorcomponentcontribution.h>
#include <seiscomp3/datamodel/momenttensorstationcontribution.h>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(MomentTensorComponentContribution, Object, "MomentTensorComponentContribution");


MomentTensorComponentContribution::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("phaseCode", "string", false, false, true, false, false, false, NULL, &MomentTensorComponentContribution::setPhaseCode, &MomentTensorComponentContribution::phaseCode));
	addProperty(Core::simpleProperty("component", "int", false, false, true, false, false, false, NULL, &MomentTensorComponentContribution::setComponent, &MomentTensorComponentContribution::component));
	addProperty(Core::simpleProperty("active", "boolean", false, false, false, false, false, false, NULL, &MomentTensorComponentContribution::setActive, &MomentTensorComponentContribution::active));
	addProperty(Core::simpleProperty("weight", "float", false, false, false, false, false, false, NULL, &MomentTensorComponentContribution::setWeight, &MomentTensorComponentContribution::weight));
	addProperty(Core::simpleProperty("timeShift", "float", false, false, false, false, false, false, NULL, &MomentTensorComponentContribution::setTimeShift, &MomentTensorComponentContribution::timeShift));
	addProperty(Core::simpleProperty("dataTimeWindow", "float", true, false, false, false, false, false, NULL, &MomentTensorComponentContribution::setDataTimeWindow, (const std::vector< double >& (MomentTensorComponentContribution::*)() const)&MomentTensorComponentContribution::dataTimeWindow));
	addProperty(Core::simpleProperty("misfit", "float", false, false, false, false, true, false, NULL, &MomentTensorComponentContribution::setMisfit, &MomentTensorComponentContribution::misfit));
	addProperty(Core::simpleProperty("snr", "float", false, false, false, false, true, false, NULL, &MomentTensorComponentContribution::setSnr, &MomentTensorComponentContribution::snr));
}


IMPLEMENT_METAOBJECT(MomentTensorComponentContribution)


MomentTensorComponentContributionIndex::MomentTensorComponentContributionIndex() {
	component = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorComponentContributionIndex::MomentTensorComponentContributionIndex(const std::string& phaseCode_,
                                                                               int component_) {
	phaseCode = phaseCode_;
	component = component_;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorComponentContributionIndex::MomentTensorComponentContributionIndex(const MomentTensorComponentContributionIndex& idx) {
	phaseCode = idx.phaseCode;
	component = idx.component;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorComponentContributionIndex::operator==(const MomentTensorComponentContributionIndex& idx) const {
	return phaseCode == idx.phaseCode &&
	       component == idx.component;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorComponentContributionIndex::operator!=(const MomentTensorComponentContributionIndex& idx) const {
	return !operator==(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorComponentContribution::MomentTensorComponentContribution() {
	_active = false;
	_weight = 0;
	_timeShift = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorComponentContribution::MomentTensorComponentContribution(const MomentTensorComponentContribution& other)
 : Object() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorComponentContribution::MomentTensorComponentContribution(const std::string& phaseCode)
{
	 _index.phaseCode = phaseCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorComponentContribution::MomentTensorComponentContribution(const std::string& phaseCode,
                                                                     int component,
                                                                     bool active,
                                                                     double weight,
                                                                     double timeShift,
                                                                     double dataTimeWindow,
                                                                     const OPT(double)& misfit,
                                                                     const OPT(double)& snr)
 : _active(active),
   _weight(weight),
   _timeShift(timeShift),
   _dataTimeWindow(dataTimeWindow),
   _misfit(misfit),
   _snr(snr) {
	_index.phaseCode = phaseCode;
	_index.component = component;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorComponentContribution::~MomentTensorComponentContribution() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorComponentContribution::operator==(const MomentTensorComponentContribution& rhs) const {
	if ( _index != rhs._index ) return false;
	if ( _active != rhs._active ) return false;
	if ( _weight != rhs._weight ) return false;
	if ( _timeShift != rhs._timeShift ) return false;
	if ( _dataTimeWindow != rhs._dataTimeWindow ) return false;
	if ( _misfit != rhs._misfit ) return false;
	if ( _snr != rhs._snr ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorComponentContribution::operator!=(const MomentTensorComponentContribution& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorComponentContribution::equal(const MomentTensorComponentContribution& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorComponentContribution::setPhaseCode(const std::string& phaseCode) {
	_index.phaseCode = phaseCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& MomentTensorComponentContribution::phaseCode() const {
	return _index.phaseCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorComponentContribution::setComponent(int component) {
	_index.component = component;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MomentTensorComponentContribution::component() const {
	return _index.component;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorComponentContribution::setActive(bool active) {
	_active = active;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorComponentContribution::active() const {
	return _active;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorComponentContribution::setWeight(double weight) {
	_weight = weight;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MomentTensorComponentContribution::weight() const {
	return _weight;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorComponentContribution::setTimeShift(double timeShift) {
	_timeShift = timeShift;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MomentTensorComponentContribution::timeShift() const {
	return _timeShift;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorComponentContribution::setDataTimeWindow(const std::vector< double > &dataTimeWindow) {
	_dataTimeWindow = dataTimeWindow;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::vector< double >& MomentTensorComponentContribution::dataTimeWindow() const {
	return _dataTimeWindow;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector< double >& MomentTensorComponentContribution::dataTimeWindow() {
	return _dataTimeWindow;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorComponentContribution::setMisfit(const OPT(double)& misfit) {
	_misfit = misfit;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MomentTensorComponentContribution::misfit() const {
	if ( _misfit )
		return *_misfit;
	throw Seiscomp::Core::ValueException("MomentTensorComponentContribution.misfit is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorComponentContribution::setSnr(const OPT(double)& snr) {
	_snr = snr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MomentTensorComponentContribution::snr() const {
	if ( _snr )
		return *_snr;
	throw Seiscomp::Core::ValueException("MomentTensorComponentContribution.snr is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const MomentTensorComponentContributionIndex& MomentTensorComponentContribution::index() const {
	return _index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorComponentContribution::equalIndex(const MomentTensorComponentContribution* lhs) const {
	if ( lhs == NULL ) return false;
	return lhs->index() == index();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorStationContribution* MomentTensorComponentContribution::momentTensorStationContribution() const {
	return static_cast<MomentTensorStationContribution*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorComponentContribution& MomentTensorComponentContribution::operator=(const MomentTensorComponentContribution& other) {
	_index = other._index;
	_active = other._active;
	_weight = other._weight;
	_timeShift = other._timeShift;
	_dataTimeWindow = other._dataTimeWindow;
	_misfit = other._misfit;
	_snr = other._snr;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorComponentContribution::assign(Object* other) {
	MomentTensorComponentContribution* otherMomentTensorComponentContribution = MomentTensorComponentContribution::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherMomentTensorComponentContribution;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorComponentContribution::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	MomentTensorStationContribution* momentTensorStationContribution = MomentTensorStationContribution::Cast(parent);
	if ( momentTensorStationContribution != NULL )
		return momentTensorStationContribution->add(this);

	SEISCOMP_ERROR("MomentTensorComponentContribution::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorComponentContribution::detachFrom(PublicObject* object) {
	if ( object == NULL ) return false;

	// check all possible parents
	MomentTensorStationContribution* momentTensorStationContribution = MomentTensorStationContribution::Cast(object);
	if ( momentTensorStationContribution != NULL ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return momentTensorStationContribution->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			MomentTensorComponentContribution* child = momentTensorStationContribution->momentTensorComponentContribution(index());
			if ( child != NULL )
				return momentTensorStationContribution->remove(child);
			else {
				SEISCOMP_DEBUG("MomentTensorComponentContribution::detachFrom(MomentTensorStationContribution): momentTensorComponentContribution has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("MomentTensorComponentContribution::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorComponentContribution::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* MomentTensorComponentContribution::clone() const {
	MomentTensorComponentContribution* clonee = new MomentTensorComponentContribution();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorComponentContribution::accept(Visitor* visitor) {
	visitor->visit(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorComponentContribution::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,9>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: MomentTensorComponentContribution skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	ar & NAMED_OBJECT_HINT("phaseCode", _index.phaseCode, Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	ar & NAMED_OBJECT_HINT("component", _index.component, Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	ar & NAMED_OBJECT_HINT("active", _active, Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("weight", _weight, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("timeShift", _timeShift, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("dataTimeWindow", _dataTimeWindow, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("misfit", _misfit, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("snr", _snr, Archive::XML_ELEMENT);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
