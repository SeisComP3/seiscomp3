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
#include <seiscomp3/datamodel/momenttensorstationcontribution.h>
#include <seiscomp3/datamodel/momenttensor.h>
#include <algorithm>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(MomentTensorStationContribution, PublicObject, "MomentTensorStationContribution");


MomentTensorStationContribution::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("active", "boolean", false, false, false, false, false, false, NULL, &MomentTensorStationContribution::setActive, &MomentTensorStationContribution::active));
	addProperty(objectProperty<WaveformStreamID>("waveformID", "WaveformStreamID", false, false, true, &MomentTensorStationContribution::setWaveformID, &MomentTensorStationContribution::waveformID));
	addProperty(Core::simpleProperty("weight", "float", false, false, false, false, true, false, NULL, &MomentTensorStationContribution::setWeight, &MomentTensorStationContribution::weight));
	addProperty(Core::simpleProperty("timeShift", "float", false, false, false, false, true, false, NULL, &MomentTensorStationContribution::setTimeShift, &MomentTensorStationContribution::timeShift));
	addProperty(arrayClassProperty<MomentTensorComponentContribution>("component", "MomentTensorComponentContribution", &MomentTensorStationContribution::momentTensorComponentContributionCount, &MomentTensorStationContribution::momentTensorComponentContribution, static_cast<bool (MomentTensorStationContribution::*)(MomentTensorComponentContribution*)>(&MomentTensorStationContribution::add), &MomentTensorStationContribution::removeMomentTensorComponentContribution, static_cast<bool (MomentTensorStationContribution::*)(MomentTensorComponentContribution*)>(&MomentTensorStationContribution::remove)));
}


IMPLEMENT_METAOBJECT(MomentTensorStationContribution)


MomentTensorStationContribution::MomentTensorStationContribution() {
	_active = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorStationContribution::MomentTensorStationContribution(const MomentTensorStationContribution& other)
 : PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorStationContribution::MomentTensorStationContribution(const std::string& publicID)
 : PublicObject(publicID) {
	_active = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorStationContribution::~MomentTensorStationContribution() {
	std::for_each(_momentTensorComponentContributions.begin(), _momentTensorComponentContributions.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&MomentTensorComponentContribution::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&MomentTensorComponentContributionPtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorStationContribution* MomentTensorStationContribution::Create() {
	MomentTensorStationContribution* object = new MomentTensorStationContribution();
	return static_cast<MomentTensorStationContribution*>(GenerateId(object));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorStationContribution* MomentTensorStationContribution::Create(const std::string& publicID) {
	if ( PublicObject::IsRegistrationEnabled() && Find(publicID) != NULL ) {
		SEISCOMP_ERROR(
			"There exists already a PublicObject with Id '%s'",
			publicID.c_str()
		);
		return NULL;
	}

	return new MomentTensorStationContribution(publicID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorStationContribution* MomentTensorStationContribution::Find(const std::string& publicID) {
	return MomentTensorStationContribution::Cast(PublicObject::Find(publicID));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorStationContribution::operator==(const MomentTensorStationContribution& rhs) const {
	if ( _active != rhs._active ) return false;
	if ( _waveformID != rhs._waveformID ) return false;
	if ( _weight != rhs._weight ) return false;
	if ( _timeShift != rhs._timeShift ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorStationContribution::operator!=(const MomentTensorStationContribution& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorStationContribution::equal(const MomentTensorStationContribution& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorStationContribution::setActive(bool active) {
	_active = active;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorStationContribution::active() const {
	return _active;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorStationContribution::setWaveformID(const OPT(WaveformStreamID)& waveformID) {
	_waveformID = waveformID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WaveformStreamID& MomentTensorStationContribution::waveformID() throw(Seiscomp::Core::ValueException) {
	if ( _waveformID )
		return *_waveformID;
	throw Seiscomp::Core::ValueException("MomentTensorStationContribution.waveformID is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const WaveformStreamID& MomentTensorStationContribution::waveformID() const throw(Seiscomp::Core::ValueException) {
	if ( _waveformID )
		return *_waveformID;
	throw Seiscomp::Core::ValueException("MomentTensorStationContribution.waveformID is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorStationContribution::setWeight(const OPT(double)& weight) {
	_weight = weight;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MomentTensorStationContribution::weight() const throw(Seiscomp::Core::ValueException) {
	if ( _weight )
		return *_weight;
	throw Seiscomp::Core::ValueException("MomentTensorStationContribution.weight is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorStationContribution::setTimeShift(const OPT(double)& timeShift) {
	_timeShift = timeShift;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MomentTensorStationContribution::timeShift() const throw(Seiscomp::Core::ValueException) {
	if ( _timeShift )
		return *_timeShift;
	throw Seiscomp::Core::ValueException("MomentTensorStationContribution.timeShift is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensor* MomentTensorStationContribution::momentTensor() const {
	return static_cast<MomentTensor*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorStationContribution& MomentTensorStationContribution::operator=(const MomentTensorStationContribution& other) {
	PublicObject::operator=(other);
	_active = other._active;
	_waveformID = other._waveformID;
	_weight = other._weight;
	_timeShift = other._timeShift;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorStationContribution::assign(Object* other) {
	MomentTensorStationContribution* otherMomentTensorStationContribution = MomentTensorStationContribution::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherMomentTensorStationContribution;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorStationContribution::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	MomentTensor* momentTensor = MomentTensor::Cast(parent);
	if ( momentTensor != NULL )
		return momentTensor->add(this);

	SEISCOMP_ERROR("MomentTensorStationContribution::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorStationContribution::detachFrom(PublicObject* object) {
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
			MomentTensorStationContribution* child = momentTensor->findMomentTensorStationContribution(publicID());
			if ( child != NULL )
				return momentTensor->remove(child);
			else {
				SEISCOMP_DEBUG("MomentTensorStationContribution::detachFrom(MomentTensor): momentTensorStationContribution has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("MomentTensorStationContribution::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorStationContribution::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* MomentTensorStationContribution::clone() const {
	MomentTensorStationContribution* clonee = new MomentTensorStationContribution();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorStationContribution::updateChild(Object* child) {
	MomentTensorComponentContribution* momentTensorComponentContributionChild = MomentTensorComponentContribution::Cast(child);
	if ( momentTensorComponentContributionChild != NULL ) {
		MomentTensorComponentContribution* momentTensorComponentContributionElement = momentTensorComponentContribution(momentTensorComponentContributionChild->index());
		if ( momentTensorComponentContributionElement != NULL ) {
			*momentTensorComponentContributionElement = *momentTensorComponentContributionChild;
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorStationContribution::accept(Visitor* visitor) {
	if ( visitor->traversal() == Visitor::TM_TOPDOWN )
		if ( !visitor->visit(this) )
			return;

	for ( std::vector<MomentTensorComponentContributionPtr>::iterator it = _momentTensorComponentContributions.begin(); it != _momentTensorComponentContributions.end(); ++it )
		(*it)->accept(visitor);

	if ( visitor->traversal() == Visitor::TM_BOTTOMUP )
		visitor->visit(this);
	else
		visitor->finished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t MomentTensorStationContribution::momentTensorComponentContributionCount() const {
	return _momentTensorComponentContributions.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorComponentContribution* MomentTensorStationContribution::momentTensorComponentContribution(size_t i) const {
	return _momentTensorComponentContributions[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorComponentContribution* MomentTensorStationContribution::momentTensorComponentContribution(const MomentTensorComponentContributionIndex& i) const {
	for ( std::vector<MomentTensorComponentContributionPtr>::const_iterator it = _momentTensorComponentContributions.begin(); it != _momentTensorComponentContributions.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorStationContribution::add(MomentTensorComponentContribution* momentTensorComponentContribution) {
	if ( momentTensorComponentContribution == NULL )
		return false;

	// Element has already a parent
	if ( momentTensorComponentContribution->parent() != NULL ) {
		SEISCOMP_ERROR("MomentTensorStationContribution::add(MomentTensorComponentContribution*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<MomentTensorComponentContributionPtr>::iterator it = _momentTensorComponentContributions.begin(); it != _momentTensorComponentContributions.end(); ++it ) {
		if ( (*it)->index() == momentTensorComponentContribution->index() ) {
			SEISCOMP_ERROR("MomentTensorStationContribution::add(MomentTensorComponentContribution*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_momentTensorComponentContributions.push_back(momentTensorComponentContribution);
	momentTensorComponentContribution->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		momentTensorComponentContribution->accept(&nc);
	}

	// Notify registered observers
	childAdded(momentTensorComponentContribution);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorStationContribution::remove(MomentTensorComponentContribution* momentTensorComponentContribution) {
	if ( momentTensorComponentContribution == NULL )
		return false;

	if ( momentTensorComponentContribution->parent() != this ) {
		SEISCOMP_ERROR("MomentTensorStationContribution::remove(MomentTensorComponentContribution*) -> element has another parent");
		return false;
	}

	std::vector<MomentTensorComponentContributionPtr>::iterator it;
	it = std::find(_momentTensorComponentContributions.begin(), _momentTensorComponentContributions.end(), momentTensorComponentContribution);
	// Element has not been found
	if ( it == _momentTensorComponentContributions.end() ) {
		SEISCOMP_ERROR("MomentTensorStationContribution::remove(MomentTensorComponentContribution*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_momentTensorComponentContributions.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorStationContribution::removeMomentTensorComponentContribution(size_t i) {
	// index out of bounds
	if ( i >= _momentTensorComponentContributions.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_momentTensorComponentContributions[i]->accept(&nc);
	}

	_momentTensorComponentContributions[i]->setParent(NULL);
	childRemoved(_momentTensorComponentContributions[i].get());
	
	_momentTensorComponentContributions.erase(_momentTensorComponentContributions.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensorStationContribution::removeMomentTensorComponentContribution(const MomentTensorComponentContributionIndex& i) {
	MomentTensorComponentContribution* object = momentTensorComponentContribution(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensorStationContribution::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,8>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: MomentTensorStationContribution skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	PublicObject::serialize(ar);
	if ( !ar.success() ) return;

	ar & NAMED_OBJECT_HINT("active", _active, Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("waveformID", _waveformID, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT("weight", _weight);
	ar & NAMED_OBJECT("timeShift", _timeShift);
	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("component",
	                       Seiscomp::Core::Generic::containerMember(_momentTensorComponentContributions,
	                       Seiscomp::Core::Generic::bindMemberFunction<MomentTensorComponentContribution>(static_cast<bool (MomentTensorStationContribution::*)(MomentTensorComponentContribution*)>(&MomentTensorStationContribution::add), this)),
	                       Archive::STATIC_TYPE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
