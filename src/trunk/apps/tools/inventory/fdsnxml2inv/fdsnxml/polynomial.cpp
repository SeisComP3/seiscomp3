/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <fdsnxml/polynomial.h>
#include <fdsnxml/polynomialcoefficient.h>
#include <algorithm>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace FDSNXML {


namespace {

static Seiscomp::Core::MetaEnumImpl<ApproximationType> metaApproximationType;

}


Polynomial::MetaObject::MetaObject(const Core::RTTI *rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(enumProperty("ApproximationType", "ApproximationType", false, false, &metaApproximationType, &Polynomial::setApproximationType, &Polynomial::approximationType));
	addProperty(objectProperty<FrequencyType>("FrequencyLowerBound", "FDSNXML::FrequencyType", false, false, &Polynomial::setFrequencyLowerBound, &Polynomial::frequencyLowerBound));
	addProperty(objectProperty<FrequencyType>("FrequencyUpperBound", "FDSNXML::FrequencyType", false, false, &Polynomial::setFrequencyUpperBound, &Polynomial::frequencyUpperBound));
	addProperty(Core::simpleProperty("ApproximationLowerBound", "float", false, false, false, false, false, false, NULL, &Polynomial::setApproximationLowerBound, &Polynomial::approximationLowerBound));
	addProperty(Core::simpleProperty("ApproximationUpperBound", "float", false, false, false, false, false, false, NULL, &Polynomial::setApproximationUpperBound, &Polynomial::approximationUpperBound));
	addProperty(Core::simpleProperty("MaximumError", "float", false, false, false, false, false, false, NULL, &Polynomial::setMaximumError, &Polynomial::maximumError));
	addProperty(arrayClassProperty<PolynomialCoefficient>("Coefficient", "FDSNXML::PolynomialCoefficient", &Polynomial::coefficientCount, &Polynomial::coefficient, static_cast<bool (Polynomial::*)(PolynomialCoefficient*)>(&Polynomial::addCoefficient), &Polynomial::removeCoefficient, static_cast<bool (Polynomial::*)(PolynomialCoefficient*)>(&Polynomial::removeCoefficient)));
}


IMPLEMENT_RTTI(Polynomial, "FDSNXML::Polynomial", BaseFilter)
IMPLEMENT_RTTI_METHODS(Polynomial)
IMPLEMENT_METAOBJECT_DERIVED(Polynomial, BaseFilter)


Polynomial::Polynomial() {
	_approximationLowerBound = 0;
	_approximationUpperBound = 0;
	_maximumError = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Polynomial::Polynomial(const Polynomial &other)
 : BaseFilter() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Polynomial::~Polynomial() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Polynomial::operator==(const Polynomial &rhs) const {
	if ( !(_approximationType == rhs._approximationType) )
		return false;
	if ( !(_frequencyLowerBound == rhs._frequencyLowerBound) )
		return false;
	if ( !(_frequencyUpperBound == rhs._frequencyUpperBound) )
		return false;
	if ( !(_approximationLowerBound == rhs._approximationLowerBound) )
		return false;
	if ( !(_approximationUpperBound == rhs._approximationUpperBound) )
		return false;
	if ( !(_maximumError == rhs._maximumError) )
		return false;
	return true;
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
void Polynomial::setFrequencyLowerBound(const FrequencyType& frequencyLowerBound) {
	_frequencyLowerBound = frequencyLowerBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FrequencyType& Polynomial::frequencyLowerBound() {
	return _frequencyLowerBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const FrequencyType& Polynomial::frequencyLowerBound() const {
	return _frequencyLowerBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Polynomial::setFrequencyUpperBound(const FrequencyType& frequencyUpperBound) {
	_frequencyUpperBound = frequencyUpperBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FrequencyType& Polynomial::frequencyUpperBound() {
	return _frequencyUpperBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const FrequencyType& Polynomial::frequencyUpperBound() const {
	return _frequencyUpperBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Polynomial::setApproximationLowerBound(double approximationLowerBound) {
	_approximationLowerBound = approximationLowerBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Polynomial::approximationLowerBound() const {
	return _approximationLowerBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Polynomial::setApproximationUpperBound(double approximationUpperBound) {
	_approximationUpperBound = approximationUpperBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Polynomial::approximationUpperBound() const {
	return _approximationUpperBound;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Polynomial::setMaximumError(double maximumError) {
	_maximumError = maximumError;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Polynomial::maximumError() const {
	return _maximumError;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Polynomial& Polynomial::operator=(const Polynomial &other) {
	BaseFilter::operator=(other);
	_approximationType = other._approximationType;
	_frequencyLowerBound = other._frequencyLowerBound;
	_frequencyUpperBound = other._frequencyUpperBound;
	_approximationLowerBound = other._approximationLowerBound;
	_approximationUpperBound = other._approximationUpperBound;
	_maximumError = other._maximumError;
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
bool Polynomial::addCoefficient(PolynomialCoefficient *obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_coefficients.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Polynomial::removeCoefficient(PolynomialCoefficient *obj) {
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
