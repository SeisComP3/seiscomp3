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

#include <seiscomp3/logging/file.h>

#include <cstdarg>
#include <iomanip>
#include <sstream>


namespace Seiscomp {
namespace Logging {


FileOutput::FileOutput()
 : _stream() {
}

FileOutput::FileOutput(const char* filename)
 : _filename(filename), _stream(filename, std::ios_base::out | std::ios_base::app) {
}

FileOutput::~FileOutput() {
	_stream.close();
}

bool FileOutput::open(const char* filename) {
	_filename = filename;
	_stream.open(filename, std::ios_base::out | std::ios_base::app);
	return _stream.is_open();
}

bool FileOutput::isOpen() {
	return _stream.is_open();
}

void FileOutput::log(const char* channelName,
                     LogLevel level,
                     const char* msg,
                     time_t time) {
	tm currentTime;

	currentTime = _useUTC ? *gmtime(&time) : *localtime(&time);

	_stream << (currentTime.tm_year + 1900) << "/";

	int w = _stream.width();
	char f = _stream.fill();

	_stream << std::setfill('0') << std::setw(2) << (currentTime.tm_mon + 1) << "/"
	        << std::setfill('0') << std::setw(2) << currentTime.tm_mday << " ";

	_stream << std::setfill('0') << std::setw(2) << currentTime.tm_hour << ":"
	        << std::setfill('0') << std::setw(2) << currentTime.tm_min << ":"
	        << std::setfill('0') << std::setw(2) << currentTime.tm_sec << " ";

	_stream.width(w);
	_stream.fill(f);

	_stream << "[" << channelName;
	if ( likely(_logComponent) )
		_stream << "/" << component();
	_stream << "] ";
	if ( unlikely(_logContext) )
		_stream << "(" << fileName() << ':' << lineNum() << ") ";
	_stream << msg << std::endl;
}


}
}
