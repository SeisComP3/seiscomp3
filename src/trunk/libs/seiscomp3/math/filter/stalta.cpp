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


#include<seiscomp3/math/filter/stalta.h>

namespace Seiscomp {
namespace Math {
namespace Filtering {

namespace // private namespace
{

template <typename T>
T _average_sq(int n, T *f)
{
	int i=n;
	T avg=0.;
	if (n<=0) return 0.;
	while (i--) avg += (*f)*(*(f++));
	return avg/n;
}

}

template<typename TYPE>
STALTA<TYPE>::STALTA(double lenSTA, double lenLTA, double fsamp)
{
	_lenSTA = lenSTA;
	_lenLTA = lenLTA;
	_sampleCount = 0;
	_initLength = 0;
	_saveIntermediate = false;
	setSamplingFrequency(fsamp);
}

/*
template<typename TYPE>
STALTA<TYPE>::STALTA(int numSTA, int numLTA)
{
	_numSTA  = numSTA;
	_numLTA  = numLTA;
	_saveIntermediate = false;
	reset();
}
*/

template<typename TYPE>
void
STALTA<TYPE>::setSamplingFrequency(double fsamp)
{
	_fsamp  = fsamp;
	_numSTA = int(_lenSTA*fsamp+0.5);
	_numLTA = int(_lenLTA*fsamp+0.5);
	_initLength = _numLTA/2;
	reset();
}


template<typename TYPE>
int
STALTA<TYPE>::setParameters(int n, const double *params)
{
	if ( n != 2 ) return 2;

	_lenSTA = params[0];
	_lenLTA = params[1];

	return 2;
}


template<typename TYPE>
void 
STALTA<TYPE>::reset()
{
	_sampleCount = 0;
	_sta =  0.;	// initial STA
	_lta =  0.;	// initial LTA set in apply()
}

#define FABS(x) (((x)<0)?(-(x)):(x))

template<typename TYPE>
void
STALTA<TYPE>::setSaveIntermediate(bool e)
{
	_saveIntermediate = e;
}


template<typename TYPE>
void
STALTA<TYPE>::apply(int ndata, TYPE *data)
{
	double inlta = 1./_numLTA, insta = 1./_numSTA;

	if (_saveIntermediate) {
		_staVector.resize(ndata);
		_ltaVector.resize(ndata);
	}

	for (int i=0; i<ndata; ++i, ++data) {	
		if (_sampleCount < _initLength) {
			// immediately after initialization
			_lta = (_sampleCount*_lta+FABS(*data))/(_sampleCount+1);
			_sta = _lta;
			*data = 1.;
			_sampleCount++;
		}
		else {
			// normal behaviour
			double q = (_sta - _lta)*inlta;
			_lta += q;
			_sta += (FABS(*data) - _sta)*insta;
			*data = (TYPE)(_sta/_lta);
		}

		if (_saveIntermediate) {
			_staVector[i] = (TYPE)_sta;
			_ltaVector[i] = (TYPE)_lta;
		}
	}
}

template <typename TYPE>
InPlaceFilter<TYPE>* STALTA<TYPE>::clone() const {
	return new STALTA<TYPE>(_lenSTA, _lenLTA, _fsamp);
}


INSTANTIATE_INPLACE_FILTER(STALTA, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(STALTA, "STALTA");


template<typename TYPE>
STALTA2<TYPE>::STALTA2(double lenSTA, double lenLTA, double eventOn,
                       double eventOff, double fsamp)
{
	_lenSTA = lenSTA;
	_lenLTA = lenLTA;
	_eventOn = eventOn;
	_eventOff = eventOff;
	_sampleCount = 0;
	_initLength = 0;
	_bleed = 1.;
	_saveIntermediate = false;
	setSamplingFrequency(fsamp);
}


template<typename TYPE>
void
STALTA2<TYPE>::setSamplingFrequency(double fsamp)
{
	_fsamp  = fsamp;
	_numSTA = int(_lenSTA*fsamp+0.5);
	_numLTA = int(_lenLTA*fsamp+0.5);
	_initLength = _numLTA/2;
	reset();
}


template<typename TYPE>
int
STALTA2<TYPE>::setParameters(int n, const double *params)
{
	if ( n != 4 ) return 4;

	_lenSTA = params[0];
	_lenLTA = params[1];
	_eventOn = params[2];
	_eventOff = params[3];

	return 4;
}


template<typename TYPE>
void
STALTA2<TYPE>::reset()
{
	_sampleCount = 0;
	_sta =  0.;	// initial STA
	_lta =  0.;	// initial LTA set in apply()
	_bleed = 1.;
}


template<typename TYPE>
void
STALTA2<TYPE>::setSaveIntermediate(bool e)
{
	_saveIntermediate = e;
}


template<typename TYPE>
void
STALTA2<TYPE>::apply(int ndata, TYPE *data)
{
	double inlta = 1./_numLTA, insta = 1./_numSTA;

	if (_saveIntermediate) {
		_staVector.resize(ndata);
		_ltaVector.resize(ndata);
	}

	for (int i=0; i<ndata; ++i, ++data) {
		if (_sampleCount < _initLength) {
			// immediately after initialization
			_lta = (_sampleCount*_lta+FABS(*data))/(_sampleCount+1);
			_sta = _lta;
			*data = 1.;
			_sampleCount++;
		}
		else {
			// normal behaviour
			double q = (_sta - _lta)*inlta;
			_lta += q*_bleed;
			_sta += (FABS(*data) - _sta)*insta;
			*data = (TYPE)(_sta/_lta);

			if ( (_bleed > 0.) && (*data > _eventOn) ) _bleed = 0.;
			else if ( (_bleed < 1.) && (*data < _eventOff) ) _bleed = 1.;
		}

		if (_saveIntermediate) {
			_staVector[i] = (TYPE)_sta;
			_ltaVector[i] = (TYPE)_lta;
		}
	}
}


template <typename TYPE>
InPlaceFilter<TYPE>* STALTA2<TYPE>::clone() const {
	return new STALTA2<TYPE>(_lenSTA, _lenLTA, _eventOn, _eventOff, _fsamp);
}


INSTANTIATE_INPLACE_FILTER(STALTA2, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(STALTA2, "STALTA2");


} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp

