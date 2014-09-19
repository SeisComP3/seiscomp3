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



#define SEISCOMP_COMPONENT Arclink


#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/greensfunction.h>
#include <seiscomp3/io/gfarchive/arclink.h>

#include <seiscomp3/io/socket.h>

#include <iostream>
#include <fstream>


namespace Seiscomp {
namespace IO {


REGISTER_GFARCHIVE(ArclinkArchive, "arclink");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkArchive::ArclinkArchive() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkArchive::ArclinkArchive(const std::string &url) {
	setSource(url);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkArchive::~ArclinkArchive() {
	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkArchive::setSource(std::string source) {
	_url = source;

	if ( _sock.isOpen() ) close();

	try {
		_sock.open(_url);
	}
	catch ( ... ) {
		return false;
	}

	if ( !_sock.isOpen() ) {
		_sock.close();
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkArchive::close() {
	_sock.interrupt();
	_archive.close();
	_requests.clear();
	_requestID = "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkArchive::setTimeSpan(const Core::TimeSpan &span) {
	_defaultTimespan = span;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkArchive::addRequest(const std::string &id,
                                const std::string &model,
                                double distance, double depth) {
	_requests.push_back(Request());
	_requests.back().id = id;
	_requests.back().model = model;
	_requests.back().distance = distance;
	_requests.back().depth = depth;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkArchive::addRequest(const std::string &id,
                                const std::string &model,
                                double distance, double depth,
                                const Core::TimeSpan &span) {
	_requests.push_back(Request());
	_requests.back().id = id;
	_requests.back().model = model;
	_requests.back().distance = distance;
	_requests.back().depth = depth;
	_requests.back().timeSpan = span;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ArclinkArchive::handshake() {
	_sock.sendRequest("HELLO", false);
	std::string r = _sock.readline();
	if ( r == "ERROR" ) {
		SEISCOMP_ERROR("Remote server did not accept HELLO");
		return -1;
	}

	std::string software = r;
	std::string organization = _sock.readline();

	SEISCOMP_DEBUG("%s running at %s", software.c_str(), organization.c_str());

	_sock.sendRequest("USER anonymous", true);
	_sock.sendRequest("REQUEST GREENSFUNC", true);


	//REQUEST GREENSFUNC
	//1970,01,01 1970,01,01 . model=iasp810 depth=5 distance=10 span=500 id=0
	//END

	for ( RequestList::iterator it = _requests.begin(); it != _requests.end(); ++it ) {
		std::string req = "1970,01,01 1970,01,01 . model=";
		req += it->model;
		req += " id=";
		req += it->id;
		req += " depth=";
		req += Core::toString(it->depth);
		req += " distance=";
		req += Core::toString(it->distance);
		req += " span=";

		if ( it->timeSpan )
			req += Core::toString(double(it->timeSpan));
		else
			req += Core::toString(double(_defaultTimespan));

		SEISCOMP_DEBUG("Arclink request: %s", req.c_str());

		_sock.sendRequest(req, false);
	}

	_requestID = _sock.sendRequest("END", true);
	_sock.sendRequest("BDOWNLOAD " + _requestID, false);
	r = _sock.readline();

	if ( r != "ERROR" ) {
		char *tail;
		int remainingBytes = strtoul(r.c_str(), &tail, 10);
		if( *tail )
			SEISCOMP_ERROR("Invalid ArcLink response: %s", r.c_str());
		else
			return remainingBytes;
    }

	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::GreensFunction* ArclinkArchive::get() {
	if ( _requestID.empty() && !_requests.empty() ) {
		_sock.startTimer();

		int bytesToRead = 0;

		try {
			bytesToRead = handshake();
		}
		catch ( Core::GeneralException &e ) {
			SEISCOMP_ERROR("%s", e.what());
			_sock.close();
			return NULL;
		}

		SEISCOMP_DEBUG("Handshake response: %d", bytesToRead);

		if ( bytesToRead <= 0 ) {
			_sock.close();
			return NULL;
		}

		try {
			_buffer.str("");
			while ( bytesToRead > 0 ) {
				std::string data = _sock.read(bytesToRead >= 512?512:bytesToRead);
				_buffer.sputn(data.c_str(), data.size());
				bytesToRead -= data.size();
			}

			_sock.sendRequest("PURGE " + _requestID, true);

			_buffer.pubseekpos(0);
			if ( !_archive.open(&_buffer) ) {
				_buffer.str("");
				_sock.close();
				return NULL;
			}
		}
		catch ( Core::GeneralException &e ) {
			_buffer.str("");
		}

		_sock.close();
	}

	if ( _buffer.in_avail() <= 0 ) return NULL;

	Core::GreensFunction *gf = NULL;
	try {
		_archive >> gf;
	}
	catch ( ... ) {
		_buffer.str("");
		gf = NULL;
	}

	return gf;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
