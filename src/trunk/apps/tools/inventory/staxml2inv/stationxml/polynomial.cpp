/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <stationxml/polynomial.h>
#include <stationxml/polynomialcoefficient.h>
#include <algorithm>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace StationXML {


namespace {

static Seiscomp::Core::MetaEnumImpl<ApproximationType> metaApproximationType;

}


Polynomial::MetaObject::MetaObject(const Core::RTTI* rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(Core::simpleProperty("InputUnits", "string", false, false, false, false, false, false, NULL, &Polynomial::setInputUnits, &Polynomial::inputUnits));
	addProperty(Core::simpleProperty("OutputUnits", "string", false, false, false, false, false, false, NULL, &Polynomial::setOutputUnits, &Polynomial::outputUnits));
	addProperty(enumProperty("ApproximationType", "ApproximationType", false, false, &metaApproximationType, &Polynomial::setApproximationType, &Polynomial::approximationType));
	addProperty(objectProperty<FrequencyType>("FreqLowerBound", "StationXML::FrequencyType", false, false, &Polynomial::setFreqLowerBound, &Polynomial::freqLowerBound));
	addProperty(objectProperty<FrequencyType>("FreqUpperBound", "StationXML::FrequencyType", false, false, &Polynomial::setFreqUpperBound, &Polynomial::freqUpperBound));
	addProperty(objectProperty<FrequencyType>("ApproxLowerBound", "StationXML::FrequencyType", false, false, &Polynomial::setApproxLowerBound, &Polynomial::approxLowerBound));
	addProperty(objectProperty<FrequencyType>("ApproxUpperBound", "StationXML::FrequencyType", false, false, &Polynomial::setApproxUpperBound, &Polynomial::approxUpperBound));
	addProperty(objectProperty<FrequencyType>("MaxError", "StationXML::FrequencyType", false, false, &Polynomial::setMaxError, &Polynomial::maxError));
	addProperty(arrayClassProperty<PolynomialCoefficient>("Coefficient", "StationXML::PolynomialCoefficient", &Polynomial::coefficientCount, &Polynomial::coefficient, static_cast<bool (Polynomial::*)(PolynomialCoefficient*)>(&Polynomial::addCoefficient), &Polynomial::removeCoefficient, static_cast<bool (Polynomial::*)(PolynomialCoefficient*)>(&Polynomial::removeCoefficient)));
}


IMPLEMENT_RTTI(Polynomial, "StationXML::Polynomial", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(Polynomial)
IMPLEMENT_METAOBJECT(Polynomial)


Polynomial::Polynomial() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Polynomial::Polynomial(const Polynomial& other)
 : Core::BaseObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Polynomial::~Polynomial() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Polynomial::operator==(const Polynomial& rhs) const {
	if ( !(_inputUnits == rhs._inputUnits) )
		return false;
	if ( !(_outputUnits == rhs._outputUnits) )
		return false;
	if ( !(_approximationType == rhs._approximationType) )
		return false;
	if ( !(_freqLowerBound == rhs._freqLowerBound) )
		return false;
	if ( !(_freqUpperBound == rhs._freqUpperBound) )
		return false;
	if ( !(_approxLowerBound == rhs._approxLowerBound) )
		return false;
	if ( !(_approxUpperBound == rhs._approxUpperBound) )
		return false;
	if ( !(_maxError == rhs._maxError) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Polynomial::setInputUnits(const std::string& inputUnits) {
	_inputUnits = inputUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Polynomial::inputUnits() const {
	return _inputUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Polynomial::setOutputUnits(const std::string& outputUnits) {
	_outputUnits = outputUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Polynomial::outputUnits() const {
	return _outputUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Polynomial::setApproximationType(ApproximationType approximationType) {
	_approximationType = approximationType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ApproximationType Polynomial::approximationType() const {
	return _approximationType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Polynomial::setFreqLowerBound(const FrequencyType& freqLowerBound) {
	_freqLowerBound = freqLowerBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FrequencyType& Polynomial::freqLowerBound() {
	return _freqLowerBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const FrequencyType& Polynomial::freqLowerBound() const {
	return _freqLowerBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Polynomial::setFreqUpperBound(const FrequencyType& freqUpperBound) {
	_freqUpperBound = freqUpperBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FrequencyType& Polynomial::freqUpperBound() {
	return _freqUpperBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const FrequencyType& Polynomial::freqUpperBound() const {
	return _freqUpperBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Polynomial::setApproxLowerBound(const FrequencyType& approxLowerBound) {
	_approxLowerBound = approxLowerBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FrequencyType& Polynomial::approxLowerBound() {
	return _approxLowerBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const FrequencyType& Polynomial::approxLowerBound() const {
	return _approxLowerBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Polynomial::setApproxUpperBound(const FrequencyType& approxUpperBound) {
	_approxUpperBound = approxUpperBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FrequencyType& Polynomial::approxUpperBound() {
	return _approxUpperBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const FrequencyType& Polynomial::approxUpperBound() const {
	return _approxUpperBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Polynomial::setMaxError(const FrequencyType& maxError) {
	_maxError = maxError;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FrequencyType& Polynomial::maxError() {
	return _maxError;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const FrequencyType& Polynomial::maxError() const {
	return _maxError;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Polynomial& Polynomial::operator=(const Polynomial& other) {
	_inputUnits = other._inputUnits;
	_outputUnits = other._outputUnits;
	_approximationType = other._approximationType;
	_freqLowerBound = other._freqLowerBound;
	_freqUpperBound = other._freqUpperBound;
	_approxLowerBound = other._approxLowerBound;
	_approxUpperBound = other._approxUpperBound;
	_maxError = other._maxError;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Polynomial::coefficientCount() const {
	return _coefficients.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PolynomialCoefficient* Polynomial::coefficient(size_t i) const {
	return _coefficients[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Polynomial::addCoefficient(PolynomialCoefficient* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_coefficients.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Polynomial::removeCoefficient(PolynomialCoefficient* obj) {
	if ( obj == NULL )
		return false;

	std::vector<PolynomialCoefficientPtr>::iterator it;
	it = std::find(_coefficients.begin(), _coefficients.end(), obj);
	// Element has not been found
	if ( it == _coefficients.end() ) {
		SEISCOMP_ERROR("Polynomial::removeCoefficient(PolynomialCoefficient*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Polynomial::removeCoefficient(size_t i) {
	// index out of bounds
	if ( i >= _coefficients.size() )
		return false;

	_coefficients.erase(_coefficients.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
