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


#ifndef __SEISCOMP_SERVICES_RECORDSTREAM_COMBINED_H__
#define __SEISCOMP_SERVICES_RECORDSTREAM_COMBINED_H__

#include <string>
#include <iostream>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/timewindow.h>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/core.h>

namespace Seiscomp {
namespace RecordStream {
namespace Combined {
namespace _private {

DEFINE_SMARTPOINTER(CombinedConnection);

class SC_SYSTEM_CORE_API CombinedConnection : public Seiscomp::IO::RecordStream {
	DECLARE_SC_CLASS(CombinedConnection)

	public:
		//! C'tor
		CombinedConnection();

		//! Initializing Constructor
		CombinedConnection(std::string serverloc);

		//! Destructor
		virtual ~CombinedConnection();

		virtual bool setRecordType(const char*);

		//! Initialize the combined connection.
		virtual bool setSource(const std::string &serverloc);

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
		virtual bool setStartTime(const Core::Time &stime);

		//! Adds the given end time to the server connection description
		virtual bool setEndTime(const Core::Time &etime);

		//! Sets timeout
		virtual bool setTimeout(int seconds);

		//! Terminates the combined connection.
		virtual void close();

		//! Returns the data stream
		virtual Record *next();

	private:
		void init();

	private:
		bool                _started;

		size_t              _nStream;
		size_t              _nArchive;
		size_t              _nRealtime;

		Core::Time          _startTime;
		Core::Time          _endTime;
		Core::Time          _curtime;
		Core::Time          _archiveEndTime;
		Core::TimeSpan      _realtimeAvailability;

		std::set<StreamIdx> _tmpStreams;
		IO::RecordStreamPtr _realtime;
		IO::RecordStreamPtr _archive;
};

} // namesapce Combined
} // namespace _private
} // namespace RecordStream
} // namespace Seiscomp

#endif

