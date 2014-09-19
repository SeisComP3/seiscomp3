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
#define SYSLOG_NAMES

#ifndef WIN32

#include <seiscomp3/logging/syslog.h>
#include <syslog.h>

#include <cstring>
#include <iostream>

#ifndef SYSLOG_FACILITY
#  define SYSLOG_FACILITY LOG_LOCAL0
#endif


namespace Seiscomp {
namespace Logging {


SyslogOutput::SyslogOutput()
	: _openFlag(false), _facility(SYSLOG_FACILITY) {
}

SyslogOutput::SyslogOutput(const char* ident, const char *facility)
    : _openFlag(false), _facility(SYSLOG_FACILITY) {
	SyslogOutput::open(ident, facility);
}

SyslogOutput::~SyslogOutput() {
	SyslogOutput::close();
}

bool SyslogOutput::open(const char* ident, const char *facility) {
	CODE *names = facilitynames;
	_facility = SYSLOG_FACILITY;

	if ( facility != NULL ) {
		_facility = -1;

		for ( ; names->c_name != NULL; ++names ) {
			if ( strcmp(names->c_name, facility) == 0 ) {
				_facility = names->c_val;
				break;
			}
		}

		if ( _facility == -1 ) {
			std::cerr << "Invalid syslog facility: " << facility << std::endl;
			return false;
		}
	}

	openlog(ident, 0, _facility);
	_openFlag = true;
	return true;
}

void SyslogOutput::close() {
	if ( _openFlag )
		closelog();
	
	_openFlag = false;
}

bool SyslogOutput::isOpen() const {
	return _openFlag;
}

void SyslogOutput::log(const char* channelName,
                       LogLevel level,
                       const char* msg,
                       time_t time) {
	
	int priority = LOG_ALERT;
	switch ( level ) {
		case LL_CRITICAL:
			priority = LOG_CRIT;
			break;
		case LL_ERROR:
			priority = LOG_ERR;
			break;
		case LL_WARNING:
			priority = LOG_WARNING;
			break;
		case LL_NOTICE:
			priority = LOG_NOTICE;
			break;
		case LL_INFO:
			priority = LOG_INFO;
			break;
		case LL_DEBUG:
		case LL_UNDEFINED:
			priority = LOG_DEBUG;
			break;
		default:
			break;
	}

	syslog(priority, "[%s/%s] %s", channelName, component(), msg);
}



}
}

#endif
