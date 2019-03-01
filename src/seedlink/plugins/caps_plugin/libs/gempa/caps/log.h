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


#ifndef __GEMPA_CAPS_LOG_H__
#define __GEMPA_CAPS_LOG_H__


namespace Gempa {
namespace CAPS {


enum LogLevel {
	LL_UNDEFINED = 0,
	LL_ERROR,
	LL_WARNING,
	LL_NOTICE,
	LL_INFO,
	LL_DEBUG,
	LL_QUANTITY
};


#define CAPS_LOG_MSG(LogLevel, ...) \
	do {\
		if ( Gempa::CAPS::LogHandler[LogLevel] != NULL )\
			Gempa::CAPS::LogHandler[LogLevel](__VA_ARGS__);\
	} while(0)


#define CAPS_ERROR(...) CAPS_LOG_MSG(Gempa::CAPS::LL_ERROR, __VA_ARGS__)
#define CAPS_WARNING(...) CAPS_LOG_MSG(Gempa::CAPS::LL_WARNING, __VA_ARGS__)
#define CAPS_NOTICE(...) CAPS_LOG_MSG(Gempa::CAPS::LL_NOTICE, __VA_ARGS__)
#define CAPS_INFO(...) CAPS_LOG_MSG(Gempa::CAPS::LL_INFO, __VA_ARGS__)
#define CAPS_DEBUG(...) CAPS_LOG_MSG(Gempa::CAPS::LL_DEBUG, __VA_ARGS__)


typedef void (*LogOutput)(const char *, ...);

extern LogOutput LogHandler[LL_QUANTITY];


void SetLogHandler(LogLevel, LogOutput);


}
}


#endif
