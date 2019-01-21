/***************************************************************************
 * libcapsclient
 * Copyright (C) 2016  gempa GmbH
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/


#include <gempa/caps/connection.h>
#include <gempa/caps/log.h>
#include <gempa/caps/mseedpacket.h>
#include <gempa/caps/rawpacket.h>
#include <gempa/caps/sessiontable.h>
#include <gempa/caps/utils.h>

#include <boost/bind.hpp>

#include <cstring>
#include <sstream>
#include <iomanip>

#include <sys/time.h>
#include <cerrno>

using namespace std;

namespace Gempa {
namespace CAPS {

namespace {

const Time InvalidTime = Time();

template <typename T>
bool fromString(T &value, const string &str);

template<> inline bool fromString(int &value, const string &str) {
	stringstream ss(str);
	ss >> value;
	return !ss.bad();
}

}

Connection::Connection() {
	_port = 18002;
	_server = "localhost";

	_metaMode = false;
	_realtime = true;
	_ssl = false;

	_sessionTable = SessionTablePtr(new SessionTable());
	_sessionTable->setItemAboutToBeRemovedFunc(
	    boost::bind(&Connection::onItemAboutToBeRemoved, this, _1));

	reset();
}

Connection::~Connection() {
	close();
}

bool Connection::setServer(const string &server) {
	close();
	reset();

	size_t pos = server.rfind(':');
	string addr = server;
	int timeout = 300;

	if ( pos == string::npos )
		_server = addr;
	else {
		_server = addr.substr(0, pos);
		if ( !fromString(_port, addr.substr(pos+1)) ) {
			CAPS_ERROR("invalid source address: %s", addr.c_str());
			return false;
		}
	}

	if ( timeout > 0 ) {
		CAPS_DEBUG("setting socket timeout to %ds", timeout);
		//_socket->setSocketTimeout(timeout,0);
	}

	return true;
}

void Connection::setCredentials(const std::string &user,
                                const std::string &password) {
	if ( user.empty() || password.empty() ) {
		_auth.clear();
		CAPS_DEBUG("authentication deactivated");
	}
	else {
		_auth = "AUTH " + user + " " + password;
		CAPS_DEBUG("credentials set: %s:***", user.c_str());
	}
}

Socket* Connection::createSocket() const {
#if !defined(CAPS_FEATURES_SSL) || CAPS_FEATURES_SSL
	return _ssl? new SSLSocket() : new Socket();
#else
	return new Socket();
#endif
}

void Connection::disconnect() {
	if ( _socket == NULL ) return;

	CAPS_DEBUG("disconnecting");
	if ( _state != Aborted )
		_state = Error;

	_socket->shutdown();
	_socket->close();
	_sessionTable->reset();
	_currentID = - 1;
	_currentItem = NULL;
}

void Connection::close() {
	boost::mutex::scoped_lock l(_mutex);
//	_state = Aborted;
	disconnect();
	_state = Aborted;
}

void Connection::abort() {
	boost::mutex::scoped_lock l(_mutex);
	if ( _state == Aborted ) {
		CAPS_WARNING("abort already requested");
		return;
	}

	if ( _socket->isValid() && _state == Active ) {
		static string line("ABORT\n");
		if ( _socket->write(line.c_str(), line.size()) != (int) line.size() ) {
			CAPS_ERROR("could not send abort request");
			disconnect();
		}
		else {
			CAPS_DEBUG("abort command sent");
		}
	}
	_state = Aborted;
}

bool Connection::setTimeout(int /*seconds*/) {
	return false;
}

bool Connection::addStream(const string &net, const string &sta,
                           const string &loc, const string &cha) {
	return addRequest(net, sta, loc, cha, _startTime, _endTime, false);
}

bool Connection::addStream(const string &net, const string &sta,
                           const string &loc, const string &cha,
                           const Time &stime,
                           const Time &etime) {
	return addRequest(net, sta, loc, cha, stime, etime, false);
}

bool Connection::addRequest(const string &net, const string &sta,
                            const string &loc, const string &cha,
                            const Time &stime,
                            const Time &etime,
                            bool receivedData) {
	boost::mutex::scoped_lock l(_mutex);
	if ( _state != EOD ) {
		if ( _state == Active )
			CAPS_WARNING("cannot add streams to an active session, invoke "
			             "abort() or close() first");
		else
			CAPS_WARNING("cannot add streams to an erroneous or aborted "
			             "session, invoke reset() first");
		return false;
	}
	string streamID = net + "." + sta + "." + loc + "." + cha;
	Request &req = _requests[streamID];
	req.net = net;
	req.sta = sta;
	req.loc = loc;
	req.cha = cha;
	req.start = stime;
	req.end = etime;
	req.receivedData = receivedData;
	return true;
}

DataRecord* Connection::next() {
	if ( !handshake() ) return NULL;

	while ( true ) {
		{
			boost::mutex::scoped_lock l(_mutex);
			// skip unread bytes of previous iteration and check connection state
			if ( !seekToReadLimit() || _state == Error || _state == EOD )
				return NULL;
		}

		ResponseHeader responseHeader;
		if ( !responseHeader.get(_socketBuf) ) {
			boost::mutex::scoped_lock l(_mutex);
			if ( _state != Aborted)
				CAPS_ERROR("could not read header");
			disconnect();
			return NULL;
		}

		CAPS_DEBUG("read header (id/size): %i/%lu", responseHeader.id,
		           (unsigned long)responseHeader.size);
		_socketBuf.set_read_limit(responseHeader.size);

//		if ( _abortRequested ) break;

		// State or session table update
		if ( responseHeader.id == 0 ) {
			while ( responseHeader.size > 0 /*&& _state == Active*/) {
				istream is(&_socketBuf);
				if ( is.getline(_lineBuf, 200).fail() ) {
					boost::mutex::scoped_lock l(_mutex);
					CAPS_ERROR("header line exceeds maximum of 200 characters");
					disconnect();
					return NULL;
				}
				responseHeader.size -= is.gcount();

				SessionTable::Status status =
				        _sessionTable->handleResponse(_lineBuf, is.gcount());
				if ( status == SessionTable::Error ) {
					boost::mutex::scoped_lock l(_mutex);
					disconnect();
					return NULL;
				}

				if ( status == SessionTable::EOD ) {
					_state = EOD;
					break;
				}
			}

			continue;
		}

		CAPS_DEBUG("reading data record");
		// data
		if ( _currentID != responseHeader.id ) {
			_currentItem = _sessionTable->getItem(responseHeader.id);
			_currentID = responseHeader.id;

			if ( _currentItem == NULL ) {
				CAPS_WARNING("unknown data request ID %d", responseHeader.id);
				continue;
			}
		}

		int size = responseHeader.size;

		// To improve performance CAPS uses an optimized protocol
		// to deliver RAW packets. The RAW packet class can't be used
		// to read the header from socket.
		DataRecord *dataRecord = NULL;
		if ( _currentItem->type == RawDataPacket ) {
			// Read optimized header
			RawResponseHeader rawResponseHeader;
			if ( !rawResponseHeader.get(_socketBuf) ) {
				boost::mutex::scoped_lock l(_mutex);
				CAPS_ERROR("failed to extract raw response header %s",
				           _currentItem->streamID.c_str());
				disconnect();
				return NULL;
			}

			Time time(rawResponseHeader.timeSeconds, rawResponseHeader.timeMicroSeconds);
			size -= rawResponseHeader.dataSize();

			// Create raw record, set header and read payload
			RawDataRecord *record = new RawDataRecord;
			record->setStartTime(time);
			record->setSamplingFrequency(_currentItem->samplingFrequency,
			                             _currentItem->samplingFrequencyDivider);
			record->setDataType(_currentItem->dataType);
			record->getData(_socketBuf, size);

			dataRecord =  record;
		}
		else if ( _currentItem->type == MSEEDPacket ) {
			MSEEDDataRecord *record = new MSEEDDataRecord;
			record->get(_socketBuf, size, InvalidTime, InvalidTime, size);
			dataRecord = record;
		}
		else {
			CAPS_ERROR("received unknown packet type %d in stream %s",
			           _currentItem->type, _currentItem->streamID.c_str());
		}

		if ( dataRecord == NULL ) {
			boost::mutex::scoped_lock l(_mutex);
			disconnect();
			return NULL;
		}

		CAPS_DEBUG("data record read");

		RequestList::iterator it = _requests.find(_currentItem->streamID);
		if ( it == _requests.end() ) {
			// TODO: Search request map for wildcard match. Add entry with
			// streamID -> (record->endTime, wildcardItem->endTime) to
			// restrict data query in case of reconnect

//			CAPS_WARNING("received unrequested record: %s: %s - %s",
//			             _currentItem->streamID.c_str(),
//			             dataRecord->startTime().iso().c_str(),
//			             dataRecord->endTime().iso().c_str());
		}
		// Update request map to reflect current stream state
		else if ( dataRecord->endTime() > it->second.start ) {
			it->second.start = dataRecord->endTime();
			it->second.receivedData = true;
		}

		return dataRecord;
	}

	return NULL;
}

bool Connection::handshake() {
	{
		boost::mutex::scoped_lock l(_mutex);
		if ( _state == Error || _state == Aborted) {
			CAPS_ERROR("cannot read from an erroneous or aborted session, invoke "
			           "reset() first");
			return false;
		}
		else if ( _state == Active )
			return true;
		// _state is set to EOD
		else if ( _requests.empty() ) {
			CAPS_WARNING("no stream requested, invoke addStream() first");
			return false;
		}
	}

	if ( _socket == NULL ) {
		_socket = SocketPtr(createSocket());
		if ( _socket == NULL ) return false;
	}

	// Connect to server if necessary
	while ( !_socket->isValid() && _state == EOD ) {
		if ( _socket->connect(_server, _port) == Socket::Success ) {
			_socketBuf.setsocket(_socket.get());
			CAPS_DEBUG("connection to %s:%d established", _server.c_str(), _port);
//			_state = ios_base::eofbit;
			break;
		}

		CAPS_WARNING("unable to connect to %s:%d, retrying in 5 seconds",
				     _server.c_str(), _port);

		// Wait 5 seconds and keep response latency low
		for ( int i = 0; (i < 10) && _state == EOD; ++i )
			usleep(500000);
	}


	if ( _state != EOD ) return false;

//	if ( _socket->isValid() ) {
//		// Read all data from session
//		if ( !seekToReadLimit() ) return false;
////		fd_set set;
////		FD_ZERO(&set);
////		FD_SET(_socket->fd(), &set);
////		timeval t = (struct timeval) {0};

////		/* select returns 0 on timeout, 1 if input/output is available, -1 on error. */
////		int retn = TEMP_FAILURE_RETRY(select(_socket->fd()+1, &set, NULL, NULL, &t));
////		if ( retn != 0 )  CAPS_ERROR("UUPS");
//	}

//	if ( _state == Aborted ) return false;

	// Request streams
	stringstream req;

	if ( !_auth.empty() )
		req << _auth << endl;

	req << "BEGIN REQUEST" << endl
	    << "META " << (_metaMode ? "ON" : "OFF") << endl
		<< "REALTIME " << ( _realtime ? "ON" : "OFF") << endl;

	// First pass: continue all previous streams
	for ( RequestList::const_iterator it = _requests.begin();
	      it != _requests.end() && _state == EOD; ++it ) {
		if ( it->second.receivedData )
			formatRequest(req, it);
	}

	// Second pass: subscribe to remaining streams
	for ( RequestList::iterator it = _requests.begin();
	      it != _requests.end() && _state == EOD; ++it ) {
		if ( !it->second.receivedData )
			formatRequest(req, it);
	}

	req << "END" << endl;

	return sendRequest(req.str());
}

bool Connection::sendRequest(const string &req) {
	boost::mutex::scoped_lock l(_mutex);

	if ( _state != EOD ) return false;

	CAPS_DEBUG("%s", req.c_str());
	if ( _socket->write(req.c_str(), req.size()) != (int) req.size() ) {
		CAPS_ERROR("could not send data request");
		disconnect();
		return false;
	}

	Gempa::CAPS::ResponseHeader header;
	if ( !header.get(_socketBuf) ) {
		// Case to retry, connection closed by peer
		CAPS_ERROR("could not read data request response header");
		disconnect();
		return false;
	}

	_socketBuf.set_read_limit(header.size);
	if ( header.id != 0 ) {
		CAPS_ERROR("invalid data request response header id, expected 0, got %d", header.id);
		disconnect();
		return false;
	}
	CAPS_DEBUG("data request response size: %lu", (unsigned long) header.size);

	istream is(&_socketBuf);
	// check line length
	if ( is.getline(_lineBuf, 200).fail() )
		CAPS_ERROR("data request response line exceeds maximum of 200 characters");
	// skip remaining header data
	else if ( !seekToReadLimit(false) ) {
		CAPS_ERROR("could not seek to data request response header end");
		return false;
	}

	if ( strncasecmp(_lineBuf, "ERROR:", 6) == 0 )
		CAPS_ERROR("server responded to data request with: %s", _lineBuf);
	else if ( strncasecmp(_lineBuf, "STATUS OK", 9) != 0 )
		CAPS_ERROR("invalid data request response: %s", _lineBuf);
	else {
		CAPS_DEBUG("handshake complete");
		_state = Active;
		return true;
	}

	disconnect();
	return false;
}


bool Connection::seekToReadLimit(bool log) {
	if ( !_socket->isValid() ) {
		disconnect();
		return false;
	}

	// Skip unread bytes from previous record
	int skippies = _socketBuf.read_limit();
	if ( skippies > 0 ) {
		if ( log )
			CAPS_WARNING("no seemless reading, skipping %d bytes", skippies);
		if ( _socketBuf.pubseekoff(skippies, ios_base::cur, ios_base::in) < 0 ) {
			CAPS_ERROR("could not seek to next header");
			disconnect();
			return false;
		}
	}
	_socketBuf.set_read_limit(-1);
	return true;
}

void Connection::formatRequest(stringstream& req, RequestList::const_iterator it) {
	req << "STREAM ADD " << it->first << endl;
	req << "TIME ";

	int year, mon, day, hour, minute, second;

	if ( it->second.start.valid() ) {
		it->second.start.get(&year, &mon, &day, &hour, &minute, &second);
		req << year << "," << mon << "," << day << ","
		    << hour << "," << minute << "," << second;
		if ( it->second.start.microseconds() > 0 ) {
			req << "," << setfill('0') << setw(6)
			    << it->second.start.microseconds() << setw(0);
		}
	}

	req << ":";

	if ( it->second.end.valid() ) {
		it->second.end.get(&year, &mon, &day, &hour, &minute, &second);
		req << year << "," << mon << "," << day << ","
		    << hour << "," << minute << "," << second;
		if ( it->second.end.microseconds() > 0 ) {
			req << "," << setfill('0') << setw(6)
			    << it->second.end.microseconds() << setw(0);
		}
	}

	req << endl;
}

void Connection::onItemAboutToBeRemoved(const SessionTableItem *item) {
	if ( _currentItem == item) {
		_currentID = -1;
		_currentItem = NULL;
	}

	// remove request since all data has been received
	if ( item != NULL ) _requests.erase(item->streamID);
}

void Connection::reset(bool clearStreams) {
	boost::mutex::scoped_lock l(_mutex);
	CAPS_DEBUG("resetting connection");
	if ( _state == Active ) {
		CAPS_WARNING("cannot reset an active connection, invoking close()");
		close();
	}
//	else if ( _state == Aborted ) {
//		fd_set set;
//		FD_ZERO(&set);
//		FD_SET(_socket->fd(), &set);
//		timeval t = (struct timeval) {0};

//		/* select returns 0 on timeout, 1 if input/output is available, -1 on error. */
//		while ( _socket->isValid() ) {
//			int retn = TEMP_FAILURE_RETRY(select(_socket->fd()+1, &set, NULL, NULL, &t));
//			if ( retn == 0 ) break;

//				if ( retn != 0 )  CAPS_ERROR("UUPS");

//	}

	_state = EOD;
	if ( clearStreams )
		_requests.clear();
}

void Connection::setStartTime(const Time &stime) {
	_startTime = stime;
	CAPS_DEBUG("set global start time to %s", _startTime.toString("%F %T.%f").c_str());
}

void Connection::setEndTime(const Time &etime) {
	_endTime = etime;
	CAPS_DEBUG("set global end time to %s", _endTime.toString("%F %T.%f").c_str());
}

void Connection::setTimeWindow(const Time &stime, const Time &etime) {
	_startTime = stime;
	_endTime = etime;
	CAPS_DEBUG("set global timewindow to %s~%s", _startTime.toString("%F %T.%f").c_str(),
	           _endTime.toString("%F %T.%f").c_str());
}


}
}
