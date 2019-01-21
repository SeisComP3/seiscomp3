/***************************************************************************
 * libcapsclient
 * Copyright (C) 2016  gempa GmbH
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/


#ifndef __GEMPA_CAPS_UTILS_H__
#define __GEMPA_CAPS_UTILS_H__

#include <gempa/caps/packet.h>

#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <string>
#include <sstream>


namespace  Gempa {
namespace CAPS {

class arraybuf : public std::streambuf {
	public:
		typedef std::streambuf::pos_type pos_type;
		typedef std::streambuf::off_type off_type;

	public:
		arraybuf(const char *buf, int len) {
			char *tmp = const_cast<char*>(buf);
			setp(tmp, tmp + len);
			setg(tmp, tmp, tmp + len);
		}

		arraybuf(const std::string &buf) {
			if ( buf.size() > 0 ) {
				char *tmp = const_cast<char*>(&buf[0]);
				setg(tmp, tmp, tmp + buf.size());
				setp(tmp, tmp + buf.size());
			}
		}

		void reset(const char *buf, int len) {
			char *tmp = const_cast<char*>(buf);
			setp(tmp, tmp + len);
		}

		virtual pos_type seekoff(off_type ofs, std::ios_base::seekdir dir,
		                         std::ios_base::openmode mode) {
			if ( mode & std::ios_base::in ) {
				char *next;

				switch ( dir ) {
					case std::ios_base::beg:
						next = eback() + ofs;
						break;
					case std::ios_base::cur:
						next = gptr() + ofs;
						break;
					case std::ios_base::end:
						next = egptr() + ofs;
						break;
					default:
						return pos_type(off_type(-1));
				}

				if ( next > egptr() || next < eback() )
					return pos_type(off_type(-1));

				gbump(next-gptr());
			}

			if ( mode & std::ios_base::out ) {
				return pos_type(off_type(-1));
			}

			return pos_type(off_type(-1));
		}

		virtual pos_type seekpos(pos_type pos, std::ios_base::openmode mode) {
			if ( mode & std::ios_base::in ) {
				char *next = eback() + pos;
				if ( next > egptr() )
					return pos_type(off_type(-1));
				gbump(next-gptr());
			}

			if ( mode & std::ios_base::out ) {
				char *next = pbase() + pos;
				if ( next > epptr() ) {
					return pos_type(off_type(-1));
				}
				pbump(pos);
			}

			return pos;
		}

		virtual std::streamsize xsgetn(char* s, std::streamsize n) {
			char *next = gptr() + n;
			if ( next >= egptr() )
				n = n - (next - egptr());

			if ( n == 0 ) return 0;

			memcpy(s, gptr(), n);
			setg(eback(), next, egptr());

			return n;
		}

		std::streampos tellg() {
			return gptr() - eback();
		}

		std::streampos tellp() {
			return pptr() - pbase();
		}
};

#define CHECK_STRING(data, str, len) \
	((len == sizeof(str)-1) && (strncasecmp(data, str, len) == 0))



/**
 * @brief Splits an address string into hostname and port
 * @param host The host
 * @param port The port
 * @param address The address
 * @param default_port The default port which will be used
 * if the addrees contains no port
 * @return True, if the address is valid
 */
inline bool splitAddress(std::string &host, unsigned short &port,
                         const std::string &address, unsigned short default_port) {
	size_t pos = address.find(':');
	if ( pos != std::string::npos ) {
		int p = -1;
		host = address.substr(0, pos);
		std::stringstream ss(address.substr(pos+1));
		ss >> p;
		if ( p > 0 && p <= 65535 )
			port = p;
		else
			return false;
	}
	else {
		host = address;
		port = default_port;
	}

	return !host.empty();
}


/*
 * Returns substring that contains leftmost characters up to the delimiter
 */
inline const char *tokenize(const char *&str, const char *delim,
                            int &len_source, int &len_tok) {
	len_tok = 0;
	for ( ; len_source; --len_source, ++str ) {
		// Hit first non delimiter?
		if ( strchr(delim, *str) == NULL ) {
			const char *tok = str;

			++str; --len_source;
			len_tok = 1;

			// Hit first delimiter?
			for ( ; len_source; --len_source, ++str, ++len_tok ) {
				if ( strchr(delim, *str) != NULL )
					break;
			}

			return tok;
		}
	}

	return NULL;
}

/*
 * Removes whitespaces from start and end of string
 */
inline const char *trim(const char *&str, int &len) {
	int i = len;
	while ( i > 0 ) {
		if ( isspace(*str) ) {
			++str;
			--len;
		}
		else
			break;
		++i;
	}

	i = len-1;
	while ( i > 0 ) {
		if ( isspace(str[i]) )
			--len;
		--i;
	}

	return str;
}


inline std::string trim(const std::string &s) {
	int l = (int)s.size();
	const char *str = &s[0];
	trim(str, l);
	return std::string(str, l);
}


/*
 * Converts  time object to timestamp
 */
inline void timeToTimestamp(TimeStamp &ts, const Time &t) {
	int year, yday, hour, min, sec, usec;
	t.get2(&year, &yday, &hour, &min, &sec, &usec);
	ts.year = year;
	ts.yday = yday;
	ts.hour = hour;
	ts.minute = min;
	ts.second = sec;
	ts.usec = usec;
}

/*
 * Converts string to int. Returns false if the conversion fails
 */
inline bool str2int(int &i, char const *s, int len = 0, int base = 10)
{
	// Return false in case of empty string
	if (*s == '\0' ) return false;

	char *end;
	long  l;
	errno = 0;
	l = strtol(s, &end, base);
	if ( errno == ERANGE ) {
		return false;
	}

	// Check if the string contains invalid chars
	if ( len == 0 ) len = strlen(s);

	int bytes_read = end - s;
	if ( bytes_read < len && *end != ' ' ) return false;

	i = l;
	return true;
}

/*
 * Converts timestamp to time object
 */
inline Time
timestampToTime(const TimeStamp &ts) {
	Time t;
	t.set(ts.year, 1, 1, ts.hour, ts.minute, ts.second, ts.usec);
	// Add the day of the year in seconds
	t += TimeSpan(ts.yday*86400);
	return t;
}

inline TimeSpan
samplesToTimeSpan(const DataRecord::Header &head, int sampleCount) {
	int64_t ms = ((int64_t)(sampleCount)) * 1000000 * head.samplingFrequencyDenominator / head.samplingFrequencyNumerator;
	return TimeSpan(ms/1000000, ms%1000000);
}

inline int
timeSpanToSamples(const DataRecord::Header &head, const TimeSpan &span) {
	int64_t ms = ((int64_t)span.seconds())*1000000 + span.microseconds();
	return ms * head.samplingFrequencyNumerator / head.samplingFrequencyDenominator / 1000000;
}

inline int
timeSpanToSamplesCeil(const DataRecord::Header &head, const TimeSpan &span) {
	int64_t ms = ((int64_t)span.seconds())*1000000 + span.microseconds();
	return (ms * head.samplingFrequencyNumerator / head.samplingFrequencyDenominator + 999999) / 1000000;
}

inline int
timeSpanToSamplesFloor(const DataRecord::Header &head, const TimeSpan &span) {
	int64_t ms = ((int64_t)span.seconds())*1000000 + span.microseconds();
	return ms * head.samplingFrequencyNumerator / head.samplingFrequencyDenominator / 1000000;
}

#if defined(WIN32)
inline void usleep(__int64 usec)
{
	HANDLE timer;
	LARGE_INTEGER ft;

	ft.QuadPart = -(10*usec); // Convert to 100 nanosecond interval, negative value indicates relative time

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}
#endif

inline uint8_t dataTypeSize(DataType dt) {
	if ( dt == DT_INT8 )
		return sizeof(int8_t);
	if ( dt == DT_INT16 )
		return sizeof(int16_t);
	else if ( dt == DT_INT32 )
		return sizeof(int32_t);
	else if ( dt == DT_INT64 )
		return sizeof(DT_INT64);
	else if ( dt == DT_FLOAT )
		return sizeof(float);
	else if ( dt == DT_DOUBLE )
		return sizeof(double);

	return 0;
}

}
}

#endif
