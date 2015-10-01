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
#include <vector>
#include <list>
#include <iostream>
#include <sstream>

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>

#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/timewindow.h>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/core.h>

namespace Seiscomp {
namespace RecordStream {
namespace Balanced {
namespace _private {

DEFINE_SMARTPOINTER(BalancedConnection);

class SC_SYSTEM_CORE_API BalancedConnection : public Seiscomp::IO::RecordStream {
	DECLARE_SC_CLASS(BalancedConnection)

	public:
		//! C'tor
		BalancedConnection();

		//! Initializing Constructor
		BalancedConnection(std::string serverloc);

		//! Destructor
		virtual ~BalancedConnection();

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
		int streamHash(const std::string &sta);
		void putRecord(Record* rec);
		Record* getRecord();
		void acquiThread(IO::RecordStreamPtr rs);

	private:
		bool _started;
		int _nthreads;
		std::vector<std::pair<IO::RecordStreamPtr, bool> > _rsarray;
		std::list<boost::thread *> _threads;
		std::list<Record*> _buffer;
		std::istringstream _stream;
		boost::mutex _mtx;
		boost::condition _buf_not_full;
		boost::condition _buf_not_empty;
};

} // namesapce Balanced
} // namespace _private
} // namespace RecordStream
} // namespace Seiscomp

#endif

