/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_UTILS_UNITS_H__
#define __SEISCOMP_UTILS_UNITS_H__


#include <seiscomp3/core.h>
#include <string>
#include <map>


namespace Seiscomp {
namespace Util {


struct SC_SYSTEM_CORE_API UnitConversion {
	UnitConversion() {}
	UnitConversion(const std::string &fromUnit_, const std::string &toUnit_,
	               const std::string &toQMLUnit_, const std::string &toSEEDUnit_,
	               double s)
	: fromUnit(fromUnit_)
	, toUnit(toUnit_)
	, toQMLUnit(toQMLUnit_)
	, toSEEDUnit(toSEEDUnit_)
	, scale(s) {}

	std::string fromUnit;
	std::string toUnit;
	std::string toQMLUnit;
	std::string toSEEDUnit;
	double      scale;

	template <typename T>
	T operator()(T value) const;

	//! Convert from input unit to SI unit
	template <typename T>
	T convert(T value) const;

	//! Convert from SI unit back to input unit
	template <typename T>
	T revert(T value) const;
};


/**
 * @brief The UnitConverter class provides a simple interface to retrieve
 *        information to convert a value from one unit to another.
 *
 * The class supports variants of displacement and motion units such as
 * velocity and acceleration.
 *
 * @code
 * double value = 123;
 * double convertedValue;
 * // Convert from m/s
 * const UnitConversion *uc = UnitConverter::get('cm/s');
 * if ( uc != NULL )
 *     convertedValue = uc->convert(value);
 * @endcode
 */
class SC_SYSTEM_CORE_API UnitConverter {
	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Returns a conversion object for a particular input unit.
		 * @param fromUnit The unit to convert from.
		 * @return A pointer to the conversion object. If no conversion is
		 *         available then NULL is returned.
		 */
		static const UnitConversion *get(const std::string &fromUnit);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		typedef std::map<std::string, UnitConversion> ConversionMap;
		static ConversionMap _conversionMap;
};


template <typename T>
inline T UnitConversion::convert(T value) const {
	return value*scale;
}


template <typename T>
inline T UnitConversion::revert(T value) const {
	return value/scale;
}


template <typename T>
inline T UnitConversion::operator()(T value) const {
	return convert(value);
}


}
}


#endif
