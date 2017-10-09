/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Author: Jan Becker, gempa GmbH                                        *
 ***************************************************************************/


#include "blackman.h"


namespace Seiscomp {
namespace Math {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename TYPE>
BlackmanWindow<TYPE>::BlackmanWindow(TYPE alpha) : _alpha(alpha) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename TYPE>
void BlackmanWindow<TYPE>::process(int n, TYPE *inout, double width) const {
	if ( width > 0.5 ) width = 0.5;
	else if ( width < 0 ) width = 0;

	TYPE n2 = n*width;
	if ( n2 > n ) n2 = n;
	int in2 = (int)n2;
	int w = in2*2;

	TYPE a0 = (1-_alpha)*0.5,
	     a1 = 0.5,
	     a2 = _alpha*0.5,
	     o = 1.0/(w-1);

	for ( int i = 0; i < in2; ++i ) {
		inout[i] *= a0 - a1*cos(2*M_PI*i*o) + a2*cos(4*M_PI*i*o);
		inout[n-in2+i] *= a0 - a1*cos(2*M_PI*(in2+i)*o) + a2*cos(4*M_PI*(in2+i)*o);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template class SC_SYSTEM_CORE_API BlackmanWindow<float>;
template class SC_SYSTEM_CORE_API BlackmanWindow<double>;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
