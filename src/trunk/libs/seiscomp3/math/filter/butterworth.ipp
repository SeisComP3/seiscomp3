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


// This file is included by "butterworth.cpp"


template<class TYPE>
ButterworthLowpass<TYPE>::ButterworthLowpass(int order, double fmax, double fsamp)
: _order(order), _fmax(fmax), _fsamp(0.0) {
	if ( fsamp )
		setSamplingFrequency(fsamp);
}

template<class TYPE>
void ButterworthLowpass<TYPE>::setSamplingFrequency(double fsamp) {
	if (_fsamp == fsamp)
		return;

	_fsamp = fsamp;

	BiquadCascade<TYPE>::set(init_bw_biquads(_order, 0, _fmax, _fsamp, BUTTERWORTH_LOWPASS));
}

template<typename TYPE>
int ButterworthLowpass<TYPE>::setParameters(int n, const double *params) {
	if ( n != 2 ) return 2;

	int order = (int)params[0];

	if ( order <= 0 )
		return -1;

	_order = order;
	_fmax = params[1];

	return 2;
}

template<typename TYPE>
InPlaceFilter<TYPE>* ButterworthLowpass<TYPE>::clone() const {
	return new ButterworthLowpass<TYPE>(_order, _fmax, _fsamp);
}

template<class TYPE>
ButterworthHighpass<TYPE>::ButterworthHighpass(int order, double fmin, double fsamp)
: _order(order), _fmin(fmin), _fsamp(0.0) {
	if ( fsamp )
		setSamplingFrequency(fsamp);
}

template<class TYPE>
void ButterworthHighpass<TYPE>::setSamplingFrequency(double fsamp) {
	if ( _fsamp == fsamp )
		return;

	_fsamp = fsamp;

	BiquadCascade<TYPE>::set(init_bw_biquads(_order, _fmin, 0, _fsamp, BUTTERWORTH_HIGHPASS));
}

template<typename TYPE>
int ButterworthHighpass<TYPE>::setParameters(int n, const double *params) {
	if ( n != 2 ) return 2;

	int order = (int)params[0];

	if ( order <= 0 )
		return -1;

	_order = order;
	_fmin = params[1];

	return 2;
}

template<typename TYPE>
InPlaceFilter<TYPE>* ButterworthHighpass<TYPE>::clone() const {
	return new ButterworthHighpass<TYPE>(_order, _fmin, _fsamp);
}


template<class TYPE>
ButterworthBandpass<TYPE>::ButterworthBandpass(int order, double fmin, double fmax,
                                               double fsamp)
: _order(order), _fmin(fmin), _fmax(fmax), _fsamp(0.0){
	if ( fsamp )
		setSamplingFrequency(fsamp);
}


template<class TYPE>
void ButterworthBandpass<TYPE>::setSamplingFrequency(double fsamp) {
	if ( _fsamp == fsamp )
		return;

	_fsamp = fsamp;

	BiquadCascade<TYPE>::set(init_bw_biquads(_order,_fmin,_fmax,_fsamp, BUTTERWORTH_BANDPASS));
}

template<typename TYPE>
int ButterworthBandpass<TYPE>::setParameters(int n, const double *params) {
	if ( n != 3 ) return 3;

	_order = (int)params[0];
	_fmin = params[1];
	_fmax = params[2];

	return n;
}

template<typename TYPE>
InPlaceFilter<TYPE>* ButterworthBandpass<TYPE>::clone() const {
	return new ButterworthBandpass<TYPE>(_order, _fmin, _fmax, _fsamp);
}

template<class TYPE>
ButterworthBandstop<TYPE>::ButterworthBandstop(int order, double fmin, double fmax,
                                               double fsamp)
: _order(order), _fmin(fmin), _fmax(fmax), _fsamp(0.0) {
	if ( fsamp )
		setSamplingFrequency(fsamp);
}


template<class TYPE>
void ButterworthBandstop<TYPE>::setSamplingFrequency(double fsamp) {
	if ( _fsamp == fsamp )
		return;

	_fsamp = fsamp;

	BiquadCascade<TYPE>::set(init_bw_biquads(_order,_fmin,_fmax,_fsamp, BUTTERWORTH_BANDSTOP));
}

template<typename TYPE>
int ButterworthBandstop<TYPE>::setParameters(int n, const double *params) {
	if ( n != 3 ) return 3;

	_order = (int)params[0];
	_fmin = params[1];
	_fmax = params[2];

	return n;
}

template<typename TYPE>
InPlaceFilter<TYPE>* ButterworthBandstop<TYPE>::clone() const {
	return new ButterworthBandstop<TYPE>(_order, _fmin, _fmax, _fsamp);
}

template<class TYPE>
ButterworthHighLowpass<TYPE>::ButterworthHighLowpass(int order, double fmin,
                                                     double fmax, double fsamp)
: _order(order), _fmin(fmin), _fmax(fmax), _fsamp(0.0) {
	if ( fsamp )
		setSamplingFrequency(fsamp);
}

template<class TYPE>
void ButterworthHighLowpass<TYPE>::setSamplingFrequency(double fsamp) {
	if ( _fsamp == fsamp )
		return;

	_fsamp = fsamp;

	BiquadCascade<TYPE>::set(init_bw_biquads(_order, _fmin, _fmax, _fsamp, BUTTERWORTH_HIGHLOWPASS));
}


template<class TYPE>
int ButterworthHighLowpass<TYPE>::setParameters(int n, const double *params) {
	if ( n != 3 ) return 3;

	_order = (int)params[0];
	_fmin = params[1];
	_fmax = params[2];

	return n;
}


template<class TYPE>
InPlaceFilter<TYPE>* ButterworthHighLowpass<TYPE>::clone() const {
	return new ButterworthHighLowpass<TYPE>(_order, _fmin, _fmax, _fsamp);
}
