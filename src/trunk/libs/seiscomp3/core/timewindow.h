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


#ifndef __SC_CORE_TIMESPAN_H__
#define __SC_CORE_TIMESPAN_H__

#include<seiscomp3/core.h>
#include<seiscomp3/core/datetime.h>

namespace Seiscomp {
namespace Core {

class SC_SYSTEM_CORE_API TimeWindow {

	//  Xstruction
	public:
		TimeWindow();
		TimeWindow(const Time &startTime, double length);
		TimeWindow(const Time &startTime, const Time &endTime);
		TimeWindow(const TimeWindow &tw);
		~TimeWindow() {}

	//  Operators
	public:
		bool operator==(const TimeWindow&) const;
		bool operator!=(const TimeWindow&) const;
		operator bool() const;

		//! Returns the minimal timewindow including this and other
		TimeWindow operator|(const TimeWindow &other) const;

		// more operators :-)

	//  Interface
	public:
		Time startTime() const;
		Time endTime() const;
		double length() const;

		void set(const Time &t1, const Time &t2);
		void setStartTime(const Time &t);
		void setEndTime(const Time &t);
		//! set length in seconds, affects endTime
		void setLength(double length);

		//! does it contain time t?
		bool contains(const Time &t) const;

		//! does it contain time window tw completely?
		bool contains(const TimeWindow &tw) const;

		//! is equal to time window?
		//! +/- tolerance in seconds
		bool equals(const TimeWindow &tw, double tolerance=0.0) const;

		//! does it overlap with time window tw?
		bool overlaps(const TimeWindow &tw) const;

		//! compute overlap with time window tw
		TimeWindow overlap(const TimeWindow &tw) const;

		//! test if this+other would form a contiguous time window
		bool contiguous(const TimeWindow&, double tolerance=0) const;

		//! extend time window by appending the other (without check!)
		void extend(const TimeWindow&);

		//! merges this and other to the minimal timewindow overlapping both
		TimeWindow merge(const TimeWindow&) const;

	//  Implementation
	private:
		Time _startTime, _endTime;
};


inline TimeWindow::TimeWindow()
{
	set(Time(), Time());
}

inline TimeWindow::TimeWindow(const Time &startTime, double length)
{
	set(startTime, startTime + Time(length)); // FIXME
}

inline TimeWindow::TimeWindow(const Time &startTime, const Time &endTime)
{
	set(startTime, endTime);
}

inline TimeWindow::TimeWindow(const TimeWindow &tw)
{
	set(tw._startTime, tw._endTime);
}

inline bool TimeWindow::operator==(const TimeWindow &tw) const
{
	return _startTime == tw._startTime && _endTime == tw._endTime;
}

inline bool TimeWindow::operator!=(const TimeWindow &tw) const
{
	return _startTime != tw._startTime || _endTime != tw._endTime;
}

inline TimeWindow::operator bool() const
{
	return (bool)_startTime && (bool)_endTime;
}

inline Time TimeWindow::startTime() const
{
	return _startTime;
}

inline Time TimeWindow::endTime() const
{
	return _endTime;
}

inline double TimeWindow::length() const
{
	return (double)(_endTime-_startTime);
}

inline void TimeWindow::set(const Time &startTime, const Time &endTime)
{
	_startTime = startTime;
	_endTime = endTime;
}

inline void TimeWindow::setStartTime(const Time &t)
{
	_startTime = t;
}

inline void TimeWindow::setEndTime(const Time &t)
{
	_endTime = t;
}

inline void TimeWindow::setLength(double length)
{
	_endTime = _startTime + Time(length); // FIXME
}

inline bool TimeWindow::contains(const Time &t) const
{
	return t >= _startTime && t <= _endTime;
}

inline bool TimeWindow::contains(const TimeWindow &tw) const
{
	return tw._startTime >= _startTime && tw._endTime <= _endTime;
}

}
}

#endif
