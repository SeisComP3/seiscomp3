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


// This file is included by "biquad.cpp"

template<typename TYPE>
Biquad<TYPE>::Biquad(double a0, double a1, double a2,
	             double b0, double b1, double b2)
	: _Biquad(a0, a1, a2, b0, b1, b2),
	  InPlaceFilter<TYPE>() { }

template<typename TYPE>
Biquad<TYPE>::Biquad(_Biquad const &other)
	: _Biquad(other),
	  InPlaceFilter<TYPE>() { }

template<typename TYPE>
Biquad<TYPE>::Biquad(Biquad<TYPE> const &bq)
	: _Biquad(bq.a0, bq.a1, bq.a2, bq.b0, bq.b1, bq.b2),
	  InPlaceFilter<TYPE>() { }

template<typename TYPE>
void Biquad<TYPE>::apply(int n, TYPE *inout)
{
	// here we really need optimum performance so we *don't*
	// use the std::vector<>::iterator - *DO* *NOT* *CHANGE*
	// (std::vector is guaranteed to be contiguous in memory)
	TYPE *ff = inout; // no problem
	for (int i=0;  i < n;  i++)
	{	// XXX this assumes that b0==1 XXX
		double v0 =      ff[i]  - b1*v1 - b2*v2;
		ff[i]     =  TYPE(a0*v0 + a1*v1 + a2*v2);
		v2 = v1; v1 = v0;
	}
}

template<typename TYPE>
InPlaceFilter<TYPE>* Biquad<TYPE>::clone() const
{
	return new Biquad<TYPE>(a0, a1, a2, b0, b1, b2);
}

template<typename TYPE>
void Biquad<TYPE>::setSamplingFrequency(double fsamp) {}

template<typename TYPE>
int Biquad<TYPE>::setParameters(int n, const double *params) {
	if ( n != 6 ) return 6;

	set(params[0], params[1], params[2],
	    params[3], params[4], params[5]);

	return n;
}

template<typename TYPE>
std::vector<TYPE> Biquad<TYPE>::filter(std::vector<TYPE> const &f)
{
	std::vector<TYPE> copy(f.begin(), f.end());
	apply(copy.size(), &(copy[0]));
	return copy;
}






template<typename TYPE>
BiquadCascade<TYPE>::BiquadCascade() {}

template<typename TYPE>
BiquadCascade<TYPE>::BiquadCascade(BiquadCascade const &other)
{
	typename std::vector< Biquad<TYPE> >::const_iterator biq;
	for (biq = other._biq.begin(); biq!=other._biq.end(); biq++)
		_biq.push_back(*biq);
}

template<typename TYPE>
BiquadCascade<TYPE>::~BiquadCascade() {}

template<typename TYPE>
int BiquadCascade<TYPE>::size() const { return _biq.size(); }

template<typename TYPE>
void BiquadCascade<TYPE>::apply(int n, TYPE *inout)
{
	typename std::vector< Biquad<TYPE> >::iterator biq;
	for (biq = _biq.begin(); biq != _biq.end(); biq++)
		biq->apply(n, inout);
}

template<typename TYPE>
int BiquadCascade<TYPE>::setParameters(int /*n*/, const double* /*params*/) {
	return 0;
}

template<typename TYPE>
InPlaceFilter<TYPE>* BiquadCascade<TYPE>::clone() const {
	return new BiquadCascade<TYPE>(*this);
}


template<typename TYPE>
std::vector<TYPE> BiquadCascade<TYPE>::filter (std::vector<TYPE> const &f)
{
	std::vector<TYPE> copy(f.begin(), f.end());
	InPlaceFilter<TYPE>::apply(copy);
	return copy;
}

template<typename TYPE>
void BiquadCascade<TYPE>::reset()
{
	typename std::vector< Biquad<TYPE> >::iterator biq;
	for (biq = _biq.begin(); biq != _biq.end(); biq++)
		biq->reset();
}

template<typename TYPE>
std::string BiquadCascade<TYPE>::print() const
{
	std::ostringstream s;
	int i=0;
	typename std::vector< Biquad<TYPE> >::const_iterator biq;
	for (biq = _biq.begin(); biq != _biq.end(); biq++)
		s << "Biquad #" << ++i << std::endl << (biq->print());
	return s.str();
}

template<typename TYPE>
void BiquadCascade<TYPE>::append(Biquad<TYPE> const &biq)
{
	_biq.push_back(biq);
}

/*	
template<typename TYPE>
void BiquadCascade<TYPE>::extend(BiquadCascade const &other)
{
        typename std::vector< Biquad<TYPE> >::const_iterator biq;
	for (biq = other._biq.begin(); biq!=other._biq.end(); biq++)
		_biq.push_back(*biq);
}
*/

