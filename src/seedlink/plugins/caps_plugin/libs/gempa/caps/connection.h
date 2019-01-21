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


#ifndef __GEMPA_CAPS_CONNECTION_H__
#define __GEMPA_CAPS_CONNECTION_H__

#include <gempa/caps/datetime.h>
#include <gempa/caps/sessiontable.h>
#include <gempa/caps/socket.h>

#include <boost/shared_ptr.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

//#include <iostream>
#include <map>

namespace Gempa {
namespace CAPS {

class SessionTableItem;
class Time;

class Connection {
	public:
		//! ConnectionStates:
		//! EOD     -> connection not yet established or all streams finished (or aborted)
		//! Active  -> connection was successfully established, data request was sent
		//! Error   -> connection aborted due to server error
		//! Aborted -> connection aborted by user
		enum State { EOD, Active, Error, Aborted };

		//! Constructor
		Connection();

		//! Destructor
		virtual ~Connection();

		//! Sets server parameter
		bool setServer(const std::string &server);

		//! Enables SSL feature
		void enableSSL(bool enable) { _ssl = enable; }

		//! Sets user name and password
		void setCredentials(const std::string &user, const std::string &password);

		//! Sets meta mode. If enabled only the packet header information is
		//! transmitted.
		void setMetaMode(bool enable) { _metaMode = enable; }

		//! Sets realtime mode.
		void setRealtime(bool enable) { _realtime = enable; }

		//! Disconnect from server, connection unuseable until reset() is
		//! called. Thread safe.
		void close();

		//! Send abort command to server, keep socket open, requires reset.
		//! Thread safe.
		void abort();

		//! Adds a stream request
		bool addStream(const std::string &net, const std::string &sta,
		               const std::string &loc, const std::string &cha);

		//! Adds a seismic stream requests
		bool addStream(const std::string &net, const std::string &sta,
		               const std::string &loc, const std::string &cha,
		               const Time &stime,
		               const Time &etime);

		//! Resets connection state to eof
		void reset(bool clearStreams = false);

		//! Sets the given start time
		void setStartTime(const Time &stime);

		//! Sets the given end time
		void setEndTime(const Time &etime);

		//! Sets the given time window
		void setTimeWindow(const Time &stime, const Time &etime);

		//! Sets timeout
		bool setTimeout(int seconds);

		//! Returns the next record. If the record is NULL the caller has
		//! to check the state of the connection. Automatically creates a new
		//! connection if required, sends data requests and evaluates updates
		//! of session table. If the connection state is neither 'good' nor
		//!'eof' the connection has to be resetted before invoking this method.
		DataRecord* next();


		State state() { return _state; }


	// ----------------------------------------------------------------------
	// protected methods
	// ----------------------------------------------------------------------
	protected:
		virtual Socket* createSocket() const;

	private:
		typedef boost::shared_ptr<SessionTable> SessionTablePtr;

		struct Request {
			std::string      net;
			std::string      sta;
			std::string      loc;
			std::string      cha;
			Time             start;
			Time             end;
			bool             receivedData;
		};
		typedef std::map<std::string, Request> RequestList;

	// ----------------------------------------------------------------------
	// private methods
	// ----------------------------------------------------------------------
	private:
		bool addRequest(const std::string &net, const std::string &sta,
		                const std::string &loc, const std::string &cha,
		                const Time &stime,
		                const Time &etime,
		                bool receivedData);
		void onItemAboutToBeRemoved(const SessionTableItem *item);

		void disconnect();
		bool handshake();
		bool sendRequest(const std::string &req);

		bool seekToReadLimit(bool log = true);
		void formatRequest(std::stringstream& req, RequestList::const_iterator it);


	// ----------------------------------------------------------------------
	// private data members
	// ----------------------------------------------------------------------
	protected:
		SessionTablePtr                     _sessionTable;
		RequestList                         _requests;
		SocketPtr                           _socket;
		socketbuf<Socket, 512>              _socketBuf;
		char                                _lineBuf[201];
		volatile State                      _state;
		SessionTableItem                   *_currentItem;
		uint16_t                            _currentID;
		boost::mutex                        _mutex;

		std::string                         _server;
		int                                 _port;
		std::string                         _auth;
		Time                                _startTime;
		Time                                _endTime;
		bool                                _realtime;
		bool                                _metaMode;
		bool                                _ssl;
};

typedef boost::shared_ptr<Connection> ConnectionPtr;


}
}

#endif
