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



#define SEISCOMP_COMPONENT SLConnection

#include <cstring>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/io/records/mseedrecord.h>
#include "slconnection.h"

#include <libmseed.h>
/* Seedlink packets consist of an 8-byte Seedlink header ... */
#define HEADSIZE 8
/* ... followed by a 512-byte MiniSEED record */
#define RECSIZE 512
/* ... server terminates a requested time window with the token END */
#define TERMTOKEN "END"
/* ... or in case of problems with ERROR */
#define ERRTOKEN "ERROR"


namespace Seiscomp {
namespace RecordStream {

using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::IO;

const string DefaultHost = "localhost";
const string DefaultPort = "18000";

IMPLEMENT_SC_CLASS_DERIVED(SLConnection,
                           Seiscomp::IO::RecordStream,
                           "SeedLinkConnection");

REGISTER_RECORDSTREAM(SLConnection, "slink");


SLStreamIdx::SLStreamIdx() {}

SLStreamIdx::SLStreamIdx(const string &net, const string &sta, const string &loc,
                         const string &cha)
: _net(net), _sta(sta), _loc(loc), _cha(cha) {}

SLStreamIdx::SLStreamIdx(const string &net, const string &sta, const string &loc,
                         const string &cha, const Time &stime, const Time &etime)
: _net(net), _sta(sta), _loc(loc), _cha(cha)
, _stime(stime), _etime(etime) {}

SLStreamIdx& SLStreamIdx::operator=(const SLStreamIdx &other) {
	if ( this != &other ) {
		this->~SLStreamIdx();
		new(this) SLStreamIdx(other);
	}

	return *this;
}

bool SLStreamIdx::operator<(const SLStreamIdx &other) const {
	if ( _net < other._net )
		return true;
	else if ( _net == other._net ) {
		if ( _sta < other._sta )
			return true;
		else if ( _sta == other._sta ) {
			if ( _loc < other._loc )
				return true;
			else if ( _loc == other._loc ) {
				if ( _cha < other._cha ) return true;
			}
		}
	}

	return false;
}

bool SLStreamIdx::operator==(const SLStreamIdx &other) const {
	return (_net == other._net && _sta == other._sta &&
	        _loc == other._loc && _cha == other._cha);
}

const string &SLStreamIdx::network() const {
	return _net;
}

const string &SLStreamIdx::station() const {
	return _sta;
}

const string &SLStreamIdx::channel() const {
	return _cha;
}

const string &SLStreamIdx::location() const {
	return _loc;
}

string SLStreamIdx::selector() const {
	string loc = _loc;
	string cha = _cha;
	string::size_type pos = loc.find('*',0);

	if (loc.length() > 0) {
		if (pos != string::npos) loc.replace(pos,1,1,'?');
		if (loc.length() < 2) loc.append(2-loc.length(),'?');
	}

	pos = cha.find('*',0);
	if (pos != string::npos) cha.replace(pos,1,1,'?');
	if (cha.length() < 3) cha.append(3-cha.length(),'?');

	string selector = loc + cha + ".D";
	return selector;
}

Time SLStreamIdx::startTime() const {
	return _stime;
}

Time SLStreamIdx::endTime() const {
	return _etime;
}

Time SLStreamIdx::timestamp() const {
	return _timestamp;
}

void SLStreamIdx::setTimestamp(Time &rectime) const {
	if (_timestamp < rectime)
		_timestamp = rectime;
}


SLConnection::StreamBuffer::StreamBuffer() {}

streambuf *SLConnection::StreamBuffer::setbuf(char *s, streamsize n) {
	setp(NULL, NULL);
	setg(s, s, s + n);
	return this;
}

SLConnection::SLConnection()
: RecordStream() {
	_readingData = false;
	_sock.setTimeout(300); // default
	_maxRetries = -1; // default
	_retriesLeft = -1;
	_useBatch = true;
}

SLConnection::SLConnection(string serverloc)
: RecordStream() {
	_readingData = false;
	_sock.setTimeout(300); // default
	_maxRetries = -1; // default
	_retriesLeft = -1;
	_useBatch = true;
}

SLConnection::~SLConnection() {}

bool SLConnection::setSource(const string &source) {
	_useBatch = true;

	size_t pos = source.find('?');
	if ( pos != std::string::npos ) {
		_serverloc = source.substr(0, pos);
		std::string params = source.substr(pos+1);
		std::vector<std::string> toks;
		split(toks, params.c_str(), "&");
		if ( !toks.empty() ) {
			for ( std::vector<std::string>::iterator it = toks.begin();
			      it != toks.end(); ++it ) {
				std::string name, value;

				pos = it->find('=');
				if ( pos != std::string::npos ) {
					name = it->substr(0, pos);
					value = it->substr(pos+1);
				}
				else {
					name = *it;
					value = "";
				}

				if ( name == "timeout" ) {
					unsigned int seconds;
					if ( Core::fromString(seconds, value) )
						_sock.setTimeout(seconds);
					else
						return false;
				}
				else if ( name == "retries" ) {
					if ( !Core::fromString(_maxRetries, value) )
						return false;
				}
				else if ( name == "no-batch" )
					_useBatch = false;
			}
		}
	}
	else
		_serverloc = source;

	// set address defaults if necessary
	if ( _serverloc.empty() || _serverloc == ":" )
		_serverloc = DefaultHost + ":" + DefaultPort;
	else {
		pos = _serverloc.find(':');
		if ( pos == string::npos )
			_serverloc += ":" + DefaultPort;
		else if ( pos == _serverloc.length()-1 )
			_serverloc += DefaultPort;
		else if ( pos == 0 )
			_serverloc.insert(0, DefaultHost);
	}

	_retriesLeft = -1;

	return true;
}

bool SLConnection::clear() {
	this->~SLConnection();
	new(this) SLConnection(_serverloc);

	return true;
}

void SLConnection::close() {
	_sock.interrupt();
	_retriesLeft = -1;
}

bool SLConnection::reconnect() {
	_sock.close();
	_readingData = false;
	--_retriesLeft;
	return true;
}

bool SLConnection::setRecordType(const char* type) {
	return !strcmp(type, "mseed");
}

bool SLConnection::addStream(const string &net, const string &sta,
                             const string &loc, const string &cha) {
	pair<set<SLStreamIdx>::iterator, bool> result;
	result = _streams.insert(SLStreamIdx(net, sta, loc, cha));
	return result.second;
}

bool SLConnection::addStream(const string &net, const string &sta,
                             const string &loc, const string &cha,
                             const Seiscomp::Core::Time &stime,
                             const Seiscomp::Core::Time &etime) {
	pair<set<SLStreamIdx>::iterator, bool> result;
	result = _streams.insert(SLStreamIdx(net, sta, loc, cha, stime, etime));
	return result.second;
}

bool SLConnection::setStartTime(const Seiscomp::Core::Time &stime) {
	_stime = stime;
	return true;
}

bool SLConnection::setEndTime(const Seiscomp::Core::Time &etime) {
	_etime = etime;
	return true;
}

bool SLConnection::setTimeout(int seconds) {
	_sock.setTimeout(seconds);
	return true;
}

void SLConnection::handshake() {
	Util::StopWatch aStopWatch;

	bool batchmode = false;
	if ( _useBatch ) {
		_sock.sendRequest("BATCH",false);
		string response = _sock.readline();

		if (response == "OK") {
			batchmode = true;
			SEISCOMP_INFO("Seedlink server supports BATCH command");
		}
		else
			SEISCOMP_INFO("Seedlink server does not support BATCH command");
	}
	else
		SEISCOMP_INFO("BATCH mode requests disabled");

	for (set<SLStreamIdx>::iterator it = _streams.begin(); it != _streams.end(); ++it) {
		try {
			Time stime = (it->startTime() != Time()) ? it->startTime() : _stime;
			Time etime = (it->endTime() != Time()) ? it->endTime() : _etime;

			// Seedlink does not support microseconds so shift the end of
			// one second if a fraction of a seconds is requested
			if ( etime.microseconds() > 0 )
				etime += Time(1,0);

			if ( it->timestamp().valid() )
				stime = it->timestamp() + Time(1,0);
			else if ( !stime.valid() ) {
				if ( etime > Time::GMT() )
					stime = Time::GMT();
			}

			// Remove microseconds
			stime.setUSecs(0);
			etime.setUSecs(0);

			// Empty time windows are not requested
			if ( stime.valid() && etime.valid() && stime >= etime ) {
				SEISCOMP_DEBUG("Seedlink: ignoring empty request for %s.%s %s %s %s",
				               it->network().data(), it->station().data(),
				               it->selector().data(),
				               stime.toString("%Y,%m,%d,%H,%M,%S").data(),
				               etime.toString("%Y,%m,%d,%H,%M,%S").data());
				continue;
			}

			string timestr;

			if ( stime.valid() ) {
				timestr = stime.toString("%Y,%m,%d,%H,%M,%S");
				if ( etime.valid() )
					timestr += " " + etime.toString("%Y,%m,%d,%H,%M,%S");
			}

			_sock.startTimer();
			_sock.sendRequest("STATION " + it->station() + " " + it->network(), !batchmode);
			SEISCOMP_DEBUG("Seedlink command: STATION %s %s", it->station().c_str(),it->network().c_str());
			_sock.sendRequest("SELECT " + it->selector(), !batchmode);
			SEISCOMP_DEBUG("Seedlink command: SELECT %s", it->selector().c_str());

			if ( timestr.length() > 0 ) {
				_sock.sendRequest("TIME " + timestr, !batchmode);
				SEISCOMP_DEBUG("Seedlink command: TIME %s", timestr.c_str());
			}
			else {
				_sock.sendRequest("DATA", !batchmode);
				SEISCOMP_DEBUG("Seedlink command: DATA");
			}
		} catch (SocketCommandException) {}
	}
	_sock.sendRequest("END",false);

	SEISCOMP_DEBUG("handshake done in %f seconds", (double)aStopWatch.elapsed());
}

Time getEndtime(MSRecord *prec) {
	double diff = 0;
	Time stime = Time((hptime_t)prec->starttime / HPTMODULUS, (hptime_t)prec->starttime % HPTMODULUS);

	if (prec->samprate > 0)
		diff = prec->samplecnt / prec->samprate;

	if (diff == 0)
		return stime;
	return stime + Time(diff);
}

void updateStreams(std::set<SLStreamIdx> &streams, MSRecord *prec) {
	Time rectime = getEndtime(prec);
	string net = prec->network;
	string sta = prec->station;
	string loc = prec->location;
	string cha = prec->channel;

	SLStreamIdx idx(net,sta,loc,cha);
	set<SLStreamIdx>::iterator it = streams.find(idx);
	if (it != streams.end())
		it->setTimestamp(rectime);
}


Record *SLConnection::next() {
	if (_readingData && !_sock.isOpen()) {
		SEISCOMP_DEBUG("Socket is closed -> set stream's eofbit");
		return NULL;
	}

	// _sock.startTimer();
	bool trials = false;

	while ( !_sock.isInterrupted() ) {
		try {
			if ( !_readingData ) {
				if ( _streams.empty() )
					break;

				try {
					if ( _retriesLeft < 0 ) _retriesLeft = _maxRetries;
					_sock.open(_serverloc);
					_sock.startTimer();
					SEISCOMP_DEBUG("Handshaking SeedLink server at %s", _serverloc.c_str());
					handshake();
					_readingData = true;
					_retriesLeft = -1;
				}
				catch ( SocketTimeout &ex ) {
					Core::msleep(500);
					if (_sock.isInterrupted()) {
						_sock.close();
						break;
					}
					reconnect();
					if ( _retriesLeft < 0 && _maxRetries >= 0 ) break;
					trials = true;
					continue;
				}
				catch ( SocketResolveError &ex ) {
					Core::msleep(500);
					if (_sock.isInterrupted()) {
						_sock.close();
						break;
					}
					reconnect();
					if ( _retriesLeft < 0 && _maxRetries >= 0 ) break;
					trials = true;
					continue;
				}
			}

			_sock.startTimer();
			/*** termination? ***/
			_slrecord = _sock.read(strlen(TERMTOKEN));
			if ( !_slrecord.compare(TERMTOKEN) ) {
				_sock.close();
				break;
			}

			_slrecord += _sock.read(strlen(ERRTOKEN)-strlen(TERMTOKEN));
			if ( !_slrecord.compare(ERRTOKEN) ) {
				_sock.close();
				break;
			}
			/********************/

			_slrecord += _sock.read(HEADSIZE+RECSIZE-strlen(ERRTOKEN));
			char *data = const_cast<char *>(_slrecord.c_str());
			if ( !MS_ISVALIDHEADER(data+HEADSIZE) ) {
				SEISCOMP_WARNING("Invalid MSEED record received (MS_ISVALIDHEADER failed)");
				continue;
			}

			MSRecord *prec = NULL;

			if (msr_unpack(data+HEADSIZE,RECSIZE,&prec,0,0) == MS_NOERROR) {
				int samprate_fact = prec->fsdh->samprate_fact;
				int numsamples = prec->fsdh->numsamples;

				updateStreams(_streams,prec);
				msr_free(&prec);

				/* Test for a so-called end-of-detection-record */
				if ( !(samprate_fact == 0 && numsamples == 0) ) {
					istream stream(&_streambuf);
					stream.clear();
					stream.rdbuf()->pubsetbuf(data+HEADSIZE,RECSIZE);

					IO::MSeedRecord *rec = new IO::MSeedRecord();

					try {
						rec->read(stream);
					}
					catch ( ... ) {
						delete rec;
						continue;
					}

					return rec;
				}
			}
			else
				SEISCOMP_WARNING("Could not parse the incoming MiniSEED record. Ignore it.");
		}
		catch ( SocketException &ex ) {
			if ( _sock.tryReconnect() ) {
				if (!trials)
					SEISCOMP_ERROR("SocketException: %s; Try to reconnect",ex.what());
				/* sleep before reconnect */
				Core::msleep(500);
				/**************************/
				if (_sock.isInterrupted()) {
					_sock.close();
					break;
				}
				reconnect();
				if ( _retriesLeft < 0 && _maxRetries >= 0 ) break;
				trials = true;
				continue;
			}
			else {
				_sock.close();
				break;
			}
		}
		catch ( GeneralException & ) {
			_sock.close();
			break;
		}
	}

	return NULL;
}


}
}

