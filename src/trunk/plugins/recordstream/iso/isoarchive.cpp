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


#define SEISCOMP_COMPONENT ISOARCHIVE

#include <sstream>
#include <cdio/logging.h>
#include <seiscomp3/logging/log.h>

#include "isoarchive.h"


using namespace Seiscomp::Core;
using namespace Seiscomp::IO;
using namespace std;


namespace Seiscomp {
namespace RecordStream {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IMPLEMENT_SC_CLASS_DERIVED(IsoArchive,
                           Seiscomp::IO::RecordStream,
                           "isoarchive");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_RECORDSTREAM(IsoArchive, "isoarchive");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IsoArchive::IsoArchive() : RecordStream() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IsoArchive::IsoArchive(const string arcroot) 
: RecordStream()
, _arcroot(arcroot) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IsoArchive::IsoArchive(const IsoArchive &mem)
: RecordStream() {
	setSource(mem.archiveRoot());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IsoArchive::~IsoArchive() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IsoArchive &IsoArchive::operator=(const IsoArchive &mem) {
	if ( this != &mem )
		_arcroot = mem.archiveRoot();

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool IsoArchive::setSource(const string &src) {
	_arcroot = src;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool IsoArchive::addStream(const string &net, const string &sta,
                           const string &loc, const string &cha) {
	pair<set<StreamIdx>::iterator, bool> result;
	result = _streams.insert(StreamIdx(net,sta,loc,cha));
	return result.second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool IsoArchive::addStream(const string &net, const string &sta,
                           const string &loc, const string &cha,
                           const Time &stime, const Time &etime) {
	pair<set<StreamIdx>::iterator, bool> result;
	result = _streams.insert(StreamIdx(net,sta,loc,cha,stime,etime));
	return result.second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool IsoArchive::setStartTime(const Time &stime) {
	_stime = stime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool IsoArchive::setEndTime(const Time &etime) {
	_etime = etime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void IsoArchive::close() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string IsoArchive::archiveRoot() const {
	return _arcroot;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void IsoArchive::setFilenames() {
	set<StreamIdx>::iterator iter;
	for (iter = _streams.begin(); iter != _streams.end(); ++iter) {
		Time stime = (iter->startTime() == Time())?_stime:iter->startTime();
		Time etime = (iter->endTime() == Time())?_etime:iter->endTime();
		int syear, eyear;
		stime.get(&syear);
		etime.get(&eyear);

		for (int year = syear; year <= eyear; ++year) {
			stringstream ss;
			ss << year;
			string path = _arcroot + "/" + ss.str() + "/" + iter->network() + "/" +
			              iter->station() + "." + iter->network() + "." + ss.str() + ".iso";
			_fnames[path].insert(const_cast<StreamIdx *>(&*iter));
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/* redirect the cdio log messages into seiscomp log messages */
static void _LOG_HANDLER(cdio_log_level_t level, const char message[]) {
	if ( level == CDIO_LOG_DEBUG )
		return;

	if ( level == CDIO_LOG_INFO || level == CDIO_LOG_ASSERT )
		SEISCOMP_INFO("%s",message);

	if ( level == CDIO_LOG_WARN )
		SEISCOMP_WARNING("%s",message);

	if ( level == CDIO_LOG_ERROR )
		SEISCOMP_ERROR("%s",message);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *IsoArchive::next() {
	if ( _recstream ) {
		/* eof check: try to read from stream */
		Record *rec = _recstream->next();
		if ( rec != NULL )
			return rec;
	}
	else {
		setFilenames();
		_curiter = _fnames.begin();
	}

	cdio_log_set_handler(_LOG_HANDLER);

	while ( _curiter != _fnames.end() ) {
		_recstream = new IsoFile;
		if ( !_recstream ) {
			SEISCOMP_ERROR("Could not create isofile stream");
			return NULL;
		}

		if ( !_recstream->setSource(_curiter->first) ) {
			SEISCOMP_DEBUG("file %s not found",_curiter->first.c_str());
			++_curiter;
		}
		else {
			set<StreamIdx *>::iterator idxiter = _curiter->second.begin();
			SEISCOMP_DEBUG("ISO image request: %s", _curiter->first.c_str());
			while ( idxiter != _curiter->second.end() ) {
				if ( ((*idxiter)->startTime() == Time() && _stime == Time()) )
					SEISCOMP_WARNING("... has invalid time window -> ignore this request above");
				else {
					Time stime = ((*idxiter)->startTime() == Time())?_stime:(*idxiter)->startTime();
					Time etime = ((*idxiter)->endTime() == Time())?_etime:(*idxiter)->endTime();
					if ( !etime.valid() )
						etime = Time::GMT();
					_recstream->addStream((*idxiter)->network(),
					                      (*idxiter)->station(),
					                      (*idxiter)->location(),
					                      (*idxiter)->channel(),
					                      stime, etime);
				}

				++idxiter;
			}

			++_curiter;

			/* eof check: try to read from stream */
			Record *rec = _recstream->next();
			if ( rec != NULL )
				return rec;
		}
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
