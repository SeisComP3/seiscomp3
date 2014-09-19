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


#define SEISCOMP_COMPONENT RECORDFILE
#include "file.h"
#include <seiscomp3/logging/log.h>

using namespace Seiscomp::RecordStream;


IMPLEMENT_SC_CLASS_DERIVED(File,
                           Seiscomp::IO::RecordStream,
                           "RecordFile");

REGISTER_RECORDSTREAM(File, "file");

File::File() : RecordStream(), _current(&_fstream) {}

File::File(std::string name) : RecordStream(), _current(&_fstream)  {
  setSource(name);
}

File::File(const File &f) : RecordStream(), _current(&_fstream) {
  setSource(f.name());
}

File::~File() {
  close();
}

File& File::operator=(const File &f) {
  if (this != &f) {
    if (_fstream.is_open())
      _fstream.close();
    _name = f.name();
    if (_name != "-")
      _fstream.open(_name.c_str(),std::ifstream::in|std::ifstream::binary);
  }
  
  return *this;
}

bool File::setSource(std::string name) {
  _name = name;

  if (_fstream.is_open())
      _fstream.close();

  setRecordType("mseed");

  if (_name != "-") {
    _fstream.open(_name.c_str(),std::ifstream::in|std::ifstream::binary);

    size_t pos = name.rfind('.');
    if ( pos != std::string::npos ) {
      std::string ext = name.substr(pos+1);
      if ( ext == "xml" )
        setRecordType("xml");
      else if ( ext == "bin" )
        setRecordType("binary");
      else if ( ext == "mseed" )
        setRecordType("mseed");
      else if ( ext == "ah" )
        setRecordType("ah");
    }

    _current = &_fstream;
    return !_fstream.fail();
  }
  _current = &std::cin;
  return !std::cin.bad();
}

bool File::addStream(std::string net, std::string sta, std::string loc, std::string cha) {
  return false;
}

bool File::addStream(std::string net, std::string sta, std::string loc, std::string cha,
  const Seiscomp::Core::Time &stime, const Seiscomp::Core::Time &etime) {
  return false;
}

bool File::setStartTime(const Seiscomp::Core::Time &stime) {
  return false;
}

bool File::setEndTime(const Seiscomp::Core::Time &etime) {
  return false;
}

bool File::setTimeWindow(const Seiscomp::Core::TimeWindow &w) {
            return setStartTime(w.startTime()) && setEndTime(w.endTime());
}

bool File::setTimeout(int seconds) {
  return false;
}

void File::close() {
  if (_name != "-")
    _fstream.close();
  _current = &_fstream;
}

std::string File::name() const {
  return _name;
}

std::istream& File::stream() {
  return *_current;
}

size_t File::tell() {
  return (size_t)_fstream.tellg();
}

File &File::seek(size_t pos) {
	_fstream.seekg((std::streampos)pos);
	return *this;
}

File &File::seek(int off, SeekDir dir) {
	_fstream.seekg((std::streamoff)off, (std::ios_base::seekdir)dir);
	return *this;
}
