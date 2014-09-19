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


#define SEISCOMP_COMPONENT CombinedConnection

#include <cstdio>
#include <string>
#include <iostream>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/io/recordstream/arclink.h>
#include <seiscomp3/io/recordstream/slconnection.h>

#include "combined.h"

namespace Seiscomp {
namespace RecordStream {
namespace Combined {
namespace _private {

using namespace std;
using namespace Seiscomp::Core;
using namespace Seiscomp::IO;

const TimeSpan DefaultRealtimeAvailability = 3600;

IMPLEMENT_SC_CLASS_DERIVED(CombinedConnection, Seiscomp::IO::RecordStream,
                           "CombinedConnection");

REGISTER_RECORDSTREAM(CombinedConnection, "combined");

CombinedConnection::CombinedConnection() {
	init();
}

CombinedConnection::CombinedConnection(std::string serverloc) {
	init();
	setSource(serverloc);
}

CombinedConnection::~CombinedConnection() {}

void CombinedConnection::init() {
	_started = false;
	_nStream = _nArchive = _nRealtime = 0;
	_realtimeAvailability = DefaultRealtimeAvailability;

	_realtime = new SLConnection();
	_archive = new Arclink::ArclinkConnection();
}

bool CombinedConnection::setRecordType(const char* type) {
	return _realtime->setRecordType(type) && _archive->setRecordType(type);
}

bool CombinedConnection::setSource(std::string serverloc) {
	// read optional parameters for combined source, separated by '??' after
	// last URL
	size_t param_sep = serverloc.find("??");
	if ( param_sep == string::npos )
		param_sep = serverloc.length();
	else {
		string params = serverloc.substr(param_sep+2);
		serverloc.erase(param_sep);

		vector<string> toks;
		Core::split(toks, params.c_str(), "&");
		if ( !toks.empty() ) {
			for ( std::vector<std::string>::iterator it = toks.begin();
			      it != toks.end(); ++it ) {
				std::string name, value;

				size_t pos = it->find('=');
				if ( pos != std::string::npos ) {
					name = it->substr(0, pos);
					value = it->substr(pos+1);
				}
				else {
					name = *it;
					value = "";
				}

				if ( name == "slinkMax" || name == "rtMax" ) {
					double seconds;
					if ( !Core::fromString(seconds, value) ) {
						SEISCOMP_ERROR("Invalid RecordStream URL '%s', "
						               "value of parameter '%s' not number",
						               serverloc.c_str(), name.c_str());
						throw RecordStreamException("Invalid RecordStream URL");
					}

					SEISCOMP_DEBUG("setting realtime availability to %f",
					               seconds);
					_realtimeAvailability = Core::TimeSpan(seconds);
				}
			}
		}
	}

	// read URLs
	size_t sep = serverloc.find(';');
	if ( sep == string::npos || serverloc.find(';', sep+1) != string::npos ) {
		SEISCOMP_ERROR("Invalid RecordStream URL '%s': exact 2 sources "
		               "separated by ';' required", serverloc.c_str());
		throw RecordStreamException("Invalid RecordStream URL");
	}
	if ( param_sep < sep ) {
		SEISCOMP_ERROR("Invalid RecordStream URL '%s': parameter separator "
		               "'\?\?' found before source separator ';'",
		               serverloc.c_str());
		throw RecordStreamException("invalid address format");
	}

	string rtSource = serverloc.substr(0, sep);
	string arSource = serverloc.substr(sep + 1, param_sep - sep - 1);
	string service;

	// Change realtime stream if specified
	sep = rtSource.find("/");
	if ( sep != string::npos ) {
		service = rtSource.substr(0, sep);
		rtSource = sep+1 < rtSource.length() ? rtSource.substr(sep+1) : "";
		IO::RecordStream *stream = IO::RecordStream::Create(service.c_str());
		if ( !stream ) {
			string msg = "Could not create realtime RecordStream: " + service;
			SEISCOMP_ERROR_S(msg);
			throw new RecordStreamException(msg);
		}
		else
			_realtime = stream;
	}

	// Change archive stream if specified
	sep = arSource.find("/");
	if ( sep != string::npos ) {
		service = arSource.substr(0, sep);
		arSource = sep+1 < arSource.length() ? arSource.substr(sep+1) : "";
		IO::RecordStream *stream = IO::RecordStream::Create(service.c_str());
		if ( !stream ) {
			string msg = "Could not create archive RecordStream: " + service;
			SEISCOMP_ERROR_S(msg);
			throw RecordStreamException(msg);
		}
		else
			_archive = stream;
	}

	_realtime->setSource(rtSource);
	_archive->setSource(arSource);

	_curtime = Time::GMT();
	_archiveEndTime = _curtime - _realtimeAvailability;

	return true;
}

bool CombinedConnection::addStream(std::string net, std::string sta,
                                   std::string loc, std::string cha) {
	SEISCOMP_DEBUG("add stream %lu %s.%s.%s.%s", (unsigned long) _nStream, net.c_str(),
	               sta.c_str(), loc.c_str(), cha.c_str());
	// Streams without a time span are inserted into a temporary list
	// and will be resolved when the data is requested the first time
	pair<set<StreamIdx>::iterator, bool> result;
	result = _tmpStreams.insert(StreamIdx(net, sta, loc, cha));
	return result.second;
}

bool CombinedConnection::addStream(std::string net, std::string sta,
                                   std::string loc, std::string cha,
                                   const Seiscomp::Core::Time &stime,
                                   const Seiscomp::Core::Time &etime) {
	SEISCOMP_DEBUG("add stream %lu %s.%s.%s.%s", (unsigned long) _nStream, net.c_str(),
	               sta.c_str(), loc.c_str(), cha.c_str());

	if ( stime.valid() && stime < _archiveEndTime ) {
		if ( etime.valid() && etime <= _archiveEndTime ) {
			_archive->addStream(net, sta, loc, cha, stime, etime);
			++_nArchive;
		}
		else {
			_archive->addStream(net, sta, loc, cha, stime, _archiveEndTime);
			_realtime->addStream(net, sta, loc, cha, _archiveEndTime, etime);
			++_nArchive;
			++_nRealtime;
		}
	}
	else {
		_realtime->addStream(net, sta, loc, cha, stime, etime);
		++_nRealtime;
	}

	++_nStream;
	return true;
}

bool CombinedConnection::setStartTime(const Seiscomp::Core::Time &stime) {
	_startTime = stime;
	return true;
}

bool CombinedConnection::setEndTime(const Seiscomp::Core::Time &etime) {
	_endTime = etime;
	return true;
}

bool CombinedConnection::setTimeWindow(const Seiscomp::Core::TimeWindow &w) {
	return setStartTime(w.startTime()) && setEndTime(w.endTime());
}

bool CombinedConnection::setTimeout(int seconds) {
	_realtime->setTimeout(seconds);
	_archive->setTimeout(seconds);
	return true;
}

void CombinedConnection::close() {
	_realtime->close();
	_archive->close();
	_nArchive = 0;
}

std::istream& CombinedConnection::stream() {
	if ( !_started ) {
		_started = true;

		// add the temporary streams (added without a time span) now and split
		// them correctly
		for ( set<StreamIdx>::iterator it = _tmpStreams.begin();
		      it != _tmpStreams.end(); ++it )
			addStream(it->network(), it->station(), it->location(),
			          it->channel(), _startTime, _endTime);
		_tmpStreams.clear();

		if ( _nArchive > 0 )
			SEISCOMP_DEBUG("start %lu archive requests", (unsigned long) _nArchive);
		else
			SEISCOMP_DEBUG("start %lu realtime requests", (unsigned long) _nRealtime);
	}

	if ( _nArchive > 0 ) {
		std::istream &is = _archive->stream();
		if ( !is.eof() )
			return is;
		else {
			_archive->close();
			_nArchive = 0;
			SEISCOMP_DEBUG("start %lu realtime requests", (unsigned long) _nRealtime);
		}

		return _realtime->stream();
	}
	else
		return _realtime->stream();
}

Record* CombinedConnection::createRecord(Array::DataType dt, Record::Hint h) {
	if ( _nArchive > 0 )
		return _archive->createRecord(dt, h);
	else
		return _realtime->createRecord(dt, h);
}

} // namesapce Combined
} // namespace _private
} // namespace RecordStream
} // namespace Seiscomp

