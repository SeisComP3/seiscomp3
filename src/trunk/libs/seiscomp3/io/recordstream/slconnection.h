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


#ifndef __SEISCOMP_IO_RECORDSTREAM_SLINK_H__
#define __SEISCOMP_IO_RECORDSTREAM_SLINK_H__

#include <string>
#include <set>
#include <iostream>
#include <sstream>
#include <signal.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/core.h>
#include <seiscomp3/io/socket.h>
#include <seiscomp3/io/recordstream/streamidx.h>


namespace Seiscomp {
namespace RecordStream {


class SC_SYSTEM_CORE_API SeedlinkException: public Seiscomp::IO::RecordStreamException {
	public:
		SeedlinkException(): RecordStreamException("Seedlink exception") {}
		SeedlinkException(const std::string& what): RecordStreamException(what) {}
};

class SC_SYSTEM_CORE_API SeedlinkCommandException: public SeedlinkException {
	public:
		SeedlinkCommandException(): SeedlinkException("command not accepted") {}
		SeedlinkCommandException(const std::string& what): SeedlinkException(what) {}
};

DEFINE_SMARTPOINTER(SLConnection);

class SC_SYSTEM_CORE_API SLConnection : public Seiscomp::IO::RecordStream {
	DECLARE_SC_CLASS(SLConnection);

	public:
		//! C'tor
		SLConnection();

		//! Initializing Constructor
		SLConnection(std::string serverloc);

		//! Destructor
		virtual ~SLConnection();

		//! The recordtype cannot be selected when using a seedlink
		//! connection. It will always create MiniSeed records
		bool setRecordType(const char*);

		//! Initialize the seedlink connection.
		bool setSource(std::string serverloc);

		//! Adds the given stream to the server connection description
		bool addStream(std::string net, std::string sta, std::string loc, std::string cha);

		//! Adds a seismic stream request to the record stream (not implemented)
		bool addStream(std::string net, std::string sta, std::string loc, std::string cha,
		               const Core::Time &stime, const Core::Time &etime);

		//! Removes the given stream from the connection description.
		//! Returns true on success; false otherwise.
		bool removeStream(std::string net, std::string sta, std::string loc, std::string cha);

		//! Adds the given start time to the server connection description
		bool setStartTime(const Seiscomp::Core::Time &stime);

		//! Adds the given end time to the server connection description
		bool setEndTime(const Seiscomp::Core::Time &etime);

		//! Adds the given end time window to the server connection description
		bool setTimeWindow(const Seiscomp::Core::TimeWindow &w);

		//! Sets timeout
		bool setTimeout(int seconds);

		//! Removes all stream list, time window, etc. -entries from the connection description object.
		bool clear();

		//! Disconnects and terminates (!) the seedlink connection.
		void close();

		//! Reconnects a terminated seedlink connection.
		bool reconnect();

		//! Returns the data stream
		std::istream& stream();


	private:
		class StreamBuffer : public std::streambuf {
			public:
				StreamBuffer();
				std::streambuf *setbuf(char *s, std::streamsize n);
			};

			StreamBuffer        _streambuf;
			std::istream        _stream;
			std::string         _serverloc;
			std::string         _slrecord;
			IO::Socket          _sock;
			std::set<StreamIdx> _streams;
			Core::Time          _stime;
			Core::Time          _etime;
			bool                _readingData;
			int                 _maxRetries;
			int                 _retriesLeft;

			void handshake();
};


}
}

#endif 
