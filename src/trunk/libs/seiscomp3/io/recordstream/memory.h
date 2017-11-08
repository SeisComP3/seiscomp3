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


#ifndef __SEISCOMP_SERVICES_RECORDSTREAM_MEMORY_H__
#define __SEISCOMP_SERVICES_RECORDSTREAM_MEMORY_H__

#include <iostream>
#include <sstream>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/core.h>


namespace Seiscomp {
namespace RecordStream {


DEFINE_SMARTPOINTER(Memory);

class SC_SYSTEM_CORE_API Memory:  public Seiscomp::IO::RecordStream {
	DECLARE_SC_CLASS(Memory);

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Memory();
		Memory(const char *data, int size);
		Memory(const Memory &mem);
		virtual ~Memory();


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Memory &operator=(const Memory &mem);

		virtual bool setSource(const std::string &);
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

		virtual void close();

		virtual bool setRecordType(const char *type);

		Record *next();


	private:
		RecordFactory      *_factory;
		std::istringstream  _stream;
};

}
}

#endif
