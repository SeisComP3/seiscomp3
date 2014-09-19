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


#include<math.h>
#include<vector>
#include<sstream>

#include<seiscomp3/math/filter/biquad.h>

namespace Seiscomp {
namespace Math {
namespace Filtering {
namespace IIR {

_Biquad::_Biquad(double _a0, double _a1, double _a2, double _b0, double _b1, double _b2)
//	Initializes a Biquad structure using coefficients
{
	a0 = _a0;  a1 = _a1;  a2 = _a2;
	b0 = _b0;  b1 = _b1;  b2 = _b2;
	v1 = v2 = 0.;
}

_Biquad::_Biquad(_Biquad const &other)
{
	a0 = other.a0;  a1 = other.a1;  a2 = other.a2;
	b0 = other.b0;  b1 = other.b1;  b2 = other.b2;
	v1 = v2 = 0.;
}

void _Biquad:: set(double _a0, double _a1, double _a2,
		   double _b0, double _b1, double _b2)
{
	a0 = _a0;  a1 = _a1;  a2 = _a2;
	b0 = _b0;  b1 = _b1;  b2 = _b2;
	v1 = v2 = 0.;
}

void _Biquad::reset()
{
	v1 = v2 = 0.;
}


std::string _Biquad::print() const
{
	std::ostringstream s;
	s << "a: " << a0 << ", " << a1 << ", " << a2 << std::endl;
	s << "b: " << b0 << ", " << b1 << ", " << b2 << std::endl;
	return s.str();
}


int
_Biquad::delay(int n_samp, double *delay_val)
{
// Biquad section group delay calculation.
//
// It calculates an array of group delay values for a single biquad section
// over the range 0.0 - pi. For cascaded sections, the delays may be added.
// The biquad section is double normalized, i.e.
//
//                     1.0 + b1*z(-1) + b2*z(-2)
//      H(z) =  gain * -------------------------
//                     1.0 + a1*z(-1) + a2*z(-2)

// XXX SWAP a and b
	int i;
	double cos2w, cosw, w, T1, T2, T3, T4;
// XXX	double a1=self->b1, a2=self->b2, b1=self->a1, b2=self->a2;

	for(i=0; i<n_samp; i++) {
		w     = M_PI*i/(double)n_samp;
		cos2w = cos(2*w);
		cosw  = cos(w);

		T1 =     b1*b1 + 2.*b2*b2 +    b1*(1.+3.*b2)*cosw + 2.*b2*cos2w;
		T2 = 1.+ b1*b1 +    b2*b2 + 2.*b1*(1.+   b2)*cosw + 2.*b2*cos2w;
		T3 =     a1*a1 + 2.*a2*a2 +    a1*(1.+3.*a2)*cosw + 2.*a2*cos2w;
		T4 = 1.+ a1*a1 +    a2*a2 + 2.*a1*(1.+   a2)*cosw + 2.*a2*cos2w;
		if(T2 != 0. && T4 != 0.)
			delay_val[i] = T1/T2 - T3/T4;
	}
	return 0;
}

int
_Biquad::delay_one(double freq /* 0...PI */, double *delay)
{

// Biquad section group delay calculation.
//
// It calculates an array of group delay values for a single biquad section
// over the range 0.0 - pi. For cascaded sections, the delays may be added.
// The biquad section is double normalized, i.e.
//
//                     1.0 + b1*z(-1) + b2*z(-2)
//      H(z) =  gain * -------------------------
//                     1.0 + a1*z(-1) + a2*z(-2)

	double cos2w, cosw, w, T1, T2, T3, T4;
// XXX	double a1=self->b1, a2=self->b2, b1=self->a1, b2=self->a2;

	w	= freq;
	cos2w	= cos(2*w);
	cosw	= cos(w);

	T1 =     b1*b1 + 2.*b2*b2 +    b1*(1.+3.*b2)*cosw + 2.*b2*cos2w;
	T2 = 1.+ b1*b1 +    b2*b2 + 2.*b1*(1.+   b2)*cosw + 2.*b2*cos2w;
	T3 =     a1*a1 + 2.*a2*a2 +    a1*(1.+3.*a2)*cosw + 2.*a2*cos2w;
	T4 = 1.+ a1*a1 +    a2*a2 + 2.*a1*(1.+   a2)*cosw + 2.*a2*cos2w;
	if(T2 != 0. && T4 != 0.)
		*delay = T1/T2 - T3/T4;

	return 0;
}

int
_Biquad::delay2(int n_samp, double *delay_val)
{
	/* like delay() */

	for(int i=0; i<n_samp; i++) {
		double w = M_PI*i/(double)n_samp;
		delay_one(w, delay_val+i);
	}
	return 0;
}



// load the template class definitions
#include<seiscomp3/math/filter/biquad.ipp>
template class SC_SYSTEM_CORE_API Biquad<float>;
template class SC_SYSTEM_CORE_API Biquad<double>;

template class SC_SYSTEM_CORE_API BiquadCascade<float>;
template class SC_SYSTEM_CORE_API BiquadCascade<double>;


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
