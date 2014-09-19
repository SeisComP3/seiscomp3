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
#include <seiscomp3/logging/filerotator.h>

#include <cstdarg>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdio.h>

#ifndef WIN32
#include <sys/stat.h>
#endif

namespace Seiscomp {
namespace Logging {


FileRotatorOutput::FileRotatorOutput(int timeSpan, int historySize)
 : _timeSpan(timeSpan), _historySize(historySize), _lastInterval(-1) {
}

FileRotatorOutput::FileRotatorOutput(const char* filename, int timeSpan, int historySize)
 : FileOutput(filename), _timeSpan(timeSpan), _historySize(historySize), _lastInterval(-1) {
}

bool FileRotatorOutput::open(const char* filename) {
	if ( !FileOutput::open(filename) ) return false;

#ifndef WIN32
	struct stat st;
	if ( stat(filename, &st) == 0 )
		_lastInterval = st.st_mtime / _timeSpan;

#endif
	return true;
}

void FileRotatorOutput::log(const char* channelName,
                            LogLevel level,
                            const char* msg,
                            time_t time) {
	int currentInterval = (int)(time / (time_t)_timeSpan);

	if ( _lastInterval == -1 )
		_lastInterval = currentInterval;

	if ( _lastInterval != currentInterval ) {
		rotateLogs();
		_lastInterval = currentInterval;
	}

	FileOutput::log(channelName, level, msg, time);
}

void FileRotatorOutput::removeLog(int index) {
	std::stringstream ss;
	ss << _filename << "." << index;
	unlink(ss.str().c_str());
}

void FileRotatorOutput::renameLog(int oldIndex, int newIndex) {
	std::stringstream oldFile, newFile;
	oldFile << _filename;
	if ( oldIndex > 0 )
		oldFile << "." << oldIndex;

	newFile << _filename;
	if ( newFile > 0 )
		newFile << "." << newIndex;

	rename(oldFile.str().c_str(), newFile.str().c_str());
}

void FileRotatorOutput::rotateLogs() {
	// Close current stream
	if ( _stream.is_open() )
		_stream.close();

	// Rotate the log files
	removeLog(_historySize);

	for ( int i = _historySize-1; i >= 0; --i )
		renameLog(i, i+1);

	// Open the new stream
	int tmp(_lastInterval);
	open(_filename.c_str());
	_lastInterval = tmp;
}


}
}
