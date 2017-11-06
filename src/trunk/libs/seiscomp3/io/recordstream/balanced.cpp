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
#include <boost/bind.hpp>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/io/recordinput.h>

#include "balanced.h"

#include <seiscomp3/client/queue.ipp>

namespace Seiscomp {
namespace RecordStream {
namespace Balanced {
namespace _private {

using namespace std;
using namespace Seiscomp::Core;
using namespace Seiscomp::IO;

const size_t QueueSize = 1024;


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

BalancedConnection::BalancedConnection(): _started(false), _nthreads(0), _queue(QueueSize),
	_stream(std::istringstream::in|std::istringstream::binary) {
}

BalancedConnection::BalancedConnection(std::string serverloc): _started(false), _nthreads(0), _queue(QueueSize),
	_stream(std::istringstream::in|std::istringstream::binary) {
	setSource(serverloc);
}

BalancedConnection::~BalancedConnection() {
	close();
}

bool BalancedConnection::setSource(const std::string &source) {
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

	string serverloc = source;

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

		// Source surrounded by parentheses
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
	size_t i = 0;
	for ( const char* p = sta.c_str(); *p != 0; ++p ) i += *p;

	return i % _rsarray.size();
}

bool BalancedConnection::addStream(const std::string &net, const std::string &sta,
                                   const std::string &loc, const std::string &cha) {
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

bool BalancedConnection::addStream(const std::string &net, const std::string &sta,
                                   const std::string &loc, const std::string &cha,
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

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setStartTime(stime) )
			return false;
	}

	return true;
}

bool BalancedConnection::setEndTime(const Seiscomp::Core::Time &etime) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setEndTime(etime) )
			return false;
	}

	return true;
}

bool BalancedConnection::setTimeWindow(const Seiscomp::Core::TimeWindow &w) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setTimeWindow(w) )
			return false;
	}

	return true;
}

bool BalancedConnection::setRecordType(const char* type) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setRecordType(type) )
			return false;
	}

	return true;
}

bool BalancedConnection::setTimeout(int seconds) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setTimeout(seconds) )
			return false;
	}

	return true;
}

void BalancedConnection::close() {
	boost::mutex::scoped_lock lock(_mtx);

	if ( _rsarray.empty() )
		return;

	for ( size_t i = 0; i < _rsarray.size(); ++i)
		_rsarray[i].first->close();

	_queue.close();

	for ( list<boost::thread *>::iterator it = _threads.begin(); it != _threads.end(); ++it)
		(*it)->join();

	_threads.clear();
	_started = false;
}

void BalancedConnection::acquiThread(RecordStreamPtr rs) {
	SEISCOMP_DEBUG("Starting acquisition thread");

	Record *rec;
	try {
		while ( (rec = rs->next()) )
			_queue.push(rec);
	}
	catch ( OperationInterrupted &e ) {
		SEISCOMP_DEBUG("Interrupted acquisition thread, msg: '%s'", e.what());
	}
	catch ( exception& e ) {
		SEISCOMP_ERROR("Exception in acquisition thread: '%s'", e.what());
	}

	SEISCOMP_DEBUG("Finished acquisition thread");

	_queue.push(NULL);
}

Record *BalancedConnection::next() {
	if ( !_started ) {
		_started = true;

		for ( size_t i = 0; i < _rsarray.size(); ++i) {
			if ( _rsarray[i].second ) {
				_rsarray[i].first->setDataType(_dataType);
				_rsarray[i].first->setDataHint(_hint);
				_threads.push_back(new boost::thread(boost::bind(&BalancedConnection::acquiThread, this, _rsarray[i].first)));
				++_nthreads;
			}
		}
	}

	while (_nthreads > 0) {
		Record *rec = _queue.pop();

		if ( rec == NULL ) {
			--_nthreads;
			continue;
		}

		return rec;
	}

	SEISCOMP_DEBUG("All acquisition threads finished -> finish iteration");

	return NULL;
}


} // namesapce Balanced
} // namespace _private
} // namespace RecordStream
} // namespace Seiscomp

