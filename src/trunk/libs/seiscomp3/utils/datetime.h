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


#ifndef __SEISCOMP_UTILS_DATETIME_H__
#define __SEISCOMP_UTILS_DATETIME_H__


#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core.h>

namespace Seiscomp {
namespace Util {


/**
 * Returns the number of seconds passed since the
 * last day of a given time has been started.
 * @param time The given time
 * @return The number of seconds including microseconds
 */
SC_SYSTEM_CORE_API double timeOfDay(const Seiscomp::Core::Time& time);

/**
 * Returns the number of seconds passed since the current
 * day has been started.
 * @return The number of seconds including microseconds
 */
SC_SYSTEM_CORE_API double timeOfDay();


struct TimeVal {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int usec;
	TimeVal() :
		year(0), month(0), day(0), hour(0), minute(0), second(), usec(0) {
	}
};

SC_SYSTEM_CORE_API bool getTime(const Core::Time& time, TimeVal& tv);

SC_SYSTEM_CORE_API void setTime(Core::Time& time, const TimeVal& tv);

}
}

#endif
