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


#define SEISCOMP_COMPONENT Resample

#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/math/math.h>
#include <seiscomp3/io/recordstream/remez/remez.h>
#include <seiscomp3/io/recordfilter/resample.h>

#include <limits.h>
#include <string.h>
#include <ctype.h>


namespace Seiscomp {
namespace IO {


namespace {

// sinc(x*PI)
double sincpi(double x) {
	if ( x == 0.0 ) return 1.0;
	return sin(M_PI*x) / (M_PI*x);
}


// Lanczos kernel with width a
double Lanczos(double x, double a) {
	if ( -a < x && x < a )
		return sincpi(x) * sincpi(x/a);
	return 0;
}


// Basic upsample template
template <typename T>
int upsample(T *input, T *y, int xsamps, double from, double to, int a) {
	T down_ratio = from / to;
	T x;
	int ysamps = (int)(xsamps / down_ratio) - a/down_ratio;
	int xi, j, i;

	for ( j = a/down_ratio; j < ysamps; ++j ) {
		x = j * down_ratio;
		xi = (int)x;
		T s = 0;
		for ( i = xi-a; i <= xi+a; ++i )
			s += input[i]*Lanczos(x-i,a);
		y[j] = s;
	}

	return ysamps;
}


bool getFraction(int &num, int &den, double value, double epsilon = 1E-5, int maxIterations = 100) {
	int64_t overflow = INT_MAX;
	double r0 = value;
	int64_t a0 = (int64_t)r0;
	if ( abs(a0) > overflow ) return false;

	// check for (almost) integer arguments, which should not go
	// to iterations.
	if ( fabs(a0 - value) < epsilon) {
		num = (int)a0;
		den = 1;
		return true;
	}

	int64_t p0 = 1;
	int64_t q0 = 0;
	int64_t p1 = a0;
	int64_t q1 = 1;

	int64_t p2 = 0;
	int64_t q2 = 1;

	int n = 0;
	bool stop = false;

	do {
		++n;
		double r1 = 1.0 / (r0 - a0);
		int64_t a1 = (int64_t)r1;
		p2 = (a1 * p1) + p0;
		q2 = (a1 * q1) + q0;
		if ( (abs(p2) > overflow) || (abs(q2) > overflow) ) {
			return false;
		}

		double convergent = (double)p2 / (double)q2;
		if ( n < maxIterations && fabs(convergent - value) > epsilon && q2 < INT_MAX) {
			p0 = p1;
			p1 = p2;
			q0 = q1;
			q1 = q2;
			a0 = a1;
			r0 = r1;
		}
		else
			stop = true;
	}
	while (!stop);

	if ( n >= maxIterations ) return false;

	if ( q2 < INT_MAX ) {
		num = (int) p2;
		den = (int) q2;
	}
	else {
		num = (int) p1;
		den = (int) q1;
	}

	return true;
}



GenericRecord *createRecord(const Record *rec) {
	return new GenericRecord(rec->networkCode(), rec->stationCode(),
	                         rec->locationCode(), rec->channelCode(),
	                         rec->startTime(), rec->samplingFrequency());
}


GenericRecord *convert(const Record *rec) {
	if ( rec->data() == NULL ) return NULL;

	GenericRecord *out;

	switch ( rec->dataType() ) {
		case Array::CHAR:
		case Array::INT:
		case Array::FLOAT:
		case Array::DOUBLE:
			out = createRecord(rec);
			break;
		default:
			return NULL;
	}

	ArrayPtr data = rec->data()->clone();
	out->setData(data.get());

	return out;
}


template <typename T>
Array::DataType dataType() {
	throw Core::TypeConversionException("RecordFilter::IIR: wrong data type");
}


template <>
Array::DataType dataType<float>() {
	return Array::FLOAT;
}


template <>
Array::DataType dataType<double>() {
	return Array::DOUBLE;
}


template <>
Array::DataType dataType<int>() {
	return Array::INT;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordResamplerBase::_instanceCount = 0;
RecordResamplerBase::CoefficientMap RecordResamplerBase::_coefficients;
Util::mutex RecordResamplerBase::_coefficientMutex;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordResamplerBase::RecordResamplerBase() {
	_coefficientMutex.lock();
	++_instanceCount;
	_coefficientMutex.unlock();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordResamplerBase::~RecordResamplerBase() {
	// If the last instance is deleted clean the coefficients table
	_coefficientMutex.lock();
	--_instanceCount;
	if ( _instanceCount == 0 ) {
		 CoefficientMap::iterator it;
		 for ( it = _coefficients.begin(); it != _coefficients.end(); ++it )
				 delete it->second;
		 _coefficients.clear();
	}
	_coefficientMutex.unlock();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *RecordResamplerBase::flush() {
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordResamplerBase::reset() {
	_currentRate = -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
RecordResampler<T>::RecordResampler(double targetFrequency, double fp,
                        double fs, double coeffScale, int lanzcosWidth)
: _downsampler(NULL), _upsampler(NULL) {
	_fp = fp;
	_fs = fs;
	_coeffScale = coeffScale;
	_maxN = 500/_coeffScale;
	_lanzcosKernelWidth = lanzcosWidth;

	_targetRate = targetFrequency;
	_currentRate = -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
RecordResampler<T>::~RecordResampler() {
	if ( _downsampler ) delete _downsampler;
	if ( _upsampler ) delete _upsampler;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void RecordResampler<T>::init(DownsampleStage *stage, const Record *rec, int upscale, int N) {
	stage->N = N;
	stage->passThrough = stage->N <= 0;
	stage->sampleRate = rec->samplingFrequency()*upscale;
	stage->targetRate = stage->sampleRate / N;
	stage->dt = 0;
	if ( !stage->passThrough ) initCoefficients(stage);
	else stage->coefficients = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void RecordResampler<T>::init(UpsampleStage *stage, const Record *rec, int N) {
	stage->N = N;
	stage->downRatio = 1.0/(double)stage->N;
	stage->sampleRate = rec->samplingFrequency();
	stage->targetRate = stage->sampleRate*stage->N;
	stage->width = _lanzcosKernelWidth;
	stage->N2 = stage->width;
	// The sample itself, the width to the right and to the left and a buffer
	// on the left side
	stage->buffer.resize(stage->width*2+1+1);
	stage->dt = 1.0 / stage->sampleRate;
	stage->reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
GenericRecord *RecordResampler<T>::resample(DownsampleStage *stage, const Record *rec) {
	Core::Time endTime;
	try {
		endTime = rec->endTime();
	}
	catch ( ... ) {
		SEISCOMP_WARNING("[dec] %s: invalid end time -> ignoring",
		                 rec->streamID().c_str());
		return NULL;
	}

	if ( stage->lastEndTime.valid() ) {
		double diff = rec->startTime() - stage->lastEndTime;
		if ( fabs(diff) > stage->dt*0.5 ) {
			if ( diff < 0 )
				// Ignore overlap
				return NULL;

			SEISCOMP_DEBUG("[dec] %s: gap of %f secs -> reset processing",
			               rec->streamID().c_str(), diff);
			stage->reset();
		}
	}

	stage->lastEndTime = endTime;

	ArrayPtr tmp_ar;
	const TypedArray<T> *ar = TypedArray<T>::ConstCast(rec->data());
	if ( ar == NULL ) {
		tmp_ar = rec->data()->copy(dataType<T>());
		ar = TypedArray<T>::ConstCast(tmp_ar);
		if ( ar == NULL ) {
			SEISCOMP_ERROR("[dec] internal error: wrong converted type received");
			return NULL;
		}
	}

	size_t data_len = (size_t)ar->size();
	const T *data = ar->typedData();
	T *buffer = &stage->buffer[0];

	if ( stage->missingSamples > 0 ) {
		size_t toCopy = std::min(stage->missingSamples, data_len);
		memcpy(buffer + stage->buffer.size() - stage->missingSamples,
		       data, toCopy*sizeof(T));
		data += toCopy;
		data_len -= toCopy;
		stage->missingSamples -= toCopy;

		if ( !stage->startTime.valid() )
			stage->startTime = rec->startTime();

		// Still samples missing and no more data available, return
		if ( stage->missingSamples > 0 ) return NULL;

		// Resampling can start now
		stage->samplesToSkip = 0;
	}

	// Ring buffer is filled at this point.
	typename Core::SmartPointer< TypedArray<T> >::Impl resampled_data;
	Core::Time startTime;

	do {
		if ( stage->samplesToSkip == 0 ) {
			// Calculate scalar product of coefficients and ring buffer
			double *coeff = &((*stage->coefficients)[0]);
			double weightedSum = 0;

			for ( size_t i = stage->front; i < stage->buffer.size(); ++i )
				weightedSum += buffer[i] * *(coeff++);
			for ( size_t i = 0; i < stage->front; ++i )
				weightedSum += buffer[i] * *(coeff++);

			if ( !resampled_data ) {
				startTime = stage->startTime + Core::TimeSpan(stage->dt*stage->N2);
				resampled_data = new TypedArray<T>;
			}

			T sample = (T)weightedSum;
			resampled_data->append(1, &sample);
			if ( Math::isNaN(sample) )
				SEISCOMP_WARNING("[dec] produced NaN sample");

			// Still need to wait until N samples have been fed.
			stage->samplesToSkip = stage->N;
		}

		size_t num_samples = std::min(stage->samplesToSkip, data_len);

		size_t chunk_size = std::min(num_samples, stage->buffer.size()-stage->front);
		memcpy(buffer + stage->front, data, chunk_size*sizeof(T));

		data += chunk_size;

		// Split chunks
		if ( chunk_size < num_samples ) {
			chunk_size = num_samples - chunk_size;

			memcpy(buffer, data, chunk_size*sizeof(T));

			stage->front = chunk_size;

			data += chunk_size;
		}
		else {
			stage->front += chunk_size;
			if ( stage->front >= stage->buffer.size() )
				stage->front -= stage->buffer.size();
		}

		stage->startTime += Core::TimeSpan(stage->dt*num_samples);
		stage->samplesToSkip -= num_samples;
		data_len -= num_samples;
	}
	while ( data_len > 0 );

	// Create the record and push it
	if ( resampled_data ) {
		GenericRecord *grec;
		grec = new GenericRecord(rec->networkCode(), rec->stationCode(),
		                         rec->locationCode(), rec->channelCode(),
		                         startTime, stage->targetRate);
		grec->setData(resampled_data.get());

		GenericRecord *tmp;

		if ( stage->nextStage ) {
			GenericRecord *nrec = resample(stage->nextStage, grec);
			delete grec;
			tmp = nrec;
		}
		else
			tmp = grec;

		return tmp;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
GenericRecord *RecordResampler<T>::resample(UpsampleStage *stage, const Record *rec) {
	Core::Time endTime;
	try {
		endTime = rec->endTime();
	}
	catch ( ... ) {
		SEISCOMP_WARNING("[ups] %s: invalid end time -> ignoring",
		                 rec->streamID().c_str());
		return NULL;
	}

	if ( stage->lastEndTime.valid() ) {
		double diff = rec->startTime() - stage->lastEndTime;
		if ( fabs(diff) > stage->dt*0.5 ) {
			SEISCOMP_DEBUG("[ups] %s: gap/overlap of %f secs -> reset processing",
			               rec->streamID().c_str(), diff);
			stage->reset();
		}
	}

	stage->lastEndTime = endTime;

	ArrayPtr tmp_ar;
	const TypedArray<T> *ar = TypedArray<T>::ConstCast(rec->data());
	if ( ar == NULL ) {
		tmp_ar = rec->data()->copy(dataType<T>());
		ar = TypedArray<T>::ConstCast(tmp_ar);
		if ( ar == NULL ) {
			SEISCOMP_ERROR("[dec] internal error: wrong conversion type received");
			return NULL;
		}
	}

	size_t data_len = (size_t)ar->size();
	if ( data_len == 0 ) return NULL;

	const T *data = ar->typedData();
	T *buffer = &stage->buffer[0];

	if ( stage->missingSamples > 0 ) {
		size_t toCopy = std::min(stage->missingSamples, data_len);
		memcpy(buffer + stage->buffer.size() - stage->missingSamples,
		       data, toCopy*sizeof(T));
		data += toCopy;
		data_len -= toCopy;
		stage->missingSamples -= toCopy;

		if ( !stage->startTime.valid() )
			stage->startTime = rec->startTime();

		// Still samples missing and no more data available, return
		if ( stage->missingSamples > 0 ) return NULL;
	}

	// Ring buffer is filled at this point.
	typename Core::SmartPointer< TypedArray<T> >::Impl resampled_data;
	Core::Time startTime;

	while ( data_len > 0 ) {
		double xi = 0.0;

		if ( !resampled_data ) {
			startTime = stage->startTime + Core::TimeSpan(stage->dt*stage->N2);
			resampled_data = new TypedArray<T>;
		}

		// Generate N new samples (upsampling)
		for ( int n = 0; n < stage->N; ++n ) {
			// Calculate Lanzcos kernel
			double weightedSum = 0;
			int a = -stage->width;
			size_t bi = stage->front;

			for ( ; a <= stage->width; ++a, ++bi ) {
				if ( bi == stage->buffer.size() ) bi -= stage->buffer.size();
				weightedSum += buffer[bi]*Lanczos(xi-a,stage->width);
			}

			xi += stage->downRatio;

			T sample = (T)weightedSum;
			resampled_data->append(1, &sample);
		}

		// Push the sample to the ring buffer
		size_t chunk_size = std::min((size_t)1, stage->buffer.size()-stage->front);
		memcpy(buffer + stage->front, data, chunk_size*sizeof(T));

		data += chunk_size;

		// Split chunks
		if ( chunk_size < 1 ) {
			chunk_size = 1 - chunk_size;
			memcpy(buffer, data, chunk_size*sizeof(T));
			stage->front = chunk_size;
			data += chunk_size;
		}
		else {
			stage->front += chunk_size;
			if ( stage->front >= stage->buffer.size() )
				stage->front -= stage->buffer.size();
		}

		stage->startTime += Core::TimeSpan(stage->dt);
		--data_len;
	}

	// Create the record and push it
	if ( resampled_data ) {
		GenericRecord *grec;
		grec = new GenericRecord(rec->networkCode(), rec->stationCode(),
		                         rec->locationCode(), rec->channelCode(),
		                         startTime, stage->targetRate);
		grec->setData(resampled_data.get());

		return grec;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
Record *RecordResampler<T>::feed(const Record *record) {
	//SEISCOMP_DEBUG("O %s %s %f", record->startTime().iso().data(), record->endTime().iso().data(),
	//               record->samplingFrequency());

	double rate = record->samplingFrequency();

	// Target rate matching the current sampling rate -> passthrough
	if ( rate == _targetRate )
		return convert(record);

	if ( _currentRate != rate ) {
		// Init up/downsample stages
		int num, den;
		if ( !getFraction(num, den, _targetRate/rate) ) {
			SEISCOMP_WARNING("[resample] incompatible sampling frequency %f -> %f",
			                 rate, _targetRate);
			return NULL;
		}

		_currentRate = rate;

		// We need to upsample the data
		if ( num > 1 ) {
			//SEISCOMP_DEBUG("[resample] create upscaling of factor %d", num);

			if ( _upsampler == NULL )
				_upsampler = new UpsampleStage;
			else
				_upsampler->reset();

			init(_upsampler, record, num);
		}
		else if ( _upsampler != NULL ) {
			delete _upsampler;
			_upsampler = NULL;
		}

		// We need to downsample the data
		if ( den > 1 ) {
			//SEISCOMP_DEBUG("[resample] create downscaling of factor %d", den);

			if ( _downsampler == NULL )
				_downsampler = new DownsampleStage;
			else {
				_downsampler->reset();

				// Clear all subsequent stages
				if ( _downsampler->nextStage ) {
					delete _downsampler->nextStage;
					_downsampler->nextStage = NULL;
				}
			}

			init(_downsampler, record, num, den);
		}
		else if ( _downsampler != NULL ) {
			delete _downsampler;
			_downsampler = NULL;
		}
	}

	GenericRecord *tmp;

	if ( _upsampler ) {
		tmp = resample(_upsampler, record);
		/*
		if ( tmp )
			SEISCOMP_DEBUG("U %s %s %f", tmp->startTime().iso().data(), tmp->endTime().iso().data(),
			               tmp->samplingFrequency());
		*/

		if ( tmp != NULL && _downsampler != NULL ) {
			// The tmp record from previous stage is just temporary
			RecordPtr guard(tmp);
			tmp = resample(_downsampler, tmp);
			/*
			if ( tmp )
				SEISCOMP_DEBUG("D %s %s %f", tmp->startTime().iso().data(), tmp->endTime().iso().data(),
				               tmp->samplingFrequency());
			*/
		}
	}
	else if ( _downsampler != NULL ) {
		tmp = resample(_downsampler, record);
		/*
		if ( tmp )
			SEISCOMP_DEBUG("D %s %s %f", tmp->startTime().iso().data(), tmp->endTime().iso().data(),
			               tmp->samplingFrequency());
		*/
	}
	else
		tmp = NULL;

	return tmp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void RecordResampler<T>::initCoefficients(DownsampleStage *stage) {
	_coefficientMutex.lock();

	CoefficientMap::iterator it = _coefficients.find(stage->N);
	if ( it != _coefficients.end() )
		stage->coefficients = it->second;
	else {
		stage->coefficients = NULL;

		if ( stage->N > _maxN ) {
			for ( int i = _maxN; i > 1; --i ) {
				if ( stage->N % i == 0 ) {
					int nextStageN = stage->N / i;

					//SEISCOMP_DEBUG("[dec] clipping N=%d to %d and create sub stage",
					//               stage->N, i);

					stage->N = i;
					stage->targetRate = stage->sampleRate / stage->N;

					DownsampleStage *nextStage = new DownsampleStage;
					nextStage->sampleRate = stage->targetRate;
					nextStage->targetRate = _targetRate;
					nextStage->N = nextStageN;

					_coefficientMutex.unlock();
					initCoefficients(nextStage);
					_coefficientMutex.lock();

					stage->nextStage = nextStage;

					break;
				}
			}

			CoefficientMap::iterator it = _coefficients.find(stage->N);
			if ( it != _coefficients.end() )
				stage->coefficients = it->second;
		}

		if ( stage->coefficients == NULL ) {
			// Create and cache coefficients for N
			int Ncoeff = stage->N*_coeffScale*2+1;

			Coefficients *coeff = new Coefficients(Ncoeff);

			SEISCOMP_DEBUG("[dec] caching %d coefficents for N=%d", Ncoeff, stage->N);

			double bands[4] = {0,0.5*(_fp/stage->N),0.5*(_fs/stage->N),0.5};
			double weights[2] = {1,1};
			double desired[2] = {1,0};

			remez(&((*coeff)[0]), Ncoeff, 2, bands, desired, weights, BANDPASS);

			_coefficients[stage->N] = coeff;
			stage->coefficients = coeff;
		}
	}

	stage->dt = 1.0 / stage->sampleRate;
	stage->N2 = stage->coefficients->size() / 2;
	stage->buffer.resize(stage->coefficients->size());
	stage->reset();

	_coefficientMutex.unlock();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void RecordResampler<T>::reset() {
	RecordResamplerBase::reset();

	if ( _downsampler != NULL ) {
		delete _downsampler;
		_downsampler = NULL;
	}

	if ( _upsampler != NULL ) {
		delete _upsampler;
		_upsampler = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
RecordFilterInterface *RecordResampler<T>::clone() const {
	return new RecordResampler<T>(_targetRate, _fp, _fs, _coeffScale, _lanzcosKernelWidth);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template class SC_SYSTEM_CORE_API RecordResampler<float>;
template class SC_SYSTEM_CORE_API RecordResampler<double>;
template class SC_SYSTEM_CORE_API RecordResampler<int>;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
