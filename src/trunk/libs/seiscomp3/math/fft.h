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



#ifndef __SEISCOMP_MATH_FFT_H__
#define __SEISCOMP_MATH_FFT_H__


#include <complex>
#include <vector>
#include <seiscomp3/math/math.h>


namespace Seiscomp {
namespace Math {


typedef std::complex<double> Complex;
typedef std::vector<Math::Complex> ComplexArray;


template <typename T>
void fft(ComplexArray &spec, int n, const T *data);

template <typename T>
void fft(ComplexArray &spec, const std::vector<T> &data) {
	fft(spec, (int)data.size(), &data[0]);
}


template <typename T>
void ifft(int n, T *out, ComplexArray &spec);

template <typename T>
void ifft(std::vector<T> &out, ComplexArray &spec) {
	ifft((int)out.size(), &out[0], spec);
}


}
}


#endif
