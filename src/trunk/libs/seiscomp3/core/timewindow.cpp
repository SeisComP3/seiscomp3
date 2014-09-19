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


#include<seiscomp3/core/timewindow.h>

namespace Seiscomp {
namespace Core {


TimeWindow TimeWindow::operator|(const TimeWindow &other) const {
	TimeWindow tw(*this);

	if ( !tw ) {
		tw = other;
	}
	else if ( other ) {
		if ( tw.startTime() > other.startTime() )
			tw.setStartTime(other.startTime());
	
		if ( tw.endTime() < other.endTime() )
			tw.setEndTime(other.endTime());
	}

	return tw;
}


TimeWindow TimeWindow::merge(const TimeWindow &other) const {
	return *this | other;
}


bool TimeWindow::overlaps(const TimeWindow &tw) const
{
	if (contains(tw) || tw.contains(*this))
		return true;
	else {
		if (contains(tw._startTime) || contains(tw._endTime))
			return true;
	}

	return false;
}

TimeWindow TimeWindow::overlap(const TimeWindow &tw) const
{
	if (contains(tw))
		return tw;
	if (tw.contains(*this))
		return *this;

	if (contains(tw._startTime))
		return TimeWindow(tw._startTime, _endTime);
	if (contains(tw._endTime))
		return TimeWindow(_startTime, tw._endTime);

	return TimeWindow();
}

bool TimeWindow::contiguous(const TimeWindow &other, double tolerance) const
{
	double dt = (double)(other._startTime - _endTime);

	dt = dt<0 ? -dt : dt;
	if (dt>tolerance)
		return false;

	return true;
}

void TimeWindow::extend(const TimeWindow &other)
{
	// FIXME
	// if (other._endTime < _startTime)
	//	throw ...
	_endTime = other._endTime;	
}

bool TimeWindow::equals(const TimeWindow &tw, double tolerance) const {

	double sdt = (double)(_startTime - tw._startTime);
	if ((sdt<0?-sdt:sdt) > tolerance)
		return false;

	double edt = (double)(_endTime - tw._endTime);
	if ((edt<0?-edt:edt) > tolerance)
		return false;

	return true;

}


}
}

