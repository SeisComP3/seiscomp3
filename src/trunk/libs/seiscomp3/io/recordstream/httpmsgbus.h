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
		virtual bool setRecordType(const char *type);

		//! Initialize the HMB connection.
		virtual bool setSource(const std::string &source);

		//! Supply user credentials
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
		                       const Seiscomp::Core::Time &startTime,
		                       const Seiscomp::Core::Time &endTime);

		//! Adds the given start time to the server connection description
		virtual bool setStartTime(const Seiscomp::Core::Time &stime);

		//! Adds the given end time to the server connection description
		virtual bool setEndTime(const Seiscomp::Core::Time &etime);

		//! Sets timeout
		virtual bool setTimeout(int seconds);

		//! Terminates the HMB connection.
		virtual void close();

		virtual Record *next();

		//! Removes all stream list, time window, etc. -entries from the connection description object.
		bool clear();

		//! Reconnects a terminated HMB connection.
		bool reconnect();


	private:
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

