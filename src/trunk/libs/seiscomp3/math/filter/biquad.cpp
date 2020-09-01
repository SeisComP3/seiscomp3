/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#include <math.h>
#include <vector>
#include <ostream>
#include <iostream>

#include<seiscomp3/math/filter/biquad.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {
namespace IIR {


// load the template class definitions
#include<seiscomp3/math/filter/biquad.ipp>


BiquadCoefficients::BiquadCoefficients(double b0, double b1, double b2,
                                       double a0, double a1, double a2) {
	set(b0, b1, b2, a0, a1, a2);
}


BiquadCoefficients::BiquadCoefficients(BiquadCoefficients const &bq)
	: b0(bq.b0), b1(bq.b1), b2(bq.b2), a0(bq.a0), a1(bq.a1), a2(bq.a2) {}


void BiquadCoefficients::set(double b0, double b1, double b2,
                             double a0, double a1, double a2) {
	this->b0 = b0;
	this->b1 = b1;
	this->b2 = b2;
	this->a0 = a0;
	this->a1 = a1;
	this->a2 = a2;
}


std::ostream &operator<<(std::ostream &os, const BiquadCoefficients &biq) {
	os << "b: " << biq.b0 << ", " << biq.b1 << ", " << biq.b2 << std::endl
	   << "a: " << biq.a0 << ", " << biq.a1 << ", " << biq.a2 << std::endl;
	return os;
}


template class SC_SYSTEM_CORE_API Biquad<float>;
template class SC_SYSTEM_CORE_API Biquad<double>;

template class SC_SYSTEM_CORE_API BiquadCascade<float>;
template class SC_SYSTEM_CORE_API BiquadCascade<double>;


} // namespace Seiscomp::Math::Filtering::IIR
} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp
