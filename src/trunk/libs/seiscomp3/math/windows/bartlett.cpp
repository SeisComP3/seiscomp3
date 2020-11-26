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


#include "bartlett.h"


namespace Seiscomp {
namespace Math {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename TYPE>
void BartlettWindow<TYPE>::process(int n, TYPE *inout, double left, double right) const {
	TYPE a0, a1;

	// Left side
	TYPE n2 = n * left;
	if ( n2 > n ) n2 = n;
	int in2 = int(n2);
	int w = in2 * 2;

	if ( w > 1 ) {
		a0 = 2.0 / (w-1);
		a1 = (w-1)*0.5;

		for ( int i = 0; i < in2; ++i ) {
			inout[i] *= 1-fabs((i-a1))*a0;
		}
	}

	// Right side
	if ( left != right ) {
		n2 = n * right;
		if ( n2 > n ) n2 = n;
		in2 = int(n2);
		w = in2 * 2;
	}

	if ( w > 1 ) {
		a0 = 2.0 / (w-1);
		a1 = (w-1)*0.5;

		for ( int i = 0; i < in2; ++i ) {
			inout[n-in2+i] *= 1-fabs(in2+i-a1)*a0;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template class SC_SYSTEM_CORE_API BartlettWindow<float>;
template class SC_SYSTEM_CORE_API BartlettWindow<double>;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
