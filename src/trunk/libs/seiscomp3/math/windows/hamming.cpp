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


#include "hamming.h"


namespace Seiscomp {
namespace Math {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename TYPE>
void HammingWindow<TYPE>::process(int n, TYPE *inout, double width) const {
	if ( width > 0.5 ) width = 0.5;
	else if ( width < 0 ) width = 0;

	TYPE n2 = n*width;
	if ( n2 > n ) n2 = n;
	int in2 = (int)n2;
	int w = in2*2;
	double scale = 1.0 / (w-1);

	for ( int i = 0; i < in2; ++i ) {
		inout[i] *= 0.54-0.46*cos(2*M_PI*i*scale);
		inout[n-in2+i] *= 0.54-0.46*cos(2*M_PI*(i+in2)*scale);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template class SC_SYSTEM_CORE_API HammingWindow<float>;
template class SC_SYSTEM_CORE_API HammingWindow<double>;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
