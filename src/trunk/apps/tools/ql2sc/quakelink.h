/***************************************************************************
 *   Copyright (C) gempa GmbH                                              *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Author: Stephan Herrnkind                                             *
 *   Email : herrnkind@gempa.de                                            *
 ***************************************************************************/


#ifndef __SEISCOMP_QL2SC_QUAKELINK_H__
#define __SEISCOMP_QL2SC_QUAKELINK_H__


#include "config.h"

#include <seiscomp3/core/datetime.h>
#include <seiscomp3/io/quakelink/connection.h>

#include <boost/thread.hpp>

#include <string>
#include <vector>


namespace Seiscomp {
namespace QL2SC {


class QLClient : public IO::QuakeLink::Connection {

	public:
		QLClient(int notificationID, const HostConfig *config, size_t backLog = 0);
		virtual ~QLClient();

		void run();
		void join(const Seiscomp::Core::Time &until);

		const HostConfig* config() const { return _config; }

		Seiscomp::Core::Time lastUpdate() const;
		void setLastUpdate(const Seiscomp::Core::Time &time);


	protected:
		void processResponse(IO::QuakeLink::Response *response);

	private:
		void listen();

	private:
		struct Statistics {
			Statistics() : messages(0), payloadBytes(0) {};
			size_t messages;
			size_t payloadBytes;
		};

		int                         _notificationID;
		const HostConfig           *_config;
		size_t                      _backLog;
		boost::thread              *_thread;

		Seiscomp::Core::Time        _lastUpdate;
		mutable boost::mutex        _mutex;
		Statistics                  _stats;
};


} // ns QL2SC
} // ns Seiscomp


#endif // __SEISCOMP_QL2SC_QUAKELINK_H__
