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



#include <seiscomp3/math/fft.h>
#include <seiscomp3/math/filter.h>


#define TWO_PI (M_PI*2)

// NOTE: Due to the GPL licence of fftw3 we are not allowed to link against
//       fftw3. Uncomment the next line to use fftw3 instead of build in fft
//       to test or compare results. Note that the math library must then
//       additionally link agains libfftw3: uncomment the corresponding line
//       in CMakeLists.txt.

//#define MATH_USE_FFTW3


#ifdef MATH_USE_FFTW3
#include <fftw3.h>
#endif


using namespace std;


namespace Seiscomp {
namespace Math {


#ifndef MATH_USE_FFTW3
namespace {


enum FFTDirection {
	Forward,
	Backward
};



#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

template <typename T>
void fourier(T *data, long nn, int isign) {
	long n,mmax,m,j,istep,i;
	double wtemp,wr,wpr,wpi,wi,theta;
	T tempr,tempi;

	n = nn << 1;
	j = 1;

	for ( i = 1; i < n; i += 2 ) {
		if ( j > i ) {
			SWAP(data[j],data[i]);
			SWAP(data[j+1],data[i+1]);
		}

		m = n >> 1;
		while ( m >= 2 && j > m ) {
			j -= m;
			m >>= 1;
		}

		j += m;
	}

	mmax = 2;
	while ( n > mmax ) {
		istep = 2*mmax;
		theta = TWO_PI/(isign*mmax);
		wtemp = sin(0.5*theta);
		wpr = -2.0*wtemp*wtemp;
		wpi = sin(theta);
		wr = 1.0;
		wi = 0.0;

		for ( m = 1; m < mmax; m +=2 ) {
			for ( i = m; i <= n; i += istep ) {
				j = i+mmax;
				tempr = wr*data[j]-wi*data[j+1];
				tempi = wr*data[j+1]+wi*data[j];
				data[j] = data[i]-tempr;
				data[j+1] = data[i+1]-tempi;
				data[i] += tempr;
				data[i+1] += tempi;
			}

			wr = (wtemp=wr)*wpr-wi*wpi+wr;
			wi = wi*wpr+wtemp*wpi+wi;
		}

		mmax=istep;
	}
}

#undef SWAP


template <typename T>
void transform(T *data, size_t n, FFTDirection dir) {
	if ( n < 4 ) return;

	--data;
	n /= 2;

	long i,i1,i2,i3,i4,n2p3;
	T c1 = 0.5,c2,h1r,h1i,h2r,h2i;
	double wr,wi,wpr,wpi,wtemp,theta;

	theta = M_PI/(double)n;
	if ( dir == Forward ) {
		c2 = -0.5;
		fourier(data,n,1);
	}
	else {
		c2 = 0.5;
		theta = -theta;
	}

	wtemp = sin(0.5*theta);
	wpr = -2.0*wtemp*wtemp;
	wpi = sin(theta);
	wr = 1.0+wpr;
	wi = wpi;
	n2p3 = 2*n+3;
	for ( i = 2; i <= n/2; ++i ) {
		i4 = 1+(i3 = n2p3-(i2=1+(i1 = i+i-1)));
		h1r = c1*(data[i1]+data[i3]);
		h1i = c1*(data[i2]-data[i4]);
		h2r = -c2*(data[i2]+data[i4]);
		h2i = c2*(data[i1]-data[i3]);
		data[i1] = h1r+wr*h2r-wi*h2i;
		data[i2] = h1i+wr*h2i+wi*h2r;
		data[i3] = h1r-wr*h2r+wi*h2i;
		data[i4] = -h1i+wr*h2i+wi*h2r;
		wr = (wtemp = wr)*wpr-wi*wpi+wr;
		wi = wi*wpr+wtemp*wpi+wi;
	}

	if ( dir == Forward ) {
		data[1] = (h1r = data[1])+data[2];
		data[2] = h1r-data[2];
	}
	else {
		data[1] = c1*((h1r = data[1])+data[2]);
		data[2] = c1*(h1r-data[2]);
		fourier(data,n,-1);
	}
}


}
#endif


//!
//! input:  half complex spectrum, N/2+1 Points
//! output: real data, N points
//!
template <typename T>
void ifft(int n, T *out, ComplexArray &coeff) {
	int tn = coeff.size()*2;
	double *inout = reinterpret_cast<double*>(&coeff[0]);

#ifdef MATH_USE_FFTW3
	fftw_plan backward = fftw_plan_dft_c2r_1d(tn, (fftw_complex *)inout, inout,
	                                          FFTW_ESTIMATE);
	fftw_execute(backward);
	fftw_destroy_plan(backward);

	tn -= 2;
	for ( int i = 0; i < n; ++i )
		out[i] = inout[i] / tn; // normalize
#else
	// Swap sign of imaginary part
	for ( int i = 3; i < tn; i += 2 )
		inout[i] *= -1;

	transform(inout, tn, Backward); // do inverse FFT

	double norm = 2.0 / (double)tn;
	for ( int i = 0; i < n; ++i )
		out[i] = norm * inout[i]; // normalize
#endif
}



//!
//! wrapper for the NR "realft" FFT routine
//! input: real data, N points
//! output: half complex spectrum, N/2+1 Points
//!
template <typename T>
void fft(ComplexArray &out, int n, const T *data) {
	int fftn = /*npow2?*/Filtering::next_power_of_2(n)/*:n*/;
	if ( fftn <= 0 ) return;

#ifdef MATH_USE_FFTW3
	out.resize(fftn/2+1);
#else
	out.resize(fftn/2);
#endif

	double *inout = reinterpret_cast<double*>(&out[0]);

	for ( int i = 0; i < n; ++i )
		inout[i] = data[i];

	for ( int i = n; i < fftn; ++i )
		inout[i] = 0.0;

#ifdef MATH_USE_FFTW3
	fftw_plan forward = fftw_plan_dft_r2c_1d(fftn, inout, (fftw_complex *)inout,
	                                         FFTW_ESTIMATE);
	fftw_execute(forward);
	fftw_destroy_plan(forward);
#else
	transform(inout, fftn, Forward); // do FFT

	// Swap sign of imaginary part
	for ( int i = 3; i < fftn; i += 2 )
		inout[i] *= -1;
#endif
}


// Explicit template instantiation for float and double types
template SC_SYSTEM_CORE_API
void ifft<float>(int n, float *out, ComplexArray &coeff);

template SC_SYSTEM_CORE_API
void ifft<double>(int n, double *out, ComplexArray &coeff);

template SC_SYSTEM_CORE_API
void fft<float>(ComplexArray &out, int n, const float *data);

template SC_SYSTEM_CORE_API
void fft<double>(ComplexArray &out, int n, const double *data);


}
}
