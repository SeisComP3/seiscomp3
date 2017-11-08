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
		File &operator=(const File &f);


	// ----------------------------------------------------------------------
	//  Public RecordStream interface
	// ----------------------------------------------------------------------
	public:
		virtual bool setSource(const std::string &filename);

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

		virtual bool setStartTime(const Seiscomp::Core::Time &startTime);
		virtual bool setEndTime(const Seiscomp::Core::Time &endTime);

		virtual void close();

		virtual bool setRecordType(const char *type);

		virtual Record *next();


	// ----------------------------------------------------------------------
	//  Public file specific interface
	// ----------------------------------------------------------------------
	public:
		std::string name() const;
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

		RecordFactory  *_factory;
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
