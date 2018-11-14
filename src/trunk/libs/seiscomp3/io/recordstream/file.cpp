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
using namespace Seiscomp;
using namespace Seiscomp::RecordStream;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_RECORDSTREAM(File, "file");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
File::File()
: RecordStream()
, _factory(NULL)
, _current(&_fstream) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
File::File(string name)
: _factory(NULL)
, _current(&_fstream) {
	setSource(name);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
File::File(const File &f)
: _factory(NULL)
, _current(&_fstream) {
	setSource(f.name());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
File::~File() {
	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
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
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool File::setSource(const string &name) {
	_name = name;
	_closeRequested = false;

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
		return _fstream.is_open();
	}

	_current = &cin;
	return !cin.bad();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool File::addStream(const string &net, const string &sta,
                     const string &loc, const string &cha) {
	string id = net + "." + sta + "." + loc + "." + cha;
	_filter[id] = TimeWindowFilter();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool File::addStream(const string &net, const string &sta,
                     const string &loc, const string &cha,
                     const Seiscomp::Core::Time &stime,
                     const Seiscomp::Core::Time &etime) {
	string id = net + "." + sta + "." + loc + "." + cha;
	_filter[id] = TimeWindowFilter(stime, etime);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool File::setStartTime(const Seiscomp::Core::Time &stime) {
	_startTime = stime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool File::setEndTime(const Seiscomp::Core::Time &etime) {
	_endTime = etime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void File::close() {
	_closeRequested = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool File::setRecordType(const char *type) {
	RecordFactory *factory = RecordFactory::Find(type);
	if ( factory == NULL ) {
		SEISCOMP_ERROR("Unknown record type '%s'", type);
		return false;
	}

	_factory = factory;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *File::next() {
	if ( _closeRequested ) {
		if (_name != "-")
			_fstream.close();
		_current = &_fstream;
		_filter.clear();
		_closeRequested = false;
		return NULL;
	}

	if ( !*_current )
		return NULL;

	while ( true ) {
		Record *rec = _factory->create();
		if ( rec == NULL )
			return NULL;

		setupRecord(rec);

		try {
			rec->read(*_current);
		}
		catch ( Core::EndOfStreamException & ) {
			delete rec;
			return NULL;
		}
		catch ( std::exception &e ) {
			SEISCOMP_ERROR("file read exception: %s", e.what());
			delete rec;
			return NULL;
		}

		if ( !_filter.empty() ) {
			FilterMap::iterator it = _filter.find(rec->streamID());
			// Not subscribed
			if ( it == _filter.end() ) {
				delete rec;
				continue;
			}

			if ( it->second.start.valid() ) {
				if ( rec->endTime() < it->second.start ) {
					delete rec;
					continue;
				}
			}
			else if ( _startTime.valid() ) {
				if ( rec->endTime() < _startTime ) {
					delete rec;
					continue;
				}
			}

			if ( it->second.end.valid() ) {
				if ( rec->startTime() >= it->second.end ) {
					delete rec;
					continue;
				}
			}
			else if ( _endTime.valid() ) {
				if ( rec->startTime() >= _endTime ) {
					delete rec;
					continue;
				}
			}
		}
		else {
			if ( _startTime.valid() ) {
				if ( rec->endTime() < _startTime ) {
					delete rec;
					continue;
				}
			}

			if ( _endTime.valid() ) {
				if ( rec->startTime() >= _endTime ) {
					delete rec;
					continue;
				}
			}
		}

		return rec;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string File::name() const {
	return _name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t File::tell() {
	return (size_t)_fstream.tellg();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
File &File::seek(size_t pos) {
	_fstream.seekg((streampos)pos);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
File &File::seek(int off, SeekDir dir) {
	_fstream.seekg((streamoff)off, (ios_base::seekdir)dir);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
