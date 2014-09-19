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


#ifndef __SC_LOGGING_CHANNEL_H__
#define __SC_LOGGING_CHANNEL_H__

#include <string>
#include <map>

#include <seiscomp3/logging/log.h>
#include <seiscomp3/logging/node.h>

namespace Seiscomp {
namespace Logging {


class SC_SYSTEM_CORE_API Channel;

typedef std::map<std::string, Channel*> ComponentMap;

// documentation in implementation file
class SC_SYSTEM_CORE_API Channel : public Node {
	public:
		Channel(const std::string &name, LogLevel level);
		virtual ~Channel();

		virtual void publish(const Data &data);
		const std::string &name() const;

		LogLevel logLevel() const;
		void setLogLevel(LogLevel level);

	protected:
		Channel *getComponent(Channel *componentParent,
		                      const char *component);

	private:
		//! the full channel name
		std::string _name;
		LogLevel _level;

		ComponentMap subChannels;
		ComponentMap components;

		Channel(const Channel &);
		Channel &operator=(const Channel&);

	friend Channel *Seiscomp::Logging::getComponentChannel(
		const char *component,
		const char *path,
		LogLevel level);
};

}
}

#endif
