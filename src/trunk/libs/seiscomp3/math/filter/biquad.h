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


#ifndef _SEISCOMP_BIQUAD_H_
#define _SEISCOMP_BIQUAD_H_

#include<vector>
#include<string>
#include<iostream>

#include<seiscomp3/math/filter.h>

namespace Seiscomp {
namespace Math {
namespace Filtering {
namespace IIR {

// Basic biquad class implementing type-independent functionality,
// i.e. the actual filtering is not yet implemented; see Biquad<TYPE>
class _Biquad
{
    public:
	// initialize Biquad using 2x3 coefficients (default is identity)
	_Biquad(double _a0=1, double _a1=0, double _a2=0,
	        double _b0=1, double _b1=0, double _b2=0);
	_Biquad(_Biquad const &other);

	void set(double _a0, double _a1, double _a2,
		 double _b0, double _b1, double _b2);

	// compute the group delay at nsamp frequencies
	int  delay (int nsamp, double *delay_val);
	int  delay2(int nsamp, double *delay_val); // XXX preliminary
	// compute the group delay of the biquad at a single frequency
	int  delay_one(double freq /* 0...PI */, double *delay);
	// reset the filter by erasing memory of past samples
	void reset();

	// for debugging
	std::string print() const;

	// filter coefficients
        double a0, a1, a2;
        double b0, b1, b2;
//  private:
	// memory of past samples
        double v1, v2;
};


// Template class to extend the type-independent _Biquad by the
// type-dependent apply()/filter()-functions
template<typename TYPE>
class Biquad : public _Biquad, public InPlaceFilter<TYPE>
{
    public:

	Biquad(double a0=0, double a1=0, double a2=0,
	       double b0=1, double b1=0, double b2=0);
	Biquad(_Biquad const &other);
	Biquad(Biquad const &bq);
	~Biquad() {}

	// apply filter to data vector **in*place**
	void apply(int n, TYPE *inout);

	// Implement InPlaceFilter interface with default values
	void setSamplingFrequency(double fsamp);
	int setParameters(int n, const double *params);

	InPlaceFilter<TYPE>* clone() const;

	// filter data vector by returning a filtered copy
	std::vector<TYPE> filter (std::vector<TYPE> const &f);

}; // class Biquad


template<typename TYPE>
class BiquadCascade : public InPlaceFilter<TYPE>
{
    public:

	BiquadCascade();
	BiquadCascade(BiquadCascade const &other);
	~BiquadCascade();

	// number of biquads comprising the cascade
	int size() const;

	// apply filter to data vector **in*place**
	void apply(int n, TYPE *inout);
	virtual InPlaceFilter<TYPE>* clone() const;

	// filter data vector by returning a filtered copy
	std::vector<TYPE> filter (std::vector<TYPE> const &f);

	// filter data vectors **in*place**
//	void apply(std::vector<int> &f);
//	void apply(std::vector<double> &f);

//	void delay(int nsamp, double *delay_val);
//	void delay_one(double freq /* 0...PI */, double *delay);

	// resets the filter, i.e. erases the filter memory
	void reset();

	// for debugging
	std::string print() const;

	virtual void setSamplingFrequency(double /*fsamp*/) {} // FIXME
	virtual int setParameters(int n, const double *params);

	// append a single biquad to this cascade
	void append(Biquad<TYPE> const &biq);

//	void extend(BiquadCascade const &other);

    protected:
	void _clear() { _biq.clear(); }

    private:
	std::vector< Biquad<TYPE> > _biq;

}; // class BiquadCascade

} // namespace Seiscomp::Math::Filtering::IIR
} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp

#endif

// XXX Read biquad.cpp for additional information XXX
