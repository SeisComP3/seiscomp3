/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <stationxml/coefficients.h>
#include <stationxml/floattype.h>
#include <stationxml/floattype.h>
#include <algorithm>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace StationXML {


namespace {

static Seiscomp::Core::MetaEnumImpl<CfTransferFunctionType> metaCfTransferFunctionType;

}


Coefficients::MetaObject::MetaObject(const Core::RTTI* rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(Core::simpleProperty("InputUnits", "string", false, false, false, false, false, false, NULL, &Coefficients::setInputUnits, &Coefficients::inputUnits));
	addProperty(Core::simpleProperty("OutputUnits", "string", false, false, false, false, false, false, NULL, &Coefficients::setOutputUnits, &Coefficients::outputUnits));
	addProperty(enumProperty("CfTransferFunctionType", "CfTransferFunctionType", false, false, &metaCfTransferFunctionType, &Coefficients::setCfTransferFunctionType, &Coefficients::cfTransferFunctionType));
	addProperty(arrayClassProperty<FloatType>("Numerator", "StationXML::FloatType", &Coefficients::numeratorCount, &Coefficients::numerator, static_cast<bool (Coefficients::*)(FloatType*)>(&Coefficients::addNumerator), &Coefficients::removeNumerator, static_cast<bool (Coefficients::*)(FloatType*)>(&Coefficients::removeNumerator)));
	addProperty(arrayClassProperty<FloatType>("Denominator", "StationXML::FloatType", &Coefficients::denominatorCount, &Coefficients::denominator, static_cast<bool (Coefficients::*)(FloatType*)>(&Coefficients::addDenominator), &Coefficients::removeDenominator, static_cast<bool (Coefficients::*)(FloatType*)>(&Coefficients::removeDenominator)));
}


IMPLEMENT_RTTI(Coefficients, "StationXML::Coefficients", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(Coefficients)
IMPLEMENT_METAOBJECT(Coefficients)


Coefficients::Coefficients() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Coefficients::Coefficients(const Coefficients& other)
 : Core::BaseObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Coefficients::~Coefficients() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Coefficients::operator==(const Coefficients& rhs) const {
	if ( !(_inputUnits == rhs._inputUnits) )
		return false;
	if ( !(_outputUnits == rhs._outputUnits) )
		return false;
	if ( !(_cfTransferFunctionType == rhs._cfTransferFunctionType) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Coefficients::setInputUnits(const std::string& inputUnits) {
	_inputUnits = inputUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Coefficients::inputUnits() const {
	return _inputUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Coefficients::setOutputUnits(const std::string& outputUnits) {
	_outputUnits = outputUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Coefficients::outputUnits() const {
	return _outputUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Coefficients::setCfTransferFunctionType(CfTransferFunctionType cfTransferFunctionType) {
	_cfTransferFunctionType = cfTransferFunctionType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CfTransferFunctionType Coefficients::cfTransferFunctionType() const {
	return _cfTransferFunctionType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Coefficients& Coefficients::operator=(const Coefficients& other) {
	_inputUnits = other._inputUnits;
	_outputUnits = other._outputUnits;
	_cfTransferFunctionType = other._cfTransferFunctionType;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Coefficients::numeratorCount() const {
	return _numerators.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FloatType* Coefficients::numerator(size_t i) const {
	return _numerators[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Coefficients::addNumerator(FloatType* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_numerators.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Coefficients::removeNumerator(FloatType* obj) {
	if ( obj == NULL )
		return false;

	std::vector<FloatTypePtr>::iterator it;
	it = std::find(_numerators.begin(), _numerators.end(), obj);
	// Element has not been found
	if ( it == _numerators.end() ) {
		SEISCOMP_ERROR("Coefficients::removeNumerator(FloatType*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Coefficients::removeNumerator(size_t i) {
	// index out of bounds
	if ( i >= _numerators.size() )
		return false;

	_numerators.erase(_numerators.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Coefficients::denominatorCount() const {
	return _denominators.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FloatType* Coefficients::denominator(size_t i) const {
	return _denominators[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Coefficients::addDenominator(FloatType* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_denominators.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Coefficients::removeDenominator(FloatType* obj) {
	if ( obj == NULL )
		return false;

	std::vector<FloatTypePtr>::iterator it;
	it = std::find(_denominators.begin(), _denominators.end(), obj);
	// Element has not been found
	if ( it == _denominators.end() ) {
		SEISCOMP_ERROR("Coefficients::removeDenominator(FloatType*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Coefficients::removeDenominator(size_t i) {
	// index out of bounds
	if ( i >= _denominators.size() )
		return false;

	_denominators.erase(_denominators.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
