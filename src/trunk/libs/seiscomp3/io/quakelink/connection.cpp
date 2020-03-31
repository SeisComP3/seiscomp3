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

string requestFormat(RequestFormatVersion formatVersion) {
	stringstream ss;
	ss << (formatVersion == rfNative   ? " AS NATIVE" :
	       formatVersion == rfGZNative ? " AS GZNATIVE" :
	       formatVersion == rfXML      ? " AS XML" :
	       formatVersion == rfGZXML    ? " AS GZXML" :
	                                     " AS SUMMARY");
	if ( formatVersion.version() > 1 )
		ss << "/" << formatVersion.version();
	return ss.str();
}

inline
bool orderByLimitOffset(stringstream &req, Version api, OrderBy orderBy,
                        unsigned long limit, unsigned long offset) {
	if ( (orderBy || limit || offset) && !api ) {
		SEISCOMP_WARNING("Order by, limit and offset filter not supported by "
		                 "server API 0");
		return false;
	}

	if ( orderBy == obOTimeAsc )
		req << " ORDER BY OTIME ASC";
	else if ( orderBy == obOTimeDesc )
		req << " ORDER BY OTIME DESC";

	if ( limit ) {
		req << " LIMIT " << limit;
		if ( offset )
			req << " OFFSET " << offset;
	}

	return true;
}

bool readHeaderValue(string &value, const string &line, const string &key) {
	if ( !startsWith(line, key) ) return false;
	value = line.substr(key.length());
	Core::trim(value);
	return true;
}

ContentType contentType(const string &type) {
	return startsWith(type, "quakelink/xml")   ? ctXML :
	       startsWith(type, "quakelink/evsum") ? ctEvSum :
	       startsWith(type, "quakelink/evlog") ? ctEvLog :
	       startsWith(type, "text/plain")      ? ctText :
	                                             ctUndefined;
}

string intToString(int i) {
	stringstream ss;
	ss << i;
	return ss.str();
}

FormatAPIMap createAPIMap() {
	FormatAPIMap map;
	map[rfSummary]  = APIList();
	map[rfXML]      = APIList();
	map[rfGZXML]    = APIList();
	map[rfNative]   = APIList();
	map[rfGZNative] = APIList();

	// Summary format 2 added in Server API 2
	map[rfSummary].push_back(2);

	return map;
}

const FormatAPIMap APIMap = createAPIMap();

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
	disconnect();
	delete _sock;
	_sock = NULL;
	setOptions(options);

	// parse URL, e.g. qls://user:pass@host:port

	// step 1: protocol
	size_t pos;
	string protocol;
	string connection;
	pos = url.find("://");
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
	string logUserPass;
	if ( Core::split(tokens, connection.c_str(), "@") >= 2 ) {
		string login = tokens[0];
		_service = tokens[1];

		Core::split(tokens, login.c_str(), ":");
		_user = tokens.size() > 0 ? tokens[0] : "";
		_pass = tokens.size() > 1 ? tokens[1] : "";
		logUserPass = _user + ":***";
	}
	else
		_service = connection;

	// step 3: host:port
	_service = _service.substr(0, _service.find('/'));
	if ( _service.find(':') == string::npos )
		_service += DEFAULT_PORT;

	_sock = ssl ? new SSLSocket() : new Socket();

	SEISCOMP_INFO("%sservice initialized: %s://%s%s",
	              _logPrefix.c_str(), protocol.c_str(), logUserPass.c_str(),
	              _service.c_str());

	return true;
}

void Connection::disconnect() {
	if ( connected() ) {
		_sock->close();
		SEISCOMP_DEBUG("%sconnection to %s closed",
		               _logPrefix.c_str(), _service.c_str());
	}
	_serverID.clear();
	_serverAPI = 0;
}

bool Connection::hello(std::string &id, Version &api) {
	id.clear();
	api = 0;

	if ( !connect() || !sendRequest("HELLO") )
		return false;

	// read header lines
	string line, tmpID, tmpAPI;
	bool welcome = false;
	for ( size_t i = 0; i < 20 && readLine(line) != 0; ++i) {
		if ( startsWith(line, "QuakeLink") )
			tmpID = line;
		else if ( startsWith(line, "API=") )
			tmpAPI = line;
		else if ( startsWith(line, "WELCOME") ) {
			welcome = true;
			break;
		}
	}

	if ( !welcome || !assertLineBreak() ) {
		logAndDisconnect("HELLO: Server did not respond with 'WELCOME' "
		                 "followed by new line in first 20 lines of response");
		return false;
	}

	if ( !tmpAPI.empty() &&
	     !Core::fromString(api, Core::trim(tmpAPI).substr(4)) ) {
		logAndDisconnect("HELLO: Server responded with non numeric API "
		                 "version string");
		return false;
	}

	id = tmpID;
	return true;
}

bool Connection::setOptions(int options) {
	if ( _options == options ) return true;
	int oldOptions = _options;
	_options = options;
	// if connected updated only those options which have been changed
	return !connected() || sendOptions(oldOptions ^ _options);
}

bool Connection::getUpdates(Response &response, const std::string &eventid,
                            const RequestFormatVersion &formatVersion) {
	if ( !connect() ) return false;

	if ( !checkFormatVersion(response.data, formatVersion) ) {
		return false;
	}

	// send request, format is restricted to SUMMARY
	string req = "GET UPDATES OF EVENT " + eventid + requestFormat(formatVersion);
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
                     int revision, const RequestFormatVersion &formatVersion) {
	if ( !connect() ) return false;

	if ( !checkFormatVersion(response.data, formatVersion) ) {
		return false;
	}

	// send request for either the latest or a specific revision
	string req = revision < 0 ? "GET EVENT " :
	             "GET UPDATE " + intToString(revision) + " OF EVENT ";
	if ( !sendRequest(req + eventId + requestFormat(formatVersion)) ) {
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
                                const Core::Time &to,
                                const RequestFormatVersion &formatVersion,
                                const std::string &where, OrderBy orderBy,
                                unsigned long limit, unsigned long offset) {
	responses.clear();
	if ( !connect() || !isSupported(formatVersion, true) )
		return false;

	// send request
	stringstream req;
	req << "SELECT ARCHIVED EVENTS";
	if ( from )
		req << " FROM " << from.toString(RequestTimeFormat);
	if ( to )
		req << " TO " + to.toString(RequestTimeFormat);
	req << requestFormat(formatVersion);
	if ( where.size() > 0 )
		req << " WHERE " + where;

	if ( !orderByLimitOffset(req, _serverAPI, orderBy, limit, offset) ||
	     !sendRequest(req.str()) )
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
		if ( responses.back().type == ctXML && !responses.back().timestamp.valid() ) {
			responses.resize(responses.size() - 1);
			SEISCOMP_WARNING("%sinvalid timestamp in archived data, skipping",
			                 _logPrefix.c_str());
		}
	}

	return false;
}

bool Connection::select(bool archived, const Core::Time &from,
                        const Core::Time &to,
                        const RequestFormatVersion &formatVersion,
                        const std::string &where, int updatedBufferSize,
                        OrderBy orderBy, unsigned long limit,
                        unsigned long offset) {

	if ( !connect() || !isSupported(formatVersion, true) )
		return false;

	// send request
	stringstream req;
	req << (archived ? "SELECT EVENTS" : "SELECT UPDATED EVENTS");
	if ( from )
		req << " FROM " << from.toString(RequestTimeFormat);
	if ( to )
		req << " TO " << to.toString(RequestTimeFormat);
	req << requestFormat(formatVersion);
	if ( where.size() > 0 )
		req << " WHERE " << where;

	if ( !orderByLimitOffset(req, _serverAPI, orderBy, limit, offset) ||
	     !sendRequest(req.str()) )
		return false;

	// cache of updates received during selection of archived events
	ResponsesPtr updatesBetween;
	Response *response;
	string code;
	while ( readResponseCode(code) ) {
		if ( !archived && equals(code, "EOD/SELECT") ) {
			SEISCOMP_DEBUG("%sEOD/SELECT", _logPrefix.c_str());
			return true;
		}

		if ( archived && startsWith(code, "EOD/SELECT/ARCHIVED") ) {
			archived = false;
			SEISCOMP_DEBUG("%sEOD/SELECT/ARCHIVED", _logPrefix.c_str());
			if ( updatesBetween.size() > 0 ) {
				SEISCOMP_INFO("%sprocessing %lu updates received in between",
				              _logPrefix.c_str(),
				              static_cast<unsigned long>(updatesBetween.size()));
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
				if ( updatesBetween.size() < static_cast<unsigned long>(updatedBufferSize) ) {
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

const string& Connection::serverID() {
	connect();
	return _serverID;
}

Version Connection::serverAPI() {
	connect();
	return _serverAPI;
}

bool Connection::isSupported(const RequestFormatVersion &formatVersion,
                             bool log) {
	if ( !connect() )
		return false;

	const Version &version = formatVersion.version();
	if ( version == 1 )
		return true;

	const FormatAPIMap::const_iterator it = APIMap.find(formatVersion);
	if ( it != APIMap.end() && version - 2 < it->second.size() &&
	     it->second[version - 2] <= _serverAPI )
		return true;

	if ( log ) {
		SEISCOMP_WARNING("Format version %u not supported by server", version);
	}
	return false;
}

Version Connection::maximumSupportedVersion(RequestFormat format) {
	const FormatAPIMap::const_iterator map_it = APIMap.find(format);

	if ( map_it == APIMap.end() || !connect() )
		return 0;

	Version maxVersion = 1;
	for ( APIList::const_iterator list_it = map_it->second.begin();
	      list_it != map_it->second.end(); ++list_it, ++maxVersion ) {
		if ( *list_it > _serverAPI ) break;
	}

	return maxVersion;
}

////////////////////////////////////////////////////////////////////////////////
// Protected
////////////////////////////////////////////////////////////////////////////////

bool Connection::connect() {
	if ( connected() ) return true;

	disconnect(); // resets server ID and API version

	if ( !_sock ) {
		SEISCOMP_ERROR("%sinstance not initialized", _logPrefix.c_str());
		return false;
	}

	try { _sock->open(_service); }
	catch ( SocketException &se ) {
		SEISCOMP_ERROR("%scould not connect to service '%s': %s ",
		               _logPrefix.c_str(), _service.c_str(), se.what());
		_sock->close();
		return false;
	}

	if ( !hello(_serverID, _serverAPI) )
		return false;

	if ( _user.empty() ) {
		SEISCOMP_DEBUG("%sskipping authentication", _logPrefix.c_str());
	}
	else {
		SEISCOMP_DEBUG("%sperforming authentication", _logPrefix.c_str());
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
		else if ( readHeaderValue(value, line, "Disposed:") ) {
			bool disposed;
			if ( !Core::fromString(disposed, value.c_str()) ) {
				logAndDisconnect("response header: Invalid Disposed value",
				                 value.c_str());
				return false;
			}

			response.disposed = disposed;
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
		try { data += _sock->read(static_cast<int>(read)); }
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

bool Connection::checkFormatVersion(std::string &error,
                                    const RequestFormatVersion &formatVerion) {
	error.clear();
	if ( !isSupported(formatVerion) ) {
		stringstream ss;
		ss << "Format version not supported by server, requested: "
		   << formatVerion.version() << ", supported: "
		   << maximumSupportedVersion(formatVerion);
		error = ss.str();
		return false;
	}
	return true;
}

} // ns QuakeLink
} // ns IO
} // ns Seiscomp
