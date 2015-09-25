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


using namespace std;
using namespace Seiscomp::RecordStream;


REGISTER_RECORDSTREAM(File, "file");


File::File() : RecordStream(), _current(&_fstream) {}


File::File(string name)
: _current(&_fstream) {
	setSource(name);
}


File::File(const File &f)
: _current(&_fstream) {
	setSource(f.name());
}


File::~File() {
	close();
}


File &File::operator=(const File &f) {
	if (this != &f) {
		if ( _fstream.is_open() )
			_fstream.close();

		_name = f.name();
		if ( _name != "-" )
			_fstream.open(_name.c_str(),ifstream::in|ifstream::binary);
	}

	return *this;
}


bool File::setSource(string name) {
	_name = name;

	if ( _fstream.is_open() )
		_fstream.close();

	setRecordType("mseed");

	if ( _name != "-" ) {
		_fstream.open(_name.c_str(),ifstream::in|ifstream::binary);

		size_t pos = name.rfind('.');
		if ( pos != string::npos ) {
			string ext = name.substr(pos+1);
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

	_current = &cin;
	return !cin.bad();
}


bool File::addStream(string net, string sta, string loc, string cha) {
	string id = net + "." + sta + "." + loc + "." + cha;
	_filter[id] = TimeWindowFilter();
	return true;
}


bool File::addStream(string net, string sta, string loc, string cha,
                     const Seiscomp::Core::Time &stime,
                     const Seiscomp::Core::Time &etime) {
	string id = net + "." + sta + "." + loc + "." + cha;
	_filter[id] = TimeWindowFilter(stime, etime);
	return true;
}


bool File::setStartTime(const Seiscomp::Core::Time &stime) {
	_startTime = stime;
	return true;
}


bool File::setEndTime(const Seiscomp::Core::Time &etime) {
	_endTime = etime;
	return true;
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
	_filter.clear();
}


string File::name() const {
	return _name;
}


istream& File::stream() {
	return *_current;
}


bool File::filterRecord(Record *r) {
	if ( !_filter.empty() ) {
		FilterMap::iterator it = _filter.find(r->streamID());
		// Not subscribed
		if ( it == _filter.end() )
			return true;

		if ( it->second.start.valid() ) {
			if ( r->endTime() < it->second.start )
				return true;
		}
		else if ( _startTime.valid() ) {
			if ( r->endTime() < _startTime )
				return true;
		}

		if ( it->second.end.valid() ) {
			if ( r->startTime() >= it->second.end )
				return true;
		}
		else if ( _endTime.valid() ) {
			if ( r->startTime() >= _endTime )
				return true;
		}
	}
	else {
		if ( _startTime.valid() ) {
			if ( r->endTime() < _startTime )
				return true;
		}

		if ( _endTime.valid() ) {
			if ( r->startTime() >= _endTime )
				return true;
		}
	}

	return false;
}


size_t File::tell() {
	return (size_t)_fstream.tellg();
}


File &File::seek(size_t pos) {
	_fstream.seekg((streampos)pos);
	return *this;
}

File &File::seek(int off, SeekDir dir) {
	_fstream.seekg((streamoff)off, (ios_base::seekdir)dir);
	return *this;
}
