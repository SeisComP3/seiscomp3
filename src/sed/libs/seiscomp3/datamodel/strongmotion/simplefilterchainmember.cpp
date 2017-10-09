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
#include <seiscomp3/datamodel/strongmotion/simplefilterchainmember.h>
#include <seiscomp3/datamodel/strongmotion/record.h>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {
namespace StrongMotion {


IMPLEMENT_SC_CLASS_DERIVED(SimpleFilterChainMember, Object, "SimpleFilterChainMember");


SimpleFilterChainMember::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("sequenceNo", "int", false, false, true, false, false, false, NULL, &SimpleFilterChainMember::setSequenceNo, &SimpleFilterChainMember::sequenceNo));
	addProperty(Core::simpleProperty("simpleFilterID", "string", false, false, false, true, false, false, NULL, &SimpleFilterChainMember::setSimpleFilterID, &SimpleFilterChainMember::simpleFilterID));
}


IMPLEMENT_METAOBJECT(SimpleFilterChainMember)


SimpleFilterChainMemberIndex::SimpleFilterChainMemberIndex() {
	sequenceNo = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SimpleFilterChainMemberIndex::SimpleFilterChainMemberIndex(int sequenceNo_) {
	sequenceNo = sequenceNo_;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SimpleFilterChainMemberIndex::SimpleFilterChainMemberIndex(const SimpleFilterChainMemberIndex& idx) {
	sequenceNo = idx.sequenceNo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SimpleFilterChainMemberIndex::operator==(const SimpleFilterChainMemberIndex& idx) const {
	return sequenceNo == idx.sequenceNo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SimpleFilterChainMemberIndex::operator!=(const SimpleFilterChainMemberIndex& idx) const {
	return !operator==(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SimpleFilterChainMember::SimpleFilterChainMember() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SimpleFilterChainMember::SimpleFilterChainMember(const SimpleFilterChainMember& other)
 : Object() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SimpleFilterChainMember::~SimpleFilterChainMember() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SimpleFilterChainMember::operator==(const SimpleFilterChainMember& rhs) const {
	if ( _index != rhs._index ) return false;
	if ( _simpleFilterID != rhs._simpleFilterID ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SimpleFilterChainMember::operator!=(const SimpleFilterChainMember& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SimpleFilterChainMember::equal(const SimpleFilterChainMember& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SimpleFilterChainMember::setSequenceNo(int sequenceNo) {
	_index.sequenceNo = sequenceNo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SimpleFilterChainMember::sequenceNo() const {
	return _index.sequenceNo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SimpleFilterChainMember::setSimpleFilterID(const std::string& simpleFilterID) {
	_simpleFilterID = simpleFilterID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& SimpleFilterChainMember::simpleFilterID() const {
	return _simpleFilterID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const SimpleFilterChainMemberIndex& SimpleFilterChainMember::index() const {
	return _index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SimpleFilterChainMember::equalIndex(const SimpleFilterChainMember* lhs) const {
	if ( lhs == NULL ) return false;
	return lhs->index() == index();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record* SimpleFilterChainMember::record() const {
	return static_cast<Record*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SimpleFilterChainMember& SimpleFilterChainMember::operator=(const SimpleFilterChainMember& other) {
	_index = other._index;
	_simpleFilterID = other._simpleFilterID;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SimpleFilterChainMember::assign(Object* other) {
	SimpleFilterChainMember* otherSimpleFilterChainMember = SimpleFilterChainMember::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherSimpleFilterChainMember;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SimpleFilterChainMember::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	Record* record = Record::Cast(parent);
	if ( record != NULL )
		return record->add(this);

	SEISCOMP_ERROR("SimpleFilterChainMember::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SimpleFilterChainMember::detachFrom(PublicObject* object) {
	if ( object == NULL ) return false;

	// check all possible parents
	Record* record = Record::Cast(object);
	if ( record != NULL ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return record->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			SimpleFilterChainMember* child = record->simpleFilterChainMember(index());
			if ( child != NULL )
				return record->remove(child);
			else {
				SEISCOMP_DEBUG("SimpleFilterChainMember::detachFrom(Record): simpleFilterChainMember has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("SimpleFilterChainMember::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SimpleFilterChainMember::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* SimpleFilterChainMember::clone() const {
	SimpleFilterChainMember* clonee = new SimpleFilterChainMember();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SimpleFilterChainMember::accept(Visitor* visitor) {
	visitor->visit(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SimpleFilterChainMember::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,9>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: SimpleFilterChainMember skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	ar & NAMED_OBJECT_HINT("sequenceNo", _index.sequenceNo, Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	ar & NAMED_OBJECT_HINT("simpleFilterID", _simpleFilterID, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
