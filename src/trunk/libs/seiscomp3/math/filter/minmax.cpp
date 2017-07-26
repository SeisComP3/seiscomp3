/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Author: Jan Becker, jabe@gempa.de                                     *
 ***************************************************************************/


#define SEISCOMP_COMPONENT MinMax

#include <math.h>

#include <seiscomp3/math/filter/minmax.h>
#include <seiscomp3/core/exceptions.h>
#include<seiscomp3/logging/log.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
MinMax<TYPE>::MinMax(double timeSpan /*sec*/, double fsamp)
: _timeSpan(timeSpan), _fsamp(0.0) {
	if ( fsamp )
		setSamplingFrequency(fsamp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
void MinMax<TYPE>::setLength(double timeSpan) {
	_timeSpan = timeSpan;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
void MinMax<TYPE>::setSamplingFrequency(double fsamp) {
	if ( _fsamp == fsamp ) return;

	_fsamp = fsamp;
	_sampleCount = (int)(_fsamp * _timeSpan);
	if ( _sampleCount < 1 ) _sampleCount = 1;

	_index = 0;

	_buffer.resize(_sampleCount);
	_firstSample = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
int MinMax<TYPE>::setParameters(int n, const double *params) {
	if ( n != 1 ) return 1;
	if ( params[0] <= 0 )
		return -1;

	_timeSpan = params[0];
	return n;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
void MinMax<TYPE>::reset() {
	_firstSample = true;
	_index = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
Min<TYPE>::Min(double timeSpan /*sec*/, double fsamp)
: MinMax<TYPE>(timeSpan, fsamp) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
InPlaceFilter<TYPE>* Min<TYPE>::clone() const {
	return new Min<TYPE>(MinMax<TYPE>::_timeSpan, MinMax<TYPE>::_fsamp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
void Min<TYPE>::apply(int n, TYPE *inout) {
	if ( MinMax<TYPE>::_fsamp == 0.0 )
		throw Seiscomp::Core::GeneralException("Samplerate not initialized");

	// Initialize the minmax buffer with the first sample
	if ( MinMax<TYPE>::_firstSample && n ) {
		std::fill(MinMax<TYPE>::_buffer.begin(), MinMax<TYPE>::_buffer.end(), inout[0]);
		_minimum = inout[0];
		MinMax<TYPE>::_firstSample = false;
	}

	for ( int i = 0; i < n; ++i ) {
		TYPE poppedValue = MinMax<TYPE>::_buffer[MinMax<TYPE>::_index];
		MinMax<TYPE>::_buffer[MinMax<TYPE>::_index] = inout[i];

		if ( MinMax<TYPE>::_buffer[MinMax<TYPE>::_index] < _minimum )
			_minimum = MinMax<TYPE>::_buffer[MinMax<TYPE>::_index];
		else if ( poppedValue <= _minimum ) {
			// A maximum has been removed, check for new one
			TYPE oldMinimum = _minimum;

			// Recompute maximum
			_minimum = MinMax<TYPE>::_buffer[0];
			if ( _minimum > oldMinimum ) {
				for ( int j = 1; j < MinMax<TYPE>::_sampleCount; ++j ) {
					if ( MinMax<TYPE>::_buffer[j] < _minimum ) {
						_minimum = MinMax<TYPE>::_buffer[j];
						if ( _minimum <= oldMinimum ) break;
					}
				}
			}
		}

		inout[i] = _minimum;

		++MinMax<TYPE>::_index;

		if ( MinMax<TYPE>::_index >= MinMax<TYPE>::_sampleCount )
			MinMax<TYPE>::_index = 0;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
Max<TYPE>::Max(double timeSpan /*sec*/, double fsamp)
: MinMax<TYPE>(timeSpan, fsamp) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
InPlaceFilter<TYPE>* Max<TYPE>::clone() const {
	return new Max<TYPE>(MinMax<TYPE>::_timeSpan, MinMax<TYPE>::_fsamp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
void Max<TYPE>::apply(int n, TYPE *inout) {
	if ( MinMax<TYPE>::_fsamp == 0.0 )
		throw Seiscomp::Core::GeneralException("Samplerate not initialized");

	// Initialize the minmax buffer with the first sample
	if ( MinMax<TYPE>::_firstSample && n ) {
		std::fill(MinMax<TYPE>::_buffer.begin(), MinMax<TYPE>::_buffer.end(), inout[0]);
		_maximum = inout[0];
		MinMax<TYPE>::_firstSample = false;
	}

	for ( int i = 0; i < n; ++i ) {
		TYPE poppedValue = MinMax<TYPE>::_buffer[MinMax<TYPE>::_index];
		MinMax<TYPE>::_buffer[MinMax<TYPE>::_index] = inout[i];

		if ( MinMax<TYPE>::_buffer[MinMax<TYPE>::_index] > _maximum )
			_maximum = MinMax<TYPE>::_buffer[MinMax<TYPE>::_index];
		else if ( poppedValue >= _maximum ) {
			// A maximum has been removed, check for new one
			TYPE oldMaximum = _maximum;

			// Recompute maximum
			_maximum = MinMax<TYPE>::_buffer[0];
			if ( _maximum < oldMaximum ) {
				for ( int j = 1; j < MinMax<TYPE>::_sampleCount; ++j ) {
					if ( MinMax<TYPE>::_buffer[j] > _maximum ) {
						_maximum = MinMax<TYPE>::_buffer[j];
						if ( _maximum >= oldMaximum ) break;
					}
				}
			}
		}

		inout[i] = _maximum;

		++MinMax<TYPE>::_index;

		if ( MinMax<TYPE>::_index >= MinMax<TYPE>::_sampleCount )
			MinMax<TYPE>::_index = 0;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
INSTANTIATE_INPLACE_FILTER(Min, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(Min, "MIN");
INSTANTIATE_INPLACE_FILTER(Max, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(Max, "MAX");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp
