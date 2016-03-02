/***************************************************************************
 *   Copyright (C) by GFZ Potsdam and gempa GmbH                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#define SEISCOMP_COMPONENT Core

#include <iostream>
#include <math.h>

#include <seiscomp3/core/record.h>
#include <seiscomp3/core/arrayfactory.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/genericrecord.h>
#include <seiscomp3/core/timewindow.h>
#include <seiscomp3/core/recordsequence.h>
#include <seiscomp3/logging/log.h>
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
using namespace std;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {

namespace Private {

int round(double val) {
	return static_cast<int>(val + 0.5);
}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence::RecordSequence(double tolerance)
: _tolerance(tolerance)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence::~RecordSequence() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::TimeWindow RecordSequence::timeWindow() const {
	Core::TimeWindow tw;

	if ( recordCount() )
		tw.set(front()->startTime(), back()->endTime());

	return tw;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


template <typename T>
inline void updateRange(RecordSequence::Range &range, const T *arr, int start, int end) {
	for ( int i = start; i < end; ++i ) {
		if ( arr[i] < range.first )
			range.first = arr[i];
		else if ( arr[i] > range.second )
			range.second = arr[i];
	}
}


template <typename T>
inline void getRange(RecordSequence::Range &range, const T *arr, int start, int end) {
	range.first = range.second = arr[start];
	updateRange(range, arr, start+1, end);
}


bool getRange(RecordSequence::Range &range, const Array &arr, int start, int end) {
	switch ( arr.dataType() ) {
		case Array::CHAR:
			getRange<char>(range, (const char*)arr.data(), start, end);
			break;
		case Array::INT:
			getRange<int>(range, (const int*)arr.data(), start, end);
			break;
		case Array::FLOAT:
			getRange<float>(range, (const float*)arr.data(), start, end);
			break;
		case Array::DOUBLE:
			getRange<double>(range, (const double*)arr.data(), start, end);
			break;
		default:
			return false;
	}

	return true;
}


bool updateRange(RecordSequence::Range &range, const Array &arr, int start, int end) {
	switch ( arr.dataType() ) {
		case Array::CHAR:
			updateRange<char>(range, (const char*)arr.data(), start, end);
			break;
		case Array::INT:
			updateRange<int>(range, (const int*)arr.data(), start, end);
			break;
		case Array::FLOAT:
			updateRange<float>(range, (const float*)arr.data(), start, end);
			break;
		case Array::DOUBLE:
			updateRange<double>(range, (const double*)arr.data(), start, end);
			break;
		default:
			return false;
	}

	return true;
}


}


RecordSequence::Range RecordSequence::amplitudeRange(const Core::TimeWindow *tw) const {
	Range range(0.0,0.0);
	bool foundRecords = false;

	for ( const_iterator it = begin(); it != end(); ++it) {
		const Record *rec = it->get();
		const Array *data = rec->data();

		// Skip empty records
		if ( data == NULL ) continue;
		if ( data->size() == 0 ) continue;

		int imin = 0, imax = 0;

		if ( tw != NULL ) { // limit search for min/max to specified time window
			try {
				const Core::TimeWindow &rtw = rec->timeWindow();

				if ( tw->overlaps(rtw) ) {
					double fs = rec->samplingFrequency();
					double dt = tw->startTime() - rec->startTime();

					if ( dt > 0 )
						imin = int(dt*fs);

					dt = rec->endTime() - tw->endTime();
					imax = data->size();
					if ( dt > 0 )
						imax -= int(dt*fs);
				}
				else
					continue;
			}
			catch ( ... ) {
				continue;
			}
		}
		else // no time window specified -> search over whole record
			imax = data->size();

		if ( imax <= imin ) continue;

		// The first record contributing to the range?
		if ( !foundRecords ) {
			if ( getRange(range, *data, imin, imax) )
				foundRecords = true;
		}
		else
			updateRange(range, *data, imin, imax);
	}

	return range;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence::TimeWindowArray RecordSequence::windows() const {
	RecordSequence::TimeWindowArray win;

	if ( recordCount() == 0 )
		return win;

	Core::TimeWindow current;

	for ( const_iterator it = begin(); it != end(); ++it ) {
		RecordCPtr rec = (*it);
		Core::TimeWindow tw = rec->timeWindow();
		double fs = rec->samplingFrequency();

		if ( it == begin() )
			current = tw;
		else {
			if ( current.contiguous(tw, _tolerance/fs) )
				current.extend(tw);
			else {
				win.push_back(current);
				current = tw;
			}
		}
	}

	win.push_back(current);

	return win;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence::TimeWindowArray RecordSequence::gaps() const {
	RecordSequence::TimeWindowArray gaps;

	if ( empty() ) return gaps;

	Core::TimeWindow current;

	for ( const_iterator it = begin(); it != end(); ++it ) {
		RecordCPtr rec = (*it);
		Core::TimeWindow tw = rec->timeWindow();
		double fs = rec->samplingFrequency();

		if ( it == begin() )
			current = tw;
		else {
			if ( !current.contiguous(tw, _tolerance/fs) )
				gaps.push_back(Core::TimeWindow(current.endTime(), tw.startTime()));
		}

		current = tw;
	}

	return gaps;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double RecordSequence::availability(const Core::TimeWindow &tw) const {
	if ( empty() || tw.length() == 0.0 ) return 0.0;

	const_iterator it = begin();
	RecordCPtr lastRec = *it;
	Core::TimeWindow lastTw = lastRec->timeWindow();

	++it;

	double missingData = 0;

	if ( lastRec->startTime() > tw.startTime() )
		missingData += lastRec->startTime() - tw.startTime();

	for ( ; it != end(); ++it ) {
		RecordCPtr rec = *it;
		Core::TimeWindow tw = rec->timeWindow();

		double fs = rec->samplingFrequency();

		if ( !lastTw.contiguous(tw, _tolerance/fs) )
			missingData += tw.startTime() - lastTw.endTime();

		lastTw = tw;
		lastRec = rec;
	}

	if ( lastRec->endTime() < tw.endTime() )
		missingData += tw.endTime() - lastRec->endTime();

	double requiredData = tw.length();

	if ( missingData < 0 ) missingData = 0;
	else if ( missingData > requiredData ) missingData = requiredData;

	return 100.0 * (1.0-missingData/requiredData);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
GenericRecord *RecordSequence::continuousRecord(const Core::TimeWindow *tw, bool interpolate) const {
	if ( empty() ) return NULL;

	RecordCPtr lastRec;
	T lastSample = 0;
	RecordSequence::const_iterator it;

	double samplingFrequency = 0;

	typedef NumericArray<T> TArray;
	typedef typename Core::SmartPointer<TArray>::Impl TArrayPtr;

	TArrayPtr rawData = new TArray;
	GenericRecord *rawRecord = NULL;

	for ( it = begin(); it != end(); ++it ) {
		RecordCPtr rec = *it;
		if ( rec->data() == NULL ) continue;
		if ( tw != NULL && !tw->overlaps(rec->timeWindow())) continue;

		const TArray *ar = TArray::ConstCast(rec->data());
		TArrayPtr tmpData;

		if ( ar == NULL ) {
			tmpData = (TArray*)rec->data()->copy(TArray::ArrayType);
			ar = tmpData.get();
		}

		if ( !lastRec ) {
			// Remember frequency of first record
			samplingFrequency = rec->samplingFrequency();
			rawRecord = new GenericRecord(rec->networkCode(),
			                              rec->stationCode(),
			                              rec->locationCode(),
			                              rec->channelCode(),
			                              rec->startTime(),
			                              samplingFrequency);
		}
		else {
			// Incompatible sampling rate
			if ( samplingFrequency != rec->samplingFrequency() )
				break;

			double diffTime = fabs((double)(rec->startTime()-lastRec->endTime()));

			// Overlap
			if ( diffTime < -0.5/samplingFrequency ) {
				int numSamples = -diffTime * samplingFrequency + 0.5;
				// Skip entire record
				if ( numSamples >= ar->size() )
					continue;

				tmpData = ar->slice(numSamples, ar->size());
				ar = tmpData.get();
			}
			// Gap
			else if ( diffTime > 0.5/samplingFrequency ) {
				if ( !interpolate )
					break;

				// Last sample must be set if lastRec is valid
				int numSamples = diffTime * samplingFrequency + 0.5;
				if ( numSamples > 0 ) {
					int startIdx = rawData->size();
					rawData->resize(startIdx + numSamples);

					T nextSample = ar->impl().front();
					double dt = 1.0 / (numSamples+1);
					double t = dt;

					for ( int i = 0; i < numSamples; ++i, ++startIdx, t += dt )
						(*rawData)[startIdx] = lastSample*(1-t) + nextSample*t;
				}
			}
		}

		rawData->append(ar);

		lastSample = (T)ar->impl().back();
		lastRec = rec;
	}

	if ( rawRecord != NULL && rawData )
		rawRecord->setData(rawData.get());

	return rawRecord;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordSequence::findInsertPosition(const Record *rec, iterator *it) {
	// When empty, put it at the end
	if ( empty() ) {
		if ( it ) *it = end();
		return true;
	}

	// When the last record is before rec, append rec
	const Record *lastRec = back().get();
	if ( rec->endTime() > lastRec->endTime() ) {
		if ( it ) *it = end();
		return true;
	}

	// When first record is after rec, prepend rec
	if ( rec->endTime() < front()->endTime() ) {
		if ( it ) *it = begin();
		return true;
	}

	// The most timeconsuming task is now to find a
	// proper position to insert the record
	reverse_iterator current = rbegin();

	for ( ; current != rend(); ++current ) {
		if ( rec->endTime() > (*current)->endTime() ) {
			if ( it ) *it = current.base();
			return true;
		}
		// Duplicate record
		else if ( rec->endTime() == (*current)->endTime() )
			break;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordSequence::timingQuality(int &count, float &quality) const {
	double q = 0;
	count = 0;

	for (const_iterator it = begin(); it != end(); ++it ) {
		RecordCPtr rec = (*it);
		if (rec && rec->timingQuality() >= 0 ) {
			q += rec->timingQuality();
			++count;
		}
	}

	if (count == 0)
		return false;

	quality = q/count;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeWindowBuffer::TimeWindowBuffer(const Core::TimeWindow &tw, double tolerance)
: RecordSequence(tolerance)
, _timeWindow(tw) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double TimeWindowBuffer::availability() const {
	return RecordSequence::availability(_timeWindow);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TimeWindowBuffer::feed(const Record *rec) {
	// FIXME some consistency checks needed before a record is added

	// does this record overlap with the interesting time window?
	if ( ! _timeWindow.overlaps(rec->timeWindow()))
		return false;

	iterator it;
	if ( !findInsertPosition(rec, &it) )
		return false;

	insert(it, rec);

	/*
	if ( alreadyHasRecord(rec) )
		return false;

	push_back(rec);
	*/

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence *TimeWindowBuffer::copy() const {
	TimeWindowBuffer *cp = (TimeWindowBuffer*)clone();
	for (const_iterator it = begin(); it != end(); ++it)
		cp->feed(it->get()->copy());

	return (RecordSequence*) cp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence *TimeWindowBuffer::clone() const {
	return new TimeWindowBuffer(_timeWindow, _tolerance);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RingBuffer::RingBuffer(int nmax, double tolerance) {
	_nmax = nmax;
	_span = Core::TimeSpan();
	setTolerance(tolerance);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RingBuffer::RingBuffer(Core::TimeSpan span, double tolerance) {
	_span = span;
	_nmax = 0;
	setTolerance(tolerance);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RingBuffer::feed(const Record *rec) {
	// FIXME some more consistency checks needed before a record is added

	/*
	if ( alreadyHasRecord(rec) )
		return false;

	push_back(rec);
	*/
	iterator it;
	if ( !findInsertPosition(rec, &it) )
		return false;

	insert(it, rec);

	if( ! recordCount())
		return true;

	if ( _nmax ) {
		while( recordCount() > _nmax )
			pop_front();
	}
	else if ( _span ) {
		Core::Time tmin = back()->endTime() - _span;
		while ( front()->endTime() <= tmin )
			pop_front();
	}
	else
		return false;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence *RingBuffer::copy() const {
	RingBuffer *cp = (RingBuffer*)clone();

	if (!cp) return NULL;

	for (const_iterator it = begin(); it != end(); ++it)
		cp->push_back((*it)->copy());

	return (RecordSequence*) cp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence *RingBuffer::clone() const {
	if (_nmax)
		return new RingBuffer(_nmax, _tolerance);
	else
		return new RingBuffer(_span, _tolerance);

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Specialize continuousRecord for int, float and double
template
GenericRecord *RecordSequence::continuousRecord<int>(const Core::TimeWindow *tw, bool interpolate) const;

template
GenericRecord *RecordSequence::continuousRecord<float>(const Core::TimeWindow *tw, bool interpolate) const;

template
GenericRecord *RecordSequence::continuousRecord<double>(const Core::TimeWindow *tw, bool interpolate) const;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
