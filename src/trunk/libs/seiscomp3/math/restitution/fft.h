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



#ifndef __SEISCOMP_MATH_RESTITUTION_FFT_H__
#define __SEISCOMP_MATH_RESTITUTION_FFT_H__


#include <complex>
#include <vector>
#include <seiscomp3/math/math.h>
#include <seiscomp3/math/restitution/types.h>
#include <seiscomp3/math/restitution/transferfunction.h>


namespace Seiscomp {
namespace Math {
namespace Restitution {


// Transforms a time series into the spectra, deconvolves it with the
// transfer function and transforms the spectra back into the time series.
// The spectra is tapered before min_freq and after max_freq. min_freq or
// max_freq has to be greater than 0 otherwise the tapering on the
// corresponding end is disabled.
template <typename T>
bool transformFFT(int n, T *inout, double fsamp, const FFT::TransferFunction *tf,
                  double cutoff, double min_freq, double max_freq);

template <typename T>
bool transformFFT(std::vector<T> &inout, double fsamp, const FFT::TransferFunction *tf,
                  double cutoff, double min_freq, double max_freq) {
	return transformFFT(inout.size(), &inout[0], fsamp, tf, cutoff, min_freq, max_freq);
}


template <typename T>
bool transformFFT(int n, T *inout, double fsamp, int n_poles, SeismometerResponse::Pole *poles,
                  int n_zeros, SeismometerResponse::Zero *zeros, double norm,
                  double cutoff, double min_freq, double max_freq) {
	FFT::PolesAndZeros paz(n_poles, poles, n_zeros, zeros, norm);
	return transformFFT(n, inout, fsamp, &paz, cutoff, min_freq, max_freq);
}

template <typename T>
bool transformFFT(std::vector<T> &inout, double fsamp, const Poles &poles,
                  const Zeros &zeros, double norm, double cutoff,
                  double min_freq, double max_freq) {
	return transformFFT(inout.size(), &inout[0], poles.size(), &poles[0],
	                    zeros.size(), &zeros[0], norm, cutoff, min_freq, max_freq);
}


}
}
}


#endif
