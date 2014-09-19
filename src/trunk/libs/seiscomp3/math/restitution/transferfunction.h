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



#ifndef __SEISCOMP_MATH_RESTITUTION_TRANSFERFUNCTION_H__
#define __SEISCOMP_MATH_RESTITUTION_TRANSFERFUNCTION_H__


#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/math/math.h>
#include <seiscomp3/math/filter/seismometers.h>
#include <seiscomp3/math/restitution/types.h>
#include <vector>
#include <cstdlib>


namespace Seiscomp {
namespace Math {
namespace Restitution {
namespace FFT {


DEFINE_SMARTPOINTER(TransferFunction);

class TransferFunction : public Core::BaseObject {
	public:
		//! Returns a transfer function proxy object that acts as
		//! the product of 'this' and 'a'.
		//! Both input transfer function objects are referenced
		//! through a pointer and not managed by the proxy.
		TransferFunction *operator*(const TransferFunction &a) const;

		//! Returns a transfer function proxy object that acts as
		//! the quotient of 'this' and 'a'.
		//! Both input transfer function objects are referenced
		//! through a pointer and not managed by the proxy.
		TransferFunction *operator/(const TransferFunction &a) const;

		//! Evaluates the transfer function at nodes x and returns the
		//! result in out. Out must have enough space for n samples.
		void evaluate(Complex *out, int n, const double *x) const {
			evaluate_(out, n, x);
		}

		void evaluate(std::vector<Complex> &out, const std::vector<double> &x) const {
			out.resize(x.size());
			evaluate_(&out[0], (int)x.size(), &x[0]);
		}

		//! Devides the spectra by the evaluated nodes of the transfer function
		//! for {startFreq, startFreq + 1*df, ..., startFreq + (n-1)*df}
		void deconvolve(int n, Complex *spec, double startFreq, double df) const {
			deconvolve_(n, spec, startFreq, df);
		}

		//! Convenience wrapper using a vector as output
		void deconvolve(std::vector<Complex> &spec, double startFreq, double df) const {
			deconvolve_((int)spec.size(), &spec[0], startFreq, df);
		}

		//! Multiplies the spectra by the evaluated nodes of the transfer function
		//! for {startFreq, startFreq + 1*df, ..., startFreq + (n-1)*df}
		void convolve(int n, Complex *spec, double startFreq, double df) const {
			convolve_(n, spec, startFreq, df);
		}

		//! Convenience wrapper using a vector as output
		void convolve(std::vector<Complex> &spec, double startFreq, double df) const {
			convolve_((int)spec.size(), &spec[0], startFreq, df);
		}


	protected:
		//! The implementations
		virtual void evaluate_(Complex *out, int n, const double *x) const = 0;
		virtual void deconvolve_(int n, Complex *spec, double startFreq, double df) const;
		virtual void convolve_(int n, Complex *spec, double startFreq, double df) const;
};


class PolesAndZeros : public TransferFunction {
	public:
		PolesAndZeros(const SeismometerResponse::PolesAndZeros &polesAndZeros);
		PolesAndZeros(int n_poles, Pole *poles, int n_zeros, Zero *zeros, double k, int addZeros = 0);

	protected:
		void evaluate_(Complex *out, int n, const double *x) const;
		void deconvolve_(int n, Complex *spec, double startFreq, double df) const;
		void convolve_(int n, Complex *spec, double startFreq, double df) const;

	public:
		SeismometerResponse::PolesAndZeros paz;
};



}
}
}
}


#endif
