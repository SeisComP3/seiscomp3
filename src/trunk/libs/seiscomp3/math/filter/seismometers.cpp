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


#include<iostream>
using namespace std;

#include <seiscomp3/math/filter/seismometers.h>
#include <seiscomp3/core/exceptions.h>

namespace Seiscomp {
namespace Math {

namespace SeismometerResponse {

PolesAndZeros::PolesAndZeros() {}

PolesAndZeros::PolesAndZeros(const Poles &p, const Zeros &z, double n)
 : poles(p), zeros(z), norm(n) {}


PolesAndZeros::PolesAndZeros(const PolesAndZeros &other)
 : poles(other.poles), zeros(other.zeros), norm(other.norm) {}


WoodAnderson::WoodAnderson(GroundMotion input) {
	poles.clear();
	zeros.clear();

	// Poles from Seismic Handler
	poles.push_back( Pole(-6.283185, -4.712389) );
	poles.push_back( Pole(-6.283185, +4.712389) );

	norm = 2800.;

	switch(input) {
		case Displacement: zeros.push_back( 0 );
		case Velocity:     zeros.push_back( 0 );
		case Acceleration: break;
	}
}


Seismometer5sec::Seismometer5sec(GroundMotion input) {
	poles.clear();
	zeros.clear();

	// Poles from Seismic Handler
	poles.push_back( Pole(-0.88857, -0.88857) );
	poles.push_back( Pole(-0.88857, +0.88857) );

	norm = 1.;

	switch(input) {
		case Displacement: zeros.push_back( 0 );
		case Velocity:     zeros.push_back( 0 );
		case Acceleration: break;
	}
}


} // namespace SeismometerResponse


using namespace Seiscomp::Math::SeismometerResponse;


namespace Filtering {

namespace {


bool onRealAxis(complex<double> x) {
	return ( fabs(x.imag()/x.real()) < 1.E-10 );
}


GroundMotion double2gm(double v, bool &error)
{
	int value = (int)v;
	error = false;
	switch (value) {
		case 0:
			return Math::Displacement;
		case 1:
			return Math::Velocity;
		case 2:
			return Math::Acceleration;
		default:
			error = true;
			break;
	}

	return Math::Velocity;
}


template <typename T>
Math::Filtering::IIR::Biquad<T>
pole2biquad(const SeismometerResponse::Pole &p, double fsamp, int nz0=0, double gain=1)
{
	// p is an analog pole
	// nz0 is the number of zeros at 0+0j in the s-plane
	// e.g. n=2 turns a 2nd-order lowpass into a highpass

	double
		// frequency pre-warping
		wc = abs(p),
		K = wc/tan(0.5*wc/fsamp), // 2*fsamp for wc -> 0

		a0,a1,a2,b0,b1,b2;

	if ( onRealAxis(p) ) {

		switch(nz0) {
		case 0: // that's the default
			a0 = gain*1/(wc+K);
			a1 = a0;
			a2 = 0.;
			break;
		case 1: // one zero at s = 0+0j
			a0 = gain*K/(wc+K);
			a1 = -a0;
			a2 = 0.;
			break;
		default:
			// FIXME
			throw "*** pole2biquad: error 1 ***";
		}

		b0 = 1.;
		b1 = (wc-K)/(wc+K);
		b2 = 0.;
	}
	else {

		double
			phi = asin(-p.real()/abs(p)),
			a   = -wc*sin(phi),
			X   = 1./(wc*wc + K*K - 2*a*K);

		switch(nz0) {
		case 0: // that's the default
			// a0 = wc*wc*X; // XXX This was wrong!
			a0 = gain*X;
			a1 = 2*a0;
			a2 = a0;
			break;
		case 1: // one zero at s = 0+0j
			a0 = gain*K*X;
			a1 = 0;
			a2 = -a0;
			break;
		case 2: // two zeros at s = 0+0j
			a0 = gain*K*K*X;
			a1 = -2*a0;
			a2 = a0;
			break;
		default:
			// FIXME
			throw "*** pole2biquad: error 2 ***";
		}

		b0 = 1;
		b1 = 2*(wc*wc-K*K)*X;
		b2 = (wc*wc + K*K + 2*a*K)*X;
	}

	return Math::Filtering::IIR::Biquad<T>(a0, a1, a2, b0, b1, b2);
}

template <typename T>
Math::Filtering::IIR::Biquad<T>
zero2biquad(const SeismometerResponse::Zero &z, double fsamp)
{
// XXX checken
	Math::Filtering::IIR::Biquad<T> biq =
		pole2biquad<T>(z, fsamp);

	double ib0 = 1./biq.b0;
	return Math::Filtering::IIR::Biquad<T>(
		biq.b0*ib0, biq.b1*ib0, biq.b2*ib0,
		biq.a0*ib0, biq.a1*ib0, biq.a2*ib0);
}

template <typename T>
Math::Filtering::IIR::BiquadCascade<T>
pz2biquads(
	const vector<SeismometerResponse::Pole> &p,
	const vector<SeismometerResponse::Zero> &z,
	double fsamp, double norm )
{
// XXX This currently works for forward-computing the instrument responses
// XXX that consist of only zeros at (0,0) and arbitrary poles. 

	int np = p.size(), nz = z.size();

	Math::Filtering::IIR::BiquadCascade<T> cascade;
	Math::Filtering::IIR::Biquad<T> biq;

	double epsilon = 1.E-10;

	// count the zeros at (0,0)
	int nz0 = 0;
	for (int iz=0; iz<nz; iz++) {
		if (abs(z[iz]) < epsilon)
			nz0++;
	}

/*	for (int iz=0; iz<nz; iz++) {

		// if at (0,0)
		if (abs(z[iz]) < epsilon) {
			double x = 1./sqrt(0.5/fsamp);
			biq.set(x, -x, 0, 1, 1, 0);
		}
		else {
			if ( onRealAxis(z) )
				continue;
			biq = zero2biquad(z[iz], fsamp);
		}

		cascade.append(biq);
	}
*/

	for (int ip=0; ip<np; ip++) {

		if (p[ip].imag() < -epsilon)
			// We assume we also have the complex conjugate
			// This is a bit risky.
			continue;

		// if pole is at (0,0)
		if ( abs(p[ip]) < epsilon ) {
			// XXX needs to be treated separately
			cerr << "POLE AT (0,0) IGNORED" << endl;
		}
		else {
			int dnz0 = nz0;
			if (dnz0>2) dnz0 = 2;

			if ( onRealAxis(p[ip]) && dnz0 > 1 )
				dnz0 = 1;

			biq = pole2biquad<T>(p[ip], fsamp, dnz0, norm);
			cascade.append(biq);

			norm = 1.;
			nz0 -= dnz0;
		}
	
	}

	if (nz0)
		cerr << "LEFT-OVER ZEROS AT (0,0) IGNORED" << endl;

	return cascade;
}

}


namespace IIR {

REGISTER_INPLACE_FILTER(WWSSN_SP_Filter, "WWSSN_SP");
REGISTER_INPLACE_FILTER(WWSSN_LP_Filter, "WWSSN_LP");
REGISTER_INPLACE_FILTER(WoodAndersonFilter, "WA");
REGISTER_INPLACE_FILTER(Seismometer5secFilter, "SM5");


template <typename T>
Filter<T>::Filter() {}


template <typename T>
Filter<T>::Filter(
	const SeismometerResponse::Poles &poles,
	const SeismometerResponse::Zeros &zeros,
	double norm)
 : SeismometerResponse::PolesAndZeros(poles,zeros,norm) { }


template <typename T>
Filter<T>::Filter(const Filter &other)
 : SeismometerResponse::PolesAndZeros(other.poles,other.zeros,other.norm)
 , _cascade(other._cascade)
{}


template <typename T>
void Filter<T>::setSamplingFrequency(double fsamp) {
	if ( fsamp == 0.0 ) return;
	_cascade = pz2biquads<T>(poles, zeros, fsamp, norm);
}


template <typename T>
int Filter<T>::setParameters(int n, const double *params) {
	poles.clear();
	zeros.clear();

	// norm, npoles, nzeros are required
	if ( n < 3 ) return 3;

	int npoles = 0;
	int nzeros = 0;

	norm = params[0];

	npoles = (int)params[1];
	if ( npoles < 0 )
		return -1;

	if ( n < 1+1+npoles*2 )
		return 1+1+npoles*2;

	for ( int i = 0; i < npoles; ++i )
		poles.push_back(SeismometerResponse::Pole(params[1+i*2], params[1+i*2+1]));

	nzeros = (int)params[1+npoles*2];
	if ( nzeros < 0 )
		return -(1+1+npoles*2);

	if ( n < 1+1+(npoles+nzeros)*2 )
		return 1+1+(npoles+nzeros)*2;

	for ( int i = 0; i < nzeros; ++i )
		zeros.push_back(SeismometerResponse::Zero(params[1+1+npoles*2+i*2], params[1+1+npoles*2+i*2+1]));

	return n;
}


template <typename T>
void Filter<T>::apply(int n, T *inout) {
	if ( _cascade.size() == 0 )
		throw Core::GeneralException("Samplerate not initialized");

	_cascade.apply(n, inout);
}


template <typename T>
InPlaceFilter<T>* Filter<T>::clone() const {
	return new Filter<T>(poles, zeros, norm);
}

INSTANTIATE_INPLACE_FILTER(Filter, SC_SYSTEM_CORE_API);



template <typename T>
WWSSN_SP_Filter<T>::WWSSN_SP_Filter(GroundMotion input) {
	setInput(input);
}


template <typename T>
WWSSN_SP_Filter<T>::WWSSN_SP_Filter(const WWSSN_SP_Filter &other)
 : Filter<T>(other) {}

template <typename T>
void WWSSN_SP_Filter<T>::setInput(GroundMotion input) {
	Filter<T>::poles.clear();
	Filter<T>::zeros.clear();

	// Poles according to Jim Dewey (NEIC)
	Filter<T>::poles.push_back( SeismometerResponse::Pole( -3.725, -6.220) );
	Filter<T>::poles.push_back( SeismometerResponse::Pole( -3.725, +6.220) );
	Filter<T>::poles.push_back( SeismometerResponse::Pole( -5.612) );
	Filter<T>::poles.push_back( SeismometerResponse::Pole(-13.240) );
	Filter<T>::poles.push_back( SeismometerResponse::Pole(-21.080) );

/*	// Poles from Seismic Handler, also specified in NMSOP
	poles.push_back( Pole(-3.367788, -3.731514) );
	poles.push_back( Pole(-3.367788, +3.731514) );
	poles.push_back( Pole(-7.037168, -4.545562) );
	poles.push_back( Pole(-7.037168, +4.545562) );
	double norm = 13.34714;
*/
	Filter<T>::zeros.push_back( 0. );

	// make the *displacement* amplitude response 1 at 1 Hz
	Filter<T>::norm = 532.1425713966;

	switch(input) {
		case Displacement: Filter<T>::zeros.push_back( 0 );
		case Velocity:     Filter<T>::zeros.push_back( 0 );
		case Acceleration: break;
	}
}


template <typename T>
int WWSSN_SP_Filter<T>::setParameters(int n, const double *params) {
	if ( n != 1 ) return 1;

	bool error;
	GroundMotion input = double2gm(params[0], error);

	if ( error ) return -1;

	setInput(input);
	return n;
}


template <typename T>
InPlaceFilter<T>* WWSSN_SP_Filter<T>::clone() const {
	return new WWSSN_SP_Filter(*this);
}


INSTANTIATE_INPLACE_FILTER(WWSSN_SP_Filter, SC_SYSTEM_CORE_API);



template <typename T>
WWSSN_LP_Filter<T>::WWSSN_LP_Filter(GroundMotion input) {
	setInput(input);
}


template <typename T>
WWSSN_LP_Filter<T>::WWSSN_LP_Filter(const WWSSN_LP_Filter &other)
 : Filter<T>(other) {}

template <typename T>
void WWSSN_LP_Filter<T>::setInput(GroundMotion input) {
	Filter<T>::poles.clear();
	Filter<T>::zeros.clear();

        // This is the WWSSN-LP transfer function as defined by IASPEI
	// http://www.iaspei.org/commissions/CSOI/Summary_WG-Recommendations_20110909.pdf

	// Poles according to Jim Dewey (NEIC)
	Filter<T>::poles.push_back( Pole( -0.40180, -0.08559) );
	Filter<T>::poles.push_back( Pole( -0.40180, +0.08559) );
	Filter<T>::poles.push_back( Pole( -0.04841) );
	Filter<T>::poles.push_back( Pole( -0.08816) );

	Filter<T>::zeros.push_back( 0. );

	Filter<T>::norm = 0.826835;

	switch(input) {
		case Displacement: Filter<T>::zeros.push_back( 0 );
		case Velocity:     Filter<T>::zeros.push_back( 0 );
		case Acceleration: break;
	}
}


template <typename T>
int WWSSN_LP_Filter<T>::setParameters(int n, const double *params) {
	if ( n != 1 ) return 1;

	bool error;
	GroundMotion input = double2gm(params[0], error);

	if ( error ) return -1;

	setInput(input);

	return n;
}


template <typename T>
InPlaceFilter<T>* WWSSN_LP_Filter<T>::clone() const {
	return new WWSSN_LP_Filter(*this);
}


INSTANTIATE_INPLACE_FILTER(WWSSN_LP_Filter, SC_SYSTEM_CORE_API);


template <typename T>
WoodAndersonFilter<T>::WoodAndersonFilter(GroundMotion input) {
	setInput(input);
}


template <typename T>
WoodAndersonFilter<T>::WoodAndersonFilter(const WoodAndersonFilter &other)
 : Filter<T>(other) {}

template <typename T>
void WoodAndersonFilter<T>::setInput(GroundMotion input) {
	PolesAndZeros::operator=(WoodAnderson(input));
}


template <typename T>
int WoodAndersonFilter<T>::setParameters(int n, const double *params) {
	if ( n != 1 ) return 1;

	bool error;
	GroundMotion input = double2gm(params[0], error);

	if ( error ) return -1;

	setInput(input);

	return n;
}


template <typename T>
InPlaceFilter<T>* WoodAndersonFilter<T>::clone() const {
	return new WoodAndersonFilter(*this);
}


INSTANTIATE_INPLACE_FILTER(WoodAndersonFilter, SC_SYSTEM_CORE_API);




template <typename T>
Seismometer5secFilter<T>::Seismometer5secFilter(GroundMotion input) {
	setInput(input);
}


template <typename T>
Seismometer5secFilter<T>::Seismometer5secFilter(const Seismometer5secFilter &other)
 : Filter<T>(other) {}


template <typename T>
void Seismometer5secFilter<T>::setInput(GroundMotion input) {
	PolesAndZeros::operator=(Seismometer5sec(input));
}


template <typename T>
int Seismometer5secFilter<T>::setParameters(int n, const double *params) {
	if ( n != 1 ) return 1;

	bool error;
	GroundMotion input = double2gm(params[0], error);

	if ( error ) return -1;

	setInput(input);

	return n;
}


template <typename T>
InPlaceFilter<T>* Seismometer5secFilter<T>::clone() const {
	return new Seismometer5secFilter(*this);
}


INSTANTIATE_INPLACE_FILTER(Seismometer5secFilter, SC_SYSTEM_CORE_API);




template <typename T>
GenericSeismometer<T>::GenericSeismometer(double cornerPeriod, GroundMotion input) : _cornerPeriod(cornerPeriod)
{
	setInput(input);
}


template <typename T>
GenericSeismometer<T>::GenericSeismometer(const GenericSeismometer &other)
 : Filter<T>(other), _cornerPeriod(other._cornerPeriod) {}


template <typename T>
void GenericSeismometer<T>::setInput(GroundMotion input) {
	Filter<T>::poles.clear();
	Filter<T>::zeros.clear();

	double x = sqrt(2.)*M_PI/_cornerPeriod;
	Filter<T>::poles.push_back( Pole(-x, -x) );
	Filter<T>::poles.push_back( Pole(-x, +x) );
	Filter<T>::norm = 1;

	switch(input) {
		case Displacement: Filter<T>::zeros.push_back( 0 );
		case Velocity:     Filter<T>::zeros.push_back( 0 );
		case Acceleration: break;
	}
}


template <typename T>
int GenericSeismometer<T>::setParameters(int n, const double *params) {
	if ( n != 1 ) return 1;

	bool error;
	GroundMotion input = double2gm(params[0], error);

	if ( error ) return -1;

	setInput(input);

	return n;
}


template <typename T>
InPlaceFilter<T>* GenericSeismometer<T>::clone() const {
	return new GenericSeismometer(*this);
}


INSTANTIATE_INPLACE_FILTER(GenericSeismometer, SC_SYSTEM_CORE_API);

} // namespace Seiscomp::Math::Filtering::IIR

} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp
