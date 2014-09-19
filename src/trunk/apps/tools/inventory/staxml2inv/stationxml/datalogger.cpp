/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <stationxml/datalogger.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace StationXML {


Datalogger::MetaObject::MetaObject(const Core::RTTI* rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(objectProperty<CounterType>("TotalChannels", "StationXML::CounterType", false, true, &Datalogger::setTotalChannels, &Datalogger::totalChannels));
	addProperty(objectProperty<CounterType>("RecordedChannels", "StationXML::CounterType", false, true, &Datalogger::setRecordedChannels, &Datalogger::recordedChannels));
}


IMPLEMENT_RTTI(Datalogger, "StationXML::Datalogger", Equipment)
IMPLEMENT_RTTI_METHODS(Datalogger)
IMPLEMENT_METAOBJECT_DERIVED(Datalogger, Equipment)


Datalogger::Datalogger() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Datalogger::Datalogger(const Datalogger& other)
 : Equipment() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Datalogger::~Datalogger() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Datalogger::operator==(const Datalogger& rhs) const {
	if ( !(_totalChannels == rhs._totalChannels) )
		return false;
	if ( !(_recordedChannels == rhs._recordedChannels) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Datalogger::setTotalChannels(const OPT(CounterType)& totalChannels) {
	_totalChannels = totalChannels;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CounterType& Datalogger::totalChannels() throw(Seiscomp::Core::ValueException) {
	if ( _totalChannels )
		return *_totalChannels;
	throw Seiscomp::Core::ValueException("Datalogger.TotalChannels is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CounterType& Datalogger::totalChannels() const throw(Seiscomp::Core::ValueException) {
	if ( _totalChannels )
		return *_totalChannels;
	throw Seiscomp::Core::ValueException("Datalogger.TotalChannels is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Datalogger::setRecordedChannels(const OPT(CounterType)& recordedChannels) {
	_recordedChannels = recordedChannels;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CounterType& Datalogger::recordedChannels() throw(Seiscomp::Core::ValueException) {
	if ( _recordedChannels )
		return *_recordedChannels;
	throw Seiscomp::Core::ValueException("Datalogger.RecordedChannels is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CounterType& Datalogger::recordedChannels() const throw(Seiscomp::Core::ValueException) {
	if ( _recordedChannels )
		return *_recordedChannels;
	throw Seiscomp::Core::ValueException("Datalogger.RecordedChannels is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Datalogger& Datalogger::operator=(const Datalogger& other) {
	Equipment::operator=(other);
	_totalChannels = other._totalChannels;
	_recordedChannels = other._recordedChannels;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
