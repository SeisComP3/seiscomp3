/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_IO_RECORDSTREAM_WS_H__
#define __SEISCOMP_IO_RECORDSTREAM_WS_H__

#include <string>
#include <set>
#include <iostream>
#include <sstream>
#include <seiscomp3/core/interruptible.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/utils/timer.h>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/core.h>
#include <seiscomp3/io/socket.h>
#include <seiscomp3/io/recordstream/streamidx.h>

namespace Seiscomp {
namespace RecordStream {


class SC_SYSTEM_CORE_API FDSNWSConnectionBase : public IO::RecordStream {
	protected:
		//! C'tor
		FDSNWSConnectionBase(IO::Socket *socket, int defaultPort);


	public:
		//! The recordtype cannot be selected when using an arclink
		//! connection. It will always create MiniSeed records
		bool setRecordType(const char*);

		//! Initialize the arclink connection.
		bool setSource(std::string serverloc);

		//! Supply user credentials
		//! Adds the given stream to the server connection description
		bool addStream(std::string net, std::string sta, std::string loc, std::string cha);

		//! Adds the given stream to the server connection description
		bool addStream(std::string net, std::string sta, std::string loc, std::string cha,
			const Core::Time &stime, const Core::Time &etime);

		//! Removes the given stream from the connection description. Returns true on success; false otherwise.
		bool removeStream(std::string net, std::string sta, std::string loc, std::string cha);

		//! Adds the given start time to the server connection description
		bool setStartTime(const Core::Time &stime);

		//! Adds the given end time to the server connection description
		bool setEndTime(const Core::Time &etime);

		//! Adds the given end time window to the server connection description
		bool setTimeWindow(const Core::TimeWindow &w);

		//! Sets timeout
		bool setTimeout(int seconds);

		//! Removes all stream list, time window, etc. -entries from the connection description object.
		bool clear();

		//! Terminates the arclink connection.
		void close();

		//! Reconnects a terminated arclink connection.
		bool reconnect();

		//! Returns the data stream
		std::istream& stream();


	private:
		//! Blocking read from socket
		std::string readBinary(int size);
		void handshake();


	private:
		std::istringstream _stream;
		IO::SocketPtr _sock;
		std::string _host;
		std::string _url;
		int _defaultPort;
		std::set<Seiscomp::RecordStream::StreamIdx> _streams;
		Core::Time _stime;
		Core::Time _etime;
		std::string _reqID;
		bool _readingData;
		bool _chunkMode;
		int _remainingBytes;
		std::string _error;
};


class SC_SYSTEM_CORE_API FDSNWSConnection : public FDSNWSConnectionBase {
	public:
		FDSNWSConnection();
};


class SC_SYSTEM_CORE_API FDSNWSSSLConnection : public FDSNWSConnectionBase {
	public:
		FDSNWSSSLConnection();
};



}
}

#endif

