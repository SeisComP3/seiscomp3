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
		bool setSource(std::string serverloc);

		//! Adds the given stream to the server connection description
		bool addStream(std::string net, std::string sta, std::string loc, std::string cha);

		//! Adds the given stream to the server connection description
		bool addStream(std::string net, std::string sta, std::string loc, std::string cha,
		               const Core::Time &stime, const Core::Time &etime);

		//! Adds the given start time to the server connection description
		bool setStartTime(const Core::Time &stime);

		//! Adds the given end time to the server connection description
		bool setEndTime(const Core::Time &etime);

		//! Adds the given end time window to the server connection description
		bool setTimeWindow(const Core::TimeWindow &w);

		//! Sets timeout
		bool setTimeout(int seconds);

		//! Terminates the combined connection.
		void close();

		//! Returns the data stream
		std::istream& stream();

		Record* createRecord(Array::DataType, Record::Hint);

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

