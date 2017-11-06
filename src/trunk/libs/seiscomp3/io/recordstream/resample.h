/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *   Author: Jan Becker, gempa GmbH                                        *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_RECORDSTREAM_RESAMPLE_H__
#define __SEISCOMP_RECORDSTREAM_RESAMPLE_H__

#include <sstream>
#include <map>
#include <deque>

#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/io/recordfilter/demux.h>


namespace Seiscomp {
namespace RecordStream {


DEFINE_SMARTPOINTER(Resample);

class SC_SYSTEM_CORE_API Resample : public Seiscomp::IO::RecordStream {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		Resample();
		virtual ~Resample();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		virtual bool setSource(const std::string &source);
		virtual bool setRecordType(const char *type);

		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode);

		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode,
		                       const Seiscomp::Core::Time &startTime,
		                       const Seiscomp::Core::Time &endTime);

		virtual bool setStartTime(const Seiscomp::Core::Time &stime);
		virtual bool setEndTime(const Seiscomp::Core::Time &etime);
		virtual bool setTimeWindow(const Seiscomp::Core::TimeWindow &w);

		virtual bool setTimeout(int seconds);

		virtual void close();

		virtual Record *next();


	// ----------------------------------------------------------------------
	//  Private Interface
	// ----------------------------------------------------------------------
	private:
		void push(Record *rec);
		void cleanup();


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		typedef std::deque<GenericRecord*> OutputQueue;

		IO::RecordStreamPtr   _source;
		bool                  _debug;
		IO::RecordDemuxFilter _demuxer;
		OutputQueue           _queue;

		double                _targetRate;
		double                _fp;
		double                _fs;
		int                   _lanzcosKernelWidth;
		int                   _coeffScale;
};

}
}

#endif
