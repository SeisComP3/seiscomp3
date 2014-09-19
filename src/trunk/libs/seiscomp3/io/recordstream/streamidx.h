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


#ifndef __SEISCOMP_IO_RECORDSTREAM_STREAMIDX_H__
#define __SEISCOMP_IO_RECORDSTREAM_STREAMIDX_H__

#include <string>
#include <seiscomp3/core/datetime.h>

namespace Seiscomp {
namespace RecordStream {

class StreamIdx  {
	public:
		StreamIdx();

		StreamIdx(const std::string& net, const std::string& sta,
		          const std::string& loc, const std::string& cha);

		StreamIdx(const std::string& net, const std::string& sta,
		          const std::string& loc, const std::string& cha,
		          const Seiscomp::Core::Time& stime,
		          const Seiscomp::Core::Time& etime);

		StreamIdx& operator=(const StreamIdx &other);

		bool operator<(const StreamIdx &other) const;

		bool operator!=(const StreamIdx &other) const;

		bool operator==(const StreamIdx &other) const;

		bool operator>=(const StreamIdx &other) const;

		bool operator>(const StreamIdx &other) const;

		bool operator<=(const StreamIdx &other) const;

		//! Returns the network code
		const std::string &network() const;

		//! Returns the station code
		const std::string &station() const;

		//! Returns the channel code
		const std::string &channel() const;

		//! Returns the location code
		const std::string &location() const;

		//! Returns the selector in <location><channel>.D notation
		//! * wildcards are substituted by a corresponding number of ?
		std::string selector() const;

		//! Returns the start time
		Core::Time startTime() const;

		//! Returns the end time
		Core::Time endTime() const;

		//! Returns a string: <sTime> <eTime> <network> <station> <channel> <location>
		//! <*Time> in format: %Y,%m,%d,%H,%M,%S
		std::string str(const Seiscomp::Core::Time& stime,
		                const Seiscomp::Core::Time& etime) const;

		//! Returns the most recent record end time
		Seiscomp::Core::Time timestamp() const;

		//! Sets the time stamp
		void setTimestamp(Seiscomp::Core::Time &rectime) const;

	private:
		const std::string _net;
		const std::string _sta;
		const std::string _loc;
		const std::string _cha;
		const Seiscomp::Core::Time _stime;
		const Seiscomp::Core::Time _etime;
		mutable Seiscomp::Core::Time _timestamp;
};

} // namespace RecordStream
} // namespace Seiscomp

#endif
