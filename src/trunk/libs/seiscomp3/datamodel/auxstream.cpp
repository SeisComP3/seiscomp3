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
#include <seiscomp3/datamodel/auxstream.h>
#include <seiscomp3/datamodel/sensorlocation.h>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(AuxStream, Object, "AuxStream");


AuxStream::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("code", "string", false, false, true, false, false, false, NULL, &AuxStream::setCode, &AuxStream::code));
	addProperty(Core::simpleProperty("start", "datetime", false, false, true, false, false, false, NULL, &AuxStream::setStart, &AuxStream::start));
	addProperty(Core::simpleProperty("end", "datetime", false, false, false, false, true, false, NULL, &AuxStream::setEnd, &AuxStream::end));
	addProperty(Core::simpleProperty("device", "string", false, false, false, true, false, false, NULL, &AuxStream::setDevice, &AuxStream::device));
	addProperty(Core::simpleProperty("deviceSerialNumber", "string", false, false, false, false, false, false, NULL, &AuxStream::setDeviceSerialNumber, &AuxStream::deviceSerialNumber));
	addProperty(Core::simpleProperty("source", "string", false, false, false, false, false, false, NULL, &AuxStream::setSource, &AuxStream::source));
	addProperty(Core::simpleProperty("format", "string", false, false, false, false, false, false, NULL, &AuxStream::setFormat, &AuxStream::format));
	addProperty(Core::simpleProperty("flags", "string", false, false, false, false, false, false, NULL, &AuxStream::setFlags, &AuxStream::flags));
	addProperty(Core::simpleProperty("restricted", "boolean", false, false, false, false, true, false, NULL, &AuxStream::setRestricted, &AuxStream::restricted));
	addProperty(Core::simpleProperty("shared", "boolean", false, false, false, false, true, false, NULL, &AuxStream::setShared, &AuxStream::shared));
}


IMPLEMENT_METAOBJECT(AuxStream)


AuxStreamIndex::AuxStreamIndex() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AuxStreamIndex::AuxStreamIndex(const std::string& code_,
                               Seiscomp::Core::Time start_) {
	code = code_;
	start = start_;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AuxStreamIndex::AuxStreamIndex(const AuxStreamIndex& idx) {
	code = idx.code;
	start = idx.start;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AuxStreamIndex::operator==(const AuxStreamIndex& idx) const {
	return code == idx.code &&
	       start == idx.start;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AuxStreamIndex::operator!=(const AuxStreamIndex& idx) const {
	return !operator==(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AuxStream::AuxStream() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AuxStream::AuxStream(const AuxStream& other)
 : Object() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AuxStream::~AuxStream() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AuxStream::operator==(const AuxStream& rhs) const {
	if ( _index != rhs._index ) return false;
	if ( _end != rhs._end ) return false;
	if ( _device != rhs._device ) return false;
	if ( _deviceSerialNumber != rhs._deviceSerialNumber ) return false;
	if ( _source != rhs._source ) return false;
	if ( _format != rhs._format ) return false;
	if ( _flags != rhs._flags ) return false;
	if ( _restricted != rhs._restricted ) return false;
	if ( _shared != rhs._shared ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AuxStream::operator!=(const AuxStream& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AuxStream::equal(const AuxStream& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AuxStream::setCode(const std::string& code) {
	_index.code = code;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& AuxStream::code() const {
	return _index.code;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AuxStream::setStart(Seiscomp::Core::Time start) {
	_index.start = start;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time AuxStream::start() const {
	return _index.start;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AuxStream::setEnd(const OPT(Seiscomp::Core::Time)& end) {
	_end = end;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time AuxStream::end() const {
	if ( _end )
		return *_end;
	throw Seiscomp::Core::ValueException("AuxStream.end is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AuxStream::setDevice(const std::string& device) {
	_device = device;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& AuxStream::device() const {
	return _device;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AuxStream::setDeviceSerialNumber(const std::string& deviceSerialNumber) {
	_deviceSerialNumber = deviceSerialNumber;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& AuxStream::deviceSerialNumber() const {
	return _deviceSerialNumber;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AuxStream::setSource(const std::string& source) {
	_source = source;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& AuxStream::source() const {
	return _source;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AuxStream::setFormat(const std::string& format) {
	_format = format;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& AuxStream::format() const {
	return _format;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AuxStream::setFlags(const std::string& flags) {
	_flags = flags;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& AuxStream::flags() const {
	return _flags;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AuxStream::setRestricted(const OPT(bool)& restricted) {
	_restricted = restricted;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AuxStream::restricted() const {
	if ( _restricted )
		return *_restricted;
	throw Seiscomp::Core::ValueException("AuxStream.restricted is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AuxStream::setShared(const OPT(bool)& shared) {
	_shared = shared;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AuxStream::shared() const {
	if ( _shared )
		return *_shared;
	throw Seiscomp::Core::ValueException("AuxStream.shared is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const AuxStreamIndex& AuxStream::index() const {
	return _index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AuxStream::equalIndex(const AuxStream* lhs) const {
	if ( lhs == NULL ) return false;
	return lhs->index() == index();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SensorLocation* AuxStream::sensorLocation() const {
	return static_cast<SensorLocation*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AuxStream& AuxStream::operator=(const AuxStream& other) {
	_index = other._index;
	_end = other._end;
	_device = other._device;
	_deviceSerialNumber = other._deviceSerialNumber;
	_source = other._source;
	_format = other._format;
	_flags = other._flags;
	_restricted = other._restricted;
	_shared = other._shared;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AuxStream::assign(Object* other) {
	AuxStream* otherAuxStream = AuxStream::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherAuxStream;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AuxStream::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	SensorLocation* sensorLocation = SensorLocation::Cast(parent);
	if ( sensorLocation != NULL )
		return sensorLocation->add(this);

	SEISCOMP_ERROR("AuxStream::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AuxStream::detachFrom(PublicObject* object) {
	if ( object == NULL ) return false;

	// check all possible parents
	SensorLocation* sensorLocation = SensorLocation::Cast(object);
	if ( sensorLocation != NULL ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return sensorLocation->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			AuxStream* child = sensorLocation->auxStream(index());
			if ( child != NULL )
				return sensorLocation->remove(child);
			else {
				SEISCOMP_DEBUG("AuxStream::detachFrom(SensorLocation): auxStream has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("AuxStream::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AuxStream::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* AuxStream::clone() const {
	AuxStream* clonee = new AuxStream();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AuxStream::accept(Visitor* visitor) {
	visitor->visit(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AuxStream::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,10>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: AuxStream skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	ar & NAMED_OBJECT_HINT("code", _index.code, Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	if ( ar.supportsVersion<0,10>() )
		ar & NAMED_OBJECT_HINT("start", _index.start, Archive::XML_ELEMENT | Archive::SPLIT_TIME | Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	else
		ar & NAMED_OBJECT_HINT("start", _index.start, Archive::XML_ELEMENT | Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	if ( ar.supportsVersion<0,10>() )
		ar & NAMED_OBJECT_HINT("end", _end, Archive::XML_ELEMENT | Archive::SPLIT_TIME);
	else
		ar & NAMED_OBJECT_HINT("end", _end, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT("device", _device);
	ar & NAMED_OBJECT_HINT("deviceSerialNumber", _deviceSerialNumber, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("source", _source, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("format", _format, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("flags", _flags, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("restricted", _restricted, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("shared", _shared, Archive::XML_ELEMENT);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
