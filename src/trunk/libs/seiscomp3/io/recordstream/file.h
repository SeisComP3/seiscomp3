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


#ifndef __SEISCOMP_SERVICES_RECORDSTREAM_RECORDFILE_H__
#define __SEISCOMP_SERVICES_RECORDSTREAM_RECORDFILE_H__

#include <string>
#include <iostream>
#include <fstream>
#include <map>

#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/core.h>


namespace Seiscomp {
namespace RecordStream {


DEFINE_SMARTPOINTER(File);


class SC_SYSTEM_CORE_API File : public Seiscomp::IO::RecordStream {
	public:
		enum SeekDir {
			Begin = std::ios_base::beg,
			Current = std::ios_base::cur,
			End = std::ios_base::end
		};

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		File();
		File(std::string name);
		File(const File &f);
		virtual ~File();


	// ----------------------------------------------------------------------
	//  Operators
	// ----------------------------------------------------------------------
	public:
		File& operator=(const File &f);


	// ----------------------------------------------------------------------
	//  Public RecordStream interface
	// ----------------------------------------------------------------------
	public:
		bool setSource(std::string);

		//! The following five methods are not implemented yet
		//! and return always 'false'.
		bool addStream(std::string net, std::string sta, std::string loc, std::string cha);
		bool addStream(std::string net, std::string sta, std::string loc, std::string cha,
			const Seiscomp::Core::Time &stime, const Seiscomp::Core::Time &etime);
		bool setStartTime(const Seiscomp::Core::Time &stime);
		bool setEndTime(const Seiscomp::Core::Time &etime);
		bool setTimeWindow(const Seiscomp::Core::TimeWindow &w);
		bool setTimeout(int seconds);

		void close();

		std::string name() const;
		std::istream& stream();

		bool filterRecord(Record*);


	// ----------------------------------------------------------------------
	//  Public file specific interface
	// ----------------------------------------------------------------------
	public:
		size_t tell();

		File &seek(size_t pos);
		File &seek(int off, SeekDir dir);


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		struct TimeWindowFilter {
			TimeWindowFilter() {}
			TimeWindowFilter(const Core::Time &stime, const Core::Time &etime)
			: start(stime), end(etime) {}

			Core::Time  start;
			Core::Time  end;
		};

		typedef std::map<std::string, TimeWindowFilter> FilterMap;

		std::string     _name;
		bool            _closeRequested;
		std::fstream    _fstream;
		std::istream   *_current;
		FilterMap       _filter;
		Core::Time      _startTime;
		Core::Time      _endTime;
};

}
}

#endif
