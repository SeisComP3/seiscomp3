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


// some changes regarding gap handling by Mathias Hoffmann, 2008.270

#define SEISCOMP_COMPONENT Core

#include <iostream>
#include <math.h>
using namespace std;

#include <seiscomp3/core/record.h>
#include <seiscomp3/core/arrayfactory.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/genericrecord.h>
#include <seiscomp3/core/timewindow.h>
#include <seiscomp3/core/recordsequence.h>
#include <seiscomp3/logging/log.h>

namespace Seiscomp {

namespace Private {
int round(double val)
{
	return static_cast<int>(val + 0.5);
}
}

Core::TimeWindow
RecordSequence::timeWindow() const
{
	Core::TimeWindow tw;

	if ( recordCount() )
		tw.set(front()->startTime(), back()->endTime());

	return tw;
}

RecordSequence::TimeWindowArray
RecordSequence::windows() const
{
	RecordSequence::TimeWindowArray win;

	if ( recordCount()==0 )
		return win;

	Core::TimeWindow current;

	for ( const_iterator it = begin(); it != end(); ++it) {
		RecordCPtr rec = (*it);
		Core::TimeWindow tw = rec->timeWindow();
		double fs = rec->samplingFrequency();

		if (it == begin())
			current = tw;
		else {
			if (current.contiguous(tw, _tolerance/fs))
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

RecordSequence::TimeWindowArray
RecordSequence::gaps() const
{
	Core::TimeWindow current;
	RecordSequence::TimeWindowArray win = windows(), gap;
	RecordSequence::TimeWindowArray::iterator it;
	for (it = win.begin(); it!=win.end(); ++it) {
		// Start with the 2nd iteration. If there are no gaps,
		// there will be no 2nd iteration and an empty list of
		// gaps will be returned
		if (it!=win.begin()) {
			Core::TimeWindow tw(current.endTime(), (*it).startTime());
			gap.push_back(tw);
		}
		current = (*it);
	}

	return gap;
}




bool RecordSequence::fillGaps(gapFillingFunction fill) {

	if (empty())
		return(false);

	// later as an option:
	// if true, try to find a consecutive gap, where overlapping samples may fit
	bool _shiftRecords = true;

	RecordCPtr prevRec;
	double fs = front()->samplingFrequency();

	for (iterator it = begin(); it != end(); ++it) {
		RecordCPtr rec = (*it);

		if (it == begin()) {
			prevRec = rec;
			continue;
		}

		double dt = (double)(rec->startTime() - prevRec->endTime());

		//! overlap detected --> delete double samples OR shift record, if possible
		if (-dt > _tolerance/fs) {
			SEISCOMP_WARNING("[%s] Overlapping record detected! [%f s]", front()->streamID().c_str(), dt);
			
			const Array* array = rec->data();

			int overlappingSamples = Private::round(-dt*fs);

			// create new record
			GenericRecordPtr record = new GenericRecord(*rec.get());
			record->setStartTime(prevRec->endTime());

			// test, if there is a gap of the same size immediately following
			// if so, shift the record otherwise delete some samples
			iterator it2 = it;
			RecordCPtr nextRec = NULL;
			double dt2;
			if (_shiftRecords && ++it2 != end()) {
				nextRec = (*it2);
				dt2 = (double)(nextRec->startTime() - rec->endTime());
			}

			// found fitting gap --> shift record
			if (nextRec && (int)(dt2*fs) == overlappingSamples) {
				// assign original data to newly created record
				record->setData(array->size(), array->data(), array->dataType());
				SEISCOMP_WARNING("|  %d samples shifted.", overlappingSamples);
			}
			else {
				// cut dt*fs samples at beginning of record
 				//ArrayPtr newArray = array->slice(overlappingSamples, array->size());

				// cut dt*fs samples at end of record
				ArrayPtr newArray = array->slice(0, array->size()-overlappingSamples);

				// assign trimmed data to record
				record->setData(newArray->size(), newArray->data(), newArray->dataType());
				SEISCOMP_WARNING("|  %d samples deleted.", overlappingSamples);
			}

			// ... replace original record in recordSequence with the corrected one
			(*it) = record;
			
			prevRec = record;
			continue;
		}

		//! gap detected --> fill it
		if (dt > _tolerance/fs) {
			SEISCOMP_WARNING("[%s] Gap between records detected! [%f s]", front()->streamID().c_str(), dt);
 			SEISCOMP_DEBUG("|  no data from: %s until: %s", prevRec->endTime().iso().c_str(), rec->startTime().iso().c_str());

			// check, if we want to fill this gap
			if (dt > _maxGapLengthForFilling) {
				SEISCOMP_WARNING("|  Maximum Gap length reached. Gap not filled.");
				return false;
			}

			Record* fillRec;
			try{
				fillRec = fill(prevRec.get(), rec.get());
			}
			catch(const std::exception& e){
				SEISCOMP_ERROR("|  %s", e.what());
				return false;
			}

 			insert(it, fillRec);

			SEISCOMP_WARNING("|  Gap successfully filled! [%d samples]", fillRec->sampleCount());
		}

		prevRec = rec;

	}


	return true;
}




bool TimeWindowBuffer::fillGaps(gapFillingFunction fill) {

	if (empty())
		return(false);

	double fs = front()->samplingFrequency();
	double dt;

	// check front for missing data
	dt = (double)(front()->startTime() - _timeWindow.startTime());
	if (dt > _tolerance/fs) {
		GenericRecordPtr frontRec = new GenericRecord(*front().get());
		frontRec->dataUpdated();
		frontRec->setDataType(front()->dataType());
		frontRec->setStartTime(_timeWindow.startTime());
		push_front(fill(frontRec.get(), front().get()));
		SEISCOMP_WARNING("[%s] Filled up missing data at BEGINNING of TimeWindowSequence! [%f s]", front()->streamID().c_str(), dt);
		SEISCOMP_DEBUG("|  %d samples from: %s until: %s", front()->sampleCount(), front()->startTime().iso().c_str(), front()->endTime().iso().c_str());

	}

	// check back for missing data
	dt = (double)(_timeWindow.endTime() - back()->endTime());
	if (dt > _tolerance/fs) {
		GenericRecordPtr backRec = new GenericRecord(*back().get());
		backRec->setStartTime(_timeWindow.endTime());
		push_back(fill(back().get(), backRec.get()));
		SEISCOMP_WARNING("[%s] Filled up missing data at END of TimeWindowSequence! [%f s]", back()->streamID().c_str(), dt);
		SEISCOMP_DEBUG("|  %d samples from: %s until: %s", back()->sampleCount(), back()->startTime().iso().c_str(), back()->endTime().iso().c_str());
	}


	return RecordSequence::fillGaps(fill);
}



double RecordSequence::availability(const Core::TimeWindow& tw) const {

	if (empty() || tw.length() == 0.0)
		return 0.0;

	int effectiveSamples = 0;
	int estimatedSamples = Private::round(tw.length() * front()->samplingFrequency());

	for (const_iterator it = begin(); it != end(); ++it) {
		RecordCPtr rec = (*it);

		// record complete inside timeWindow
		if (tw.contains(rec->timeWindow())) {
			effectiveSamples += rec->sampleCount();
			continue;
		}

		// timeWindow complete inside record
		if (rec->timeWindow().contains(tw)) {
			effectiveSamples = estimatedSamples;
			break;
		}
		
		// record at least overlaps timeWindow
		if (tw.overlaps(rec->timeWindow())) {
			// cut record's extra data at the beginning
			double dt = (double)(tw.startTime() - rec->startTime());
			if (dt > 0) {
				effectiveSamples += rec->sampleCount() - Private::round(dt * front()->samplingFrequency());
				continue;
			}
			// cut record's extra data at the end
			dt = (double)(rec->endTime() - tw.endTime());
			if (dt > 0) {
				effectiveSamples += rec->sampleCount() - Private::round(dt * front()->samplingFrequency());
				continue;
			}
		}
		
	}

	return 100.0 * effectiveSamples / estimatedSamples;
}




double TimeWindowBuffer::availability() const {

	return RecordSequence::availability(_timeWindow);
}





Record*
RecordSequence::continuousRecord() const
// FIXME This does not (yet) take possible gaps into account.
{
	GenericRecord *cont = new GenericRecord();

	RecordCPtr first = front();

	cont->setNetworkCode( first->networkCode() );
	cont->setStationCode( first->stationCode() );
	cont->setLocationCode( first->locationCode() );
	cont->setChannelCode( first->channelCode() );

	cont->setStartTime( first->startTime() );
	cont->setSamplingFrequency( first->samplingFrequency() );

	// determine the required array length
// 	unsigned int n = (int)(timeWindow().length() * first->samplingFrequency());
	Array::DataType datatype = first->data()->dataType();
	ArrayPtr arr = ArrayFactory::Create(datatype, datatype, 0, NULL);

	// FIXME This works, but is not particularly efficient as it involves
	// a lot too much copying. To be replaced by something smarter.
	for ( const_iterator it = begin(); it != end(); ++it ) {
		RecordCPtr rec = (*it);
		arr->append( (Array *)(rec->data()) );
	}
	cont->setData(arr.get());

	return cont;
}

Record*
RecordSequence::continuousRecord(const Core::TimeWindow &tw) const
// FIXME This does not (yet) take possible gaps into account.
{
	GenericRecord *cont = (GenericRecord*) continuousRecord();
	Core::TimeWindow rec_tw(cont->timeWindow());
	double fsamp = cont->samplingFrequency();
	int    nsamp = cont->sampleCount();

	double dt_start = double(tw.startTime() - rec_tw.startTime());
	double dt_end   = double(tw.endTime()   - rec_tw.startTime());
	int    dn_start = int(fsamp*dt_start + 0.5);
	int    dn_end   = int(fsamp*dt_end   + 0.5);

	if (dn_start < 0) dn_start = 0;
	if (dn_end > nsamp) dn_end = nsamp;

	ArrayPtr arr = cont->data()->slice(dn_start, dn_end);
	cont->setData(arr.get());
	cont->setStartTime(rec_tw.startTime() + Core::TimeSpan(dn_start/fsamp));

	return cont;
}

bool
RecordSequence::alreadyHasRecord(const Record *rec) const
{
	if (empty())
		return false;

	// to be replaced by something more robust...
	const Record* lastRec = back().get();

	if ( rec->samplingFrequency() != lastRec->samplingFrequency() ) {
		SEISCOMP_DEBUG("SamplingFreq mismatch: %f (%s) <-> %f (%s)",
		       lastRec->samplingFrequency(), lastRec->streamID().c_str(),
		       rec->samplingFrequency(), rec->streamID().c_str());
	}

	if (rec->endTime() <= lastRec->endTime())
		return true;

	return false;
}

bool
RecordSequence::findInsertPosition(const Record *rec, iterator *it)
{
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

bool
RecordSequence::timingQuality(int &count, float &quality) const
{
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

bool
TimeWindowBuffer::feed(const Record *rec)
{
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


RecordSequence *
TimeWindowBuffer::copy() const
{
	TimeWindowBuffer *cp = (TimeWindowBuffer*)clone();
	for (const_iterator it = begin(); it != end(); ++it)
		cp->feed(it->get()->copy());

	return (RecordSequence*) cp;
}

RecordSequence*
TimeWindowBuffer::clone() const
{
	return new TimeWindowBuffer(_timeWindow, _tolerance);
}


RingBuffer::RingBuffer(int nmax, double tolerance)
{
	_nmax = nmax;
	_span = Core::TimeSpan();
	setTolerance(tolerance);
}

RingBuffer::RingBuffer(Core::TimeSpan span, double tolerance)
{
	_span = span;
	_nmax = 0;
	setTolerance(tolerance);
}

bool
RingBuffer::feed(const Record *rec)
{
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

	if(_nmax) {
		while(recordCount() > _nmax)
			pop_front();
	}
	else if (_span) {
		Core::Time tmin = back()->endTime()-_span;
		while(front()->endTime() <= tmin)
			pop_front();
	}
	// else ?

	return true;
}

RecordSequence *
RingBuffer::copy() const
{
	RingBuffer *cp = (RingBuffer*)clone();

	if (!cp) return NULL;

	for (const_iterator it = begin(); it != end(); ++it)
		cp->push_back((*it)->copy());

	return (RecordSequence*) cp;
}

RecordSequence *
RingBuffer::clone() const
{
	if (_nmax)
		return new RingBuffer(_nmax, _tolerance);
	else
		return new RingBuffer(_span, _tolerance);

	return NULL;
}







namespace Gap {


//! fill up gaps with zeroes
Record* fillZeroes(const Record* prev, const Record* next) {
	if (!prev || !next)
		throw Core::GeneralException("fill gaps: prev or next record must not be NULL");


 	GenericRecord* rec = new GenericRecord(*prev);
	rec->setStartTime(prev->endTime());

	Core::TimeWindow tw(prev->endTime(), next->startTime());

	int nsamp = Private::round(tw.length() * rec->samplingFrequency());
	if (nsamp == 0)
		throw Core::GeneralException("fill gaps: empty timewindow");

	DoubleArrayPtr array = new DoubleArray(nsamp);

	for (int i = 0; i < nsamp; ++i)
		array->typedData()[i] = 0.0;

	rec->setData(array->size(), array->data(), array->dataType());

	return rec;
}


//! fill up gaps with linear interpolated values, between last and first recurrent sample
Record* linearInterpolate(const Record* prev, const Record* next) {
	if (!prev || !next)
		throw Core::GeneralException("fill gaps: prev or next record must not be NULL");


 	GenericRecord* rec = new GenericRecord(*prev);
	rec->setStartTime(prev->endTime());

	Core::TimeWindow tw(prev->endTime(), next->startTime());

	int nsamp = Private::round(tw.length() * rec->samplingFrequency());
	if (nsamp == 0) {
		delete rec;
		throw Core::GeneralException("fill gaps: empty timewindow");
	}

	DoubleArray* prevData;
	DoubleArray* nextData;
	double prevSample, nextSample;
	
	// If prev or next record has no data associated, we set
	// the according last/first sample to zero.
	// Usefull, if you want to fill up missing samples at
	// the beginning/end of a RecordSequence.
	if (prev->data()) {
		prevData = static_cast<DoubleArray*>(ArrayFactory::Create(Array::DOUBLE, prev->data()));
		prevSample = prevData->typedData()[prevData->size()-1];
	}
	else {
		prevSample = 0.0;
	}
	
	if (next->data()) {
		nextData = static_cast<DoubleArray*>(ArrayFactory::Create(Array::DOUBLE, next->data()));
		nextSample = nextData->typedData()[0];
	}
	else {
		nextSample = 0.0;
	}

	double t = (nextSample - prevSample) / nsamp;

	DoubleArrayPtr array = new DoubleArray(nsamp);

	for (int i = 0; i < nsamp; ++i)
		array->typedData()[i] = i * t + prevSample;

	rec->setData(array->size(), array->data(), array->dataType());

	return rec;
}


}

}
