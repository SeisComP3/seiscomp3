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


#ifndef __SEISCOMP_IO_RECORDSTREAM_WS_H__
#define __SEISCOMP_IO_RECORDSTREAM_WS_H__

#include <string>
#include <set>
#include <sstream>

#include <seiscomp3/core.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/io/socket.h>
#include <seiscomp3/io/httpsocket.h>
#include <seiscomp3/io/recordstream/streamidx.h>

#include "bson.h"

namespace Seiscomp {
namespace RecordStream {


class SC_SYSTEM_CORE_API HMBQueue  {
	public:
		//! C'tor
		HMBQueue();

		//! Destructor
		virtual ~HMBQueue();

		//! Adds the given stream
		void addStream(std::string loc, std::string cha,
			const Seiscomp::Core::Time &stime, const Seiscomp::Core::Time &etime);

		//! Sets the sequence number
		void setSequenceNumber(int64_t seq);

		//! Removes all entries
		void clear();

		//! Returns a BSON document
		bson_t* toBSON() const;

	private:
		Core::Time _stime;
		Core::Time _etime;
		int64_t _seq;
		std::set<std::string> _topics;
};


template<typename SocketType>
class SC_SYSTEM_CORE_API HMBConnection : public Seiscomp::IO::RecordStream {
	//DECLARE_SC_CLASS(HMBConnection<SocketType>);

	public:
		//! C'tor
		HMBConnection();

		//! Initializing Constructor
		HMBConnection(std::string serverloc);

		//! Destructor
		virtual ~HMBConnection();

		//! The recordtype cannot be selected when using an HMB
		//! connection. It will always create MiniSeed records
		bool setRecordType(const char*);

		//! Initialize the HMB connection.
		bool setSource(std::string serverloc);

		//! Supply user credentials
		//! Adds the given stream to the server connection description
		bool addStream(std::string net, std::string sta, std::string loc, std::string cha);

		//! Adds the given stream to the server connection description
		bool addStream(std::string net, std::string sta, std::string loc, std::string cha,
			const Seiscomp::Core::Time &stime, const Seiscomp::Core::Time &etime);

		//! Removes the given stream from the connection description. Returns true on success; false otherwise.
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

		//! Terminates the HMB connection.
		void close();

		//! Reconnects a terminated HMB connection.
		bool reconnect();

		//! Returns the data stream
		std::istream& stream();

	private:
		std::istringstream _stream;
		IO::HttpSocket<SocketType> _sock;
		std::string _serverHost;
		std::string _serverPath;
		std::string _user;
		std::string _password;
		std::set<Seiscomp::RecordStream::StreamIdx> _streams;
		Seiscomp::Core::Time _stime;
		Seiscomp::Core::Time _etime;
		std::map<std::string, HMBQueue> _queues;
		std::string _sid;
		std::string _cid;
		bool _readingData;

		std::string bsonGetString(const bson_t *bson, const char *key);
		int64_t bsonGetInt(const bson_t *bson, const char *key);
		void bsonGetBlob(const bson_t *bson, const char *key, const void **data, int *data_len);
		void initSession();
		std::string receive();
};

}
}

#endif

