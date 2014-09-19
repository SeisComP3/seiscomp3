/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <stationxml/sensitivity.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace StationXML {


Sensitivity::MetaObject::MetaObject(const Core::RTTI* rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(Core::simpleProperty("SensitivityValue", "float", false, false, false, false, false, false, NULL, &Sensitivity::setSensitivityValue, &Sensitivity::sensitivityValue));
	addProperty(Core::simpleProperty("Frequency", "float", false, false, false, false, false, false, NULL, &Sensitivity::setFrequency, &Sensitivity::frequency));
	addProperty(Core::simpleProperty("SensitivityUnits", "string", false, false, false, false, false, false, NULL, &Sensitivity::setSensitivityUnits, &Sensitivity::sensitivityUnits));
	addProperty(Core::simpleProperty("FrequencyStart", "float", false, false, false, false, true, false, NULL, &Sensitivity::setFrequencyStart, &Sensitivity::frequencyStart));
	addProperty(Core::simpleProperty("FrequencyEnd", "float", false, false, false, false, true, false, NULL, &Sensitivity::setFrequencyEnd, &Sensitivity::frequencyEnd));
	addProperty(Core::simpleProperty("FrequencyDBVariation", "float", false, false, false, false, true, false, NULL, &Sensitivity::setFrequencyDBVariation, &Sensitivity::frequencyDBVariation));
}


IMPLEMENT_RTTI(Sensitivity, "StationXML::Sensitivity", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(Sensitivity)
IMPLEMENT_METAOBJECT(Sensitivity)


Sensitivity::Sensitivity() {
	_sensitivityValue = 0;
	_frequency = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensitivity::Sensitivity(const Sensitivity& other)
 : Core::BaseObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensitivity::~Sensitivity() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sensitivity::operator==(const Sensitivity& rhs) const {
	if ( !(_sensitivityValue == rhs._sensitivityValue) )
		return false;
	if ( !(_frequency == rhs._frequency) )
		return false;
	if ( !(_sensitivityUnits == rhs._sensitivityUnits) )
		return false;
	if ( !(_frequencyStart == rhs._frequencyStart) )
		return false;
	if ( !(_frequencyEnd == rhs._frequencyEnd) )
		return false;
	if ( !(_frequencyDBVariation == rhs._frequencyDBVariation) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensitivity::setSensitivityValue(double sensitivityValue) {
	_sensitivityValue = sensitivityValue;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Sensitivity::sensitivityValue() const {
	return _sensitivityValue;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensitivity::setFrequency(double frequency) {
	_frequency = frequency;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Sensitivity::frequency() const {
	return _frequency;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensitivity::setSensitivityUnits(const std::string& sensitivityUnits) {
	_sensitivityUnits = sensitivityUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Sensitivity::sensitivityUnits() const {
	return _sensitivityUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensitivity::setFrequencyStart(const OPT(double)& frequencyStart) {
	_frequencyStart = frequencyStart;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Sensitivity::frequencyStart() const throw(Seiscomp::Core::ValueException) {
	if ( _frequencyStart )
		return *_frequencyStart;
	throw Seiscomp::Core::ValueException("Sensitivity.FrequencyStart is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensitivity::setFrequencyEnd(const OPT(double)& frequencyEnd) {
	_frequencyEnd = frequencyEnd;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Sensitivity::frequencyEnd() const throw(Seiscomp::Core::ValueException) {
	if ( _frequencyEnd )
		return *_frequencyEnd;
	throw Seiscomp::Core::ValueException("Sensitivity.FrequencyEnd is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensitivity::setFrequencyDBVariation(const OPT(double)& frequencyDBVariation) {
	_frequencyDBVariation = frequencyDBVariation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Sensitivity::frequencyDBVariation() const throw(Seiscomp::Core::ValueException) {
	if ( _frequencyDBVariation )
		return *_frequencyDBVariation;
	throw Seiscomp::Core::ValueException("Sensitivity.FrequencyDBVariation is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensitivity& Sensitivity::operator=(const Sensitivity& other) {
	_sensitivityValue = other._sensitivityValue;
	_frequency = other._frequency;
	_sensitivityUnits = other._sensitivityUnits;
	_frequencyStart = other._frequencyStart;
	_frequencyEnd = other._frequencyEnd;
	_frequencyDBVariation = other._frequencyDBVariation;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
