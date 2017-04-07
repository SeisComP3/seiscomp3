/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *   Author: Jan Becker, gempa GmbH                                        *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SPEC

#include "spectralizer.h"

#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/math/fft.h>
#include <seiscomp3/math/windows/hann.h>

#include <cstring>

using namespace std;
using namespace Seiscomp;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


void copy(DoubleArray &dst, int iofs, vector<double> &src, int front) {
	int chunkSize = src.size()-front;
	memcpy(dst.typedData()+iofs, src.data()+front, chunkSize*sizeof(double));
	memcpy(dst.typedData()+iofs+chunkSize, src.data(), front*sizeof(double));
}


struct Mag {
	static double score(const Math::Complex &c) {
		return abs(c);
	}

	static double score(const double &c) {
		return fabs(c);
	}
};


template <typename S, typename T>
void reduce(TypedArray<T> &spec, int n) {
	int s = spec.size();

	if ( n > s ) return;

	int from = 0;
	typename TypedArray<T>::Type m;

	for ( int i = 0; i < n; ++i ) {
		int to = (i+1)*s/n;

		m = spec[from];
		for ( int j = from+1; j < to; ++j ) {
			if ( S::score(m) < S::score(spec[j]) )
				m = spec[j];
		}
		spec[i] = m;

		from = to;
	}

	spec.resize(n);
}


template <typename T>
void demean(int n, T *inout) {
	T sum = 0;
	for ( int i = 0; i < n; ++i )
		sum += inout[i];

	T mean = sum/n;
	for ( int i = 0; i < n; ++i )
		inout[i] -= mean;
}


template <typename T>
void detrend(int n, T *data) {
	if ( n <= 1 ) return;

	T a,b;

	T xm = T(n-1)*(T)0.5;
	T ym = 0;

	for ( int i = 0; i < n; ++i )
		ym += data[i];

	ym /= (T)n;

	T covar = 0, varx = 0;

	for ( int i = 0; i < n; ++i ) {
		T x = (T)i-xm;
		covar += x*(data[i]-ym);
		varx += x*x;
	}

	a = covar / varx;
	b = ym - a*xm;

	for ( int i = 0; i < n; ++i )
		data[i] = data[i] - (b + a*(T)i);
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace IO {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Spectralizer::Options::Options() {
	windowLength = 20;
	windowOverlap = 0.5;
	specSamples = -1;
	taperWidth = 0.05;
	noalign = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Spectralizer::Spectralizer() {
	// Default window length is 20s
	_windowLength = 20;
	// Default time step is 10s (50% overlap)
	_timeStep = 10;
	_specSamples = -1;
	_noalign = false;
	_filter = NULL;
	// Default taper width is 5%
	_taperWidth = 0.05;
	_buffer = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Spectralizer::~Spectralizer() {
	cleanup();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Spectralizer::setOptions(const Options &opts) {
	_windowLength = opts.windowLength;
	_taperWidth = opts.taperWidth;
	if ( _taperWidth < 0 ) _taperWidth = 0;
	else if ( _taperWidth > 0.5 ) _taperWidth = 0.5;

	SEISCOMP_DEBUG("[spec] windowLength = %fs", _windowLength);
	SEISCOMP_DEBUG("[spec] windowOverlap = %f%%", opts.windowOverlap*100);
	SEISCOMP_DEBUG("[spec] samples = %d", opts.specSamples);
	SEISCOMP_DEBUG("[spec] filter = %s", opts.filter.c_str());
	SEISCOMP_DEBUG("[spec] taperWidth = %f", _taperWidth);

	if ( opts.windowOverlap < 1 )
		_timeStep = _windowLength * (1-opts.windowOverlap);
	else
		return false;

	_specSamples = opts.specSamples;
	_noalign = opts.noalign;

	if ( !opts.filter.empty() ) {
		_filter = Math::Filtering::InPlaceFilter<double>::Create(opts.filter);
		if ( !_filter ) return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Spectralizer::cleanup() {
	if ( _buffer != NULL ) {
		delete _buffer;
		_buffer = NULL;
	}

	while ( !_nextSpectra.empty() ) {
		delete _nextSpectra.front();
		_nextSpectra.pop_front();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Spectralizer::init(const Record *rec) {
	_buffer->sampleRate = rec->samplingFrequency();
	_buffer->dt = 1.0/_buffer->sampleRate;
	_buffer->buffer.resize(_buffer->sampleRate*_windowLength);
	_buffer->tmpOffset = 0;
	_buffer->tmp.resize(_buffer->buffer.size() + _buffer->tmpOffset*2);
	_buffer->tmp.fill(0.0);
	_buffer->reset(_filter);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Spectralizer::push(const Record *rec) {
	if ( _buffer == NULL ) {
		_buffer = new SpecBuffer;
		init(rec);
	}
	else {
		// Sample rate changed? Check new settings and reset
		// the spec calculation.
		if ( _buffer->sampleRate != rec->samplingFrequency() ) {
			_buffer->reset(_filter);
			init(rec);
		}
	}

	fft(rec);

	return !_nextSpectra.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Spectrum *Spectralizer::pop() {
	if ( _nextSpectra.empty() ) return NULL;

	Spectrum *front = _nextSpectra.front();
	_nextSpectra.pop_front();
	return front;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *Spectralizer::fft(const Record *rec) {
	Core::Time endTime;
	try {
		endTime = rec->endTime();
	}
	catch ( ... ) {
		SEISCOMP_WARNING("[dec] %s: invalid end time -> ignoring",
		                 rec->streamID().c_str());
		return NULL;
	}

	if ( _buffer->lastEndTime.valid() ) {
		double diff = rec->startTime() - _buffer->lastEndTime;
		if ( fabs(diff) > _buffer->dt*0.5 ) {
			SEISCOMP_DEBUG("[spec] %s: gap/overlap of %f secs -> reset processing",
			               rec->streamID().c_str(), diff);
			_buffer->reset(_filter);
		}
	}

	_buffer->lastEndTime = endTime;

	ArrayPtr tmp_ar;
	const DoubleArray *ar = DoubleArray::ConstCast(rec->data());
	if ( ar == NULL ) {
		tmp_ar = rec->data()->copy(Array::DOUBLE);
		ar = DoubleArray::ConstCast(tmp_ar);
		if ( ar == NULL ) {
			SEISCOMP_ERROR("[spec] internal error: doubles expected");
			return NULL;
		}
	}

	size_t data_len = (size_t)ar->size();
	const double *data = ar->typedData();
	double *buffer = &_buffer->buffer[0];

	if ( _buffer->filter )
		_buffer->filter->apply(data_len, (double*)data);

	if ( _buffer->missingSamples > 0 ) {
		size_t toCopy = std::min(_buffer->missingSamples, data_len);
		memcpy(buffer + _buffer->buffer.size() - _buffer->missingSamples,
		       data, toCopy*sizeof(double));
		data += toCopy;
		data_len -= toCopy;
		_buffer->missingSamples -= toCopy;

		if ( !_buffer->startTime.valid() ) {
			_buffer->startTime = rec->startTime();

			// align to timestep if not requested otherwise
			if ( !_noalign ) {
				double mod = fmod((double)_buffer->startTime, _timeStep);
				double skip = _timeStep - mod;
				_buffer->samplesToSkip = int(skip*_buffer->sampleRate+0.5);

				Core::Time nextStep(floor(double(_buffer->startTime)/_timeStep+(_buffer->samplesToSkip > 0?1:0))*_timeStep+5E-7);
				_buffer->startTime = nextStep - Core::TimeSpan(_buffer->samplesToSkip*_buffer->dt+5E-7);
			}
		}

		// Still samples missing and no more data available, return
		if ( _buffer->missingSamples > 0 ) return NULL;
	}

	do {
		if ( _buffer->samplesToSkip == 0 ) {
			ComplexDoubleArrayPtr spec;
			Core::Time startTime;

			// Calculate spectrum from ringbuffer
			startTime = _buffer->startTime;

			// Copy data
			copy(_buffer->tmp, _buffer->tmpOffset, _buffer->buffer, _buffer->front);

			size_t sampleCount = _buffer->buffer.size();

			// Demean data excluding the padding window
			demean(_buffer->tmp.size()-_buffer->tmpOffset*2, _buffer->tmp.typedData()+_buffer->tmpOffset);
			// Detrend data excluding the padding window
			detrend(_buffer->tmp.size()-_buffer->tmpOffset*2, _buffer->tmp.typedData()+_buffer->tmpOffset);
			// Apply Von-Hann window
			Math::HannWindow<double>().apply(_buffer->tmp.size()-_buffer->tmpOffset*2, _buffer->tmp.typedData()+_buffer->tmpOffset, _taperWidth);

			spec = new ComplexDoubleArray;
			Math::fft(spec->impl(), _buffer->tmp.size(), _buffer->tmp.typedData());

			if ( _specSamples > 0 )
				reduce<Mag>(*spec, _specSamples);

			if ( spec ) {
				Spectrum *spectrum;
				spectrum = new Spectrum(startTime, startTime + Core::TimeSpan(sampleCount*_buffer->dt),
				                        _timeStep, _buffer->sampleRate*0.5,
				                        (int)sampleCount/2);
				spectrum->setData(spec.get());
				_nextSpectra.push_back(spectrum);
			}

			// Still need to wait until N samples have been fed.
			_buffer->samplesToSkip = _buffer->sampleRate * _timeStep + 0.5;
		}

		size_t num_samples = std::min(_buffer->samplesToSkip, data_len);

		size_t chunk_size = std::min(num_samples, _buffer->buffer.size()-_buffer->front);
		memcpy(buffer + _buffer->front, data, chunk_size*sizeof(double));

		data += chunk_size;

		// Split chunks
		if ( chunk_size < num_samples ) {
			chunk_size = num_samples - chunk_size;

			memcpy(buffer, data, chunk_size*sizeof(double));

			_buffer->front = chunk_size;

			data += chunk_size;
		}
		else {
			_buffer->front += chunk_size;
			if ( _buffer->front >= _buffer->buffer.size() )
				_buffer->front -= _buffer->buffer.size();
		}

		_buffer->startTime += Core::TimeSpan(_buffer->dt*num_samples+5E-7);
		_buffer->samplesToSkip -= num_samples;

		data_len -= num_samples;
	}
	while ( data_len > 0 );

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
