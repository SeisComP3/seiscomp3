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
#include <seiscomp3/datamodel/strongmotion/record.h>
#include <seiscomp3/datamodel/strongmotion/strongmotionparameters.h>
#include <seiscomp3/datamodel/strongmotion/peakmotion.h>
#include <algorithm>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {
namespace StrongMotion {


IMPLEMENT_SC_CLASS_DERIVED(Record, PublicObject, "Record");


Record::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(objectProperty<CreationInfo>("creationInfo", "CreationInfo", false, false, true, &Record::setCreationInfo, &Record::creationInfo));
	addProperty(Core::simpleProperty("gainUnit", "string", false, false, false, false, false, false, NULL, &Record::setGainUnit, &Record::gainUnit));
	addProperty(Core::simpleProperty("duration", "float", false, false, false, false, true, false, NULL, &Record::setDuration, &Record::duration));
	addProperty(objectProperty<TimeQuantity>("startTime", "TimeQuantity", false, false, false, &Record::setStartTime, &Record::startTime));
	addProperty(objectProperty<Contact>("owner", "Contact", false, false, true, &Record::setOwner, &Record::owner));
	addProperty(Core::simpleProperty("resampleRateNumerator", "int", false, false, false, false, true, false, NULL, &Record::setResampleRateNumerator, &Record::resampleRateNumerator));
	addProperty(Core::simpleProperty("resampleRateDenominator", "int", false, false, false, false, true, false, NULL, &Record::setResampleRateDenominator, &Record::resampleRateDenominator));
	addProperty(objectProperty<WaveformStreamID>("waveformID", "WaveformStreamID", false, false, false, &Record::setWaveformID, &Record::waveformID));
	addProperty(objectProperty<FileResource>("waveformFile", "FileResource", false, false, true, &Record::setWaveformFile, &Record::waveformFile));
	addProperty(arrayClassProperty<SimpleFilterChainMember>("filter", "SimpleFilterChainMember", &Record::simpleFilterChainMemberCount, &Record::simpleFilterChainMember, static_cast<bool (Record::*)(SimpleFilterChainMember*)>(&Record::add), &Record::removeSimpleFilterChainMember, static_cast<bool (Record::*)(SimpleFilterChainMember*)>(&Record::remove)));
	addProperty(arrayClassProperty<PeakMotion>("peakMotion", "PeakMotion", &Record::peakMotionCount, &Record::peakMotion, static_cast<bool (Record::*)(PeakMotion*)>(&Record::add), &Record::removePeakMotion, static_cast<bool (Record::*)(PeakMotion*)>(&Record::remove)));
}


IMPLEMENT_METAOBJECT(Record)


Record::Record() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record::Record(const Record& other)
 : PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record::Record(const std::string& publicID)
 : PublicObject(publicID) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record::~Record() {
	std::for_each(_simpleFilterChainMembers.begin(), _simpleFilterChainMembers.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&SimpleFilterChainMember::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&SimpleFilterChainMemberPtr::get)));
	std::for_each(_peakMotions.begin(), _peakMotions.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&PeakMotion::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&PeakMotionPtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record* Record::Create() {
	Record* object = new Record();
	return static_cast<Record*>(GenerateId(object));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record* Record::Create(const std::string& publicID) {
	if ( Find(publicID) != NULL ) {
		SEISCOMP_ERROR(
			"There exists already a PublicObject with Id '%s'",
			publicID.c_str()
		);
		return NULL;
	}

	return new Record(publicID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record* Record::Find(const std::string& publicID) {
	return Record::Cast(PublicObject::Find(publicID));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Record::operator==(const Record& rhs) const {
	if ( _creationInfo != rhs._creationInfo ) return false;
	if ( _gainUnit != rhs._gainUnit ) return false;
	if ( _duration != rhs._duration ) return false;
	if ( _startTime != rhs._startTime ) return false;
	if ( _owner != rhs._owner ) return false;
	if ( _resampleRateNumerator != rhs._resampleRateNumerator ) return false;
	if ( _resampleRateDenominator != rhs._resampleRateDenominator ) return false;
	if ( _waveformID != rhs._waveformID ) return false;
	if ( _waveformFile != rhs._waveformFile ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Record::operator!=(const Record& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Record::equal(const Record& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Record::setCreationInfo(const OPT(CreationInfo)& creationInfo) {
	_creationInfo = creationInfo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CreationInfo& Record::creationInfo() {
	if ( _creationInfo )
		return *_creationInfo;
	throw Seiscomp::Core::ValueException("Record.creationInfo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CreationInfo& Record::creationInfo() const {
	if ( _creationInfo )
		return *_creationInfo;
	throw Seiscomp::Core::ValueException("Record.creationInfo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Record::setGainUnit(const std::string& gainUnit) {
	_gainUnit = gainUnit;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Record::gainUnit() const {
	return _gainUnit;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Record::setDuration(const OPT(double)& duration) {
	_duration = duration;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Record::duration() const {
	if ( _duration )
		return *_duration;
	throw Seiscomp::Core::ValueException("Record.duration is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Record::setStartTime(const TimeQuantity& startTime) {
	_startTime = startTime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeQuantity& Record::startTime() {
	return _startTime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const TimeQuantity& Record::startTime() const {
	return _startTime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Record::setOwner(const OPT(Contact)& owner) {
	_owner = owner;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Contact& Record::owner() {
	if ( _owner )
		return *_owner;
	throw Seiscomp::Core::ValueException("Record.owner is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Contact& Record::owner() const {
	if ( _owner )
		return *_owner;
	throw Seiscomp::Core::ValueException("Record.owner is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Record::setResampleRateNumerator(const OPT(int)& resampleRateNumerator) {
	_resampleRateNumerator = resampleRateNumerator;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Record::resampleRateNumerator() const {
	if ( _resampleRateNumerator )
		return *_resampleRateNumerator;
	throw Seiscomp::Core::ValueException("Record.resampleRateNumerator is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Record::setResampleRateDenominator(const OPT(int)& resampleRateDenominator) {
	_resampleRateDenominator = resampleRateDenominator;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Record::resampleRateDenominator() const {
	if ( _resampleRateDenominator )
		return *_resampleRateDenominator;
	throw Seiscomp::Core::ValueException("Record.resampleRateDenominator is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Record::setWaveformID(const WaveformStreamID& waveformID) {
	_waveformID = waveformID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WaveformStreamID& Record::waveformID() {
	return _waveformID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const WaveformStreamID& Record::waveformID() const {
	return _waveformID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Record::setWaveformFile(const OPT(FileResource)& waveformFile) {
	_waveformFile = waveformFile;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FileResource& Record::waveformFile() {
	if ( _waveformFile )
		return *_waveformFile;
	throw Seiscomp::Core::ValueException("Record.waveformFile is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const FileResource& Record::waveformFile() const {
	if ( _waveformFile )
		return *_waveformFile;
	throw Seiscomp::Core::ValueException("Record.waveformFile is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StrongMotionParameters* Record::strongMotionParameters() const {
	return static_cast<StrongMotionParameters*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record& Record::operator=(const Record& other) {
	PublicObject::operator=(other);
	_creationInfo = other._creationInfo;
	_gainUnit = other._gainUnit;
	_duration = other._duration;
	_startTime = other._startTime;
	_owner = other._owner;
	_resampleRateNumerator = other._resampleRateNumerator;
	_resampleRateDenominator = other._resampleRateDenominator;
	_waveformID = other._waveformID;
	_waveformFile = other._waveformFile;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Record::assign(Object* other) {
	Record* otherRecord = Record::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherRecord;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Record::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	StrongMotionParameters* strongMotionParameters = StrongMotionParameters::Cast(parent);
	if ( strongMotionParameters != NULL )
		return strongMotionParameters->add(this);

	SEISCOMP_ERROR("Record::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Record::detachFrom(PublicObject* object) {
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
			Record* child = strongMotionParameters->findRecord(publicID());
			if ( child != NULL )
				return strongMotionParameters->remove(child);
			else {
				SEISCOMP_DEBUG("Record::detachFrom(StrongMotionParameters): record has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("Record::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Record::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* Record::clone() const {
	Record* clonee = new Record();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Record::updateChild(Object* child) {
	SimpleFilterChainMember* simpleFilterChainMemberChild = SimpleFilterChainMember::Cast(child);
	if ( simpleFilterChainMemberChild != NULL ) {
		SimpleFilterChainMember* simpleFilterChainMemberElement = simpleFilterChainMember(simpleFilterChainMemberChild->index());
		if ( simpleFilterChainMemberElement != NULL ) {
			*simpleFilterChainMemberElement = *simpleFilterChainMemberChild;
			return true;
		}
		return false;
	}

	// Do not know how to fetch child of type PeakMotion without an index

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Record::accept(Visitor* visitor) {
	if ( visitor->traversal() == Visitor::TM_TOPDOWN )
		if ( !visitor->visit(this) )
			return;

	for ( std::vector<SimpleFilterChainMemberPtr>::iterator it = _simpleFilterChainMembers.begin(); it != _simpleFilterChainMembers.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<PeakMotionPtr>::iterator it = _peakMotions.begin(); it != _peakMotions.end(); ++it )
		(*it)->accept(visitor);

	if ( visitor->traversal() == Visitor::TM_BOTTOMUP )
		visitor->visit(this);
	else
		visitor->finished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Record::simpleFilterChainMemberCount() const {
	return _simpleFilterChainMembers.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SimpleFilterChainMember* Record::simpleFilterChainMember(size_t i) const {
	return _simpleFilterChainMembers[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SimpleFilterChainMember* Record::simpleFilterChainMember(const SimpleFilterChainMemberIndex& i) const {
	for ( std::vector<SimpleFilterChainMemberPtr>::const_iterator it = _simpleFilterChainMembers.begin(); it != _simpleFilterChainMembers.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Record::add(SimpleFilterChainMember* simpleFilterChainMember) {
	if ( simpleFilterChainMember == NULL )
		return false;

	// Element has already a parent
	if ( simpleFilterChainMember->parent() != NULL ) {
		SEISCOMP_ERROR("Record::add(SimpleFilterChainMember*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<SimpleFilterChainMemberPtr>::iterator it = _simpleFilterChainMembers.begin(); it != _simpleFilterChainMembers.end(); ++it ) {
		if ( (*it)->index() == simpleFilterChainMember->index() ) {
			SEISCOMP_ERROR("Record::add(SimpleFilterChainMember*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_simpleFilterChainMembers.push_back(simpleFilterChainMember);
	simpleFilterChainMember->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		simpleFilterChainMember->accept(&nc);
	}

	// Notify registered observers
	childAdded(simpleFilterChainMember);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Record::remove(SimpleFilterChainMember* simpleFilterChainMember) {
	if ( simpleFilterChainMember == NULL )
		return false;

	if ( simpleFilterChainMember->parent() != this ) {
		SEISCOMP_ERROR("Record::remove(SimpleFilterChainMember*) -> element has another parent");
		return false;
	}

	std::vector<SimpleFilterChainMemberPtr>::iterator it;
	it = std::find(_simpleFilterChainMembers.begin(), _simpleFilterChainMembers.end(), simpleFilterChainMember);
	// Element has not been found
	if ( it == _simpleFilterChainMembers.end() ) {
		SEISCOMP_ERROR("Record::remove(SimpleFilterChainMember*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_simpleFilterChainMembers.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Record::removeSimpleFilterChainMember(size_t i) {
	// index out of bounds
	if ( i >= _simpleFilterChainMembers.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_simpleFilterChainMembers[i]->accept(&nc);
	}

	_simpleFilterChainMembers[i]->setParent(NULL);
	childRemoved(_simpleFilterChainMembers[i].get());
	
	_simpleFilterChainMembers.erase(_simpleFilterChainMembers.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Record::removeSimpleFilterChainMember(const SimpleFilterChainMemberIndex& i) {
	SimpleFilterChainMember* object = simpleFilterChainMember(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Record::peakMotionCount() const {
	return _peakMotions.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PeakMotion* Record::peakMotion(size_t i) const {
	return _peakMotions[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PeakMotion* Record::findPeakMotion(PeakMotion* peakMotion) const {
	std::vector<PeakMotionPtr>::const_iterator it;
	for ( it = _peakMotions.begin(); it != _peakMotions.end(); ++it ) {
		if ( *peakMotion == **it )
			return (*it).get();
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Record::add(PeakMotion* peakMotion) {
	if ( peakMotion == NULL )
		return false;

	// Element has already a parent
	if ( peakMotion->parent() != NULL ) {
		SEISCOMP_ERROR("Record::add(PeakMotion*) -> element has already a parent");
		return false;
	}

	// Add the element
	_peakMotions.push_back(peakMotion);
	peakMotion->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		peakMotion->accept(&nc);
	}

	// Notify registered observers
	childAdded(peakMotion);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Record::remove(PeakMotion* peakMotion) {
	if ( peakMotion == NULL )
		return false;

	if ( peakMotion->parent() != this ) {
		SEISCOMP_ERROR("Record::remove(PeakMotion*) -> element has another parent");
		return false;
	}

	std::vector<PeakMotionPtr>::iterator it;
	it = std::find(_peakMotions.begin(), _peakMotions.end(), peakMotion);
	// Element has not been found
	if ( it == _peakMotions.end() ) {
		SEISCOMP_ERROR("Record::remove(PeakMotion*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_peakMotions.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Record::removePeakMotion(size_t i) {
	// index out of bounds
	if ( i >= _peakMotions.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_peakMotions[i]->accept(&nc);
	}

	_peakMotions[i]->setParent(NULL);
	childRemoved(_peakMotions[i].get());
	
	_peakMotions.erase(_peakMotions.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Record::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,11>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: Record skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	PublicObject::serialize(ar);
	if ( !ar.success() ) return;

	ar & NAMED_OBJECT_HINT("creationInfo", _creationInfo, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("gainUnit", _gainUnit, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("duration", _duration, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("startTime", _startTime, Archive::STATIC_TYPE | Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("owner", _owner, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("resampleRateNumerator", _resampleRateNumerator, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("resampleRateDenominator", _resampleRateDenominator, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("waveformID", _waveformID, Archive::STATIC_TYPE | Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("waveformFile", _waveformFile, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("filter",
	                       Seiscomp::Core::Generic::containerMember(_simpleFilterChainMembers,
	                       Seiscomp::Core::Generic::bindMemberFunction<SimpleFilterChainMember>(static_cast<bool (Record::*)(SimpleFilterChainMember*)>(&Record::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("peakMotion",
	                       Seiscomp::Core::Generic::containerMember(_peakMotions,
	                       Seiscomp::Core::Generic::bindMemberFunction<PeakMotion>(static_cast<bool (Record::*)(PeakMotion*)>(&Record::add), this)),
	                       Archive::STATIC_TYPE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
