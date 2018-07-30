/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <fdsnxml/dataavailabilityspan.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace FDSNXML {


DataAvailabilitySpan::MetaObject::MetaObject(const Core::RTTI *rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(Core::simpleProperty("start", "datetime", false, false, false, false, false, false, NULL, &DataAvailabilitySpan::setStart, &DataAvailabilitySpan::start));
	addProperty(Core::simpleProperty("end", "datetime", false, false, false, false, false, false, NULL, &DataAvailabilitySpan::setEnd, &DataAvailabilitySpan::end));
	addProperty(Core::simpleProperty("numberSegments", "int", false, false, false, false, false, false, NULL, &DataAvailabilitySpan::setNumberSegments, &DataAvailabilitySpan::numberSegments));
	addProperty(Core::simpleProperty("maximumTimeTear", "float", false, false, false, false, true, false, NULL, &DataAvailabilitySpan::setMaximumTimeTear, &DataAvailabilitySpan::maximumTimeTear));
}


IMPLEMENT_RTTI(DataAvailabilitySpan, "FDSNXML::DataAvailabilitySpan", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(DataAvailabilitySpan)
IMPLEMENT_METAOBJECT(DataAvailabilitySpan)


DataAvailabilitySpan::DataAvailabilitySpan() {
	_numberSegments = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataAvailabilitySpan::DataAvailabilitySpan(const DataAvailabilitySpan &other)
 : Core::BaseObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataAvailabilitySpan::~DataAvailabilitySpan() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DataAvailabilitySpan::operator==(const DataAvailabilitySpan &rhs) const {
	if ( !(_start == rhs._start) )
		return false;
	if ( !(_end == rhs._end) )
		return false;
	if ( !(_numberSegments == rhs._numberSegments) )
		return false;
	if ( !(_maximumTimeTear == rhs._maximumTimeTear) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DataAvailabilitySpan::setStart(DateTime start) {
	_start = start;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DateTime DataAvailabilitySpan::start() const {
	return _start;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DataAvailabilitySpan::setEnd(DateTime end) {
	_end = end;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DateTime DataAvailabilitySpan::end() const {
	return _end;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DataAvailabilitySpan::setNumberSegments(int numberSegments) {
	_numberSegments = numberSegments;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DataAvailabilitySpan::numberSegments() const {
	return _numberSegments;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DataAvailabilitySpan::setMaximumTimeTear(const OPT(double)& maximumTimeTear) {
	_maximumTimeTear = maximumTimeTear;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double DataAvailabilitySpan::maximumTimeTear() const {
	if ( _maximumTimeTear )
		return *_maximumTimeTear;
	throw Seiscomp::Core::ValueException("DataAvailabilitySpan.maximumTimeTear is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataAvailabilitySpan& DataAvailabilitySpan::operator=(const DataAvailabilitySpan &other) {
	_start = other._start;
	_end = other._end;
	_numberSegments = other._numberSegments;
	_maximumTimeTear = other._maximumTimeTear;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
