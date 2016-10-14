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


#ifndef __SC_LOGGING_FILEROTATOR_H__
#define __SC_LOGGING_FILEROTATOR_H__

#include <seiscomp3/logging/file.h>


namespace Seiscomp {
namespace Logging {


class SC_SYSTEM_CORE_API FileRotatorOutput : public FileOutput {
	public:
		FileRotatorOutput(int timeSpan = 60*60*24, int historySize = 7,
		                  int maxFileSize = 100*1024*1024);
		/**
		 * Creates a new FileRotatorOutput instance
		 * @param filename The filename to log to
		 * @param timeSpan The timespan in seconds for the log time before rotating
		 * @param count The number of historic files to store
		 */
		FileRotatorOutput(const char* filename, int timeSpan = 60*60*24,
		                  int historySize = 7, int maxFileSize = 100*1024*1024);

		bool open(const char* filename);

	protected:
		/** Callback method for receiving log messages */
		void log(const char* channelName,
		         LogLevel level,
		         const char* msg,
		         time_t time);

	private:
		void rotateLogs();
		void removeLog(int index);
		void renameLog(int oldIndex, int newIndex);


	protected:
		//! time span to keep one log
		int _timeSpan;

		//! number of log files to keep
		int _historySize;

		//! maximum file size of a log file
		int _maxFileSize;

		//! last log file written to
		int _lastInterval;

		boost::mutex outputMutex;
};


}
}

#endif
