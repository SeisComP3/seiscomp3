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


#define SEISCOMP_COMPONENT Average

#include <math.h>

#include <seiscomp3/math/filter/average.h>
#include <seiscomp3/core/exceptions.h>
#include<seiscomp3/logging/log.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {

namespace _private {

class FilterException : public Seiscomp::Core::GeneralException {
	public:
		FilterException() : GeneralException("filter exception") {}
		FilterException(std::string what) : GeneralException(what) {}
};


}

template<typename TYPE>
Average<TYPE>::Average(double timeSpan /*sec*/, double fsamp)
 : _timeSpan(timeSpan), _fsamp(0.0) {
	if ( fsamp )
		setSamplingFrequency(fsamp);
}


template<typename TYPE>
void Average<TYPE>::apply(int n, TYPE *inout) {
	if ( _fsamp == 0.0 )
		throw _private::FilterException("Samplerate not initialized");

	// Initialize the average buffer with the first sample
	if ( _firstSample && n ) {
		std::fill(_buffer.begin(), _buffer.end(), inout[0]);
		_lastSum = inout[0] * _buffer.size();
		_firstSample = false;
	}

	for ( int i = 0; i < n; ++i ) {
		_lastValue = inout[i];

		TYPE firstValue = _buffer[_index];
		_buffer[_index] = _lastValue;

		++_index;

		if ( _index >= _sampleCount )
			_index = 0;

		_lastSum = _lastSum + _lastValue - firstValue;

		inout[i] = (TYPE)(_lastSum * _oocount);
	}
}


template<typename TYPE>
InPlaceFilter<TYPE>* Average<TYPE>::clone() const {
	return new Average<TYPE>(_timeSpan, _fsamp);
}


template<typename TYPE>
void Average<TYPE>::setLength(double timeSpan) {
	_timeSpan = timeSpan;
}


template<typename TYPE>
void Average<TYPE>::setSamplingFrequency(double fsamp) {
	if ( _fsamp == fsamp ) return;

	_fsamp = fsamp;
	_sampleCount = (int)(_fsamp * _timeSpan);
	if ( _sampleCount < 1 ) _sampleCount = 1;
	_lastSum = 0;
	_index = 0;

	_oocount = 1.0/_sampleCount;

	_buffer.resize(_sampleCount);
	_firstSample = true;
}


template<typename TYPE>
int Average<TYPE>::setParameters(int n, const double *params) {
	if ( n != 1 ) return 1;
	if ( params[0] <= 0 )
		return -1;

	_timeSpan = params[0];
	return n;
}


template<typename TYPE>
void Average<TYPE>::reset() {
	_firstSample = true;
	_lastSum = 0;
	_index = 0;
}


INSTANTIATE_INPLACE_FILTER(Average, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(Average, "AVG");


} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp
