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


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <complex>

#include<seiscomp3/math/filter/butterworth.h>

namespace Seiscomp {
namespace Math {
namespace Filtering {
namespace IIR {


// load the template class definitions
#include<seiscomp3/math/filter/butterworth.ipp>

INSTANTIATE_INPLACE_FILTER(ButterworthLowpass, SC_SYSTEM_CORE_API);
INSTANTIATE_INPLACE_FILTER(ButterworthHighpass, SC_SYSTEM_CORE_API);
INSTANTIATE_INPLACE_FILTER(ButterworthBandpass, SC_SYSTEM_CORE_API);
INSTANTIATE_INPLACE_FILTER(ButterworthHighLowpass, SC_SYSTEM_CORE_API);

REGISTER_INPLACE_FILTER(ButterworthLowpass, "BW_LP");
REGISTER_INPLACE_FILTER(ButterworthHighpass, "BW_HP");
REGISTER_INPLACE_FILTER(ButterworthBandpass, "BW_BP");
REGISTER_INPLACE_FILTER(ButterworthHighLowpass, "BW_HLP");
REGISTER_INPLACE_FILTER2(ButterworthHighLowpass, Proxy, "BW");


static void _bw_coeff_lp_hp(double phi, double L, int type, double *a, double *b)
{
	double LL=L*L;
	double ib0;

	if (phi==M_PI) {
		// special case of 1st-order stage
		if (type == BUTTER_TYPE_HIGH) {
			a[0] = L/(1.+L);
			a[1] = -a[0];
		}
		else { // BUTTER_TYPE_LOW
			a[0] = 1./(1.+L);
			a[1] = a[0];
		}
		a[2] = 0.;
		
		b[0] = 1.;
		b[1] = (1-L)/(1+L);
		b[2] = 0.;
	}
	else {
		// normal case of 2nd-order stage
		double cosphi = cos(phi);
		ib0 = 1./(1 + LL - 2*cosphi*L);
		b[0] = 1.;
		b[1] = ib0*2*(1-LL);
		b[2] = ib0*(1 + LL + 2*cosphi*L);

		if (type == BUTTER_TYPE_HIGH) {
			a[0] = ib0*LL;
			a[1] = -2*a[0];
		}
		else { /* BUTTER_TYPE_LOW */
			a[0] = ib0;
			a[1] = 2*a[0];
		}
		a[2] = a[0];
	}
}

std::vector<_Biquad>
_init_bw_biquads(int order, double fc, double fsamp, int type)
{
	/* initialize a Butterworth filter (either high or lowpass) */
	int odd_order = order&1;
	double dphi = M_PI/order, phi0 = 0.5*(dphi+M_PI);
	double L  = 1./tan(M_PI*fc/fsamp), a[3], b[3];

	std::vector<_Biquad> biq;

	if (type == BUTTER_TYPE_HIGH || type == BUTTER_TYPE_LOW) {

		for (int i=0; i<order/2; i++) {
			double phi = phi0 + i*dphi;
			_bw_coeff_lp_hp(phi, L, type, a, b);
			biq.push_back(_Biquad(a[0],a[1],a[2],b[0],b[1],b[2]));
		}

		if (odd_order) {
			/* append an additional 1st order filter */
			_bw_coeff_lp_hp(M_PI, L, type, a, b);
			biq.push_back(_Biquad(a[0], a[1], a[2], b[0], b[1], b[2]));
		}
	}
	else {
			/* let it bang loudly */
			fprintf(stderr, "fatal error - wrong filter type in _init_bw_biquads\n");
			exit(1);
	}
	return biq;
}


std::vector<_Biquad>
_init_bw_biquads2(int order, double f1, double f2, double fsamp, int type)
{
	std::vector<_Biquad> biquads;

	if (type == BUTTER_TYPE_BAND) {
		// This is a proper bandpass

		int odd_order = order&1;

		/* initialize a Butterworth filter (either bandpass or bandstop) */
		double dphi = M_PI/(2*order);
		double w1 = 2*M_PI*f1, w2 = 2*M_PI*f2, W = w2-w1;

		// This is a little more involved than high/lowpass combination
		//
		// First create a normalized prototype lowpass, then transform
		// it into a bandpass.

		double xx = 4*(w1*w2)/(W*W);
		for (int i=odd_order?0:1; i<order; i+=2) {

			if (i==0 && xx<1) {
				double x = sqrt(1-xx);
				// two real poles
				double L, pole[2] = { 0.5*W*(1-x),  0.5*W*(1+x) };

				L  = 1/tan(0.5*pole[0]/fsamp);
				double u0 = L/(1+L),  u1 = -u0;
				double v0 = 1,  v1 = (1-L)/(1+L);

				L  = 1/tan(0.5*pole[1]/fsamp);
				double w0 = 1/(1+L),  w1 = w0;
				double x0 = 1,  x1 = (1-L)/(1+L);

				// combine two 1st-order biquads into one of 2nd-order
				double a0 = u0*w0, a1 = u1*w0+u0*w1, a2 = u1*w1,
				       b0 = v0*x0, b1 = v1*x0+v0*x1, b2 = v1*x1;
				biquads.push_back( _Biquad(a0/b0, a1/b0, a2/b0, 1, b1/b0, b2/b0) );
			}
			else {
				double phi = M_PI-i*dphi;
				std::complex<double> s(cos(phi), sin(phi));
				std::complex<double> x = sqrt(s*s-xx);
				// two complex poles
				std::complex<double> pole[2] = { 0.5*W*(s-x), 0.5*W*(s+x) };

				for (int ip=0; ip<2; ip++) {
					double w  = abs(pole[ip]);
					double L  = 1/tan(0.5*w/fsamp), K=w*L, Q = w*w;

					double a0 = W*K, // this normalization needs to be re-checked
						a1 = 0, a2 = -a0, a = pole[ip].real(),
						b0 = Q + K*K - 2*a*K,
						b1 = 2*(Q-K*K),
						b2 = Q + K*K + 2*a*K;
					biquads.push_back( _Biquad(a0/b0, a1/b0, a2/b0, 1, b1/b0, b2/b0) );

					// For a pole pair at phi=M_PI, we only use one,
					// because the other is its complex conjugate 
					if(i==0) break;
				}
			}
		}

	}
	else if (type == BUTTER_TYPE_HIGHLOW) {
		// This is a bandpass obtained by catenation of lowpass and highpass
		// of same order

		std::vector< _Biquad > b1, b2;
		b1 = _init_bw_biquads(order, f1, fsamp, BUTTER_TYPE_HIGH);
		b2 = _init_bw_biquads(order, f2, fsamp, BUTTER_TYPE_LOW);

		std::vector< _Biquad >::const_iterator biq;
		for (biq = b1.begin(); biq != b1.end(); biq++)
			biquads.push_back(*biq);
		for (biq = b2.begin(); biq != b2.end(); biq++)
			biquads.push_back(*biq);
	}
	// TODO: implement bandstop
	else {
		/* let it bang loudly */
		fprintf(stderr, "fatal error - wrong filter type in _init_bw_biquads2\n");
		exit(1);
	}

	return biquads;
}



}	// namespace Seiscomp::Math::Filtering::IIR
}	// namespace Seiscomp::Math::Filtering
}	// namespace Seiscomp::Math
}	// namespace Seiscomp


/*
Robert Bristow-Johnson derived a group delay formula for a biquad with b0 = 1.
>T(w) =
>
>           b1^2 + 2*b2^2 + b1*(1 + 3*b2)*cos(w) + 2*b2*cos(2*w)
>         --------------------------------------------------------
>          1 + b1^2 + b2^2 + 2*b1*(1 + b2)*cos(w) + 2*b2*cos(2*w)
>
>
>           a1^2 + 2*a2^2 + a1*(1 + 3*a2)*cos(w) + 2*a2*cos(2*w)
>    -    --------------------------------------------------------
>          1 + a1^2 + a2^2 + 2*a1*(1 + a2)*cos(w) + 2*a2*cos(2*w)
>
>
>w is normalized radian frequency and T(w) is measured in sample units.

Josep wants a one-pole expression.  Here is the expression I worked out for
a 1-pole, 1-zero filter (with arbitrary b0):

T(w) = ((a1*b0 + b1)*(a1*b0 - b1 + (-b0 + a1*b1)*Cos(w))) / ((1 + a1^2 -
2*a1*Cos(w))* (b0^2 + b1^2 + 2*b0*b1*Cos(w)))

For the all-pole filter, set b1 to zero.  I tried it with a few examples,
and it seems to work.

This expression is consistent with Robert's, with the exception that his
result is negative to mine, and Robert uses transfer function coefficient
a1 rather than filter multiplier a1 (which turns into -a1 in the transfer
function).  I will set up the derivation of my formula below for those of
you keeping score at home.

The filter transfer function is H(z) = B(z) / A(z).  To get the phase,
multiply both sides of the division with the conjugate of the denominator.
The denominator is then real and positive, thus we can throw it out of the
phase consideration.  Evaluate on the unit circle.

Arg(H(j w)) = Arg(B(j w) * A(-j w)) = Arg((b0 + b1*e^(-j*w)) * (1 -
a1*e^(j*w)))
  = Arg(b0 - b1*a1 + b1*e^(-j*w) - b0*a1*e^(j*w))

Derive the complex argument.

Arg(H(j w)) = ArcTan(Im / Re) = ArcTan((-b1*Sin(w) - b0*a1*Sin(w)) / (b0 -
b1*a1 + b1*Cos(w) - b0*a1*Cos(w)))

Differentiate this result.  d ArcTan(x) / dx = 1 / (1 + x^2).  I won't do
the icky work here, but the result is as above.

Enjoy, Duane Wise (dwise@wholegrain-ds.com)

*/
