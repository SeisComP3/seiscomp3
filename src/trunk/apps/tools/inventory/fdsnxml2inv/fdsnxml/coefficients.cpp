/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <fdsnxml/coefficients.h>
#include <fdsnxml/floattype.h>
#include <fdsnxml/floattype.h>
#include <algorithm>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace FDSNXML {


namespace {

static Seiscomp::Core::MetaEnumImpl<CfTransferFunctionType> metaCfTransferFunctionType;

}


Coefficients::MetaObject::MetaObject(const Core::RTTI *rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(enumProperty("CfTransferFunctionType", "CfTransferFunctionType", false, false, &metaCfTransferFunctionType, &Coefficients::setCfTransferFunctionType, &Coefficients::cfTransferFunctionType));
	addProperty(arrayClassProperty<FloatType>("Numerator", "FDSNXML::FloatType", &Coefficients::numeratorCount, &Coefficients::numerator, static_cast<bool (Coefficients::*)(FloatType*)>(&Coefficients::addNumerator), &Coefficients::removeNumerator, static_cast<bool (Coefficients::*)(FloatType*)>(&Coefficients::removeNumerator)));
	addProperty(arrayClassProperty<FloatType>("Denominator", "FDSNXML::FloatType", &Coefficients::denominatorCount, &Coefficients::denominator, static_cast<bool (Coefficients::*)(FloatType*)>(&Coefficients::addDenominator), &Coefficients::removeDenominator, static_cast<bool (Coefficients::*)(FloatType*)>(&Coefficients::removeDenominator)));
}


IMPLEMENT_RTTI(Coefficients, "FDSNXML::Coefficients", BaseFilter)
IMPLEMENT_RTTI_METHODS(Coefficients)
IMPLEMENT_METAOBJECT_DERIVED(Coefficients, BaseFilter)


Coefficients::Coefficients() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Coefficients::Coefficients(const Coefficients &other)
 : BaseFilter() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Coefficients::~Coefficients() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Coefficients::operator==(const Coefficients &rhs) const {
	if ( !(_cfTransferFunctionType == rhs._cfTransferFunctionType) )
		return false;
	return true;
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
Coefficients& Coefficients::operator=(const Coefficients &other) {
	BaseFilter::operator=(other);
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
bool Coefficients::addNumerator(FloatType *obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_numerators.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Coefficients::removeNumerator(FloatType *obj) {
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
bool Coefficients::addDenominator(FloatType *obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_denominators.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Coefficients::removeDenominator(FloatType *obj) {
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
