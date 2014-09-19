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


#ifndef __SC_LOGGING_SYSLOG_H__
#define __SC_LOGGING_SYSLOG_H__

#ifndef WIN32

#include <seiscomp3/logging/output.h>


namespace Seiscomp {
namespace Logging {


class SC_SYSTEM_CORE_API SyslogOutput : public Output {
	public:
		SyslogOutput();
		SyslogOutput(const char *ident, const char *facility = NULL);
		~SyslogOutput();

		int facility() const { return _facility; }

		bool open(const char *ident, const char *facility = NULL);
		bool isOpen() const;
		void close();

	protected:
		/** Callback method for receiving log messages */
		void log(const char* channelName,
		         LogLevel level,
		         const char* msg,
		         time_t time);

	private:
		bool _openFlag;
		int  _facility;
};


}
}

#endif

#endif
