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


#ifndef __SEISCOMP_PROCESSING_STREAMBUFFER_H__
#define __SEISCOMP_PROCESSING_STREAMBUFFER_H__

#include<string>
#include<list>

#include <seiscomp3/core/recordsequence.h>
#include <seiscomp3/client.h>


namespace Seiscomp {

namespace Processing {




class SC_SYSTEM_CLIENT_API StreamBuffer {
	// ----------------------------------------------------------------------
	//  Public Types
	// ----------------------------------------------------------------------
	public:
		struct SC_SYSTEM_CLIENT_API WaveformID {
			WaveformID(const std::string& net,
			           const std::string& sta,
			           const std::string& loc,
			           const std::string& cha)
				: networkCode(net), stationCode(sta),
				  locationCode(loc), channelCode(cha) {}
		
			WaveformID(const Record *rec)
				: networkCode(rec->networkCode()), stationCode(rec->stationCode()),
				  locationCode(rec->locationCode()), channelCode(rec->channelCode()) {}


			bool operator<(const WaveformID& other) const {

				if ( networkCode < other.networkCode ) return true;
				if ( networkCode > other.networkCode ) return false;
		
				if ( stationCode < other.stationCode ) return true;
				if ( stationCode > other.stationCode ) return false;
		
				if ( locationCode < other.locationCode ) return true;
				if ( locationCode > other.locationCode ) return false;
		
				return channelCode < other.channelCode;
			}
		
		
			std::string networkCode;
			std::string stationCode;
			std::string locationCode;
			std::string channelCode;
		};


	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! Creates streambuffer with no type yet
		//! use setTimeWindow or setTimeSpan before using it
		StreamBuffer();

		//! Creates a timewindow buffer
		StreamBuffer(const Seiscomp::Core::TimeWindow& timeWindow);

		//! Creates a ringbuffer
		StreamBuffer(const Seiscomp::Core::TimeSpan& timeSpan);

		~StreamBuffer();


	// ----------------------------------------------------------------------
	//  Interface
	// ----------------------------------------------------------------------
	public:
		void setTimeWindow(const Seiscomp::Core::TimeWindow& timeWindow);
		void setTimeSpan(const Seiscomp::Core::TimeSpan& timeSpan);

		RecordSequence* sequence(const WaveformID& wid) const;
		RecordSequence* feed(const Record *rec);

		bool addedNewStream() const;

		void printStreams(std::ostream& os=std::cout) const;
		std::list<std::string> getStreams() const;

		//! Clears the streambuffer and removes all cached records
		void clear();


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	private:
		enum Mode {
			TIME_WINDOW,
			RING_BUFFER
		};

		Mode _mode;

		Seiscomp::Core::Time _timeStart;
		Seiscomp::Core::TimeSpan _timeSpan;

		typedef std::map<WaveformID, RecordSequence*> SequenceMap;
		SequenceMap _sequences;

		bool _newStreamAdded;
};

}

}

#endif
