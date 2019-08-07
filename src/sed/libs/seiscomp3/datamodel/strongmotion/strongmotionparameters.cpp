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
#include <seiscomp3/datamodel/strongmotion/strongmotionparameters.h>
#include <seiscomp3/datamodel/strongmotion/simplefilter.h>
#include <seiscomp3/datamodel/strongmotion/record.h>
#include <seiscomp3/datamodel/strongmotion/strongorigindescription.h>
#include <algorithm>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {
namespace StrongMotion {


IMPLEMENT_SC_CLASS_DERIVED(StrongMotionParameters, PublicObject, "StrongMotionParameters");


StrongMotionParameters::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(arrayObjectProperty("simpleFilter", "SimpleFilter", &StrongMotionParameters::simpleFilterCount, &StrongMotionParameters::simpleFilter, static_cast<bool (StrongMotionParameters::*)(SimpleFilter*)>(&StrongMotionParameters::add), &StrongMotionParameters::removeSimpleFilter, static_cast<bool (StrongMotionParameters::*)(SimpleFilter*)>(&StrongMotionParameters::remove)));
	addProperty(arrayObjectProperty("record", "Record", &StrongMotionParameters::recordCount, &StrongMotionParameters::record, static_cast<bool (StrongMotionParameters::*)(Record*)>(&StrongMotionParameters::add), &StrongMotionParameters::removeRecord, static_cast<bool (StrongMotionParameters::*)(Record*)>(&StrongMotionParameters::remove)));
	addProperty(arrayObjectProperty("strongOriginDescription", "StrongOriginDescription", &StrongMotionParameters::strongOriginDescriptionCount, &StrongMotionParameters::strongOriginDescription, static_cast<bool (StrongMotionParameters::*)(StrongOriginDescription*)>(&StrongMotionParameters::add), &StrongMotionParameters::removeStrongOriginDescription, static_cast<bool (StrongMotionParameters::*)(StrongOriginDescription*)>(&StrongMotionParameters::remove)));
}


IMPLEMENT_METAOBJECT(StrongMotionParameters)


StrongMotionParameters::StrongMotionParameters() : PublicObject("StrongMotionParameters") {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StrongMotionParameters::StrongMotionParameters(const StrongMotionParameters& other)
 : PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StrongMotionParameters::~StrongMotionParameters() {
	std::for_each(_simpleFilters.begin(), _simpleFilters.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&SimpleFilter::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&SimpleFilterPtr::get)));
	std::for_each(_records.begin(), _records.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Record::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&RecordPtr::get)));
	std::for_each(_strongOriginDescriptions.begin(), _strongOriginDescriptions.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&StrongOriginDescription::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&StrongOriginDescriptionPtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongMotionParameters::operator==(const StrongMotionParameters& rhs) const {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongMotionParameters::operator!=(const StrongMotionParameters& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongMotionParameters::equal(const StrongMotionParameters& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StrongMotionParameters& StrongMotionParameters::operator=(const StrongMotionParameters& other) {
	PublicObject::operator=(other);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongMotionParameters::assign(Object* other) {
	StrongMotionParameters* otherStrongMotionParameters = StrongMotionParameters::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherStrongMotionParameters;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongMotionParameters::attachTo(PublicObject* parent) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongMotionParameters::detachFrom(PublicObject* object) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongMotionParameters::detach() {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* StrongMotionParameters::clone() const {
	StrongMotionParameters* clonee = new StrongMotionParameters();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongMotionParameters::updateChild(Object* child) {
	SimpleFilter* simpleFilterChild = SimpleFilter::Cast(child);
	if ( simpleFilterChild != NULL ) {
		SimpleFilter* simpleFilterElement
			= SimpleFilter::Cast(PublicObject::Find(simpleFilterChild->publicID()));
		if ( simpleFilterElement && simpleFilterElement->parent() == this ) {
			*simpleFilterElement = *simpleFilterChild;
			return true;
		}
		return false;
	}

	Record* recordChild = Record::Cast(child);
	if ( recordChild != NULL ) {
		Record* recordElement
			= Record::Cast(PublicObject::Find(recordChild->publicID()));
		if ( recordElement && recordElement->parent() == this ) {
			*recordElement = *recordChild;
			return true;
		}
		return false;
	}

	StrongOriginDescription* strongOriginDescriptionChild = StrongOriginDescription::Cast(child);
	if ( strongOriginDescriptionChild != NULL ) {
		StrongOriginDescription* strongOriginDescriptionElement
			= StrongOriginDescription::Cast(PublicObject::Find(strongOriginDescriptionChild->publicID()));
		if ( strongOriginDescriptionElement && strongOriginDescriptionElement->parent() == this ) {
			*strongOriginDescriptionElement = *strongOriginDescriptionChild;
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StrongMotionParameters::accept(Visitor* visitor) {
	for ( std::vector<SimpleFilterPtr>::iterator it = _simpleFilters.begin(); it != _simpleFilters.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<RecordPtr>::iterator it = _records.begin(); it != _records.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<StrongOriginDescriptionPtr>::iterator it = _strongOriginDescriptions.begin(); it != _strongOriginDescriptions.end(); ++it )
		(*it)->accept(visitor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t StrongMotionParameters::simpleFilterCount() const {
	return _simpleFilters.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SimpleFilter* StrongMotionParameters::simpleFilter(size_t i) const {
	return _simpleFilters[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SimpleFilter* StrongMotionParameters::findSimpleFilter(const std::string& publicID) const {
	SimpleFilter* object = SimpleFilter::Cast(PublicObject::Find(publicID));
	if ( object != NULL && object->parent() == this )
		return object;
	
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongMotionParameters::add(SimpleFilter* simpleFilter) {
	if ( simpleFilter == NULL )
		return false;

	// Element has already a parent
	if ( simpleFilter->parent() != NULL ) {
		SEISCOMP_ERROR("StrongMotionParameters::add(SimpleFilter*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		SimpleFilter* simpleFilterCached = SimpleFilter::Find(simpleFilter->publicID());
		if ( simpleFilterCached ) {
			if ( simpleFilterCached->parent() ) {
				if ( simpleFilterCached->parent() == this )
					SEISCOMP_ERROR("StrongMotionParameters::add(SimpleFilter*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("StrongMotionParameters::add(SimpleFilter*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				simpleFilter = simpleFilterCached;
		}
	}

	// Add the element
	_simpleFilters.push_back(simpleFilter);
	simpleFilter->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		simpleFilter->accept(&nc);
	}

	// Notify registered observers
	childAdded(simpleFilter);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongMotionParameters::remove(SimpleFilter* simpleFilter) {
	if ( simpleFilter == NULL )
		return false;

	if ( simpleFilter->parent() != this ) {
		SEISCOMP_ERROR("StrongMotionParameters::remove(SimpleFilter*) -> element has another parent");
		return false;
	}

	std::vector<SimpleFilterPtr>::iterator it;
	it = std::find(_simpleFilters.begin(), _simpleFilters.end(), simpleFilter);
	// Element has not been found
	if ( it == _simpleFilters.end() ) {
		SEISCOMP_ERROR("StrongMotionParameters::remove(SimpleFilter*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_simpleFilters.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongMotionParameters::removeSimpleFilter(size_t i) {
	// index out of bounds
	if ( i >= _simpleFilters.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_simpleFilters[i]->accept(&nc);
	}

	_simpleFilters[i]->setParent(NULL);
	childRemoved(_simpleFilters[i].get());
	
	_simpleFilters.erase(_simpleFilters.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t StrongMotionParameters::recordCount() const {
	return _records.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record* StrongMotionParameters::record(size_t i) const {
	return _records[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record* StrongMotionParameters::findRecord(const std::string& publicID) const {
	Record* object = Record::Cast(PublicObject::Find(publicID));
	if ( object != NULL && object->parent() == this )
		return object;
	
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongMotionParameters::add(Record* record) {
	if ( record == NULL )
		return false;

	// Element has already a parent
	if ( record->parent() != NULL ) {
		SEISCOMP_ERROR("StrongMotionParameters::add(Record*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		Record* recordCached = Record::Find(record->publicID());
		if ( recordCached ) {
			if ( recordCached->parent() ) {
				if ( recordCached->parent() == this )
					SEISCOMP_ERROR("StrongMotionParameters::add(Record*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("StrongMotionParameters::add(Record*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				record = recordCached;
		}
	}

	// Add the element
	_records.push_back(record);
	record->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		record->accept(&nc);
	}

	// Notify registered observers
	childAdded(record);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongMotionParameters::remove(Record* record) {
	if ( record == NULL )
		return false;

	if ( record->parent() != this ) {
		SEISCOMP_ERROR("StrongMotionParameters::remove(Record*) -> element has another parent");
		return false;
	}

	std::vector<RecordPtr>::iterator it;
	it = std::find(_records.begin(), _records.end(), record);
	// Element has not been found
	if ( it == _records.end() ) {
		SEISCOMP_ERROR("StrongMotionParameters::remove(Record*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_records.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongMotionParameters::removeRecord(size_t i) {
	// index out of bounds
	if ( i >= _records.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_records[i]->accept(&nc);
	}

	_records[i]->setParent(NULL);
	childRemoved(_records[i].get());
	
	_records.erase(_records.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t StrongMotionParameters::strongOriginDescriptionCount() const {
	return _strongOriginDescriptions.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StrongOriginDescription* StrongMotionParameters::strongOriginDescription(size_t i) const {
	return _strongOriginDescriptions[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StrongOriginDescription* StrongMotionParameters::findStrongOriginDescription(const std::string& publicID) const {
	StrongOriginDescription* object = StrongOriginDescription::Cast(PublicObject::Find(publicID));
	if ( object != NULL && object->parent() == this )
		return object;
	
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongMotionParameters::add(StrongOriginDescription* strongOriginDescription) {
	if ( strongOriginDescription == NULL )
		return false;

	// Element has already a parent
	if ( strongOriginDescription->parent() != NULL ) {
		SEISCOMP_ERROR("StrongMotionParameters::add(StrongOriginDescription*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		StrongOriginDescription* strongOriginDescriptionCached = StrongOriginDescription::Find(strongOriginDescription->publicID());
		if ( strongOriginDescriptionCached ) {
			if ( strongOriginDescriptionCached->parent() ) {
				if ( strongOriginDescriptionCached->parent() == this )
					SEISCOMP_ERROR("StrongMotionParameters::add(StrongOriginDescription*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("StrongMotionParameters::add(StrongOriginDescription*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				strongOriginDescription = strongOriginDescriptionCached;
		}
	}

	// Add the element
	_strongOriginDescriptions.push_back(strongOriginDescription);
	strongOriginDescription->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		strongOriginDescription->accept(&nc);
	}

	// Notify registered observers
	childAdded(strongOriginDescription);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongMotionParameters::remove(StrongOriginDescription* strongOriginDescription) {
	if ( strongOriginDescription == NULL )
		return false;

	if ( strongOriginDescription->parent() != this ) {
		SEISCOMP_ERROR("StrongMotionParameters::remove(StrongOriginDescription*) -> element has another parent");
		return false;
	}

	std::vector<StrongOriginDescriptionPtr>::iterator it;
	it = std::find(_strongOriginDescriptions.begin(), _strongOriginDescriptions.end(), strongOriginDescription);
	// Element has not been found
	if ( it == _strongOriginDescriptions.end() ) {
		SEISCOMP_ERROR("StrongMotionParameters::remove(StrongOriginDescription*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_strongOriginDescriptions.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StrongMotionParameters::removeStrongOriginDescription(size_t i) {
	// index out of bounds
	if ( i >= _strongOriginDescriptions.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_strongOriginDescriptions[i]->accept(&nc);
	}

	_strongOriginDescriptions[i]->setParent(NULL);
	childRemoved(_strongOriginDescriptions[i].get());
	
	_strongOriginDescriptions.erase(_strongOriginDescriptions.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StrongMotionParameters::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,11>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: StrongMotionParameters skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("simpleFilter",
	                       Seiscomp::Core::Generic::containerMember(_simpleFilters,
	                       Seiscomp::Core::Generic::bindMemberFunction<SimpleFilter>(static_cast<bool (StrongMotionParameters::*)(SimpleFilter*)>(&StrongMotionParameters::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("record",
	                       Seiscomp::Core::Generic::containerMember(_records,
	                       Seiscomp::Core::Generic::bindMemberFunction<Record>(static_cast<bool (StrongMotionParameters::*)(Record*)>(&StrongMotionParameters::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("strongOriginDescription",
	                       Seiscomp::Core::Generic::containerMember(_strongOriginDescriptions,
	                       Seiscomp::Core::Generic::bindMemberFunction<StrongOriginDescription>(static_cast<bool (StrongMotionParameters::*)(StrongOriginDescription*)>(&StrongMotionParameters::add), this)),
	                       Archive::STATIC_TYPE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
