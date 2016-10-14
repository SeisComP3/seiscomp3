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
#include <seiscomp3/datamodel/confidenceellipsoid.h>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS(ConfidenceEllipsoid, "ConfidenceEllipsoid");


ConfidenceEllipsoid::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("semiMajorAxisLength", "float", false, false, false, false, false, false, NULL, &ConfidenceEllipsoid::setSemiMajorAxisLength, &ConfidenceEllipsoid::semiMajorAxisLength));
	addProperty(Core::simpleProperty("semiMinorAxisLength", "float", false, false, false, false, false, false, NULL, &ConfidenceEllipsoid::setSemiMinorAxisLength, &ConfidenceEllipsoid::semiMinorAxisLength));
	addProperty(Core::simpleProperty("semiIntermediateAxisLength", "float", false, false, false, false, false, false, NULL, &ConfidenceEllipsoid::setSemiIntermediateAxisLength, &ConfidenceEllipsoid::semiIntermediateAxisLength));
	addProperty(Core::simpleProperty("majorAxisPlunge", "float", false, false, false, false, false, false, NULL, &ConfidenceEllipsoid::setMajorAxisPlunge, &ConfidenceEllipsoid::majorAxisPlunge));
	addProperty(Core::simpleProperty("majorAxisAzimuth", "float", false, false, false, false, false, false, NULL, &ConfidenceEllipsoid::setMajorAxisAzimuth, &ConfidenceEllipsoid::majorAxisAzimuth));
	addProperty(Core::simpleProperty("majorAxisRotation", "float", false, false, false, false, false, false, NULL, &ConfidenceEllipsoid::setMajorAxisRotation, &ConfidenceEllipsoid::majorAxisRotation));
}


IMPLEMENT_METAOBJECT(ConfidenceEllipsoid)


ConfidenceEllipsoid::ConfidenceEllipsoid() {
	_semiMajorAxisLength = 0;
	_semiMinorAxisLength = 0;
	_semiIntermediateAxisLength = 0;
	_majorAxisPlunge = 0;
	_majorAxisAzimuth = 0;
	_majorAxisRotation = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ConfidenceEllipsoid::ConfidenceEllipsoid(const ConfidenceEllipsoid& other)
 : Core::BaseObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ConfidenceEllipsoid::~ConfidenceEllipsoid() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ConfidenceEllipsoid::operator==(const ConfidenceEllipsoid& rhs) const {
	if ( !(_semiMajorAxisLength == rhs._semiMajorAxisLength) )
		return false;
	if ( !(_semiMinorAxisLength == rhs._semiMinorAxisLength) )
		return false;
	if ( !(_semiIntermediateAxisLength == rhs._semiIntermediateAxisLength) )
		return false;
	if ( !(_majorAxisPlunge == rhs._majorAxisPlunge) )
		return false;
	if ( !(_majorAxisAzimuth == rhs._majorAxisAzimuth) )
		return false;
	if ( !(_majorAxisRotation == rhs._majorAxisRotation) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ConfidenceEllipsoid::operator!=(const ConfidenceEllipsoid& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ConfidenceEllipsoid::equal(const ConfidenceEllipsoid& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConfidenceEllipsoid::setSemiMajorAxisLength(double semiMajorAxisLength) {
	_semiMajorAxisLength = semiMajorAxisLength;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double ConfidenceEllipsoid::semiMajorAxisLength() const {
	return _semiMajorAxisLength;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConfidenceEllipsoid::setSemiMinorAxisLength(double semiMinorAxisLength) {
	_semiMinorAxisLength = semiMinorAxisLength;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double ConfidenceEllipsoid::semiMinorAxisLength() const {
	return _semiMinorAxisLength;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConfidenceEllipsoid::setSemiIntermediateAxisLength(double semiIntermediateAxisLength) {
	_semiIntermediateAxisLength = semiIntermediateAxisLength;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double ConfidenceEllipsoid::semiIntermediateAxisLength() const {
	return _semiIntermediateAxisLength;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConfidenceEllipsoid::setMajorAxisPlunge(double majorAxisPlunge) {
	_majorAxisPlunge = majorAxisPlunge;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double ConfidenceEllipsoid::majorAxisPlunge() const {
	return _majorAxisPlunge;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConfidenceEllipsoid::setMajorAxisAzimuth(double majorAxisAzimuth) {
	_majorAxisAzimuth = majorAxisAzimuth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double ConfidenceEllipsoid::majorAxisAzimuth() const {
	return _majorAxisAzimuth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConfidenceEllipsoid::setMajorAxisRotation(double majorAxisRotation) {
	_majorAxisRotation = majorAxisRotation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double ConfidenceEllipsoid::majorAxisRotation() const {
	return _majorAxisRotation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ConfidenceEllipsoid& ConfidenceEllipsoid::operator=(const ConfidenceEllipsoid& other) {
	_semiMajorAxisLength = other._semiMajorAxisLength;
	_semiMinorAxisLength = other._semiMinorAxisLength;
	_semiIntermediateAxisLength = other._semiIntermediateAxisLength;
	_majorAxisPlunge = other._majorAxisPlunge;
	_majorAxisAzimuth = other._majorAxisAzimuth;
	_majorAxisRotation = other._majorAxisRotation;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConfidenceEllipsoid::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,9>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: ConfidenceEllipsoid skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	ar & NAMED_OBJECT_HINT("semiMajorAxisLength", _semiMajorAxisLength, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("semiMinorAxisLength", _semiMinorAxisLength, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("semiIntermediateAxisLength", _semiIntermediateAxisLength, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("majorAxisPlunge", _majorAxisPlunge, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("majorAxisAzimuth", _majorAxisAzimuth, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("majorAxisRotation", _majorAxisRotation, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
