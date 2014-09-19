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


#define SEISCOMP_COMPONENT ARCHIVE

#include <sys/stat.h>
#include <errno.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <seiscomp3/io/recordstream/archive.h>
#include <seiscomp3/logging/log.h>

using namespace Seiscomp::RecordStream;
using namespace Seiscomp::IO;
using namespace std;
namespace fs = boost::filesystem;

IMPLEMENT_SC_CLASS_DERIVED(Archive,
                           Seiscomp::IO::RecordStream,
                           "Archive");

REGISTER_RECORDSTREAM(Archive, "archive");


Archive::Archive() : RecordStream() {}

Archive::Archive(const string arcroot) 
    : RecordStream(), _arcroot(arcroot) {
    _dirstack.push(_arcroot);
}

Archive::Archive(const Archive &mem) : RecordStream() {
    setSource(mem.archiveRoot());
}

Archive::~Archive() {}

Archive& Archive::operator=(const Archive &mem) {
  if (this != &mem) {
      _arcroot = mem.archiveRoot();
  }

  return *this;
}

bool Archive::setSource(string src) {
    _arcroot = src;
    _dirstack.push(_arcroot);
    return true;
}

bool Archive::addStream(string net, string sta, string loc, string cha) {
    return false;
}

bool Archive::addStream(string net, string sta, string loc, string cha,
			const Seiscomp::Core::Time &stime, const Seiscomp::Core::Time &etime) {
    return false;
}

bool Archive::setStartTime(const Seiscomp::Core::Time &stime) {
    return false;
}

bool Archive::setEndTime(const Seiscomp::Core::Time &etime) {
    return false;
}

bool Archive::setTimeWindow(const Seiscomp::Core::TimeWindow &w) {
    return setStartTime(w.startTime()) && setEndTime(w.endTime());
}

bool Archive::setTimeout(int seconds) {
    return false;
}

void Archive::close() {}

string Archive::archiveRoot() const {
    return _arcroot;
}

istream& Archive::stream() throw(ArchiveException) {
    if (_recstream) {
	/* eof check: try to read from stream */
	istream &tmpstream = _recstream->stream();
	 tmpstream.peek();
	/* go on at the file's stream */
	if (tmpstream.good())
	    return tmpstream;      
    } 

    while (!_dirstack.empty()) {
		fs::path dir(_dirstack.top(),fs::native);
		_dirstack.pop();

		if (is_directory(dir)) {
			fs::directory_iterator enditer;
			for (fs::directory_iterator diter(dir); diter != enditer; ++diter)
				_dirstack.push(diter->string());
		} else {
			_recstream = RecordStream::Create("file");
			if (!_recstream) {
				SEISCOMP_ERROR("Could not create file stream");
				throw ArchiveException("Could not create file stream");
			}
			if (!_recstream->setSource(dir.string().c_str())) {
				SEISCOMP_ERROR("Setting data source %s failed",dir.string().c_str());
				throw ArchiveException("Setting datasource failed");
			}
			return _recstream->stream();
		}
    }

    if (!_recstream)
	throw ArchiveException("no data files found");
    return _recstream->stream();  
}
