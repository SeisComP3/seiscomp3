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
#include <seiscomp3/datamodel/originquality.h>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS(OriginQuality, "OriginQuality");


OriginQuality::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("associatedPhaseCount", "int", false, false, false, false, true, false, NULL, &OriginQuality::setAssociatedPhaseCount, &OriginQuality::associatedPhaseCount));
	addProperty(Core::simpleProperty("usedPhaseCount", "int", false, false, false, false, true, false, NULL, &OriginQuality::setUsedPhaseCount, &OriginQuality::usedPhaseCount));
	addProperty(Core::simpleProperty("associatedStationCount", "int", false, false, false, false, true, false, NULL, &OriginQuality::setAssociatedStationCount, &OriginQuality::associatedStationCount));
	addProperty(Core::simpleProperty("usedStationCount", "int", false, false, false, false, true, false, NULL, &OriginQuality::setUsedStationCount, &OriginQuality::usedStationCount));
	addProperty(Core::simpleProperty("depthPhaseCount", "int", false, false, false, false, true, false, NULL, &OriginQuality::setDepthPhaseCount, &OriginQuality::depthPhaseCount));
	addProperty(Core::simpleProperty("standardError", "float", false, false, false, false, true, false, NULL, &OriginQuality::setStandardError, &OriginQuality::standardError));
	addProperty(Core::simpleProperty("azimuthalGap", "float", false, false, false, false, true, false, NULL, &OriginQuality::setAzimuthalGap, &OriginQuality::azimuthalGap));
	addProperty(Core::simpleProperty("secondaryAzimuthalGap", "float", false, false, false, false, true, false, NULL, &OriginQuality::setSecondaryAzimuthalGap, &OriginQuality::secondaryAzimuthalGap));
	addProperty(Core::simpleProperty("groundTruthLevel", "string", false, false, false, false, false, false, NULL, &OriginQuality::setGroundTruthLevel, &OriginQuality::groundTruthLevel));
	addProperty(Core::simpleProperty("maximumDistance", "float", false, false, false, false, true, false, NULL, &OriginQuality::setMaximumDistance, &OriginQuality::maximumDistance));
	addProperty(Core::simpleProperty("minimumDistance", "float", false, false, false, false, true, false, NULL, &OriginQuality::setMinimumDistance, &OriginQuality::minimumDistance));
	addProperty(Core::simpleProperty("medianDistance", "float", false, false, false, false, true, false, NULL, &OriginQuality::setMedianDistance, &OriginQuality::medianDistance));
}


IMPLEMENT_METAOBJECT(OriginQuality)


OriginQuality::OriginQuality() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginQuality::OriginQuality(const OriginQuality& other)
: Core::BaseObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginQuality::~OriginQuality() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginQuality::operator==(const OriginQuality& rhs) const {
	if ( !(_associatedPhaseCount == rhs._associatedPhaseCount) )
		return false;
	if ( !(_usedPhaseCount == rhs._usedPhaseCount) )
		return false;
	if ( !(_associatedStationCount == rhs._associatedStationCount) )
		return false;
	if ( !(_usedStationCount == rhs._usedStationCount) )
		return false;
	if ( !(_depthPhaseCount == rhs._depthPhaseCount) )
		return false;
	if ( !(_standardError == rhs._standardError) )
		return false;
	if ( !(_azimuthalGap == rhs._azimuthalGap) )
		return false;
	if ( !(_secondaryAzimuthalGap == rhs._secondaryAzimuthalGap) )
		return false;
	if ( !(_groundTruthLevel == rhs._groundTruthLevel) )
		return false;
	if ( !(_maximumDistance == rhs._maximumDistance) )
		return false;
	if ( !(_minimumDistance == rhs._minimumDistance) )
		return false;
	if ( !(_medianDistance == rhs._medianDistance) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginQuality::operator!=(const OriginQuality& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginQuality::equal(const OriginQuality& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginQuality::setAssociatedPhaseCount(const OPT(int)& associatedPhaseCount) {
	_associatedPhaseCount = associatedPhaseCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int OriginQuality::associatedPhaseCount() const {
	if ( _associatedPhaseCount )
		return *_associatedPhaseCount;
	throw Seiscomp::Core::ValueException("OriginQuality.associatedPhaseCount is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginQuality::setUsedPhaseCount(const OPT(int)& usedPhaseCount) {
	_usedPhaseCount = usedPhaseCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int OriginQuality::usedPhaseCount() const {
	if ( _usedPhaseCount )
		return *_usedPhaseCount;
	throw Seiscomp::Core::ValueException("OriginQuality.usedPhaseCount is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginQuality::setAssociatedStationCount(const OPT(int)& associatedStationCount) {
	_associatedStationCount = associatedStationCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int OriginQuality::associatedStationCount() const {
	if ( _associatedStationCount )
		return *_associatedStationCount;
	throw Seiscomp::Core::ValueException("OriginQuality.associatedStationCount is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginQuality::setUsedStationCount(const OPT(int)& usedStationCount) {
	_usedStationCount = usedStationCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int OriginQuality::usedStationCount() const {
	if ( _usedStationCount )
		return *_usedStationCount;
	throw Seiscomp::Core::ValueException("OriginQuality.usedStationCount is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginQuality::setDepthPhaseCount(const OPT(int)& depthPhaseCount) {
	_depthPhaseCount = depthPhaseCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int OriginQuality::depthPhaseCount() const {
	if ( _depthPhaseCount )
		return *_depthPhaseCount;
	throw Seiscomp::Core::ValueException("OriginQuality.depthPhaseCount is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginQuality::setStandardError(const OPT(double)& standardError) {
	_standardError = standardError;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double OriginQuality::standardError() const {
	if ( _standardError )
		return *_standardError;
	throw Seiscomp::Core::ValueException("OriginQuality.standardError is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginQuality::setAzimuthalGap(const OPT(double)& azimuthalGap) {
	_azimuthalGap = azimuthalGap;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double OriginQuality::azimuthalGap() const {
	if ( _azimuthalGap )
		return *_azimuthalGap;
	throw Seiscomp::Core::ValueException("OriginQuality.azimuthalGap is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginQuality::setSecondaryAzimuthalGap(const OPT(double)& secondaryAzimuthalGap) {
	_secondaryAzimuthalGap = secondaryAzimuthalGap;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double OriginQuality::secondaryAzimuthalGap() const {
	if ( _secondaryAzimuthalGap )
		return *_secondaryAzimuthalGap;
	throw Seiscomp::Core::ValueException("OriginQuality.secondaryAzimuthalGap is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginQuality::setGroundTruthLevel(const std::string& groundTruthLevel) {
	_groundTruthLevel = groundTruthLevel;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& OriginQuality::groundTruthLevel() const {
	return _groundTruthLevel;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginQuality::setMaximumDistance(const OPT(double)& maximumDistance) {
	_maximumDistance = maximumDistance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double OriginQuality::maximumDistance() const {
	if ( _maximumDistance )
		return *_maximumDistance;
	throw Seiscomp::Core::ValueException("OriginQuality.maximumDistance is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginQuality::setMinimumDistance(const OPT(double)& minimumDistance) {
	_minimumDistance = minimumDistance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double OriginQuality::minimumDistance() const {
	if ( _minimumDistance )
		return *_minimumDistance;
	throw Seiscomp::Core::ValueException("OriginQuality.minimumDistance is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginQuality::setMedianDistance(const OPT(double)& medianDistance) {
	_medianDistance = medianDistance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double OriginQuality::medianDistance() const {
	if ( _medianDistance )
		return *_medianDistance;
	throw Seiscomp::Core::ValueException("OriginQuality.medianDistance is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginQuality& OriginQuality::operator=(const OriginQuality& other) {
	_associatedPhaseCount = other._associatedPhaseCount;
	_usedPhaseCount = other._usedPhaseCount;
	_associatedStationCount = other._associatedStationCount;
	_usedStationCount = other._usedStationCount;
	_depthPhaseCount = other._depthPhaseCount;
	_standardError = other._standardError;
	_azimuthalGap = other._azimuthalGap;
	_secondaryAzimuthalGap = other._secondaryAzimuthalGap;
	_groundTruthLevel = other._groundTruthLevel;
	_maximumDistance = other._maximumDistance;
	_minimumDistance = other._minimumDistance;
	_medianDistance = other._medianDistance;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginQuality::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,10>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: OriginQuality skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	ar & NAMED_OBJECT_HINT("associatedPhaseCount", _associatedPhaseCount, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("usedPhaseCount", _usedPhaseCount, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("associatedStationCount", _associatedStationCount, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("usedStationCount", _usedStationCount, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("depthPhaseCount", _depthPhaseCount, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("standardError", _standardError, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("azimuthalGap", _azimuthalGap, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("secondaryAzimuthalGap", _secondaryAzimuthalGap, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("groundTruthLevel", _groundTruthLevel, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("maximumDistance", _maximumDistance, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("minimumDistance", _minimumDistance, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("medianDistance", _medianDistance, Archive::XML_ELEMENT);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
