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
#include <vector>
#include <complex>
#include <stdexcept>

#include <seiscomp3/math/filter/butterworth.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {
namespace IIR {


typedef std::complex<double> Complex;

#define BUTTERWORTH_HIGHPASS    0
#define BUTTERWORTH_LOWPASS     1
#define BUTTERWORTH_BANDPASS    2
#define BUTTERWORTH_BANDSTOP    3
#define BUTTERWORTH_HIGHLOWPASS 4 // combination of BUTTERWORTH_LOWPASS and BUTTERWORTH_HIGHPASS


namespace {


std::vector<Complex> makepoles(int order) {
	// Create a set of basic poles for a given filter order. For each
	// conjugate pole pair only one pole is included

	std::vector<Complex> poles;

	// one pole for each complex conjugate pair
	int halforder = order / 2;
	for ( int k = 0; k < halforder; ++k ) {
		double phi = M_PI * (0.5 + (k+0.5)/order);
		poles.push_back( Complex(cos(phi), sin(phi)) );
	}

	// additional pole at -1 for odd filter orders
	if ( order & 1 )
		poles.push_back( Complex(-1) );

	return poles;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<_Biquad> poles2bp(const std::vector<Complex> poles, double fmin, double fmax) {
	// convert basic set of poles to an analog bandpass

	double a = 2*M_PI * 2*M_PI * fmin*fmax;
	double b = 2*M_PI * (fmax-fmin);

	std::vector<_Biquad> biquads;

	for (std::vector<Complex>::const_iterator
	     it = poles.begin(); it != poles.end(); ++it) {

		const Complex &pole = *it;
		double a0=0, a1=b, a2=0, b0, b1, b2;

		if ( pole != Complex(-1) ) {
			Complex pb = pole*b;
			Complex tmp = sqrt(pb*pb - 4*a);
			Complex p1 = 0.5 * (pb + tmp);
			Complex p2 = 0.5 * (pb - tmp);

			// a0 = 0;
			// a1 = b;
			// a2 = 0;

			b0 = (p1*conj(p1)).real();
			b1 = -2*p1.real();
			b2 = 1;

			biquads.push_back( _Biquad(a0,a1,a2,b0,b1,b2) );

			// a0 = 0;
			// a1 = b;
			// a2 = 0;

			b0 = (p2*conj(p2)).real();
			b1 = -2*p2.real();
			b2 = 1;

			biquads.push_back( _Biquad(a0,a1,a2,b0,b1,b2) );
		}
		else {
			// pole at -1+0j

			// a0 = 0;
			// a1 = b;
			// a2 = 0;

			b0 = a;
			b1 = b;
			b2 = 1;

			biquads.push_back( _Biquad(a0,a1,a2,b0,b1,b2) );
		}
	}

	return biquads;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<_Biquad> poles2bs(const std::vector<Complex> poles, double fmin, double fmax) {
	// convert basic set of poles to an analog bandstop

	double a = 2*M_PI * 2*M_PI * fmin*fmax;
	double b = 2*M_PI * (fmax-fmin);

	std::vector<_Biquad> biquads;

//	for ( size_t i = 0; i < poles.size(); ++i ) {
	for (std::vector<Complex>::const_iterator
	     it = poles.begin(); it != poles.end(); ++it) {

		const Complex &pole = *it;
		double a0=a, a1=0, a2=1, b0, b1, b2;

		if ( pole != Complex(-1) ) {
			Complex bp = b / pole;
			Complex tmp = sqrt(bp*bp - 4*a);
			Complex p1 = 0.5 * (bp + tmp);
			Complex p2 = 0.5 * (bp - tmp);

			// a0 = a;
			// a1 = 0;
			// a2 = 1;

			b0 = (p1 * conj(p1)).real();
			b1 = -2 * p1.real();
			b2 =  1;

			biquads.push_back( _Biquad(a0,a1,a2,b0,b1,b2) );

			// a0 = a;
			// a1 = 0;
			// a2 = 1;

			b0 = (p2 * conj(p2)).real();
			b1 = -2 * p2.real();
			b2 = 1;

			biquads.push_back( _Biquad(a0,a1,a2,b0,b1,b2) );
		}
		else {
			// pole at -1+0j

			// a0 = a;
			// a1 = 0;
			// a2 = 1;
			b0 = a;
			b1 = b;
			b2 = 1;
			biquads.push_back( _Biquad(a0,a1,a2,b0,b1,b2) );
		}
	}

	return biquads;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<_Biquad> poles2lp(const std::vector<Complex> poles, double fmax) {
	// convert basic set of poles to an analog lowpass

	double s = 1/(2*M_PI*fmax);

	std::vector<_Biquad> biquads;

//	for ( size_t i = 0; i < poles.size(); ++i ) {
	for (std::vector<Complex>::const_iterator
	     it = poles.begin(); it != poles.end(); ++it) {

		const Complex &pole = *it;
		double a0=1, a1=0, a2=0, b0, b1, b2;

		if ( pole != Complex(-1) ) {
			// a0 = 1;
			// a1 = 0;
			// a2 = 0;

			b0 = (pole * conj(pole)).real();
			b1 = -2 * pole.real();
			b2 =  1;

			b1 *= s;
			b2 *= s*s;

			biquads.push_back( _Biquad(a0,a1,a2,b0,b1,b2) );
		}
		else {
			// pole at -1+0j

			// a0 = 1;
			// a1 = 0;
			// a2 = 0;

			b0 =  1;
			b1 =  s;
			b2 =  0;

			biquads.push_back( _Biquad(a0,a1,a2,b0,b1,b2) );
		}
	}

	return biquads;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<_Biquad> poles2hp(const std::vector<Complex> poles, double fmin) {
	// convert basic set of poles to an analog highpass

	double s = 1/(2*M_PI*fmin);

	std::vector<_Biquad> biquads;

//	for ( size_t i = 0; i < poles.size(); ++i ) {
	for (std::vector<Complex>::const_iterator
	     it = poles.begin(); it != poles.end(); ++it) {

		const Complex &pole = *it;
		double a0, a1, a2, b0, b1, b2;

		if ( pole != Complex(-1) ) {
			a0 =  0;
			a1 =  0;
			a2 =  s*s;

			b0 =  1;
			b1 = -2*s * pole.real();
			b2 =  s*s * (pole * conj(pole)).real();

			biquads.push_back( _Biquad(a0,a1,a2,b0,b1,b2) );
		}
		else {
			// pole at -1+0j

			a0 =  0;
			a1 =  s;
			a2 =  0;

			b0 =  1;
			b1 =  s;
			b2 =  0;

			biquads.push_back( _Biquad(a0,a1,a2,b0,b1,b2) );
		}
	}

	return biquads;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void analog2digital(_Biquad &biquad) {
	// convert a biquad from analog to digital

	double c0 = biquad.b0, c1 = biquad.b1, c2 = biquad.b2;
	double scale = 1./(c0+c1+c2);

	biquad.b0 = 1;
	biquad.b1 = scale * (2 * (c0 - c2));
	biquad.b2 = scale * (c2 - c1 + c0);

	c0 = biquad.a0;
	c1 = biquad.a1;
	c2 = biquad.a2;

	biquad.a0 = scale * (c0 + c1 + c2);
	biquad.a1 = scale * (2 * (c0 - c2));
	biquad.a2 = scale * (c2 - c1 + c0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void analog2digital(std::vector<_Biquad>  &biquads) {
	// convert filter from analog to digital

	for (std::vector<_Biquad>::iterator biq = biquads.begin(); biq != biquads.end(); ++biq)
		analog2digital(*biq);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<_Biquad> init_bw_biquads(size_t order, double fmin, double fmax, double fsamp, int type) {
	// This is the main Butterworth filter initialization routine.
	// For the self-explaining set of input parameters a vector of biquads is returned.

	if ( order > 20 )
		throw std::runtime_error("Filter order exceeded maximum of 20");

	if ( fsamp <= 0 )
		throw std::runtime_error("Sample rate must be greater than zero");

	std::vector<_Biquad> biquads;

	double fnyquist = 0.5*fsamp;

	// Input validation
	switch ( type ) {
		case BUTTERWORTH_BANDPASS:
		case BUTTERWORTH_BANDSTOP:
			if ( fmax < fmin )
				throw std::runtime_error("High frequency cutoff must be greater than low freq");

			if ( fmin <= 0.0 )
				throw std::runtime_error("Low frequency cutoff must be greater than zero");

			// The missing break here is intentional as the low pass check is
			// also required for the bandpass and bandreject

		case BUTTERWORTH_LOWPASS:
			if ( fmax <= 0.0 )
				throw std::runtime_error("High frequency cutoff must be greater than zero");

			if ( fmin >= fnyquist )
				throw std::runtime_error("High frequency cutoff must be lower than Nyquist frequency");

			break;

		case BUTTERWORTH_HIGHPASS:
			if ( fmin <= 0.0 )
				throw std::runtime_error("Low frequency cutoff must be greater than zero");

			if ( fmin >= fnyquist )
				throw std::runtime_error("High frequency cutoff must be lower than Nyquist frequency");

			break;
	}

	std::vector<Complex> p = makepoles(order);

	// frequency warping
	double warped_fmin = tan(M_PI*fmin/fsamp) / (2*M_PI);
	double warped_fmax = tan(M_PI*fmax/fsamp) / (2*M_PI);

	// cascade generation
	switch ( type ) {
		case BUTTERWORTH_BANDPASS:
			biquads = poles2bp(p, warped_fmin, warped_fmax);
			analog2digital(biquads);
			break;

		case BUTTERWORTH_BANDSTOP:
			biquads = poles2bs(p, warped_fmin, warped_fmax);
			analog2digital(biquads);
			break;

		case BUTTERWORTH_LOWPASS:
			biquads = poles2lp(p, warped_fmax);
			analog2digital(biquads);
			break;

		case BUTTERWORTH_HIGHPASS:
			biquads = poles2hp(p, warped_fmin);
			analog2digital(biquads);
			break;

		case BUTTERWORTH_HIGHLOWPASS: {
			// This is a bandpass obtained by combining lowpass and highpass
			std::vector< _Biquad > b1, b2;
			b1 = init_bw_biquads(order, fmin, 0, fsamp, BUTTERWORTH_HIGHPASS);
			b2 = init_bw_biquads(order, 0, fmax, fsamp, BUTTERWORTH_LOWPASS);

			std::vector< _Biquad >::const_iterator biq;
			for (biq = b1.begin(); biq != b1.end(); biq++)
				biquads.push_back(*biq);
			for (biq = b2.begin(); biq != b2.end(); biq++)
				biquads.push_back(*biq);
			}
			break;

		default:
			throw std::runtime_error("Invalid filter type");
	}

	return biquads;
}


}


// load the template class definitions
#include <seiscomp3/math/filter/butterworth.ipp>


INSTANTIATE_INPLACE_FILTER(ButterworthLowpass,     SC_SYSTEM_CORE_API);
INSTANTIATE_INPLACE_FILTER(ButterworthHighpass,    SC_SYSTEM_CORE_API);
INSTANTIATE_INPLACE_FILTER(ButterworthBandpass,    SC_SYSTEM_CORE_API);
INSTANTIATE_INPLACE_FILTER(ButterworthBandstop,    SC_SYSTEM_CORE_API);
INSTANTIATE_INPLACE_FILTER(ButterworthHighLowpass, SC_SYSTEM_CORE_API);

REGISTER_INPLACE_FILTER(ButterworthLowpass,     "BW_LP");
REGISTER_INPLACE_FILTER(ButterworthHighpass,    "BW_HP");
REGISTER_INPLACE_FILTER(ButterworthBandpass,    "BW_BP");
REGISTER_INPLACE_FILTER(ButterworthBandstop,    "BW_BS");
REGISTER_INPLACE_FILTER(ButterworthHighLowpass, "BW_HLP");
REGISTER_INPLACE_FILTER2(ButterworthBandpass, Proxy, "BW");


}	// namespace Seiscomp::Math::Filtering::IIR
}	// namespace Seiscomp::Math::Filtering
}	// namespace Seiscomp::Math
}	// namespace Seiscomp
