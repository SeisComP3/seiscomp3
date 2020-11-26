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

namespace Seiscomp {
namespace Math {
namespace Filtering {


// returns the next power of 2 which is greater or equal n
long next_power_of_2(long n) {
	int i = 1;

	if ( n <= 0 ) return 0;
	while ( i < n) i <<= 1;
	return i;
}

template<typename T>
void cosRamp(std::vector<T> &ramp, T f1, T f2) {
	int n = ramp.size();
	double df = 0.5*(f2-f1), x = M_PI/n;
	for (int i = 0; i < n; ++i )
		ramp[i] = f1 + T(df*(1-cos(i*x)));
}


template <typename T>
void cosRamp(size_t n, T *inout, size_t istart, size_t iend, size_t estart, size_t eend) {
	size_t taperLength = iend - istart;

	for ( size_t i = 0; i < istart; ++i )
		inout[i] = 0;

	for ( size_t i = 0; i < taperLength; ++i ) {
		double frac = double(i)/taperLength;
		inout[istart+i] *= 0.5*(1-cos(M_PI*frac));
	}

	taperLength = eend - estart;

	for ( size_t i = 0; i < taperLength; ++i ) {
		double frac = double(i)/taperLength;
		inout[estart+i] *= 0.5*(1+cos(M_PI*frac));
	}

	for ( size_t i = eend; i < n; ++i )
		inout[i] = 0;
}


template void cosRamp<float>(std::vector<float> &ramp, float f1, float f2);
template void cosRamp<double>(std::vector<double> &ramp, double f1, double f2);

template void cosRamp<float>(size_t, float *, size_t, size_t, size_t, size_t);
template void cosRamp<double>(size_t, double *, size_t, size_t, size_t, size_t);


}	// namespace Seiscomp::Math::Filtering
}	// namespace Seiscomp::Math
}	// namespace Seiscomp
