/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <fdsnxml/dataavailability.h>
#include <fdsnxml/dataavailabilityspan.h>
#include <algorithm>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace FDSNXML {


DataAvailability::MetaObject::MetaObject(const Core::RTTI *rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(objectProperty<DataAvailabilityExtent>("extent", "FDSNXML::DataAvailabilityExtent", false, true, &DataAvailability::setExtent, &DataAvailability::extent));
	addProperty(arrayClassProperty<DataAvailabilitySpan>("span", "FDSNXML::DataAvailabilitySpan", &DataAvailability::spanCount, &DataAvailability::span, static_cast<bool (DataAvailability::*)(DataAvailabilitySpan*)>(&DataAvailability::addSpan), &DataAvailability::removeSpan, static_cast<bool (DataAvailability::*)(DataAvailabilitySpan*)>(&DataAvailability::removeSpan)));
}


IMPLEMENT_RTTI(DataAvailability, "FDSNXML::DataAvailability", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(DataAvailability)
IMPLEMENT_METAOBJECT(DataAvailability)


DataAvailability::DataAvailability() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataAvailability::DataAvailability(const DataAvailability &other)
 : Core::BaseObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataAvailability::~DataAvailability() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DataAvailability::operator==(const DataAvailability &rhs) const {
	if ( !(_extent == rhs._extent) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DataAvailability::setExtent(const OPT(DataAvailabilityExtent)& extent) {
	_extent = extent;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataAvailabilityExtent& DataAvailability::extent() {
	if ( _extent )
		return *_extent;
	throw Seiscomp::Core::ValueException("DataAvailability.extent is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataAvailabilityExtent& DataAvailability::extent() const {
	if ( _extent )
		return *_extent;
	throw Seiscomp::Core::ValueException("DataAvailability.extent is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataAvailability& DataAvailability::operator=(const DataAvailability &other) {
	_extent = other._extent;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t DataAvailability::spanCount() const {
	return _spans.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataAvailabilitySpan* DataAvailability::span(size_t i) const {
	return _spans[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DataAvailability::addSpan(DataAvailabilitySpan *obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_spans.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DataAvailability::removeSpan(DataAvailabilitySpan *obj) {
	if ( obj == NULL )
		return false;

	std::vector<DataAvailabilitySpanPtr>::iterator it;
	it = std::find(_spans.begin(), _spans.end(), obj);
	// Element has not been found
	if ( it == _spans.end() ) {
		SEISCOMP_ERROR("DataAvailability::removeSpan(DataAvailabilitySpan*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DataAvailability::removeSpan(size_t i) {
	// index out of bounds
	if ( i >= _spans.size() )
		return false;

	_spans.erase(_spans.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
