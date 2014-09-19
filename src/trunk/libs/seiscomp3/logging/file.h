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


#ifndef __SC_LOGGING_FILE_H__
#define __SC_LOGGING_FILE_H__

#include <seiscomp3/logging/output.h>
#include <fstream>


namespace Seiscomp {
namespace Logging {


class SC_SYSTEM_CORE_API FileOutput : public Output {
	public:
		FileOutput();
		FileOutput(const char* filename);
		~FileOutput();

		virtual bool open(const char* filename);
		bool isOpen();

	protected:
		/** Callback method for receiving log messages */
		void log(const char* channelName,
		         LogLevel level,
		         const char* msg,
		         time_t time);

	protected:
		std::string _filename;
		mutable std::ofstream _stream;
};


}
}

#endif
