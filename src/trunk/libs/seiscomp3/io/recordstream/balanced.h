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

#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/timewindow.h>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/core.h>
#include <seiscomp3/client/queue.h>

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

	public:
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

		//! Adds the given end time window to the server connection description
		virtual bool setTimeWindow(const Core::TimeWindow &w);

		//! Sets timeout
		virtual bool setTimeout(int seconds);

		//! Terminates the combined connection.
		virtual void close();

		virtual Record *next();


	private:
		int streamHash(const std::string &sta);
		void putRecord(RecordPtr rec);
		Record* getRecord();
		void acquiThread(IO::RecordStreamPtr rs);

	private:
		bool _started;
		int _nthreads;
		std::vector<std::pair<IO::RecordStreamPtr, bool> > _rsarray;
		std::list<boost::thread *> _threads;
		Client::ThreadedQueue<Record*> _queue;
		std::istringstream _stream;
		boost::mutex _mtx;
};

} // namesapce Balanced
} // namespace _private
} // namespace RecordStream
} // namespace Seiscomp

#endif

