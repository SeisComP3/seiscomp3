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


#include "hann.h"


namespace Seiscomp {
namespace Math {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename TYPE>
void HannWindow<TYPE>::process(int n, TYPE *inout, double left, double right) const {
	// Left side
	TYPE n2 = n * left;
	if ( n2 > n ) n2 = n;
	int in2 = int(n2);
	int w = in2 * 2;
	double scale;

	if ( w > 1 ) {
		scale = 1.0 / (w-1);
		for ( int i = 0; i < in2; ++i ) {
			inout[i] *= 0.5*(1-cos(2*M_PI*i*scale));
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
		scale = 1.0 / (w-1);
		for ( int i = 0; i < in2; ++i ) {
			inout[n-in2+i] *= 0.5*(1-cos(2*M_PI*(i+in2)*scale));
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template class SC_SYSTEM_CORE_API HannWindow<float>;
template class SC_SYSTEM_CORE_API HannWindow<double>;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
