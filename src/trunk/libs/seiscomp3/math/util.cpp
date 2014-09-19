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


#include <seiscomp3/math/filter.h>

namespace Seiscomp
{

namespace Math
{

namespace Filtering
{

	
// returns the next power of 2 which is greater or equal n
long next_power_of_2 (long n)
{
	int i = 1;

	if (n<=0)   return 0;
	while (i<n) i<<=1;
	return i;
}

template<typename TYPE>
void cosRamp(std::vector<TYPE> &ramp, TYPE f1, TYPE f2) {
	int n=ramp.size();
	double df = 0.5*(f2-f1), x=M_PI/n;
	for(int i=0; i<n; i++)
		ramp[i] = f1 + (TYPE)(df*(1-cos(i*x)));
}


template void cosRamp<float>(std::vector<float> &ramp, float f1, float f2);
template void cosRamp<double>(std::vector<double> &ramp, double f1, double f2);


}	// namespace Seiscomp::Math::Filtering

}	// namespace Seiscomp::Math

}	// namespace Seiscomp
