/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#define SEISCOMP_COMPONENT FDSNWSConnection

#include <string>
#include <set>
#include <utility>
#include <limits>
#include <cerrno>
#include <fcntl.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <io.h>
#endif

#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/plugin.h>
#include <seiscomp3/io/records/mseedrecord.h>
#include <libmseed.h>
#include "fdsnws.h"

#ifdef WIN32
#undef min
#undef max
#define posix_read _read
typedef int ssize_t;
#else
#define posix_read read
#endif


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::IO;
using namespace Seiscomp::RecordStream;


REGISTER_RECORDSTREAM(FDSNWSConnection, "fdsnws");
REGISTER_RECORDSTREAM(FDSNWSSSLConnection, "fdsnwss");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FDSNWSConnectionBase::FDSNWSConnectionBase(const char *protocol, IO::Socket *socket, int defaultPort)
: _protocol(protocol)
, _sock(socket)
, _defaultPort(defaultPort)
, _readingData(false)
, _chunkMode(false)
, _remainingBytes(0)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::setSource(const std::string &source) {
	size_t pos = source.find('/');
	if ( pos != string::npos ) {
		_url = source.substr(pos);
		_host = source.substr(0, pos);
	}
	else {
		_url = "/fdsnws/dataselect/1/query";
		_host = source;
	}

	if ( _host.find(':') == string::npos ) {
		_host += ":";
		_host += Core::toString(_defaultPort);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::setRecordType(const char* type) {
	return !strcmp(type, "mseed");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::addStream(const string &net, const string &sta,
                                     const string &loc, const string &cha) {
	pair<set<StreamIdx>::iterator, bool> result;
	result = _streams.insert(StreamIdx(net, sta, loc, cha));
	return result.second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::addStream(const string &net, const string &sta,
                                     const string &loc, const string &cha,
                                     const Seiscomp::Core::Time &stime,
                                     const Seiscomp::Core::Time &etime) {
	pair<set<StreamIdx>::iterator, bool> result;
	result = _streams.insert(StreamIdx(net, sta, loc, cha, stime, etime));
	return result.second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::setStartTime(const Seiscomp::Core::Time &stime) {
	_stime = stime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::setEndTime(const Seiscomp::Core::Time &etime) {
	_etime = etime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::setTimeout(int seconds) {
	_sock->setTimeout(seconds);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::clear() {
	this->~FDSNWSConnectionBase();
	new(this) FDSNWSConnectionBase(_protocol, _sock.get(), _defaultPort);
	setSource(_host + _url);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Hopefully safe to be called from another thread
void FDSNWSConnectionBase::close() {
	_sock->interrupt();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::reconnect() {
	if ( _sock->isOpen() )
		_sock->close();

	_readingData = false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FDSNWSConnectionBase::handshake() {
	string request;

	for ( set<StreamIdx>::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
		SEISCOMP_DEBUG("Request: %s", it->str(_stime, _etime).c_str());
		if ( (it->startTime() == Time() && _stime == Time()) ||
			 (it->endTime() == Time() && _etime == Time()) ) {
			/* invalid time window ignore stream */
			SEISCOMP_WARNING("... has invalid time window -> ignore this request above");
			continue;
		}

		request += it->network() + " " + it->station() + " ";
		if ( it->location().empty() )
			request += "--";
		else
			request += it->location();

		request += " ";
		request += it->channel();
		request += " ";

		if ( it->startTime().valid() )
			request += it->startTime().toString("%FT%T.%f");
		else
			request += _stime.toString("%FT%T.%f");
		request += " ";
		if ( it->endTime().valid() )
			request += it->endTime().toString("%FT%T.%f");
		else
			request += _etime.toString("%FT%T.%f");
		request += "\r\n";
	}

	SEISCOMP_DEBUG("POST %s://%s%s", _protocol, _host.c_str(), _url.c_str());
	SEISCOMP_DEBUG("Sending request:\n%s", request.c_str());

	_sock->sendRequest(string("POST ") + _url + " HTTP/1.1", false);
	_sock->sendRequest(string("Host: ") + _host, false);
	_sock->sendRequest("User-Agent: Mosaic/1.0", false);
	_sock->sendRequest("Content-Type: text/plain", false);
	_sock->sendRequest(string("Content-Length: ") + toString(request.size()), false);
	_sock->sendRequest("", false);
	_sock->write(request);

	string line = _sock->readline();
	if ( line.compare(0, 7, "HTTP/1.") != 0 )
		throw GeneralException(("server sent invalid response: " + line).c_str());

	size_t pos;
	pos = line.find(' ');
	if ( pos == string::npos )
		throw GeneralException(("server sent invalid response: " + line).c_str());

	line.erase(0, pos+1);

	pos = line.find(' ');
	if ( pos == string::npos )
		throw GeneralException(("server sent invalid response: " + line).c_str());

	int code;
	if ( !fromString(code, line.substr(0, pos)) )
		throw GeneralException(("server sent invalid status code: " + line.substr(0, pos)).c_str());

	if ( code == 200 ) {
		// Keep on reading body
	}
	else if ( code == 204 ) {
		// No data
		_remainingBytes = 0;
		return;
	}
	else if ( code == 302 ) {
		// Redirect
	}
	else
		_error = "server request error: " + line;

	_remainingBytes = -1;

	int lc = 0;
	string redirectLocation;

	while ( !_sock->isInterrupted() ) {
		++lc;
		line = _sock->readline();
		if ( line.empty() ) break;

		SEISCOMP_DEBUG("[%02d] %s", lc, line.c_str());

		// Remove whitespaces
		trim(line);

		pos = line.find(':');
		if ( pos != string::npos )
			// Transform header values to upper case
			transform(line.begin(), line.begin()+pos, line.begin(), ::toupper);

		if ( line.compare(0, 18, "TRANSFER-ENCODING:") == 0 ) {
			line.erase(line.begin(), line.begin()+18);
			trim(line);
			if ( line == "chunked" ) {
				_chunkMode = true;
				SEISCOMP_DEBUG(" -> enabled 'chunked' transfer");
			}
		}
		else if ( line.compare(0, 15, "CONTENT-LENGTH:") == 0 ) {
			if ( !fromString(_remainingBytes, line.substr(15)) )
				throw GeneralException("invalid Content-Length response");
			if ( _remainingBytes < 0 )
				throw GeneralException("Content-Length must be positive");
		}
		else if ( line.compare(0, 9, "LOCATION:") == 0 ) {
			redirectLocation = line.substr(pos+1);
			trim(redirectLocation);
		}
	}

	if ( _chunkMode ) {
		if ( _remainingBytes >= 0 )
			throw GeneralException("protocol error: transfer encoding is chunked and content length given");
		_remainingBytes = 0;
	}

	if ( code == 302 ) {
		if ( redirectLocation.empty() ) {
			_error = "Invalid redirect response";
			SEISCOMP_ERROR("302 returned but empty Location header");
		}
		else {
			SEISCOMP_DEBUG("FDSNWS request was redirected to %s", redirectLocation.c_str());
			pos = redirectLocation.find("://");
			if ( pos == string::npos ) {
				if ( redirectLocation[0] == '/' ) {
					redirectLocation = _host + redirectLocation;
					_sock->close();
				}
				else {
					_error = "Invalid redirect location protocol";
					SEISCOMP_ERROR("Redirect URL invalid: %s", redirectLocation.c_str());
					return;
				}
			}
			else {
				if ( redirectLocation.compare(0, pos, "http") == 0 )
					_sock = new IO::Socket;
				else if ( redirectLocation.compare(0, pos, "https") == 0 )
					_sock = new IO::SSLSocket;
				else {
					_error = "Invalid redirect location protocol";
					SEISCOMP_ERROR("Redirect URL invalid: %s", redirectLocation.c_str());
					return;
				}

				redirectLocation.erase(0, pos+3);
			}

			setSource(redirectLocation);
			_sock->open(_host);
			handshake();
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *FDSNWSConnectionBase::next() {
	if ( _readingData && !_sock->isOpen() )
		return NULL;

	_sock->startTimer();

	if ( !_readingData ) {
		_sock->open(_host);
		try {
			handshake();
		}
		catch ( const GeneralException &e ) {
			SEISCOMP_ERROR("fdsnws: %s", e.what());
			_sock->close();
			return NULL;
		}

		_readingData = true;
		if ( !_chunkMode && _remainingBytes <= 0 ) {
			SEISCOMP_DEBUG("Content length is 0, nothing to read");
			_sock->close();
			return NULL;
		}
	}

	try {
		while ( true ) {
			_sock->startTimer();

			if ( _error.empty() ) {
				// HACK to retrieve the record length
				string data = readBinary(RECSIZE);
				if ( !data.empty() ) {
					int reclen = ms_detect(data.c_str(), RECSIZE);
					std::istringstream stream(std::istringstream::in|std::istringstream::binary);
					if ( reclen > RECSIZE )
						stream.str(data + readBinary(reclen - RECSIZE));
					else {
						if ( reclen <= 0 )
							SEISCOMP_ERROR("Retrieving the record length failed (try 512 Byte)!");
						reclen = RECSIZE;
						stream.str(data);
					}

					IO::MSeedRecord *rec = new IO::MSeedRecord;
					setupRecord(rec);
					try {
						rec->read(stream);
					}
					catch ( ... ) {
						delete rec;
						continue;
					}

					return rec;
				}
				else {
					_sock->close();
					break;
				}
			}
			else
				_error += readBinary(_chunkMode?512:_remainingBytes);

			if ( !_sock->isOpen() ) {
				if ( _error.size() )
					throw GeneralException(_error.c_str());
			}
		}
	}
	catch ( const GeneralException &e ) {
		SEISCOMP_ERROR("fdsnws: %s", e.what());
		_sock->close();
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string FDSNWSConnectionBase::readBinary(int size) {
	if ( size <= 0 ) return "";

	string data;
	int bytesLeft = size;

	while ( bytesLeft > 0 ) {
		if ( _chunkMode && _remainingBytes <= 0 ) {
			string r = _sock->readline();
			size_t pos = r.find(' ');
			unsigned int remainingBytes;

			if ( sscanf(r.substr(0, pos).c_str(), "%X", &remainingBytes) !=  1 )
				throw GeneralException((string("invalid chunk header: ") + r).c_str());

			_remainingBytes = remainingBytes;

			if ( _remainingBytes <= 0 ) {
				if ( _error.size() )
					throw GeneralException(_error.c_str());
				_sock->close();
				break;
			}
		}

		int toBeRead = bytesLeft > BUFSIZE ? BUFSIZE : bytesLeft;
		if ( toBeRead > _remainingBytes ) toBeRead = _remainingBytes;

		int bytesRead = (int)data.size();
		data += _sock->read(toBeRead);
		bytesRead = (int)data.size() - bytesRead;
		if ( bytesRead <= 0 ) {
			SEISCOMP_WARNING("socket read returned not data");
			break;
		}

		_remainingBytes -= bytesRead;
		bytesLeft -= bytesRead;

		if ( _remainingBytes <= 0 ) {
			if ( _chunkMode ) {
				// Read trailing new line
				_sock->readline();
			}
			else {
				_sock->close();
			}
		}
	}

	return data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FDSNWSConnection::FDSNWSConnection()
: FDSNWSConnectionBase("http", new IO::Socket, 80) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FDSNWSSSLConnection::FDSNWSSSLConnection()
: FDSNWSConnectionBase("https", new IO::SSLSocket, 443) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
