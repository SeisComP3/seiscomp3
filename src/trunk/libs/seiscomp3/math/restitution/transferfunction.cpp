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


void lagrange(const SeismometerResponse::FAP *faps, int order, double freq,
              double &amp, double &phaseAngle) {
	amp = phaseAngle = 0.0;

	// Waterlevel
	if ( freq <= 1.0e-20 )
		freq = 1.0e-20;

	// Lagrange interpolation works better for log func.
	freq = log10(freq);

	for ( int k = 0; k < order; ++k ) {
		double prod = 1.0;
		double logfk = faps[k].frequency;
		if ( logfk <= 1.0e-20 ) logfk = 1.0e-20;
		logfk = log10(logfk);
		for ( int i = 0; i < order; ++i ) {
			if ( i != k ) {
				double logfi = faps[i].frequency;
				if ( logfi <= 1.0e-20 ) logfi = 1.0e-20;
				logfi = log10(logfi);

				prod *= (freq-logfi) / (logfk-logfi);
			}
		}

		amp += prod * faps[k].amplitude;
		phaseAngle += prod * faps[k].phaseAngle;
	}
}


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


ResponseList::ResponseList(const SeismometerResponse::FAPs &faps, int addZeros)
: faps(faps)
, nZeros(addZeros) {}


ResponseList::ResponseList(int n_tuples, const SeismometerResponse::FAP *faps,
                           int addZeros)
: nZeros(addZeros) {
	this->faps.assign(faps, faps+n_tuples);
}


void ResponseList::evaluate_(Complex *out, int n, const double *x) const {
	for ( int i = 0; i < n; ++i ) {
		size_t k;
		for ( k = 0; k < faps.size(); ++k ) {
			if ( x[i] < faps[k].frequency )
				break;
		}

		int j = k-2;
		int order = 4;

		if ( j < 0 ) {
			j = 0;
			order = 2;
		}
		else if ( j > (int)faps.size()-4 ) {
			j = faps.size()-2;
			order = 2;
		}

		double amp, phase;
		lagrange(&faps[j], order, x[i], amp, phase);

		out[i] = Complex(amp*cos(deg2rad(phase)), amp*sin(deg2rad(phase)));

		for ( int z = 0; z < nZeros; ++z )
			out[i] *= Complex(0, x[i]*TWO_PI);
	}
}


void ResponseList::deconvolve_(int n, Complex *spec, double startFreq, double df) const {
	double f = startFreq;
	for ( int i = 0; i < n; ++i, f += df ) {
		Complex y;
		evaluate_(&y, 1, &f);
		spec[i] /= y;
	}
}


void ResponseList::convolve_(int n, Complex *spec, double startFreq, double df) const {
	double f = startFreq;
	for ( int i = 0; i < n; ++i, f += df ) {
		Complex y;
		evaluate_(&y, 1, &f);
		spec[i] *= y;
	}
}


}
}
}
}
