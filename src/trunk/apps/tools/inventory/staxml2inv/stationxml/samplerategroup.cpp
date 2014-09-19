/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <stationxml/samplerategroup.h>
#include <stationxml/sampleratetype.h>
#include <stationxml/samplerateratiotype.h>
#include <algorithm>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace StationXML {


SampleRateGroup::MetaObject::MetaObject(const Core::RTTI* rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(arrayClassProperty<SampleRateType>("SampleRate", "StationXML::SampleRateType", &SampleRateGroup::sampleRateCount, &SampleRateGroup::sampleRate, static_cast<bool (SampleRateGroup::*)(SampleRateType*)>(&SampleRateGroup::addSampleRate), &SampleRateGroup::removeSampleRate, static_cast<bool (SampleRateGroup::*)(SampleRateType*)>(&SampleRateGroup::removeSampleRate)));
	addProperty(arrayClassProperty<SampleRateRatioType>("SampleRateRatio", "StationXML::SampleRateRatioType", &SampleRateGroup::sampleRateRatioCount, &SampleRateGroup::sampleRateRatio, static_cast<bool (SampleRateGroup::*)(SampleRateRatioType*)>(&SampleRateGroup::addSampleRateRatio), &SampleRateGroup::removeSampleRateRatio, static_cast<bool (SampleRateGroup::*)(SampleRateRatioType*)>(&SampleRateGroup::removeSampleRateRatio)));
}


IMPLEMENT_RTTI(SampleRateGroup, "StationXML::SampleRateGroup", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(SampleRateGroup)
IMPLEMENT_METAOBJECT(SampleRateGroup)


SampleRateGroup::SampleRateGroup() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SampleRateGroup::SampleRateGroup(const SampleRateGroup& other)
 : Core::BaseObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SampleRateGroup::~SampleRateGroup() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SampleRateGroup::operator==(const SampleRateGroup& rhs) const {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SampleRateGroup& SampleRateGroup::operator=(const SampleRateGroup& other) {
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t SampleRateGroup::sampleRateCount() const {
	return _sampleRates.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SampleRateType* SampleRateGroup::sampleRate(size_t i) const {
	return _sampleRates[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SampleRateGroup::addSampleRate(SampleRateType* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_sampleRates.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SampleRateGroup::removeSampleRate(SampleRateType* obj) {
	if ( obj == NULL )
		return false;

	std::vector<SampleRateTypePtr>::iterator it;
	it = std::find(_sampleRates.begin(), _sampleRates.end(), obj);
	// Element has not been found
	if ( it == _sampleRates.end() ) {
		SEISCOMP_ERROR("SampleRateGroup::removeSampleRate(SampleRateType*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SampleRateGroup::removeSampleRate(size_t i) {
	// index out of bounds
	if ( i >= _sampleRates.size() )
		return false;

	_sampleRates.erase(_sampleRates.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t SampleRateGroup::sampleRateRatioCount() const {
	return _sampleRateRatios.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SampleRateRatioType* SampleRateGroup::sampleRateRatio(size_t i) const {
	return _sampleRateRatios[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SampleRateGroup::addSampleRateRatio(SampleRateRatioType* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_sampleRateRatios.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SampleRateGroup::removeSampleRateRatio(SampleRateRatioType* obj) {
	if ( obj == NULL )
		return false;

	std::vector<SampleRateRatioTypePtr>::iterator it;
	it = std::find(_sampleRateRatios.begin(), _sampleRateRatios.end(), obj);
	// Element has not been found
	if ( it == _sampleRateRatios.end() ) {
		SEISCOMP_ERROR("SampleRateGroup::removeSampleRateRatio(SampleRateRatioType*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SampleRateGroup::removeSampleRateRatio(size_t i) {
	// index out of bounds
	if ( i >= _sampleRateRatios.size() )
		return false;

	_sampleRateRatios.erase(_sampleRateRatios.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
