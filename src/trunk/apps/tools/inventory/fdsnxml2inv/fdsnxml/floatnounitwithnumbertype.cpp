/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <fdsnxml/floatnounitwithnumbertype.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace FDSNXML {


FloatNoUnitWithNumberType::MetaObject::MetaObject(const Core::RTTI *rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(objectProperty<CounterType>("number", "FDSNXML::CounterType", false, true, &FloatNoUnitWithNumberType::setNumber, &FloatNoUnitWithNumberType::number));
}


IMPLEMENT_RTTI(FloatNoUnitWithNumberType, "FDSNXML::FloatNoUnitWithNumberType", FloatNoUnitType)
IMPLEMENT_RTTI_METHODS(FloatNoUnitWithNumberType)
IMPLEMENT_METAOBJECT_DERIVED(FloatNoUnitWithNumberType, FloatNoUnitType)


FloatNoUnitWithNumberType::FloatNoUnitWithNumberType() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FloatNoUnitWithNumberType::FloatNoUnitWithNumberType(const FloatNoUnitWithNumberType &other)
 : FloatNoUnitType() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FloatNoUnitWithNumberType::~FloatNoUnitWithNumberType() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FloatNoUnitWithNumberType::operator==(const FloatNoUnitWithNumberType &rhs) const {
	if ( !(_number == rhs._number) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FloatNoUnitWithNumberType::setNumber(const OPT(CounterType)& number) {
	_number = number;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CounterType& FloatNoUnitWithNumberType::number() {
	if ( _number )
		return *_number;
	throw Seiscomp::Core::ValueException("FloatNoUnitWithNumberType.number is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CounterType& FloatNoUnitWithNumberType::number() const {
	if ( _number )
		return *_number;
	throw Seiscomp::Core::ValueException("FloatNoUnitWithNumberType.number is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FloatNoUnitWithNumberType& FloatNoUnitWithNumberType::operator=(const FloatNoUnitWithNumberType &other) {
	FloatNoUnitType::operator=(other);
	_number = other._number;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
