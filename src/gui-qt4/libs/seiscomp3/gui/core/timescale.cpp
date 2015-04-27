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



#include <cstdio>
#include <cmath>
#include <iostream>

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>

#include "timescale.h"

#include <seiscomp3/core/exceptions.h>

namespace Seiscomp {
namespace Gui {


TimeScale::TimeScale(QWidget *parent, Qt::WindowFlags f, Position pos)
 : Ruler(parent, f, pos) {
	_showAbsoluteValues = false;
	_showAbsoluteDate= false;
	setLimits(Seiscomp::Core::TimeSpan::MinTime, Seiscomp::Core::TimeSpan::MaxTime, 0.001, 315360000.0);
}


void TimeScale::setAlignment(const Seiscomp::Core::Time& t) {
	_alignment = t;
	_ofs = _showAbsoluteValues ? (double)_alignment : 0;
	emit changedInterval(_drx[0], _drx[1], _ofs);
}


void TimeScale::setAbsoluteTimeEnabled(bool absoluteTime, bool absoluteDate) {
	absoluteDate = absoluteTime && absoluteDate;
	if ( _showAbsoluteValues == absoluteTime &&
	     _showAbsoluteDate == absoluteDate )
		return;

	_showAbsoluteValues = absoluteTime;
	_showAbsoluteDate = absoluteDate;
	_ofs = _showAbsoluteValues ? (double)_alignment : 0;
	setLineCount(_showAbsoluteDate ? 2 : 1);

	updateIntervals();
	update();
}

void TimeScale::updateIntervals() {
	struct Spacing {
		double      major;
		double      minor;
		const char *absoluteFormat;
		const char *absoluteSecondFormat;
		const char *relativeFormat;
	};

	const Spacing spacings[] = {
		{0.001, 0.0002, "%S.%3f", "%F %H:%M", ""},
		{0.005, 0.001, "%S.%3f", "%F %H:%M", ""},
		{0.01, 0.002, "%S.%2f", "%F %H:%M", ""},
		{0.05, 0.01, "%S.%2f", "%F %H:%M", ""},
		{0.1, 0.02, "%T.%1f", "%F", ""},
		{0.5, 0.1, "%T.%1f", "%F", ""},
		{1, 0.2, "%T", "%F", ""},
		{2, 0.5, "%T", "%F", ""},
		{5, 1, "%T", "%F", ""},
		{10, 2, "%T", "%F", ""},
		{15, 3, "%T", "%F", ""},
		{30, 5, "%T", "%F", ""},
		{60, 10, "%T", "%F", ""},
		{120, 20, "%T", "%F", ""},
		{300, 60, "%T", "%F", ""},
		{600, 120, "%T", "%F", ""},
		{900, 300, "%T", "%F", ""},
		{1800, 300, "%T", "%F", ""},
		{3600, 600, "%T", "%F", ""},
		{2*3600, 1200, "%T", "%F", ""},
		{6*3600, 3600, "%T", "%F", ""},
		{12*3600, 2*3600, "%T", "%F", ""},
		{24*3600, 6*3600, "%b, %d", "%Y", ""},
		{2*24*3600, 12*3600, "%b, %d", "%Y", ""},
		{7*24*3600, 24*3600, "%b, %d", "%Y", ""},
		{14*24*3600, 2*24*3600, "%b, %d", "%Y", ""}
	};

	unsigned int imax = sizeof(spacings)/sizeof(Spacing);

	_max = _min + rulerWidth() / _scl;
	_drx[0] = spacings[imax-1].major;
	_drx[1] = spacings[imax-1].minor;
	_primaryTimeFormat = spacings[imax-1].absoluteFormat;
	_secondaryTimeFormat = spacings[imax-1].absoluteSecondFormat;
	_relativeTimeFormat = spacings[imax-1].relativeFormat;

	int minDistance = fontMetrics().width("  XXXX-XX-XX.X  ");
	unsigned int i;
	for ( i = 0; i < imax; ++i ) {
		if ( spacings[i].major*_scl >= minDistance ) {
			_drx[0] = spacings[i].major;
			_drx[1] = spacings[i].minor;
			_primaryTimeFormat = spacings[i].absoluteFormat;
			_secondaryTimeFormat = spacings[i].absoluteSecondFormat;
			_relativeTimeFormat = spacings[imax-1].relativeFormat;
			break;
		}
	}

	if ( i == imax && _scl > 0 ) {
		while ( _drx[0]*_scl < minDistance ) {
			_drx[0] *= 2;
			_drx[1] *= 2;
		}
	}

	emit changedInterval(_drx[0], _drx[1], _ofs);
}

bool TimeScale::getTickText(double pos, double lastPos,
                            int line, QString &str) const {
	if ( !_showAbsoluteValues )
		return Ruler::getTickText(pos, lastPos, line, str);

	// Fixed floating point precision error
	pos += 0.0005;

	if ( line == 0 ) {
		try {
			Seiscomp::Core::Time time(pos);
			if ( !time.valid() ) return false;

			str = time.toString(_primaryTimeFormat).c_str();
			return true;
		}
		catch ( const Core::OverflowException&) { return false; }
		
	}

	if ( line == 1 && _showAbsoluteDate ) {
		try {
			Seiscomp::Core::Time time(pos);
			if ( time.valid() ) {
				std::string timeStr = time.toString(_secondaryTimeFormat);
				Seiscomp::Core::Time prevTime(lastPos);
				if ( !prevTime.valid() ||
				     prevTime.toString(_secondaryTimeFormat) != timeStr ) {
					str = timeStr.c_str();
					return true;
				}
			}
		}
		catch ( const Core::OverflowException&) { return false; }
	}

	return false;
}

} // ns Gui
} // ns Seiscomp
