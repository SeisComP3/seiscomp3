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


#include <seiscomp3/math/filter/iirintegrate.h>

namespace Seiscomp
{
namespace Math
{
namespace Filtering
{


template <typename T>
IIRIntegrate<T>::IIRIntegrate(double a, double fsamp) {
	init(a);
	setSamplingFrequency(fsamp);
}


template <typename T>
IIRIntegrate<T>::IIRIntegrate(const IIRIntegrate<T> &other)
 : _ia0(other._ia0), _ia1(other._ia1), _ia2(other._ia2),
   _a0(other._a0), _a1(other._a1), _a2(other._a2),
   _b0(other._b0), _b1(other._b1), _b2(other._b2)
{
	reset();
}


template <typename T>
void IIRIntegrate<T>::init(double a) {
	_ia0 = _a0 = (3.0-a)/6.0;
	_ia1 = _a1 = 2.0*(3.0+a)/6.0;
	_ia2 = _a2 = (3.0-a)/6.0;

	_b0 = 1;
	_b1 = 0;
	_b2 = -1;

	_v1 = _v2 = 0;
}


template <typename T>
void IIRIntegrate<T>::reset() {
	_v1 = _v2 = 0;
}


template <typename T>
void IIRIntegrate<T>::setSamplingFrequency(double fsamp) {
	if ( !fsamp ) return;

	double dt = 1./fsamp;

	_a0 = _ia0 * dt;
	_a1 = _ia1 * dt;
	_a2 = _ia2 * dt;
}


template <typename T>
int IIRIntegrate<T>::setParameters(int n, const double *params) {
	if ( n != 1 ) return 1;
	init(params[0]);
	return n;
}


template <typename T>
void IIRIntegrate<T>::apply(int n, T *inout) {
	for ( int i = 0;  i < n; ++i ) {
		double v = inout[i];
		double v0 = _b0*v - _b1*_v1 - _b2*_v2;
		inout[i] =  T(_a0*v0 + _a1*_v1 + _a2*_v2);
		_v2 = _v1; _v1 = (T)v0;
	}
}


template <typename T>
InPlaceFilter<T>* IIRIntegrate<T>::clone() const {
	return new IIRIntegrate<T>(*this);
}


INSTANTIATE_INPLACE_FILTER(IIRIntegrate, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(IIRIntegrate, "INT");


}
}
}
