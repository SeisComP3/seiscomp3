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


#include <seiscomp3/core/datetime.h>
#include <sstream>
#include <cmath>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <seiscomp3/core/exceptions.h>

#ifdef WIN32
#include <time.h>
#endif


using namespace Seiscomp::Core;


/* We are linking against the multithreaded versions
   of the Microsoft runtimes - this makes gmtime
   equiv to gmtime_r in that Windows gmtime is threadsafe
*/
#if defined (WIN32)
static struct tm* gmtime_r(const time_t *timep, struct tm* result)
{
        struct tm *local;

        local = gmtime(timep);
        memcpy(result,local,sizeof(struct tm));
        return result;
}
#endif


#if defined(WIN32)

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif


struct timezone
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag;

  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);

    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    /*converting file time to unix epoch*/
    tmpres /= 10;  /*convert into microseconds*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS;
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }

  if (NULL != tz)
  {
    if (!tzflag)
    {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime = _daylight;
  }

  return 0;
}
#endif


#if defined(WIN32)
extern "C" {
#include <seiscomp3/core/strptime.h>
}
#endif
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define MICROS 1000000

namespace {


#ifdef __sun__
#define NO_COMPACT_DATE
#endif


#if defined(__SUNPRO_CC) || defined(__sun__) || defined(WIN32)
time_t timegm(struct tm *t) {
	time_t tl, tb;
	struct tm tg;

	t->tm_isdst = 0;

	tl = mktime (t);
	if (tl == -1) {
		t->tm_hour--;
		tl = mktime (t);
		if (tl == -1)
			return -1; /* can't deal with output from strptime */
		tl += 3600;
	}

	gmtime_r(&tl, &tg);
	tg.tm_isdst = 0;
	tb = mktime (tg);
	if (tb == -1) {
		--tg.tm_hour;
		tb = mktime(&tg);
		if (tb == -1)
			return -1; /* can't deal with output from gmtime */
		tb += 3600;
	}

	return (tl - (tb - tl));
}
#endif


template <typename T, typename U>
inline void normalize(T &sec, U &usec) {
	if ( usec < 0 ) {
		if ( sec > 0 || usec <= -MICROS ) {
			usec += MICROS;
			sec -= 1;
		}
	}
	else if ( usec > 0 ) {
		if ( sec < 0 || usec >= MICROS ) {
			usec -= MICROS;
			sec += 1;
		}
	}
}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan::TimeSpan() {
	_timeval.tv_sec = 0;
	_timeval.tv_usec = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan::TimeSpan(struct timeval* t) {
	_timeval.tv_sec = t->tv_sec;
	_timeval.tv_usec = t->tv_usec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan::TimeSpan(const struct timeval& t) {
	_timeval.tv_sec = t.tv_sec;
	_timeval.tv_usec = t.tv_usec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan::TimeSpan(double t) {
	*this = t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan::TimeSpan(long secs, long usecs) {
	_timeval.tv_sec = secs + (usecs / MICROS);
	_timeval.tv_usec = usecs % MICROS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan::TimeSpan(const TimeSpan& ts) {
	_timeval.tv_sec = ts._timeval.tv_sec;
	_timeval.tv_usec = ts._timeval.tv_usec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TimeSpan::operator==(const TimeSpan& t) const {
	return _timeval.tv_sec == t._timeval.tv_sec &&
	       _timeval.tv_usec == t._timeval.tv_usec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TimeSpan::operator!=(const TimeSpan& t) const {
	return !(*this == t);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TimeSpan::operator< (const TimeSpan& t) const {
	if ( _timeval.tv_sec > t._timeval.tv_sec )
		return false;
	if ( _timeval.tv_sec < t._timeval.tv_sec )
		return true;
	return _timeval.tv_usec < t._timeval.tv_usec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TimeSpan::operator<=(const TimeSpan& t) const {
	if ( _timeval.tv_sec > t._timeval.tv_sec )
		return false;
	if ( _timeval.tv_sec < t._timeval.tv_sec )
		return true;
	return _timeval.tv_usec <= t._timeval.tv_usec;

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TimeSpan::operator> (const TimeSpan& t) const {
	if ( _timeval.tv_sec < t._timeval.tv_sec )
		return false;
	if ( _timeval.tv_sec > t._timeval.tv_sec )
		return true;
	return _timeval.tv_usec > t._timeval.tv_usec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TimeSpan::operator>=(const TimeSpan& t) const {
	if ( _timeval.tv_sec < t._timeval.tv_sec )
		return false;
	if ( _timeval.tv_sec > t._timeval.tv_sec )
		return true;
	return _timeval.tv_usec >= t._timeval.tv_usec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan::operator double() const {
	return (double)_timeval.tv_sec +
	       (double)_timeval.tv_usec * 0.000001;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan::operator const timeval&() const {
	return _timeval;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan& TimeSpan::operator=(long t) {
	_timeval.tv_sec = t;
	_timeval.tv_usec = 0;

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan& TimeSpan::operator=(double t) {
	if(t>(double)0x7fffffff||t<-(double)0x80000000)
		throw Core::OverflowException("TimeSpan::operator=(): double doesn't fit int");
	_timeval.tv_sec = (long)t;
	_timeval.tv_usec = (long)((t-_timeval.tv_sec)*MICROS);

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan TimeSpan::operator+(const TimeSpan& t) const {
	long diff_usec = _timeval.tv_usec + t._timeval.tv_usec;
	long int sec = _timeval.tv_sec + t._timeval.tv_sec;

	normalize(sec, diff_usec);

	return TimeSpan(sec, diff_usec);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan TimeSpan::operator-(const TimeSpan& t) const {
	long diff_usec = _timeval.tv_usec - t._timeval.tv_usec;
	long int sec = _timeval.tv_sec - t._timeval.tv_sec;

	normalize(sec, diff_usec);

	return TimeSpan(sec, diff_usec);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan& TimeSpan::operator+=(const TimeSpan& t) {
	_timeval.tv_sec += t._timeval.tv_sec;
	_timeval.tv_usec += t._timeval.tv_usec;

	normalize(_timeval.tv_sec, _timeval.tv_usec);

	return *this;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
TimeSpan& TimeSpan::operator-=(const TimeSpan& t) {
	_timeval.tv_usec -= t._timeval.tv_usec;
	_timeval.tv_sec -= t._timeval.tv_sec;

	normalize(_timeval.tv_sec, _timeval.tv_usec);

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan& TimeSpan::set(long secs) {
	_timeval.tv_sec = secs;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan& TimeSpan::setUSecs(long usecs) {
	_timeval.tv_usec = usecs % MICROS;
	_timeval.tv_sec += usecs / MICROS;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<
void TimeSpan::elapsedTime(int* days, int* hours,
                           int* minutes, int* seconds) const
{
	int elapsed = TimeSpan::seconds();
	if (days)
		*days = elapsed / 86400;
	if (hours)
		*hours = (elapsed % 86400) / 3600;
	if (minutes)
		*minutes = ((elapsed % 86400) % 3600) / 60;
	if (seconds)
		*seconds = ((elapsed % 86400) % 3600) % 60;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan TimeSpan::abs() const {
	return TimeSpan(::abs(_timeval.tv_sec), ::abs(_timeval.tv_usec));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double TimeSpan::length() const {
	return double(*this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
long TimeSpan::seconds() const {
	return _timeval.tv_sec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
long TimeSpan::microseconds() const {
	return _timeval.tv_usec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Time Time::Null(0.0);
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::Time() : TimeSpan() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::Time(const TimeSpan& ts)
 : TimeSpan(ts) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::Time(const struct timeval& t)
 : TimeSpan(t) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::Time(struct timeval* t)
 : TimeSpan(t) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::Time(double t) {
	*this = t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::Time(int year, int month, int day,
           int hour, int min, int sec,
           int usec) {
	set(year, month, day, hour, min, sec, usec);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::Time(const Time& t)
 : TimeSpan(t) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::Time(long secs, long usecs)
 : TimeSpan(secs, usecs) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::operator bool() const {
	return valid();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::operator time_t() const {
	return (time_t)_timeval.tv_sec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::operator=(const struct timeval& t) {
	_timeval = t;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::operator=(struct timeval* t) {
	_timeval = *t;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::operator=(time_t t) {
	_timeval.tv_sec = (long)t;
	_timeval.tv_usec = 0;

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::operator=(double t) {
	_timeval.tv_sec = (long)t;
	_timeval.tv_usec = (long)((t-(double)_timeval.tv_sec)*MICROS);

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::operator+(const TimeSpan& t) const {
	return Time((TimeSpan&)*this + t);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::operator-(const TimeSpan& ts) const {
	return Time(TimeSpan::operator- (ts));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::operator+=(const TimeSpan& ts) {
	TimeSpan::operator+=(ts);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::operator-=(const TimeSpan& ts) {
	TimeSpan::operator-=(ts);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan Time::operator-(const Time& ts) const {
	return TimeSpan::operator-(ts);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::set(int year, int month, int day,
                int hour, int min, int sec,
                int usec) {
	tm t;

	t.tm_year = year - 1900;
	t.tm_mon = month - 1;
	t.tm_mday = day;
	t.tm_hour = hour;
	t.tm_min = min;
	t.tm_sec = sec;
	t.tm_isdst = -1;

	_timeval.tv_sec = (long)timegm(&t);
	setUSecs(usec);

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Time::get(int *year, int *month, int *day,
               int *hour, int *min, int *sec,
               int *usec) const {
	time_t time = (time_t)_timeval.tv_sec;
	struct tm t;
	gmtime_r(&time, &t);

	if ( year )  *year = t.tm_year + 1900;
	if ( month ) *month = t.tm_mon + 1;
	if ( day )   *day = t.tm_mday;

	if ( hour )  *hour = t.tm_hour;
	if ( min )   *min = t.tm_min;
	if ( sec )   *sec = t.tm_sec;

	if ( usec )  *usec = _timeval.tv_usec;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Time::get2(int *year, int *yday, int *hour, int *min, int *sec,
                int *usec) const {
	time_t time = (time_t)_timeval.tv_sec;
	struct tm t;
	gmtime_r(&time, &t);

	if ( year )  *year = t.tm_year + 1900;
	if ( yday )   *yday = t.tm_yday;

	if ( hour )  *hour = t.tm_hour;
	if ( min )   *min = t.tm_min;
	if ( sec )   *sec = t.tm_sec;

	if ( usec )  *usec = _timeval.tv_usec;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::LocalTime() {
	Time t;
	t.localtime();
	return t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::GMT() {
	Time t;
	t.gmt();
	return t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::FromYearDay(int year, int year_day) {
	std::stringstream ss;
	ss << year << " " << year_day;
	return FromString(ss.str().c_str(), "%Y %j");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::localtime() {
	gettimeofday(&_timeval, NULL);
	time_t secs = (time_t)_timeval.tv_sec;
	struct tm _tm;
	_timeval.tv_sec = (long)timegm(::localtime_r(&secs, &_tm));

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::gmt() {
	gettimeofday(&_timeval, NULL);
	time_t secs = (time_t)_timeval.tv_sec;
	struct tm _tm;
	_timeval.tv_sec = (long)mktime(::localtime_r(&secs, &_tm));

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::toLocalTime() const {
	Time ret;
	time_t secs = _timeval.tv_sec;
	struct tm _tm;
	ret._timeval.tv_sec = (long)timegm(::localtime_r(&secs, &_tm));
	ret._timeval.tv_usec = _timeval.tv_usec;

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::toGMT() const {
	Time ret;
	time_t secs = _timeval.tv_sec;
	struct tm _tm;
	ret._timeval.tv_sec = _timeval.tv_sec - ((long)timegm(::localtime_r(&secs, &_tm)) - _timeval.tv_sec);
	ret._timeval.tv_usec = _timeval.tv_usec;

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Time::valid() const {
	return _timeval.tv_sec != 0 || _timeval.tv_usec != 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Time::toString(const char* fmt) const {
#define BUFFER_SIZE 64
	char data[BUFFER_SIZE];
	char predata[BUFFER_SIZE];

	time_t secs = (time_t)_timeval.tv_sec, usecs = _timeval.tv_usec;

	tm t;
	gmtime_r(&secs, &t);
	const char *f = fmt, *last = fmt;
	char *tgt = predata;

	while ( (f = strchr(f, '%')) != NULL ) {
		int specSize = 3;

		char spec = *(f+1);
		if ( spec == '\0' ) break;
		char type = *(f+2);

		if ( (spec >= 'a' && spec <= 'z') || (spec >= 'A' && spec <= 'Z') ) {
			specSize = 2;
			type = spec;
		}

		if ( type == 'f' ) {
			int width = -1;
			if ( spec >= '0' && spec <= '6' )
				width = spec - '0';

			memcpy(tgt, last, f-last);
			tgt += f-last;

			char number[12];
			size_t numberOfDigits;
			if ( usecs > 0 ) {
				numberOfDigits = sprintf(number, "%06ld", usecs);
				if ( width != -1 )
					numberOfDigits = width;
				else {
					while ( number[numberOfDigits-1] == '0' ) --numberOfDigits;
				}
			}
			else {
				if ( width == -1 )
					numberOfDigits = 4;
				else
					numberOfDigits = width;
				sprintf(number, "%0*d", (int)numberOfDigits, 0);
			}

			memcpy(tgt, number, numberOfDigits);
			tgt += numberOfDigits;

			last = f+specSize;
		}
#if defined(WIN32) || defined(NO_COMPACT_DATE)
		else if ( type == 'F' ) {
			memcpy(tgt, last, f-last);
			tgt += f-last;
			memcpy(tgt, "%Y-%m-%d", 8);
			tgt += 8;
			last = f+specSize;
		}
#endif
#if defined(WIN32)
		else if ( type == 'T' ) {
			memcpy(tgt, last, f-last);
			tgt += f-last;
			memcpy(tgt, "%H:%M:%S", 8);
			tgt += 8;
			last = f+specSize;
		}
#endif

		++f;
	}

	strcpy(tgt, last);
	strftime(data, BUFFER_SIZE-1, predata, &t);

	return data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Time::iso() const {
	return toString("%FT%T.%fZ");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Time::fromString(const char* str, const char* fmt) {
	struct tm t;
	char data[BUFFER_SIZE];
	char tmpFmt[BUFFER_SIZE];
	long usec = 0;

	const char* microSeconds = strstr(fmt, "%f");
	if ( microSeconds != NULL ) {
		const char* start = str;
		if ( microSeconds != fmt ) {
			start = strrchr(str, *(microSeconds-1));
			if ( start == NULL )
				return false;
			++start;
		}

		const char* end = start;
		while ( *end >= '0' && *end <= '9' )
			++end;

		int size = end-start;
		if ( size > 6 ) size = 6;

		int multiplier = 100000;
		char *startNumber, *endNumber;

		memcpy(data, start, size);
		data[size] = '\0';

		for ( startNumber = data; *startNumber == '0' && *startNumber != '\0'; ++startNumber )
			multiplier /= 10;

		for ( endNumber = data + size-1; *endNumber == '0' && endNumber > startNumber; --endNumber )
			*endNumber = '\0';

		while ( endNumber-- > startNumber )
			multiplier /= 10;

		usec = atoi(data) * multiplier;

		memcpy(data, str, start-str);
		data[start - str] = '\0';
		strcat(data, "%");
		strcat(data, end);
		str = data;

		strcpy(tmpFmt, fmt);
		tmpFmt[microSeconds - fmt + 1] = '%';
		fmt = tmpFmt;
	}

#ifdef NO_COMPACT_DATE
	char tmpFmtDate[BUFFER_SIZE];
	const char* compactDate = strstr(fmt, "%F");
	if ( compactDate != NULL ) {
		char *dst = tmpFmtDate;
		while ( fmt != compactDate ) { *dst++ = *fmt++; }
		strcpy(dst, "%Y-%m-%d");
		dst += 8;
		fmt += 2;
		while ( *fmt != '\0' ) { *dst++ = *fmt++; }
		*dst = '\0';
		fmt = tmpFmtDate;
	}
#endif

	time_t tmp_t = 0;
	gmtime_r(&tmp_t, &t);
	if ( strptime(str, fmt, &t) == NULL ) {
		*this = (time_t)0;
		return false;
	}
	else {
		*this = timegm(&t);
		setUSecs(usec);
	}

	return true;
#undef BUFFER_SIZE
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::FromString(const char* str, const char* fmt) {
	Time t;
	t.fromString(str, fmt);
	return t;
}
