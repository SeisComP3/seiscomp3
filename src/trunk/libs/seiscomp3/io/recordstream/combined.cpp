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


namespace {


size_t findClosingParenthesis(const string &s, size_t p) {
	int cnt = 1;
	for ( size_t i = p; i < s.size(); ++i ) {
		if ( s[i] == '(' ) ++cnt;
		else if ( s[i] == ')' ) --cnt;
		if ( !cnt ) return i;
	}

	return string::npos;
}


}


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
	size_t p1,p2;

	/*
	 * Format of source is:
	 *  type1/source1;type2/source2
	 * where
	 *  sourceN is either source or (source)
	 */

	// Find first slash
	p1 = serverloc.find('/');
	string type1;

	if ( p1 == string::npos ) {
		type1 = "slink";
		p1 = 0;
	}
	else {
		type1 = serverloc.substr(0, p1);
		// Move behind '/'
		++p1;
	}

	string source1;

	// Extract source1
	if ( p1 >= serverloc.size() ) {
		SEISCOMP_ERROR("Invalid RecordStream URL '%s': missing second source",
		               serverloc.c_str());
		throw RecordStreamException("Invalid RecordStream URL");
	}

	// Source sourrounded by parenthesis
	if ( serverloc[p1] == '(' ) {
		++p1;
		// Find closing parenthesis
		p2 = findClosingParenthesis(serverloc, p1);
		if ( p2 == string::npos ) {
			SEISCOMP_ERROR("Invalid RecordStream URL '%s': expected closing parenthesis",
			               serverloc.c_str());
			throw RecordStreamException("Invalid RecordStream URL");
		}

		source1 = serverloc.substr(p1, p2-p1);
		++p2;

		if ( p2 >= serverloc.size() || serverloc[p2] != ';' ) {
			SEISCOMP_ERROR("Invalid RecordStream URL '%s': expected ';' at %d",
			               serverloc.c_str(), (int)p2);
			throw RecordStreamException("Invalid RecordStream URL");
		}

		p1 = p2+1;
	}
	else {
		p2 = serverloc.find(';', p1);
		if ( p2 == string::npos ) {
			SEISCOMP_ERROR("Invalid RecordStream URL '%s': missing second source, expected ';'",
			               serverloc.c_str());
			throw RecordStreamException("Invalid RecordStream URL");
		}

		source1 = serverloc.substr(p1, p2-p1);
		p1 = p2+1;
	}

	// Find first slash
	string type2;
	p2 = serverloc.find('/', p1);
	if ( p2 == string::npos ) {
		type2 = "arclink";
		p2 = p1;
	}
	else {
		type2 = serverloc.substr(p1, p2-p1);
		// Move behind '/'
		++p2;
	}

	string source2;

	// Extract source2
	// source2 can be empty
	if ( p2 < serverloc.size() ) {
		p1 = p2;
		// Source sourrounded by parenthesis
		if ( serverloc[p1] == '(' ) {
			++p1;
			// Find closing parenthesis
			p2 = findClosingParenthesis(serverloc, p1);
			if ( p2 == string::npos ) {
				SEISCOMP_ERROR("Invalid RecordStream URL '%s': expected closing parenthesis",
				               serverloc.c_str());
				throw RecordStreamException("Invalid RecordStream URL");
			}

			source2 = serverloc.substr(p1, p2-p1);
			p1 = p2+1;
		}
		else {
			p2 = serverloc.find("??", p1);
			if ( p2 == string::npos )
				source2 = serverloc.substr(p1);
			else
				source2 = serverloc.substr(p1, p2-p1);
			p1 = p2;
		}
	}

	string params;
	if ( (p1 <= serverloc.size()-2) && !serverloc.compare(p1,2,"??") )
		params = serverloc.substr(p1+2);
	else if ( p1 < serverloc.size() ) {
		SEISCOMP_ERROR("Invalid RecordStream URL '%s': undefined trailing content '%s'",
		               serverloc.c_str(), serverloc.c_str()+p1);
		throw RecordStreamException("Invalid RecordStream URL");
	}

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

			if ( name == "slinkMax" || name == "rtMax" || name == "1stMax" ) {
				if ( value.empty() ) {
					SEISCOMP_ERROR("Invalid RecordStream URL '%s', "
					               "value of parameter '%s' is empty",
					               serverloc.c_str(), name.c_str());
					throw RecordStreamException("Invalid RecordStream URL");
				}

				double number;
				double multiplicator = 1.0;

				char last = *value.rbegin();
				switch ( last ) {
					case 's':
						// seconds
						break;
					case 'm':
						// minutes
						multiplicator = 60;
						break;
					case 'h':
						// hours
						multiplicator = 3600;
					case 'd':
						// days
						multiplicator = 86400;
						break;
					case 'w':
						// weeks
						multiplicator = 86400*7;
						break;
					default:
						break;
				}

				if ( multiplicator > 1.0 )
					value.resize(value.size()-1);

				if ( !Core::fromString(number, value) ) {
					SEISCOMP_ERROR("Invalid RecordStream URL '%s', "
					               "value of parameter '%s' not number",
					               serverloc.c_str(), name.c_str());
					throw RecordStreamException("Invalid RecordStream URL");
				}

				SEISCOMP_DEBUG("setting realtime availability to %f (v:%f, m:%f)",
				               number*multiplicator, number, multiplicator);
				_realtimeAvailability = Core::TimeSpan(number*multiplicator);
			}
		}
	}

	SEISCOMP_DEBUG("Type1   : %s", type1.c_str());
	SEISCOMP_DEBUG("Source1 : %s", source1.c_str());
	SEISCOMP_DEBUG("Type2   : %s", type2.c_str());
	SEISCOMP_DEBUG("Source2 : %s", source2.c_str());
	SEISCOMP_DEBUG("Params  : %s", params.c_str());

	_realtime = IO::RecordStream::Create(type1.c_str());
	if ( !_realtime ) {
		string msg = "Could not create realtime RecordStream: " + type1;
		SEISCOMP_ERROR_S(msg);
		throw new RecordStreamException(msg);
	}

	_archive = IO::RecordStream::Create(type2.c_str());
	if ( !_archive ) {
		string msg = "Could not create archive RecordStream: " + type2;
		SEISCOMP_ERROR_S(msg);
		throw RecordStreamException(msg);
	}

	if ( !_realtime->setSource(source1) ) {
		_realtime = _archive = NULL;
		return false;
	}

	if ( !_archive->setSource(source2) ) {
		_realtime = _archive = NULL;
		return false;
	}

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

