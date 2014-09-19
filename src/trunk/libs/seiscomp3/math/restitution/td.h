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

#ifndef _SEIS_SIGNAL_TDRESTITUTION_H_
#define _SEIS_SIGNAL_TDRESTITUTION_H_

#include <math.h>
#include <seiscomp3/math/filter.h>
#include <seiscomp3/math/filter/butterworth.h>

namespace Seiscomp {
namespace Math {
namespace Restitution {

// subroutines to compute parameters for the recursive filter

// from seismometer eigenperiod T0 and damping parameter h bool 
bool coefficients_from_T0_h(double fsamp, double gain, double T0, double h, double *c0, double *c1, double *c2);

// from the two seismometer eigenperiods T1 and T2
bool coefficients_from_T1_T2(double fsamp, double gain, double T1, double T2, double *c0, double *c1, double *c2);

template<typename TYPE>
class TimeDomain : public Filtering::InPlaceFilter<TYPE> {
	public:
		TimeDomain();
		~TimeDomain();

		// configuration
		void setBandpass(int order, double fmin, double fmax);
		void setSamplingFrequency(double fsamp);
		void setCoefficients(double c0, double c1, double c2);

		virtual int setParameters(int n, const double *params);

		virtual void reset() {}
		virtual void apply(int n, TYPE *inout);
		virtual std::string print() const;

	protected:
		virtual void init();

	protected:
		// configuration
		double c0, c1, c2;
		double fsamp, dt;
		double gain;

	private:
		// filter configuration
		int order;
		double fmin, fmax;
		// temp variables
		double y0, y1, y2, a1, a2;
		double cumsum1, cumsum2;

		Filtering::IIR::ButterworthBandpass<TYPE> *bandpass;
};

template<typename TYPE>
class TimeDomain_from_T0_h: public TimeDomain<TYPE> {
	public:
		TimeDomain_from_T0_h(double T0, double h, double gain, double fsamp=0);

		void setBandpass(int order, double fmin, double fmax);
		virtual std::string print() const;

		Filtering::InPlaceFilter<TYPE>* clone() const;

	protected:
		virtual void init();
	
	private:
		// configuration
		double T0, h;
};

template<typename TYPE>
class TimeDomain_from_T1_T2: public TimeDomain<TYPE> {
	public:
		TimeDomain_from_T1_T2(double T1, double T2, double gain, double fsamp=0);

		void setBandpass(int order, double fmin, double fmax);
		virtual std::string print() const;

		Filtering::InPlaceFilter<TYPE>* clone() const;

	protected:
		virtual void init();

	private:
		// configuration
		double T1, T2;
};


} // namespace Seiscomp::Math::Restitution
} // namespace Seiscomp::Math
} // namespace Seiscomp

#endif
