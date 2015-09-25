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


#define SEISCOMP_COMPONENT BalancedConnection

#include <cstdio>
#include <string>
#include <iostream>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/io/recordinput.h>

#include "balanced.h"

namespace Seiscomp {
namespace RecordStream {
namespace Balanced {
namespace _private {

using namespace std;
using namespace Seiscomp::Core;
using namespace Seiscomp::IO;

const unsigned int BufferSize = 1024;


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


IMPLEMENT_SC_CLASS_DERIVED(BalancedConnection, Seiscomp::IO::RecordStream,
                           "BalancedConnection");

REGISTER_RECORDSTREAM(BalancedConnection, "balanced");

BalancedConnection::BalancedConnection(): _started(false), _nthreads(0),
	_stream(std::istringstream::in|std::istringstream::binary) {
}

BalancedConnection::BalancedConnection(std::string serverloc): _started(false), _nthreads(0),
	_stream(std::istringstream::in|std::istringstream::binary) {
	setSource(serverloc);
}

BalancedConnection::~BalancedConnection() {}

bool BalancedConnection::setSource(std::string serverloc) {
	if ( _started )
		return false;

	_rsarray.clear();

	size_t p1,p2;

	/*
	 * Format of source is:
	 *  type1/source1;type2/source2;...;typeN/sourceN
	 * where
	 *  sourceN is either source or (source)
	 */

	while (true) {
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
			SEISCOMP_ERROR("Invalid RecordStream URL '%s': missing source",
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
		}
		else {
			p2 = serverloc.find(';', p1);
			if ( p2 == string::npos ) {
				p2 = serverloc.length();
			}

			source1 = serverloc.substr(p1, p2-p1);
		}

		SEISCOMP_DEBUG("Type   : %s", type1.c_str());
		SEISCOMP_DEBUG("Source : %s", source1.c_str());

		RecordStreamPtr rs = RecordStream::Create(type1.c_str());

		if ( rs == NULL ) {
			SEISCOMP_ERROR("Invalid RecordStream type: %s", type1.c_str());
			return false;
		}

		if ( !rs->setSource(source1) ) {
			SEISCOMP_ERROR("Invalid RecordStream source: %s", source1.c_str());
			return false;
		}

		_rsarray.push_back(make_pair(rs, 0));

		if ( p2 == serverloc.length() )
			break;

		serverloc = serverloc.substr(p2 + 1, string::npos);
	}

	return true;
}

int BalancedConnection::streamHash(const string &sta) {
	unsigned int i = 0;
	for ( const char* p = sta.c_str(); *p != 0; ++p ) i += *p;

	return i % _rsarray.size();
}

bool BalancedConnection::addStream(std::string net, std::string sta,
                                   std::string loc, std::string cha) {
	SEISCOMP_DEBUG("add stream %s.%s.%s.%s", net.c_str(),
	               sta.c_str(), loc.c_str(), cha.c_str());

	if ( _rsarray.empty() )
		return false;

	int i = streamHash(sta);

	if ( !_rsarray[i].first->addStream(net, sta, loc, cha) )
		return false;

	_rsarray[i].second = true;

	return true;
}

bool BalancedConnection::addStream(std::string net, std::string sta,
                                   std::string loc, std::string cha,
                                   const Seiscomp::Core::Time &stime,
                                   const Seiscomp::Core::Time &etime) {
	SEISCOMP_DEBUG("add stream %s.%s.%s.%s", net.c_str(),
	               sta.c_str(), loc.c_str(), cha.c_str());

	if ( _rsarray.empty() )
		return false;

	int i = streamHash(sta);

	if ( !_rsarray[i].first->addStream(net, sta, loc, cha, stime, etime) )
		return false;

	_rsarray[i].second = true;

	return true;
}

bool BalancedConnection::setStartTime(const Seiscomp::Core::Time &stime) {
	if ( _rsarray.empty() )
		return false;

	for ( unsigned int i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setStartTime(stime) )
			return false;
	}

	return true;
}

bool BalancedConnection::setEndTime(const Seiscomp::Core::Time &etime) {
	if ( _rsarray.empty() )
		return false;

	for ( unsigned int i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setEndTime(etime) )
			return false;
	}

	return true;
}

bool BalancedConnection::setTimeWindow(const Seiscomp::Core::TimeWindow &w) {
	if ( _rsarray.empty() )
		return false;

	for ( unsigned int i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setTimeWindow(w) )
			return false;
	}

	return true;
}

bool BalancedConnection::setRecordType(const char* type) {
	if ( _rsarray.empty() )
		return false;

	for ( unsigned int i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setRecordType(type) )
			return false;
	}

	return true;
}

bool BalancedConnection::setTimeout(int seconds) {
	if ( _rsarray.empty() )
		return false;

	for ( unsigned int i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setTimeout(seconds) )
			return false;
	}

	return true;
}

void BalancedConnection::close() {
	if ( _rsarray.empty() )
		return;

	for ( unsigned int i = 0; i < _rsarray.size(); ++i)
		_rsarray[i].first->close();
}

void BalancedConnection::putRecord(Record* rec) {
	{
		boost::unique_lock<boost::mutex> lock(_mtx);

		while(_buffer.size() >= BufferSize) {
			_buf_not_full.wait(lock);
		}

		_buffer.push_back(rec);
	}

	_buf_not_empty.notify_one();
}

Record* BalancedConnection::getRecord() {
	Record* rec;

	{
		boost::unique_lock<boost::mutex> lock(_mtx);

		while( _buffer.empty() ) {
			_buf_not_empty.wait(lock);
		}

		rec = _buffer.front();
		_buffer.pop_front();
	}

	_buf_not_full.notify_one();

	return rec;
}

void BalancedConnection::acquiThread(RecordStreamPtr rs) {
	SEISCOMP_DEBUG("Starting acquisition thread");

	RecordInput recInput(rs.get(), Array::INT, Record::SAVE_RAW);

	try {
		for ( RecordIterator it = recInput.begin(); it != recInput.end(); ++it )
			putRecord(*it);
	}
	catch ( OperationInterrupted& e ) {
		SEISCOMP_DEBUG("Interrupted acquisition thread, msg: '%s'", e.what());
	}
	catch ( exception& e ) {
		SEISCOMP_ERROR("Exception in acquisition thread: '%s'", e.what());
	}

	SEISCOMP_DEBUG("Finished acquisition thread");

	putRecord(NULL);
}

std::istream& BalancedConnection::stream() {
	if ( !_started ) {
		_started = true;

		for ( unsigned int i = 0; i < _rsarray.size(); ++i) {
			if ( _rsarray[i].second ) {
				_threads.push_back(new boost::thread(boost::bind(&BalancedConnection::acquiThread, this, _rsarray[i].first)));
				++_nthreads;
			}
		}
	}

	while (_nthreads > 0) {
		Record* rec = getRecord();

		if (rec == NULL) {
			--_nthreads;
			continue;
		}

		_stream.clear();
		_stream.str(rec->raw()->str());

		delete rec;

		return _stream;
	}

	SEISCOMP_DEBUG("All acquisition threads finished -> set stream's eofbit");

	for ( list<boost::thread *>::iterator it = _threads.begin(); it != _threads.end(); ++it)
		(*it)->join();

	_started = false;
	_threads.clear();
	_stream.clear(ios::eofbit);

	return _stream;
}

Record* BalancedConnection::createRecord(Array::DataType dt, Record::Hint h) {
	if ( _rsarray.empty() )
		return NULL;
	
	return _rsarray[0].first->createRecord(dt, h);
}

} // namesapce Balanced
} // namespace _private
} // namespace RecordStream
} // namespace Seiscomp

