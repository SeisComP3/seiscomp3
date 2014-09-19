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


#ifndef _SEISCOMP_MATH_FILTER_ABS_H_
#define _SEISCOMP_MATH_FILTER_ABS_H_

#include <seiscomp3/math/filter.h>

namespace Seiscomp
{
namespace Math
{
namespace Filtering
{

template<typename T>
class AbsFilter : public InPlaceFilter<T> {
	public:
		AbsFilter();

	public:
		void setSamplingFrequency(double fsamp);
		int setParameters(int n, const double *params);

		void apply(int n, T *inout);

		InPlaceFilter<T>* clone() const;
};

}
}
}

#endif

