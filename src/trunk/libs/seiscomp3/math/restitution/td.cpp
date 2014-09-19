#include <iostream>
#include <sstream>
#include <string>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <seiscomp3/math/restitution/td.h>

namespace Seiscomp {
namespace Math {
namespace Restitution {

bool coefficients_from_T0_h(double fsamp, double gain, double T0, double h, double *c0, double *c1, double *c2)
{
	// from Kanamori and Rivera (2008)
	double w0 = 2*M_PI/T0;
	double dt = 1./fsamp;
	double q  = 1./(gain*dt);

	*c0 = q;
	*c1 = -2*(1+h*w0*dt)*q;
	*c2 = (1+2*h*w0*dt+(w0*dt)*(w0*dt))*q;
	return true;
}

bool coefficients_from_T1_T2(double fsamp, double gain, double T1, double T2, double *c0, double *c1, double *c2)
{
	// derived by Joachim Saul
	double w1 = 2*M_PI/T1, w2 = 2*M_PI/T2;
	double dt = 1./fsamp;
	double q  = 1./(gain*dt);

	*c0 = q;
	*c1 = -2*(1+0.5*(w1+w2)*dt)*q;
	*c2 = (1+(w1+w2)*dt+w1*w2*dt*dt)*q;
	return true;
}


template<class TYPE>
TimeDomain<TYPE>::TimeDomain()
{
	bandpass = 0;
	fsamp = 0;
	dt = 0;
	init();
}

template<class TYPE>
TimeDomain<TYPE>::~TimeDomain()
{
	if ( bandpass != NULL ) delete bandpass;
}

template<class TYPE>
void TimeDomain<TYPE>::init()
{
	a2 = a1 = y1 = y0 = 0.;
	cumsum1 = cumsum2 = 0;
}


template<class TYPE>
int TimeDomain<TYPE>::setParameters(int n, const double *params) {
	return 0;
}


template<class TYPE>
void TimeDomain<TYPE>::apply(int n, TYPE *inout)
{
	for (int i=0; i<n; i++) {
		y2  = inout[i];
		a2 = a1 + c2*y2 + c1*y1 + c0*y0;
		y0 = y1;
		y1 = y2;
		inout[i] = a1 = a2;
	}

	// Apply bandpass before double integration, which according
	// to Kanamori and Rivera (2008) is numerically advantageous
	// compared to first double integration followed by bandpass
	// filtering, even though analytically equivalent.
	if (bandpass)
		bandpass->apply(n,inout);
	// else ???

	// Double-integration *after* bandpas filtering, see Kanamori & Rivera (2008)
	for (int i=0; i<n; i++) {
		// cause a delay of one sample (half a sample per integration)
		cumsum1 += inout[i]*dt;
		inout[i] = cumsum2;
		cumsum2 += cumsum1*dt;
	}

	// FIXME: We still observe about half a sample of delay compared to
	// FIXME: FFT version. This isn't going to be a big problem for most
	// FIXME: applications, but there must be an explanation or fix.
}

template<class TYPE>
void TimeDomain<TYPE>::setBandpass(int _order, double _fmin, double _fmax)
{
	order = _order;
	fmin  = _fmin;
	fmax  = _fmax;

	bandpass = new Filtering::IIR::ButterworthHighLowpass<TYPE>(order, fmin, fmax);
	if ( fsamp>0 )
		bandpass->setSamplingFrequency(fsamp);
}

template<class TYPE>
void TimeDomain<TYPE>::setSamplingFrequency(double _fsamp)
{
	fsamp = _fsamp;
	if ( _fsamp>0 ) {
		dt = 1.0 / fsamp;
		init();
	}

	if ( bandpass && _fsamp > 0 )
		bandpass->setSamplingFrequency(_fsamp);
}

template<class TYPE>
void TimeDomain<TYPE>::setCoefficients(double _c0, double _c1, double _c2)
{
	c0 = _c0;  c1 = _c1;  c2 = _c2;
}

template<class TYPE>
std::string TimeDomain<TYPE>::print() const
{
	std::stringstream tmp;
	if ( fsamp <= 0. )
		tmp << "  Not yet initialized" << std::endl;
	else {
		tmp << "  fsamp   = " <<   fsamp << std::endl;
		tmp << "  gain    = " <<    gain << std::endl;
		tmp << "  c0*gain = " << c0*gain << std::endl;
		tmp << "  c1*gain = " << c1*gain << std::endl;
		tmp << "  c2*gain = " << c2*gain << std::endl;
	}
	return tmp.str();
}


template<class TYPE>
TimeDomain_from_T0_h<TYPE>::TimeDomain_from_T0_h(double T0, double h, double _gain, double fsamp)
  : T0(T0), h(h) {
	TimeDomain<TYPE>::gain=_gain;

	if ( fsamp )
		TimeDomain<TYPE>::setSamplingFrequency(fsamp);
}

template<class TYPE>
void TimeDomain_from_T0_h<TYPE>::init()
{
	double c0,c1,c2;
	coefficients_from_T0_h(TimeDomain<TYPE>::fsamp,
				 TimeDomain<TYPE>::gain,
				 T0, h, &c0, &c1, &c2);
	TimeDomain<TYPE>::setCoefficients(c0,c1,c2);
	TimeDomain<TYPE>::init();
}


template<class TYPE>
void TimeDomain_from_T0_h<TYPE>::setBandpass(int order, double fmin, double fmax) {
	TimeDomain<TYPE>::setBandpass(order, fmin, fmax);
}


template<class TYPE>
Filtering::InPlaceFilter<TYPE>* TimeDomain_from_T0_h<TYPE>::clone() const {
	return new TimeDomain_from_T0_h<TYPE>(T0, h, TimeDomain<TYPE>::gain, TimeDomain<TYPE>::fsamp);
}

template<class TYPE>
std::string TimeDomain_from_T0_h<TYPE>::print() const
{
	std::stringstream tmp;
	tmp << "TimeDomainRestitution_from_T0_h instance:" << std::endl;
	tmp << "  T0      = " << T0   << std::endl;
	tmp << "  h       = " << h    << std::endl;
	tmp << TimeDomain<TYPE>::print();
	return tmp.str();
}


template<class TYPE>
TimeDomain_from_T1_T2<TYPE>::TimeDomain_from_T1_T2(double T1, double T2, double _gain, double fsamp)
  : T1(T1), T2(T2) {
	TimeDomain<TYPE>::gain = _gain;
	if ( fsamp )
		TimeDomain<TYPE>::setSamplingFrequency(fsamp);
}

template<class TYPE>
void TimeDomain_from_T1_T2<TYPE>::init()
{
	double c0,c1,c2;
	coefficients_from_T1_T2(TimeDomain<TYPE>::fsamp, TimeDomain<TYPE>::gain, T1, T2, &c0, &c1, &c2);

	TimeDomain<TYPE>::setCoefficients(c0,c1,c2);
	TimeDomain<TYPE>::init();
}

template<class TYPE>
	void TimeDomain_from_T1_T2<TYPE>::setBandpass(int order, double fmin, double fmax) {
	TimeDomain<TYPE>::setBandpass(order, fmin, fmax);
}


template<class TYPE>
Filtering::InPlaceFilter<TYPE>* TimeDomain_from_T1_T2<TYPE>::clone() const {
	return new TimeDomain_from_T1_T2<TYPE>(T1, T2, TimeDomain<TYPE>::gain, TimeDomain<TYPE>::fsamp);
}

template<class TYPE>
std::string TimeDomain_from_T1_T2<TYPE>::print() const
{
	std::stringstream tmp;
	tmp << "TimeDomainRestitution_from_T1_T2 instance:" << std::endl;
	tmp << "  T1      = " << T1   << std::endl;
	tmp << "  T2      = " << T2   << std::endl;
	tmp << TimeDomain<TYPE>::print();
	return tmp.str();
}


template class SC_SYSTEM_CORE_API TimeDomain<float>;
template class SC_SYSTEM_CORE_API TimeDomain<double>;

template class SC_SYSTEM_CORE_API TimeDomain_from_T0_h<float>;
template class SC_SYSTEM_CORE_API TimeDomain_from_T0_h<double>;

template class SC_SYSTEM_CORE_API TimeDomain_from_T1_T2<float>;
template class SC_SYSTEM_CORE_API TimeDomain_from_T1_T2<double>;


}       // namespace Seiscomp::Math::Filtering
}       // namespace Seiscomp::Math
}       // namespace Seiscomp

