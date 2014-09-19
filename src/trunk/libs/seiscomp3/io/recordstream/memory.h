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
		
	public:
		Memory();
		Memory(const char *data, int size);
		Memory(const Memory &mem);
		virtual ~Memory();

	public:
		Memory& operator=(const Memory &mem);
		bool setSource(std::string);
		bool addStream(std::string net, std::string sta, std::string loc, std::string cha);
		bool addStream(std::string net, std::string sta, std::string loc, std::string cha,
		               const Seiscomp::Core::Time &stime, const Seiscomp::Core::Time &etime);
		bool setStartTime(const Seiscomp::Core::Time &stime);
		bool setEndTime(const Seiscomp::Core::Time &etime);
		bool setTimeWindow(const Seiscomp::Core::TimeWindow &w);
		bool setTimeout(int seconds);
		std::istream& stream();
		void close();
		
	private:
		std::istringstream _stream;
};

}
}

#endif
