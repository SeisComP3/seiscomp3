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


#ifndef __SEISCOMP_SERVICES_RECORDSTREAM_RESAMPLE_H__
#define __SEISCOMP_SERVICES_RECORDSTREAM_RESAMPLE_H__

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
		bool setSource(std::string);
		bool setRecordType(const char*);

		//! The following five methods are not implemented yet
		//! and return always 'false'.
		bool addStream(std::string net, std::string sta, std::string loc, std::string cha);
		bool addStream(std::string net, std::string sta, std::string loc, std::string cha,
			const Seiscomp::Core::Time &stime, const Seiscomp::Core::Time &etime);
		bool setStartTime(const Seiscomp::Core::Time &stime);
		bool setEndTime(const Seiscomp::Core::Time &etime);
		bool setTimeWindow(const Seiscomp::Core::TimeWindow &w);
		bool setTimeout(int seconds);

		Record* createRecord(Array::DataType, Record::Hint);
		void recordStored(Record*);

		void close();

		std::istream& stream();


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
		std::stringstream     _stream;
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
