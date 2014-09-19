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


#define SEISCOMP_COMPONENT MEMORY
#include <seiscomp3/io/recordstream/memory.h>
#include <seiscomp3/logging/log.h>

using namespace Seiscomp::RecordStream;


IMPLEMENT_SC_CLASS_DERIVED(Memory,
                           Seiscomp::IO::RecordStream,
                           "Memory");

REGISTER_RECORDSTREAM(Memory, "memory");


Memory::Memory() {}

Memory::Memory(const char *data, int size) : RecordStream() {
  setSource(std::string(data,size));
}

Memory::Memory(const Memory &mem) : RecordStream() {
  _stream.str(mem._stream.str());
}

Memory::~Memory() {}

Memory& Memory::operator=(const Memory &mem) {
  if (this != &mem) {
    _stream.str(mem._stream.str());
  }

  return *this;
}

bool Memory::setSource(std::string data) {
  _stream.str(data);

  return !_stream.fail();
}

bool Memory::addStream(std::string net, std::string sta, std::string loc, std::string cha) {
  return false;
}

bool Memory::addStream(std::string net, std::string sta, std::string loc, std::string cha,
  const Seiscomp::Core::Time &stime, const Seiscomp::Core::Time &etime) {
  return false;
}

bool Memory::setStartTime(const Seiscomp::Core::Time &stime) {
  return false;
}

bool Memory::setEndTime(const Seiscomp::Core::Time &etime) {
  return false;
}

bool Memory::setTimeWindow(const Seiscomp::Core::TimeWindow &w) {
	return setStartTime(w.startTime()) && setEndTime(w.endTime());
}


bool Memory::setTimeout(int seconds) {
  return false;
}

void Memory::close() {}

std::istream& Memory::stream() {
  return _stream;
}
