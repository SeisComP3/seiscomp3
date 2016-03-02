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

#define SEISCOMP_COMPONENT log

#include <seiscomp3/logging/fd.h>
#include <stdio.h>
#include <sstream>


namespace Seiscomp {
namespace Logging {


#ifdef _WIN32
#  include <io.h>
#  define write(fd, buf, n) _write((fd), (buf), static_cast<unsigned>(n))
#else
#  include <unistd.h>
#  define USE_COLOURS
#endif

namespace {

const char kNormalColor[] = "\033[0m";
const char kRedColor[]    = "\033[31m";
const char kGreenColor[]  = "\033[32m";
const char kYellowColor[] = "\033[33m";

}


FdOutput::FdOutput(int fdOut) : _fdOut(fdOut)
{
#ifdef USE_COLOURS
    _colorize = isatty(fdOut);
#else
    _colorize = false;
#endif
}


FdOutput::~FdOutput() {
}


void FdOutput::log(const char* channelName,
                   LogLevel level,
                   const char* msg,
                   time_t time) {
	char timeStamp[32];

	tm currentTime;

	currentTime = _useUTC ? *gmtime(&time) : *localtime(&time);

	const char *color = NULL;

	if ( _colorize ) {
		sprintf(timeStamp, "%s%02i:%02i:%02i%s ",
			kGreenColor,
			currentTime.tm_hour,
			currentTime.tm_min,
			currentTime.tm_sec,
			kNormalColor);

		switch(level) {
			case LL_CRITICAL:
			case LL_ERROR:
				color = kRedColor;
				break;
			case LL_WARNING:
				color = kYellowColor;
				break;
			case LL_NOTICE:
			case LL_INFO:
			case LL_DEBUG:
			default:
				break;
		}
	}
	else {
		sprintf(timeStamp, "%02i:%02i:%02i ",
			currentTime.tm_hour,
			currentTime.tm_min,
			currentTime.tm_sec);
	}

	std::ostringstream ss;

	ss << timeStamp;
	ss << '[' << channelName;
	if ( likely(_logComponent) )
		ss << "/" << component();
	ss << "] ";
	if ( unlikely(_logContext) )
		ss << "(" << fileName() << ':' << lineNum() << ") ";

#ifndef _WIN32
	// THREAD ID
	/*
	char tid[16] = "";
	snprintf(tid,15,"%lu",pthread_self());
	ss << "[tid:" << tid << "] ";
	*/
#endif

	if( color )
		ss << color;

	ss << msg;

	if ( color )
		ss << kNormalColor;

	ss << std::endl;

	std::string out = ss.str();
	ssize_t len = write(_fdOut, out.c_str(), out.length());
	if ( len == -1 ) {}
}


}
}
