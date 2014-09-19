/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <fdsnxml/fir.h>
#include <fdsnxml/numeratorcoefficient.h>
#include <algorithm>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace FDSNXML {


namespace {

static Seiscomp::Core::MetaEnumImpl<SymmetryType> metaSymmetryType;

}


FIR::MetaObject::MetaObject(const Core::RTTI *rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(enumProperty("Symmetry", "SymmetryType", false, false, &metaSymmetryType, &FIR::setSymmetry, &FIR::symmetry));
	addProperty(arrayClassProperty<NumeratorCoefficient>("NumeratorCoefficient", "FDSNXML::NumeratorCoefficient", &FIR::numeratorCoefficientCount, &FIR::numeratorCoefficient, static_cast<bool (FIR::*)(NumeratorCoefficient*)>(&FIR::addNumeratorCoefficient), &FIR::removeNumeratorCoefficient, static_cast<bool (FIR::*)(NumeratorCoefficient*)>(&FIR::removeNumeratorCoefficient)));
}


IMPLEMENT_RTTI(FIR, "FDSNXML::FIR", BaseFilter)
IMPLEMENT_RTTI_METHODS(FIR)
IMPLEMENT_METAOBJECT_DERIVED(FIR, BaseFilter)


FIR::FIR() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FIR::FIR(const FIR &other)
 : BaseFilter() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FIR::~FIR() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FIR::operator==(const FIR &rhs) const {
	if ( !(_symmetry == rhs._symmetry) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FIR::setSymmetry(SymmetryType symmetry) {
	_symmetry = symmetry;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SymmetryType FIR::symmetry() const {
	return _symmetry;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FIR& FIR::operator=(const FIR &other) {
	BaseFilter::operator=(other);
	_symmetry = other._symmetry;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t FIR::numeratorCoefficientCount() const {
	return _numeratorCoefficients.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NumeratorCoefficient* FIR::numeratorCoefficient(size_t i) const {
	return _numeratorCoefficients[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FIR::addNumeratorCoefficient(NumeratorCoefficient *obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_numeratorCoefficients.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FIR::removeNumeratorCoefficient(NumeratorCoefficient *obj) {
	if ( obj == NULL )
		return false;

	std::vector<NumeratorCoefficientPtr>::iterator it;
	it = std::find(_numeratorCoefficients.begin(), _numeratorCoefficients.end(), obj);
	// Element has not been found
	if ( it == _numeratorCoefficients.end() ) {
		SEISCOMP_ERROR("FIR::removeNumeratorCoefficient(NumeratorCoefficient*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FIR::removeNumeratorCoefficient(size_t i) {
	// index out of bounds
	if ( i >= _numeratorCoefficients.size() )
		return false;

	_numeratorCoefficients.erase(_numeratorCoefficients.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
