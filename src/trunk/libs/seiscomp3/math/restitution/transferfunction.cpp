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



#include <seiscomp3/math/restitution/transferfunction.h>
#include <iostream>


namespace Seiscomp {
namespace Math {
namespace Restitution {
namespace FFT {


#define TWO_PI (M_PI*2)


namespace {


class TransferFunctionProduct : public TransferFunction {
	public:
		//! Pointers are not managed within this object.
		TransferFunctionProduct(const TransferFunction *a, const TransferFunction *b)
		: _a(a), _b(b) {}

	protected:
		void evaluate_(Complex *out, int n, const double *x) const {
			_a->evaluate(out, n, x);
			for ( int i = 0; i < n; ++i ) {
				Complex tmp;
				_b->evaluate(&tmp, 1, &x[i]);
				out[i] *= tmp;
			}
		}

		void deconvolve_(int n, Complex *spec, double startFreq, double df) const {
			_a->deconvolve(n, spec, startFreq, df);
			_b->deconvolve(n, spec, startFreq, df);
		}

		void convolve_(int n, Complex *spec, double startFreq, double df) const {
			_a->convolve(n, spec, startFreq, df);
			_b->convolve(n, spec, startFreq, df);
		}

	private:
		const TransferFunction *_a;
		const TransferFunction *_b;
};


class TransferFunctionRatio : public TransferFunction {
	public:
		//! Pointers are not managed within this object.
		TransferFunctionRatio(const TransferFunction *a, const TransferFunction *b)
		: _a(a), _b(b) {}

	protected:
		void evaluate_(Complex *out, int n, const double *x) const {
			_a->evaluate(out, n, x);
			for ( int i = 0; i < n; ++i ) {
				Complex tmp;
				_b->evaluate(&tmp, 1, &x[i]);
				out[i] /= tmp;
			}
		}

		void deconvolve_(int n, Complex *spec, double startFreq, double df) const {
			_a->deconvolve(n, spec, startFreq, df);
			_b->convolve(n, spec, startFreq, df);
		}

		void convolve_(int n, Complex *spec, double startFreq, double df) const {
			_a->convolve(n, spec, startFreq, df);
			_b->deconvolve(n, spec, startFreq, df);
		}

	private:
		const TransferFunction *_a;
		const TransferFunction *_b;
};



}


TransferFunction *TransferFunction::operator*(const TransferFunction &a) const {
	return new TransferFunctionProduct(this, &a);
}


TransferFunction *TransferFunction::operator/(const TransferFunction &a) const {
	return new TransferFunctionRatio(this, &a);
}


void TransferFunction::deconvolve_(int n, Complex *spec, double startFreq, double df) const {
	for ( int i = 0; i < n; ++i ) {
		double x = startFreq + i*df;

		Complex y;
		evaluate(&y, 1, &x);

		spec[i] = spec[i] / y;
	}
}


void TransferFunction::convolve_(int n, Complex *spec, double startFreq, double df) const {
	for ( int i = 0; i < n; ++i ) {
		double x = startFreq + i*df;

		Complex y;
		evaluate(&y, 1, &x);

		spec[i] = spec[i] * y;
	}
}


PolesAndZeros::PolesAndZeros(const SeismometerResponse::PolesAndZeros &polesAndZeros)
: paz(polesAndZeros) {}


PolesAndZeros::PolesAndZeros(int n_poles, Pole *poles, int n_zeros, Zero *zeros, double norm, int addZeros) {
	paz.poles.assign(poles, poles + n_poles);
	paz.zeros.assign(zeros, zeros + n_zeros);
	for ( int i = 0; i < addZeros; ++i )
		paz.zeros.push_back(0.0);
	paz.norm = norm;
}


void PolesAndZeros::evaluate_(Complex *out, int n, const double *x) const {
	for ( int i = 0; i < n; ++i ) {
		Complex res(paz.norm, 0.0);
		Complex iomeg(0, x[i]*TWO_PI);
		for ( size_t z = 0; z < paz.zeros.size(); ++z ) {
			res *= iomeg - paz.zeros[z];
		}

		for ( size_t p = 0; p < paz.poles.size(); ++p ) {
			res /= iomeg - paz.poles[p];
		}

		out[i] = res;
	}
}


void PolesAndZeros::deconvolve_(int n, Complex *spec, double startFreq, double df) const {
	for ( int i = 0; i < n; ++i ) {
		double x = TWO_PI * (startFreq + i*df);

		Complex y(1.0 / paz.norm, 0.0);
		for ( size_t p = 0; p < paz.poles.size(); ++p )
			y *= Complex(-paz.poles[p].real(), x - paz.poles[p].imag());

		for ( size_t z = 0; z < paz.zeros.size(); ++z )
			y /= Complex(-paz.zeros[z].real(), x - paz.zeros[z].imag());

		spec[i] *= y;
	}
}


void PolesAndZeros::convolve_(int n, Complex *spec, double startFreq, double df) const {
	for ( int i = 0; i < n; ++i ) {
		double x = TWO_PI * (startFreq + i*df);

		Complex y(paz.norm, 0.0);
		for ( size_t z = 0; z < paz.zeros.size(); ++z )
			y *= Complex(-paz.zeros[z].real(), x - paz.zeros[z].imag());

		for ( size_t p = 0; p < paz.poles.size(); ++p )
			y /= Complex(-paz.poles[p].real(), x - paz.poles[p].imag());

		spec[i] *= y;
	}
}


}
}
}
}
