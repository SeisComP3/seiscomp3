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


#ifndef __SEISCOMP_RECORDSEQUENCE_H__
#define __SEISCOMP_RECORDSEQUENCE_H__


#include <seiscomp3/core/genericrecord.h>
#include <seiscomp3/core/timewindow.h>

#include <deque>


namespace Seiscomp {


/**
 * RecordSequence
 *
 * A container class for record sequences (i.e. records sorted in time)
 * The class RecordSequence is a container for Record objects. It forms
 * the basis for the implementation as RingBuffer or TimeWindowBuffer.
 */
class SC_SYSTEM_CORE_API RecordSequence : public std::deque<RecordCPtr> {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		typedef std::deque<Core::TimeWindow> TimeWindowArray;
		typedef std::pair<double,double> Range;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		RecordSequence(double tolerance=0.5);

		//! D'tor
		virtual ~RecordSequence();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Feed a record to buffer. Returns true if the record
		//! was actually added.
		virtual bool feed(const Record*) = 0;

		//! Returns a copy of the sequence including all fed
		//! records.
		virtual RecordSequence *copy() const = 0;

		//! Returns a cloned sequence without containing records.
		virtual RecordSequence *clone() const = 0;

		//! Return the time tolerance in samples
		//! The tolerance is the maximum gap/overlap (in samples)
		//! that will not break the continuity of the sequence.
		double tolerance() const;

		//! Set the time tolerance in samples
		void setTolerance(double);

		//! Return Record's as one contiguous record. Compiled in is support for
		//! float, double and int. If interpolation is enabled gaps will be linearly
		//! interpolated between the last and the next sample.
		template <typename T>
		GenericRecord *contiguousRecord(const Seiscomp::Core::TimeWindow *tw = NULL,
		                                bool interpolate = false) const;

		//! DECPRECATED: For backward compatibility, does exactly the same as
		//!              contiguousRecord. Please use contiguousRecord in your
		//!              code. This method will be removed in future releases.
		template <typename T>
		GenericRecord *continuousRecord(const Seiscomp::Core::TimeWindow *tw = NULL,
		                                bool interpolate = false) const;


		//! Time window currently in buffer, irrespective of gaps
		Core::TimeWindow timeWindow() const;

		//! The amplitude range in a given time window.
		//! Returns (0,0) if the sequence is empty or no records fall inside
		//! the given optional time window.
		Range amplitudeRange(const Seiscomp::Core::TimeWindow *tw = NULL) const;

		//! returns the continuous time windows available
		//! This is usually one time window but may be split into
		//! several if there are any gaps.
		TimeWindowArray windows() const;

		//! returns the gaps that exceed dt *samples*
		TimeWindowArray gaps() const;

		//! Does the buffer contain all data for the time window?
		bool contains(const Core::TimeWindow &tw) const;

		//! Returns the percentage of available data within a given time window
		double availability(const Core::TimeWindow& tw) const;

		//! Returns the number of stored records, same as size().
		size_t recordCount() const;

		//! Determine average timing quality from the records stored in this
		//! sequence.
		//! Returns true on success; in that case, count and quality are set
		bool timingQuality(int &count, float &quality) const;


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		bool alreadyHasRecord(const Record*) const;
		bool findInsertPosition(const Record*, iterator*);


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	protected:
		double _tolerance;
};


/**
 * TimeWindowBuffer
 *
 * A container class for record sequences (i.e. records sorted in time)
 * The records are stored only if they at least overlap with the fixed time
 * window. This is useful if only a certain fixed time window is of interest.
 */
class SC_SYSTEM_CORE_API TimeWindowBuffer : public RecordSequence {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		TimeWindowBuffer(const Core::TimeWindow &tw, double tolerance=0.5);


	// ----------------------------------------------------------------------
	//  Public RecordSequence interface overrides
	// ----------------------------------------------------------------------
	public:
		virtual bool feed(const Record*);
		virtual RecordSequence *copy() const;
		virtual RecordSequence *clone() const;


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Return percentage of available data within TimeWindowBuffer's TimeWindow
		double availability() const;


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
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
class SC_SYSTEM_CORE_API RingBuffer : public RecordSequence {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! Create RingBuffer for fixed maximum number of records
		//! This version stores at most nmax most recent records.
		RingBuffer(int nmax, double tolerance=0.5);

		//! Create RingBuffer for fixed time span
		//! This version stores at least a certain time
		//! window length of available data ending at the
		//! end time of the last record.
		RingBuffer(Core::TimeSpan span, double tolerance=0.5);


	// ----------------------------------------------------------------------
	//  Public RecordSequence interface overrides
	// ----------------------------------------------------------------------
	public:
		virtual bool feed(const Record*);
		virtual RecordSequence *copy() const;
		virtual RecordSequence *clone() const;


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! clear the buffer
		void reset() { clear(); }


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	private:
		unsigned int   _nmax;
		Core::TimeSpan _span;
};



// ----------------------------------------------------------------------
//  Inline implementations
// ----------------------------------------------------------------------

inline double RecordSequence::tolerance() const {
	return _tolerance;
}

inline void RecordSequence::setTolerance(double dt) {
	_tolerance = dt;
}

inline size_t RecordSequence::recordCount() const {
	return size();
}


}

#endif

