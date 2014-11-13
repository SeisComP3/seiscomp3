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
 *                                                                         *
 *   Author: Stephan Herrnkind                                             *
 *   Email : herrnkind@gempa.de                                            *
 ***************************************************************************/


#define SEISCOMP_COMPONENT QLClient

#define DEFAULT_PORT                ":18010"
#define MAX_CONTENT_LENGTH          10485760 // 10MiB

#include <sstream>
#include <string>

//#include <seiscomp3/client/application.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/logging/log.h>

#include "connection.h"


using namespace std;

namespace Seiscomp {
namespace IO {
namespace QuakeLink {

namespace {

bool equals(const string &found, const string &expected) {
	return found.length() == expected.length() &&
	       strncmp(found.c_str(), expected.c_str(), expected.length()) == 0;
}

bool startsWith(const string &found, const string &expected) {
	return found.length() >= expected.length() &&
	       strncmp(found.c_str(), expected.c_str(), expected.length()) == 0;
}

string requestFormat(RequestFormat format) {
	return format == rfNative   ? " AS NATIVE" :
	       format == rfGZNative ? " AS GZNATIVE" :
	       format == rfXML      ? " AS XML" :
	       format == rfGZXML    ? " AS GZXML" :
	                              " AS SUMMARY";
}

bool readHeaderValue(string &value, const string &line, const string &key) {
	if ( !startsWith(line, key) ) return false;
	value = line.substr(key.length());
	Core::trim(value);
	return true;
}

ContentType contentType(const string &type) {
	return startsWith(type, "quakelink/xml")   ? ctXML :
	       startsWith(type, "quakelink/evlog") ? ctEvLog :
	       startsWith(type, "quakelink/evsum") ? ctEvSum :
	                                             ctUndefined;
}

string intToString(int i) {
	stringstream ss;
	ss << i;
	return ss.str();
}

} // ns anonymous

const char *SummaryTimeFormat = "%F %T";
const char *RequestTimeFormat = "%Y,%m,%d,%H,%M,%S,%6f";

Connection::Connection() : _sock(NULL), _options(opIgnore) {}

Connection::~Connection() {
	if ( _sock ) {
		disconnect();
		delete _sock;
		_sock = NULL;
	}
}

bool Connection::connected() const {
	return _sock && _sock->isOpen();
}

bool Connection::init(const string &url, int options) {
	SEISCOMP_DEBUG("%sinitializing service with URL: %s",
	               _logPrefix.c_str(), url.c_str());

	disconnect();
	delete _sock;
	_sock = NULL;
	setOptions(options);

	// parse URL, e.g. qls://user:pass@host:port

	// step 1: protocol
	size_t pos;
	string protocol;
	string connection;
	pos = url.find_first_of("://");
	if ( pos == string::npos ) {
		protocol = "ql";
		connection = url;
	}
	else {
		protocol = url.substr(0, pos);
		connection = url.substr(pos + 3);
	}

	bool ssl = protocol == "qls";
	if ( !ssl && protocol != "ql" ) {
		SEISCOMP_ERROR("%sunsupported protocol: %s",
		               _logPrefix.c_str(), protocol.c_str());
		return false;
	}

	// step 2: user:pass
	vector<string> tokens;
	if ( Core::split(tokens, connection.c_str(), "@") >= 2 ) {
		string login = tokens[0];
		_service = tokens[1];

		Core::split(tokens, login.c_str(), ":");
		_user = tokens.size() > 0 ? tokens[0] : "";
		_pass = tokens.size() > 1 ? tokens[1] : "";
	}
	else
		_service = connection;

	// step 3: host:port
	_service = _service.substr(0, _service.find_first_of('/'));
	if ( _service.find(':') == string::npos )
		_service += DEFAULT_PORT;

	_sock = ssl ? new SSLSocket() : new Socket();

	SEISCOMP_DEBUG("%sservice initialized: %s, ssl=%i",
	               _logPrefix.c_str(), _service.c_str(), ssl);

	return true;
}

void Connection::disconnect() {
	if ( connected() ) {
		_sock->close();
		SEISCOMP_DEBUG("%sconnection to %s closed",
		               _logPrefix.c_str(), _service.c_str());
	}
}

bool Connection::setOptions(int options) {
	if ( _options == options ) return true;
	int oldOptions = _options;
	_options = options;
	// if connected updated only those options which have been changed
	return !connected() || sendOptions(oldOptions ^ _options);
}

bool Connection::getUpdates(Response &response, const std::string &eventid) {
	if ( !connect() ) return false;

	// send request, format is restricted to SUMMARY
	string req = "GET UPDATES OF EVENT " + eventid + requestFormat(rfSummary);
	if ( !sendRequest(req) ) {
		response.data = "Error sending data request";
		return false;
	}

	// evaluate response code
	string code;
	if ( !readResponseCode(code) ) return false;

	// data found
	if ( startsWith(code, "DATA/GET 200") ) {
		return readResponse(response);
	}

	// no data available
	if ( startsWith(code, "DATA/GET 404") )
		assertLineBreak();
	else
		logInvalidResp("DATA/GET 200", code.c_str());
	return false;
}

bool Connection::get(Response &response, const std::string &eventId,
                     int revision, RequestFormat format) {
	if ( !connect() ) return false;

	// send request for either the latest or a specific revision
	string req = revision < 0 ? "GET EVENT " :
	             "GET UPDATE " + intToString(revision) + " OF EVENT ";
	if ( !sendRequest(req + eventId + requestFormat(format)) ) {
		response.data = "Error sending data request";
		return false;
	}

	// evaluate response code
	string code;
	if ( !readResponseCode(code) ) return false;

	// data found
	if ( startsWith(code, "DATA/GET 200") ) {
		return readResponse(response);
	}

	// no data available
	if ( startsWith(code, "DATA/GET 404") )
		assertLineBreak();
	else
		logInvalidResp("DATA/GET 200", code.c_str());
	return false;
}

bool Connection::selectArchived(Responses &responses, const Core::Time &from,
                                const Core::Time &to, RequestFormat format,
                                const std::string &where) {
	responses.clear();
	if ( !connect() ) return false;

	// send request
	string req = "SELECT ARCHIVED EVENTS";
	if ( from ) req += " FROM " + from.toString(RequestTimeFormat);
	if ( to ) req += " TO " + to.toString(RequestTimeFormat);
	req += requestFormat(format);
	if ( where.size() > 0 ) req += " WHERE " + where;

	if ( !sendRequest(req) )
		return false;

	// read responses
	string code;
	while ( readResponseCode(code) ) {
		if ( startsWith(code, "EOD/SELECT/ARCHIVED") ) {
			if ( !readResponseCode(code) ) break;
			if ( !equals(code, "EOD/SELECT") ) {
				logInvalidResp("EOD/SELECT", code.c_str());
				break;
			}
			return true;
		}

		if ( !startsWith(code, "DATA/SELECT/ARCHIVED 200") ) {
			logInvalidResp("DATA/SELECT/ARCHIVED 200", code.c_str());
			break;
		}

		responses.resize(responses.size() + 1);
		if ( !readResponse(responses.back()) ) {
			responses.resize(responses.size() - 1);
			return false;
		}
		if ( !responses.back().timestamp.valid() ) {
			responses.resize(responses.size() - 1);
			SEISCOMP_WARNING("%sinvalid timestamp in archived data, skipping",
			                 _logPrefix.c_str());
		}
	}

	return false;
}

bool Connection::select(bool archived, const Core::Time &from,
                        const Core::Time &to, RequestFormat format,
                        const std::string &where, int updatedBufferSize) {
	if ( !connect() ) return false;

	// send request
	string req = archived ? "SELECT EVENTS" : "SELECT UPDATED EVENTS";
	if ( from ) req += " FROM " + from.toString(RequestTimeFormat);
	if ( to ) req += " TO " + to.toString(RequestTimeFormat);
	req += requestFormat(format);
	if ( where.size() > 0 ) req += " WHERE " + where;

	if ( !sendRequest(req) )
		return false;

	// cache of updates received during selection of archived events
	ResponsesPtr updatesBetween;
	Response *response;
	string code;
	while ( readResponseCode(code) ) {
		if ( !archived && equals(code, "EOD/SELECT") ) {
			SEISCOMP_NOTICE("%sEOD/SELECT", _logPrefix.c_str());
			return true;
		}

		if ( archived && startsWith(code, "EOD/SELECT/ARCHIVED") ) {
			archived = false;
			SEISCOMP_NOTICE("%sEOD/SELECT/ARCHIVED", _logPrefix.c_str());
			if ( updatesBetween.size() > 0 ) {
				SEISCOMP_INFO("%sprocessing %lu updates received in between",
				              _logPrefix.c_str(),
				              (unsigned long) updatesBetween.size());
				for ( ResponsesPtr::iterator it = updatesBetween.begin();
				      it != updatesBetween.end(); ++it )
					processResponse(*it);
				updatesBetween.clear();
			}
			SEISCOMP_INFO("%swaiting for data updates", _logPrefix.c_str());
			continue;
		}

		if ( !startsWith(code, "DATA/SELECT/") ) {
			logInvalidResp("DATA/SELECT/*", code.c_str());
			break;
		}

		response = new Response();
		if ( readResponse(*response) ) {
			if ( archived && startsWith(code, "DATA/SELECT/ARCHIVED 200") ) {
				processResponse(response);
				continue;
			}
			else if ( startsWith(code, "DATA/SELECT/UPDATED 200") ) {
				response->timestamp = Core::Time::GMT();

				// archived data processed or cache disabled: sent update immediately
				if ( !archived || updatedBufferSize < 0 ) {
					processResponse(response);
					continue;
				}

				// intermediate update, try to buffer
				if ( updatesBetween.size() < (unsigned) updatedBufferSize ) {
					updatesBetween.push_back(response);
					continue;
				}

				SEISCOMP_WARNING("%sreceived to many updates while still "
				                 "proccessing archived events, ignoring "
				                 "archived data now", _logPrefix.c_str());
			}
			else
				SEISCOMP_WARNING("%sunsupported DATA/SELECT mode: %s",
				                 _logPrefix.c_str(), code.c_str());
		}
		delete response;
	}
	for ( ResponsesPtr::iterator it = updatesBetween.begin();
	      it != updatesBetween.end(); ++it )
		delete *it;
	return false;
}

bool Connection::abort() {
	if ( connected() )
		return sendRequest("ABORT");
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// Private
////////////////////////////////////////////////////////////////////////////////

bool Connection::connect() {
	if ( connected() ) return true;
	if ( !_sock ) {
		SEISCOMP_ERROR("%sinstance not initialized", _logPrefix.c_str());
		return false;
	}

	try { _sock->open(_service); }
	catch ( SocketException &se ) {
		SEISCOMP_ERROR("%scould not connect to service '%s': %s ",
		               _logPrefix.c_str(), _service.c_str(), se.what());
		return false;
	}

	if ( _user.empty() ) {
		SEISCOMP_DEBUG("%sskipping authentication", _logPrefix.c_str());
	}
	else {
		SEISCOMP_DEBUG("%sperfoming authentication", _logPrefix.c_str());
		if ( sendRequest("auth " + _user + " " + _pass, false) )
			SEISCOMP_DEBUG("%sauthentication successful", _logPrefix.c_str());
		else
			SEISCOMP_ERROR("%scould not authenticate to service %s",
			               _logPrefix.c_str(), _service.c_str());
	}

	return connected() && sendOptions(opAll);
}

bool Connection::sendRequest(const string& req, bool log) {
	SEISCOMP_DEBUG("%ssending request: %s", _logPrefix.c_str(),
	               log ? req.c_str() : "***");
	try {
		_sock->write(req + "\r\n");
	}
	catch ( SocketException &se ) {
		SEISCOMP_ERROR("%scould not send request '%s': %s", _logPrefix.c_str(),
		               log ? req.c_str() : "***", se.what());
		disconnect();
		return false;
	}
	return true;
}

bool Connection::sendOptions(int changedOptions) {
	if ( changedOptions <= 0 ) return true;
	if ( !connected() ) return false;

	// special case: if defaults are in place they must be always set prior to
	// any other option
	if ( _options & opDefaults && !sendRequest("SET DEFAULTS") ) return false;

	// send only those options which have changed
	return updateOption(opXMLIndent,      "XML.INDENT",      changedOptions) &&
	       updateOption(opDataPicks,      "DATA.PICKS",      changedOptions) &&
	       updateOption(opDataAmplitudes, "DATA.AMPLITUDES", changedOptions) &&
	       updateOption(opDataStaMags,    "DATA.STAMAGS",    changedOptions) &&
	       updateOption(opDataArrivals,   "DATA.ARRIVALS",   changedOptions) &&
	       updateOption(opDataStaMts,     "DATA.STAMTS",     changedOptions) &&
	       updateOption(opDataPreferred,  "DATA.PREFERRED",  changedOptions) &&
	       updateOption(opKeepAlive,      "KEEPALIVE",       changedOptions);
}

bool Connection::updateOption(Options option, const char *cmd,
                             int changedOptions) {
	if ( ! (changedOptions & option ) ) return true;
	const static string req = "SET ";
	return sendRequest(req + cmd + (_options & option ? " ON" : " OFF"));
}

bool Connection::readResponse(Response &response) {
	// reset response object
	response.reset();

	// read header lines
	string line, value;
	while ( readLine(line) != 0 ) {
		if ( readHeaderValue(value, line, "Content-Type:") ) {
			if ( (response.type = contentType(value)) == ctUndefined ) {
				logAndDisconnect("response header: Unsupported Content-Type",
				                 value.c_str());
				return false;
			}
		}
		else if ( readHeaderValue(value, line, "Content-Length:") ) {
			if ( !Core::fromString(response.length, value) ) {
				logAndDisconnect("response header: Invalid Content-Length",
				                 value.c_str());
				return false;
			}
			if ( response.length > MAX_CONTENT_LENGTH ) {
				logAndDisconnect("response header: Content-Length exceeds "
				                 "maximum of %s bytes", value.c_str());
				return false;
			}
		}
		else if ( readHeaderValue(value, line, "Content-Format:") ) {
			response.format = value;
		}
		else if ( readHeaderValue(value, line, "Content-Encoding:") ) {
			response.gzip = startsWith(value, "gzip");
			if ( !response.gzip ) {
				logAndDisconnect("response header: Invalid Content-Encoding",
				                 value.c_str());
				return false;
			}
		}
		else if ( readHeaderValue(value, line, "Content-Timestamp:") ) {
			if ( !response.timestamp.fromString(value.c_str(), "%FT%T.%f") ) {
				logAndDisconnect("response header: Invalid Content-Timestamp",
				                 value.c_str());
				return false;
			}
		}
		else if ( readHeaderValue(value, line, "Content-Revision:") ) {
			int rev;
			if ( !Core::fromString(rev, value.c_str()) ) {
				logAndDisconnect("response header: Invalid Content-Revision",
				                 value.c_str());
				return false;
			}

			response.revision = rev;
		}
		else {
			SEISCOMP_DEBUG("%sreponse header: Unsupported header line: %s",
			               _logPrefix.c_str(), line.c_str());
		}
	}

	// the header must at least contain a content type and length
	if ( response.type == ctUndefined ) {
		logAndDisconnect("response header: Missing Content-Type");
		return false;
	}
	if ( response.length <= 0 ) {
		logAndDisconnect("response header: Missing Content-Length");
		return false;
	}

	// read payload
	if ( !readPayload(response.data, response.length) ) return false;

	// read terminating new line
	return assertLineBreak();
}

size_t Connection::readLine(string &line) {
	line.clear();
	try {
		line = _sock->readline();
	}
	catch ( SocketException& ) {}

	// strip carriage return
	size_t len = line.size();
	if ( len && line[len-1] == '\r' )
		line.resize(--len);
	return len;
}

void Connection::logAndDisconnect(const char *msg, const char *detail) {
	if ( detail != 0 )
		SEISCOMP_ERROR("%s%s: %s, disconnecting", _logPrefix.c_str(), msg, detail);
	else
		SEISCOMP_ERROR("%s%s, disconnecting", _logPrefix.c_str(), msg);
	disconnect();
}

void Connection::logInvalidResp(const char *expected, const char *got) {
	string detail = string("expected: '") + expected + string("', got: '") +
	                got + string("'");
	logAndDisconnect("received unexpected response code", detail.c_str());
}

bool Connection::readResponseCode(std::string &code) {
	while (readLine(code)) {
		if ( code == "ALIVE")
			SEISCOMP_DEBUG("%sreceived ALIVE message", _logPrefix.c_str());
		else {
			SEISCOMP_DEBUG("%sread response code: %s", _logPrefix.c_str(),
			               code.c_str());
			return true;
		}
	}
	logAndDisconnect("received empty response code");
	return false;
}

bool Connection::assertResponseCode(const std::string &expected) {
	string code;
	if ( !readResponseCode(code) ) return false;
	if ( startsWith(code, expected) ) return true;
	logInvalidResp(expected.c_str(), code.c_str());
	return false;
}

bool Connection::assertLineBreak() {
	try {
		string s = _sock->read(1);
		if ( s == "\r") s = _sock->read(1);
		if ( s == "\n") return true;
		logAndDisconnect("could not read line break");
	}
	catch ( SocketException& se) {
		logAndDisconnect("could not read line break", se.what());
	}
	return false;
}

bool Connection::readPayload(string &data, uint n) {
	SEISCOMP_DEBUG("%sreading %u payload bytes", _logPrefix.c_str(), n);

	uint read = 0;
	uint total = 0;
	data.reserve(n);

	while ( total < n ) {
		read = n - total < BUFSIZE ? n - total : BUFSIZE;
		try { data += _sock->read(read); }
		catch ( SocketException& se) {
			logAndDisconnect("could not read response payload", se.what());
			return false;
		}
		total += read;
	}

	if ( total < n ) {
		logAndDisconnect("read incomplete response payload");
		return false;
	}
	return true;
}

} // ns QuakeLink
} // ns IO
} // ns Seiscomp
