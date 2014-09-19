/************************************************************************
 *                                                                      *
 * Copyright (C) 2012 OVSM/IPGP                                         *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * This program is part of 'Projet TSUAREG - INTERREG IV Caraïbes'.     *
 * It has been co-financed by the European Union and le Ministère de    *
 * l'Ecologie, du Développement Durable, des Transports et du Logement. *
 *                                                                      *
 ************************************************************************/

#define SEISCOMP_COMPONENT L4C_1Hz

#include "l4c1hz.h"
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace Math {

namespace SeismometerResponse {

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
L4C_1Hz::L4C_1Hz(GroundMotion input) {

	poles.clear();
	zeros.clear();

	poles.push_back(Pole(-4.33452, -4.54866));
	poles.push_back(Pole(-4.33452, +4.54866));

	zeros.push_back(.0);
	zeros.push_back(.0);

	norm = .999568;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}// namespace SeismometerResponse



namespace Filtering {

namespace {

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GroundMotion double2gm(double v, bool& error) {

	int value = (int) v;
	error = false;
	switch ( value ) {
		case 0:
			return Math::Displacement;
		case 1:
			return Math::Velocity;
		case 2:
			return Math::Acceleration;
		default:
			error = true;
		break;
	}

	return Math::Velocity;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}



namespace IIR {

REGISTER_INPLACE_FILTER(L4C_1Hz_Filter, "L4C_1Hz_Filter");


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
L4C_1Hz_Filter<T>::L4C_1Hz_Filter(GroundMotion input) {
	setInput(input);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
L4C_1Hz_Filter<T>::L4C_1Hz_Filter(const L4C_1Hz_Filter& other) :
		Filter<T>(other) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
int L4C_1Hz_Filter<T>::setParameters(int n, const double* params) {

	if ( n != 1 )
		return 1;

	bool error;
	GroundMotion input = double2gm(params[0], error);

	if ( error )
		return -1;

	setInput(input);

	return n;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
Math::Filtering::InPlaceFilter<T>* L4C_1Hz_Filter<T>::clone() const {
	return new L4C_1Hz_Filter(*this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void L4C_1Hz_Filter<T>::setInput(GroundMotion input) {

	Filter<T>::poles.clear();
	Filter<T>::zeros.clear();

	Filter<T>::poles.push_back(std::complex<double>(-4.33452, -4.54866));
	Filter<T>::poles.push_back(std::complex<double>(-4.33452, +4.54866));

	Filter<T>::zeros.push_back(.0);
	Filter<T>::zeros.push_back(.0);

	Filter<T>::norm = .999568;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


INSTANTIATE_INPLACE_FILTER(L4C_1Hz_Filter, SC_SYSTEM_CORE_API);


} // namespace IIR
} // namespace Filtering

} // namespace Math
} // namspace Seiscomp
