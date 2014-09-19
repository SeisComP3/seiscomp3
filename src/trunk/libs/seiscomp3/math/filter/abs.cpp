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
#include <seiscomp3/math/filter/abs.h>

namespace Seiscomp
{
namespace Math
{
namespace Filtering
{


template<typename T>
AbsFilter<T>::AbsFilter() {}


template<typename T>
void AbsFilter<T>::setSamplingFrequency(double fsamp) {}


template<typename T>
int AbsFilter<T>::setParameters(int n, const double *params) {
	return 0;
}


template<typename T>
void AbsFilter<T>::apply(int n, T *inout) {
	for ( int i = 0; i < n; ++i )
		inout[i] = (T)fabs(inout[i]);
}


template<typename T>
InPlaceFilter<T>* AbsFilter<T>::clone() const {
	return new AbsFilter<T>();
}


INSTANTIATE_INPLACE_FILTER(AbsFilter, SC_SYSTEM_CORE_API);


}
}
}

