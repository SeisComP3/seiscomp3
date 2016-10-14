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
#include <seiscomp3/datamodel/nodalplanes.h>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS(NodalPlanes, "NodalPlanes");


NodalPlanes::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(objectProperty<NodalPlane>("nodalPlane1", "NodalPlane", false, false, true, &NodalPlanes::setNodalPlane1, &NodalPlanes::nodalPlane1));
	addProperty(objectProperty<NodalPlane>("nodalPlane2", "NodalPlane", false, false, true, &NodalPlanes::setNodalPlane2, &NodalPlanes::nodalPlane2));
	addProperty(Core::simpleProperty("preferredPlane", "int", false, false, false, false, true, false, NULL, &NodalPlanes::setPreferredPlane, &NodalPlanes::preferredPlane));
}


IMPLEMENT_METAOBJECT(NodalPlanes)


NodalPlanes::NodalPlanes() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NodalPlanes::NodalPlanes(const NodalPlanes& other)
 : Core::BaseObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NodalPlanes::~NodalPlanes() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool NodalPlanes::operator==(const NodalPlanes& rhs) const {
	if ( !(_nodalPlane1 == rhs._nodalPlane1) )
		return false;
	if ( !(_nodalPlane2 == rhs._nodalPlane2) )
		return false;
	if ( !(_preferredPlane == rhs._preferredPlane) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool NodalPlanes::operator!=(const NodalPlanes& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool NodalPlanes::equal(const NodalPlanes& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void NodalPlanes::setNodalPlane1(const OPT(NodalPlane)& nodalPlane1) {
	_nodalPlane1 = nodalPlane1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NodalPlane& NodalPlanes::nodalPlane1() throw(Seiscomp::Core::ValueException) {
	if ( _nodalPlane1 )
		return *_nodalPlane1;
	throw Seiscomp::Core::ValueException("NodalPlanes.nodalPlane1 is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const NodalPlane& NodalPlanes::nodalPlane1() const throw(Seiscomp::Core::ValueException) {
	if ( _nodalPlane1 )
		return *_nodalPlane1;
	throw Seiscomp::Core::ValueException("NodalPlanes.nodalPlane1 is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void NodalPlanes::setNodalPlane2(const OPT(NodalPlane)& nodalPlane2) {
	_nodalPlane2 = nodalPlane2;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NodalPlane& NodalPlanes::nodalPlane2() throw(Seiscomp::Core::ValueException) {
	if ( _nodalPlane2 )
		return *_nodalPlane2;
	throw Seiscomp::Core::ValueException("NodalPlanes.nodalPlane2 is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const NodalPlane& NodalPlanes::nodalPlane2() const throw(Seiscomp::Core::ValueException) {
	if ( _nodalPlane2 )
		return *_nodalPlane2;
	throw Seiscomp::Core::ValueException("NodalPlanes.nodalPlane2 is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void NodalPlanes::setPreferredPlane(const OPT(int)& preferredPlane) {
	_preferredPlane = preferredPlane;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int NodalPlanes::preferredPlane() const throw(Seiscomp::Core::ValueException) {
	if ( _preferredPlane )
		return *_preferredPlane;
	throw Seiscomp::Core::ValueException("NodalPlanes.preferredPlane is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NodalPlanes& NodalPlanes::operator=(const NodalPlanes& other) {
	_nodalPlane1 = other._nodalPlane1;
	_nodalPlane2 = other._nodalPlane2;
	_preferredPlane = other._preferredPlane;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void NodalPlanes::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,9>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: NodalPlanes skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	ar & NAMED_OBJECT_HINT("nodalPlane1", _nodalPlane1, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("nodalPlane2", _nodalPlane2, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT("preferredPlane", _preferredPlane);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
