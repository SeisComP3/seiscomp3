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


#include <seiscomp3/math/filter/iirdifferentiate.h>

namespace Seiscomp
{
namespace Math
{
namespace Filtering
{


template <typename T>
IIRDifferentiate<T>::IIRDifferentiate(double fsamp) : _fsamp(fsamp) {
	reset();
	setSamplingFrequency(fsamp);
}


template <typename T>
IIRDifferentiate<T>::IIRDifferentiate(const IIRDifferentiate<T> &other) {
	_fsamp = 0.0;
	reset();
}


template <typename T>
void IIRDifferentiate<T>::reset() {
	_v1 = 0;
	_init = false;
}


template <typename T>
void IIRDifferentiate<T>::setSamplingFrequency(double fsamp) {
	_fsamp = fsamp;
}


template <typename T>
int IIRDifferentiate<T>::setParameters(int n, const double *params) {
	if ( n != 0 ) return 0;
	return n;
}


template <typename T>
void IIRDifferentiate<T>::apply(int n, T *inout) {
	for ( int i = 0;  i < n; ++i ) {
		T v = inout[i];
		if ( !_init ) {
			inout[i] = 0;
			_init = true;
		}
		else
			inout[i] = (v-_v1)*_fsamp;
		_v1 = v;
	}
}


template <typename T>
InPlaceFilter<T>* IIRDifferentiate<T>::clone() const {
	return new IIRDifferentiate<T>(_fsamp);
}


INSTANTIATE_INPLACE_FILTER(IIRDifferentiate, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(IIRDifferentiate, "DIFF");


}
}
}
