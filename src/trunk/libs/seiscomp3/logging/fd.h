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


#ifndef __SC_LOGGING_FD_H__
#define __SC_LOGGING_FD_H__

#include <seiscomp3/logging/output.h>


namespace Seiscomp {
namespace Logging {


class SC_SYSTEM_CORE_API FdOutput : public Output {
	public:
		FdOutput(int fdOut = 2);
		~FdOutput();

	protected:
		/** Callback method for receiving log messages */
		void log(const char* channelName,
		         LogLevel level,
		         const char* msg,
		         time_t time);

	private:
		bool _colorize;
		int _fdOut;
};


}
}

#endif
