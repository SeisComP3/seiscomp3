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


#include <seiscomp3/math/filter/const.h>

namespace Seiscomp
{
namespace Math
{
namespace Filtering
{

template<typename T>
ConstFilter<T>::ConstFilter(T c) : _const(c) {}


template<typename T>
void ConstFilter<T>::setSamplingFrequency(double fsamp) {}


template<typename T>
int ConstFilter<T>::setParameters(int n, const double *params) {
	if ( n != 1 ) return 1;
	_const = (T)params[0];
	return n;
}


template<typename T>
void ConstFilter<T>::apply(int n, T *inout) {
	for ( int i = 0; i < n; ++i )
		inout[i] = _const;
}


template<typename T>
InPlaceFilter<T>* ConstFilter<T>::clone() const {
	return new ConstFilter<T>(_const);
}


INSTANTIATE_INPLACE_FILTER(ConstFilter, SC_SYSTEM_CORE_API);


}
}
}
