/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <fdsnxml/sensitivity.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace FDSNXML {


Sensitivity::MetaObject::MetaObject(const Core::RTTI *rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(objectProperty<UnitsType>("InputUnits", "FDSNXML::UnitsType", false, false, &Sensitivity::setInputUnits, &Sensitivity::inputUnits));
	addProperty(objectProperty<UnitsType>("OutputUnits", "FDSNXML::UnitsType", false, false, &Sensitivity::setOutputUnits, &Sensitivity::outputUnits));
	addProperty(Core::simpleProperty("FrequencyStart", "float", false, false, false, false, true, false, NULL, &Sensitivity::setFrequencyStart, &Sensitivity::frequencyStart));
	addProperty(Core::simpleProperty("FrequencyEnd", "float", false, false, false, false, true, false, NULL, &Sensitivity::setFrequencyEnd, &Sensitivity::frequencyEnd));
	addProperty(Core::simpleProperty("FrequencyDBVariation", "float", false, false, false, false, true, false, NULL, &Sensitivity::setFrequencyDBVariation, &Sensitivity::frequencyDBVariation));
}


IMPLEMENT_RTTI(Sensitivity, "FDSNXML::Sensitivity", Gain)
IMPLEMENT_RTTI_METHODS(Sensitivity)
IMPLEMENT_METAOBJECT_DERIVED(Sensitivity, Gain)


Sensitivity::Sensitivity() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensitivity::Sensitivity(const Sensitivity &other)
 : Gain() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensitivity::~Sensitivity() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sensitivity::operator==(const Sensitivity &rhs) const {
	if ( !(_inputUnits == rhs._inputUnits) )
		return false;
	if ( !(_outputUnits == rhs._outputUnits) )
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
void Sensitivity::setInputUnits(const UnitsType& inputUnits) {
	_inputUnits = inputUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
UnitsType& Sensitivity::inputUnits() {
	return _inputUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const UnitsType& Sensitivity::inputUnits() const {
	return _inputUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensitivity::setOutputUnits(const UnitsType& outputUnits) {
	_outputUnits = outputUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
UnitsType& Sensitivity::outputUnits() {
	return _outputUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const UnitsType& Sensitivity::outputUnits() const {
	return _outputUnits;
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
Sensitivity& Sensitivity::operator=(const Sensitivity &other) {
	Gain::operator=(other);
	_inputUnits = other._inputUnits;
	_outputUnits = other._outputUnits;
	_frequencyStart = other._frequencyStart;
	_frequencyEnd = other._frequencyEnd;
	_frequencyDBVariation = other._frequencyDBVariation;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
