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


#include <seiscomp3/math/filter/iirfilter.h>

namespace Seiscomp
{
namespace Math
{
namespace Filtering
{


template <typename T>
IIRFilter<T>::IIRFilter() {
}


template <typename T>
IIRFilter<T>::IIRFilter(int na, int nb, const double *a, const double *b) {
	setCoefficients(na, nb, a, b);
}


template <typename T>
IIRFilter<T>::~IIRFilter() {
}


template <typename T>
void IIRFilter<T>::setCoefficients(int na, int nb, const double *a, const double *b) {
	_na = na;
	_nb = nb;

	_a.assign(a, a+na);
	_b.assign(b, b+nb);

	int n = std::max<int>(na, nb);
	_lastValues.resize(n, 0);
}


template <typename T>
void IIRFilter<T>::setSamplingFrequency(double fsamp) {}


template <typename T>
int IIRFilter<T>::setParameters(int n, const double *params) {
	if ( n < 2 ) return 2;

	// na is first parameter
	int na = (int)params[0];
	int nb = 0;
	const double *a = NULL;
	const double *b = NULL;

	if ( na ) {
		// The a coefficients are stored directly after the
		// first parameters
		a = params + 1;
		// At least parameter nb is missing
		if ( n < na+2 )
			return na+2;

		nb = (int)params[na+1];
		if ( n < na+nb+2 )
			return na+nb+2;

		b = params + na+2;
	}
	else {
		nb = (int)params[1];
		// b coefficients are missing or not complete
		if ( n < nb+2 )
			return nb+2;

		b = params + 2;
	}

	setCoefficients(na, nb, a, b);

	return n;
}


template <typename T>
void IIRFilter<T>::apply(int n, T *inout) {
	/*
	// TODO: Store lastValues
	T sum = 0;

	for ( int i = 0; i < n; ++i ) {
		for ( int k = 0; k < _na; ++k )
			sum += _a[k] * inout[i-k];

		for ( int k = 0; k < _nb; ++k )
			sum += _b[k] * inout[i-k];

		inout[i] = sum;
	}
	*/
}


template <typename T>
InPlaceFilter<T>* IIRFilter<T>::clone() const {
	return new IIRFilter<T>(_na, _nb, &_a[0], &_b[0]);
}


INSTANTIATE_INPLACE_FILTER(IIRFilter, SC_SYSTEM_CORE_API);


}
}
}
