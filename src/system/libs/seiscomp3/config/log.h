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


#ifndef __SEISCOMP_CONFIG_LOG_H__
#define __SEISCOMP_CONFIG_LOG_H__


#include <seiscomp3/config/api.h>
#include <cstdio>


namespace Seiscomp {
namespace Config {


enum LogLevel {
	ERROR,
	WARNING,
	INFO,
	DEBUG
};


struct SC_CONFIG_API Logger {
	virtual ~Logger();
	virtual void log(LogLevel, const char *filename, int line, const char *msg);
};


extern char log_msg_buffer[1024];


#define CONFIG_LOG_CHANNEL(chan, msg, ...) \
	if ( _logger ) {\
		snprintf(log_msg_buffer, 1023, msg, __VA_ARGS__);\
		_logger->log(chan, _fileName.c_str(), _line, log_msg_buffer);\
	}


#define CONFIG_ERROR(msg, ...) CONFIG_LOG_CHANNEL(ERROR, msg, __VA_ARGS__)
#define CONFIG_WARNING(msg, ...) CONFIG_LOG_CHANNEL(WARNING, msg, __VA_ARGS__)
#define CONFIG_INFO(msg, ...) CONFIG_LOG_CHANNEL(INFO, msg, __VA_ARGS__)
#define CONFIG_DEBUG(msg, ...) CONFIG_LOG_CHANNEL(DEBUG, msg, __VA_ARGS__)


}
}


#endif
