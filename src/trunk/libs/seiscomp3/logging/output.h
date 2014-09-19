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


#ifndef __SC_LOGGING_OUTPUT_H__
#define __SC_LOGGING_OUTPUT_H__

#include <seiscomp3/logging/node.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace Logging {

/**
 * \brief Logging output class
 *
 * To implement a special kind of logging output,
 * derive from this class and implement the
 * method Output::log(...) to receive logging
 * messages.
 * \code
 * MyOutput log;
 * log.subscribe(GetAll());
 * \endcode
 */
class SC_SYSTEM_CORE_API Output : public Node {
	protected:
		Output();

	public:
		virtual ~Output() {}

	public:
		/** Subscribe to a particular channel */
		bool subscribe(Channel* channel);
		bool unsubscribe(Channel* channel);
		void logComponent(bool e) { _logComponent = e; }
		void logContext(bool e) { _logContext = e; }

	protected:
		/** Callback method for receiving log messages */
		virtual void log(const char* channelName,
		                 LogLevel level,
		                 const char* msg,
		                 time_t time) = 0;

		/** The following methods calls are only valid inside the
		    log(...) method */

		/** Returns the current component name */
		const char* component() const;
		/** Returns the sourcefile of the current log entry */
		const char* fileName() const;
		/** Returns the function name of the current log entry */
		const char* functionName() const;
		/** Returns the line number of the current log entry */
		int lineNum() const;

	private:
		void publish(const Data &data);

	protected:
		bool _logComponent;
		bool _logContext;

	private:
		PublishLoc* _publisher;
};

}
}

#endif
