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


#ifndef __SEISCOMP_RECORDSEQUENCE_H__
#define __SEISCOMP_RECORDSEQUENCE_H__

#include <deque>
#include <seiscomp3/core/record.h>
#include <seiscomp3/core/genericrecord.h>
#include <seiscomp3/core/timewindow.h>

namespace Seiscomp {


namespace Gap {

//! predefined methods for filling up gaps
//!
//! fill up gaps with zeroes
Record* fillZeroes(const Record* prev, const Record* next);

//! linear interpolate between last and recurrent sample
Record* linearInterpolate(const Record* prev, const Record* next);

}

//! define a gap filling function type
typedef Record*(*gapFillingFunction)(const Record* previousRecord, const Record* nextRecord);


DEFINE_SMARTPOINTER(RecordSequence);

/**
 * RecordSequence
 *
 * A container class for record sequences (i.e. records sorted in time)
 * The class RecordSequence is a container for Record objects. It forms
 * the basis for the implementation as RingBuffer or TimeWindowBuffer.
 *
 * Originally, it was supposed to be a derivative of Record as well,
 * offering the same interface in order to access a sequence of records
 * as a whole; this functionality comes with some problems and for the
 * time being has been postponed; maybe it's not required at all.
 */
class SC_SYSTEM_CORE_API RecordSequence : public std::deque<RecordCPtr> // , public Record
{
	public:
		typedef std::deque<Core::TimeWindow> TimeWindowArray;

	public:
		RecordSequence(double tolerance=0.5, double maxGapLength=600.0)
			: _tolerance(tolerance),
			  _maxGapLengthForFilling(maxGapLength)
		 {}

		virtual ~RecordSequence() {}

		//! Feed a record to buffer. Returns true if the record
		//! was actually added.
		virtual bool feed(const Record*) = 0;

		//! Returns a copy of the sequence including all fed
		//! records.
		virtual RecordSequence* copy() const = 0;

		//! Returns a cloned sequence without containing records.
		virtual RecordSequence* clone() const = 0;

		//! get the time tolerance in samples
		//! The tolerance is the maximum gap/overlap (in samples)
		//! that will not break the continuity of the sequence.
		double tolerance() const;

		//! set the time tolerance in samples
		void setTolerance(double);

		//! Return Record's as one continuous Record
		Record* continuousRecord() const;

		//! same but trim to specified time window
		Record* continuousRecord(const Core::TimeWindow &tw) const;

		//! Time window currently in buffer, irrespective of gaps
		Core::TimeWindow timeWindow() const;

		//! returns the continuous time windows available
		//! This is usually one time window but may be split into
		//! several if there are any gaps.
		TimeWindowArray windows() const;

		//! returns the gaps that exceed dt *samples*
		TimeWindowArray gaps() const;

		//! Does the buffer contain all data for the time window?
		bool contains(const Core::TimeWindow &tw) const;

		//! Record interface to return the data array for
		//! the record. The array returned is a copy.
		// FIXME currently dummy implementation
//                Array* data() const { return NULL; }

		//! Record interface
		//! 'Do nothing' should be enough for buffering only
//		void getFrom(std::istream &in) throw(Core::StreamException) {}

		//! Fill gaps using associated method
		//! Skip filling, if gap length is greather than _maxGapLengthForFilling [seconds]
		//! We provide this methods: Gap::fillZeroes, Gap::linearInterpolate, (Gap::spline)
		//! Default: Gap::linearInterpolate
		virtual bool fillGaps(gapFillingFunction=Gap::linearInterpolate);

		//! return percentage of available data within TimeWindow
		double availability(const Core::TimeWindow& tw) const;


		unsigned int recordCount() const;

		// determine average timing quality from the records stored in this sequence
		//
		// returns true on success; in that case, count and quality are set
		bool timingQuality(int &count, float &quality) const;

	protected:
		bool alreadyHasRecord(const Record*) const;
		bool findInsertPosition(const Record*, iterator*);

	protected:
		double _tolerance;
		double _maxGapLengthForFilling; // in seconds

};

inline double RecordSequence::tolerance() const
{
	return _tolerance;
}

inline void RecordSequence::setTolerance(double dt)
{
	_tolerance = dt;
}

inline unsigned int RecordSequence::recordCount() const
{
	return size();
}


/**
 * TimeWindowBuffer
 *
 * A container class for record sequences (i.e. records sorted in time)
 * The records are stored only if they at least overlap with the fixed time
 * window. This is useful if only a certain fixed time window is of interest.
 */
DEFINE_SMARTPOINTER(TimeWindowBuffer);

class SC_SYSTEM_CORE_API TimeWindowBuffer : public RecordSequence
{
	public:
		TimeWindowBuffer(const Core::TimeWindow &tw, double tolerance=0.5)
			: _timeWindow(tw)  { _tolerance=tolerance; }
		~TimeWindowBuffer() {}

		//! Feed a record to buffer. Returns true upon acceptance
		bool feed(const Record*);

		RecordSequence* copy() const;

		RecordSequence* clone() const;

		//! Fill gaps using associated method
		//! Skip filling, if gap length is greater than _maxGapLengthForFilling [seconds]
		//! We provide this methods: Gap::fillZeroes, Gap::linearInterpolate
		//! Default: Gap::linearInterpolate
		//! here: re-implemented from RecordSequence::fillGaps
		//! --> fill up missing data at beginning/end of TimeWindowBuffer
		//! --> assume first/last sample has value zero, then apply fill method
		bool fillGaps(gapFillingFunction=Gap::linearInterpolate);

		//! return percentage of available data within TimeWindowBuffer's TimeWindow
		double availability() const;

		//! returns the gaps
		// This will be a re-implementation because in a time window
		// buffer, unavailable data also should count as a gap
		// => DISCUSSION NEEDED
//		TimeWindowArray gaps() const;

	private:
		Core::TimeWindow _timeWindow;
};








/**
 * RingBuffer
 *
 * A container class for record sequences (i.e. records sorted in time)
 * The records are stored only up to a maximum number; once this maximum
 * number is reached, the oldest record is removed each time a new record is
 * stored. Note that this doesn't usually guarantee a certain time window
 * length.
 */
DEFINE_SMARTPOINTER(RingBuffer);

class SC_SYSTEM_CORE_API RingBuffer : public RecordSequence
{
	public:
		//! Create RingBuffer for fixed maximum number of records
		//! This version stores at most nmax most recent records.
		RingBuffer(int nmax, double tolerance=0.5);

		//! Create RingBuffer for fixed time span
		//! This version stores at least a certain time
		//! window length of available data ending at the
		//! end time of the last record.
		RingBuffer(Core::TimeSpan span, double tolerance=0.5);

		//! Feed a record to buffer. Returns true upon acceptance
		bool feed(const Record*);

		RecordSequence* copy() const;

		RecordSequence* clone() const;

		//! clear the buffer
		void reset() { clear(); }
	private:
		unsigned int _nmax;
		Core::TimeSpan _span;

};


}

#endif

