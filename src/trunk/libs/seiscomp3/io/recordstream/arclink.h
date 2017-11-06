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


#ifndef __SEISCOMP_IO_RECORDSTREAM_ARCLINK_H__
#define __SEISCOMP_IO_RECORDSTREAM_ARCLINK_H__

#include <string>
#include <set>
#include <iostream>
#include <sstream>
#include <fstream>
#include <seiscomp3/core/interruptible.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/utils/timer.h>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/core.h>
#include <seiscomp3/io/socket.h>
#include <seiscomp3/io/recordstream/streamidx.h>

namespace Seiscomp {
namespace RecordStream {
namespace Arclink {
namespace _private {

class SC_SYSTEM_CORE_API ArclinkException: public Seiscomp::IO::RecordStreamException {
	public:
		ArclinkException(): RecordStreamException("ArcLink exception") {}
		ArclinkException(const std::string& what): RecordStreamException(what) {}
};

class SC_SYSTEM_CORE_API ArclinkCommandException: public ArclinkException {
	public:
		ArclinkCommandException(): ArclinkException("command not accepted") {}
		ArclinkCommandException(const std::string& what): ArclinkException(what) {}
};


DEFINE_SMARTPOINTER(ArclinkConnection);

class SC_SYSTEM_CORE_API ArclinkConnection : public Seiscomp::IO::RecordStream {
	DECLARE_SC_CLASS(ArclinkConnection);

	public:
		//! C'tor
		ArclinkConnection();
		
		//! Initializing Constructor
		ArclinkConnection(std::string serverloc);

		//! Destructor
		virtual ~ArclinkConnection();

	public:
		//! The recordtype cannot be selected when using an arclink
		//! connection. It will always create MiniSeed records
		virtual bool setRecordType(const char*);

		//! Initialize the arclink connection.
		virtual bool setSource(const std::string &serverloc);
		
		//! Supply user credentials
		bool setUser(std::string name, std::string password);
		
		//! Adds the given stream to the server connection description
		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode);

		//! Adds the given stream to the server connection description
		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode,
		                       const Seiscomp::Core::Time &stime,
		                       const Seiscomp::Core::Time &etime);
  
		//! Adds the given start time to the server connection description
		virtual bool setStartTime(const Seiscomp::Core::Time &stime);
		
		//! Adds the given end time to the server connection description
		virtual bool setEndTime(const Seiscomp::Core::Time &etime);

		//! Sets timeout
		virtual bool setTimeout(int seconds);

		//! Terminates the arclink connection.
		virtual void close();

		virtual Record *next();

		//! Removes all stream list, time window, etc. -entries from the connection description object.
		bool clear();

		//! Reconnects a terminated arclink connection.
		bool reconnect();


	private:
		Seiscomp::IO::Socket _sock;
		std::string _serverloc;
		std::string _user;
		std::string _passwd;
		std::list<StreamIdx> _ordered;
		std::set<StreamIdx> _streams;
		Seiscomp::Core::Time _stime;
		Seiscomp::Core::Time _etime;
		std::string _reqID;
		bool _readingData;
		bool _chunkMode;
		int _remainingBytes;
		std::ofstream _dump;

		void handshake();
		void cleanup();
};

} // namespace _private

//using _private::ArclinkException;
//using _private::ArclinkCommandException;
using _private::ArclinkConnection;
using _private::ArclinkConnectionPtr;

} // namespace Arclink
} // namespace RecordStream
} // namespace Seiscomp

#endif

