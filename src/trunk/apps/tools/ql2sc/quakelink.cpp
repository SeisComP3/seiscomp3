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

#define SEISCOMP_COMPONENT QL2SC

#include "quakelink.h"

#include <seiscomp3/client/application.h>
#include <seiscomp3/logging/log.h>

using namespace std;

namespace Seiscomp {
namespace QL2SC {

namespace {

#if BOOST_VERSION >= 103500
boost::posix_time::time_duration wait(const Core::Time &until) {
	double diff = until - Core::Time::GMT();
	if ( diff <= 0 ) diff = 0.001; // make sure wait is positive
	int s = (int) boost::posix_time::time_duration::ticks_per_second() * diff;
	return boost::posix_time::time_duration(0, 0, 0, s);
}
#endif

} // ns anonymous

QLClient::QLClient(int notificationID, const HostConfig *config, size_t backLog)
 : IO::QuakeLink::Connection(), _notificationID(notificationID), _config(config),
   _backLog(backLog), _thread(NULL) {
	setLogPrefix("[QL " + _config->host + "] ");
}

QLClient::~QLClient() {
	if ( _thread != NULL ) {
		delete _thread;
		_thread = NULL;
	}

	SEISCOMP_INFO("%sterminated, messages/bytes received: %lu/%lu",
	              _logPrefix.c_str(), (unsigned long)_stats.messages,
	              (unsigned long)_stats.payloadBytes);
}

void QLClient::run() {
	SEISCOMP_INFO("%sconnecting to URL '%s'", _logPrefix.c_str(),
	              _config->url.c_str());
	_thread = new boost::thread(boost::bind(&QLClient::listen, this));
}


void QLClient::join(const Core::Time &until) {
	if ( _thread ) {
		SEISCOMP_DEBUG("%swaiting for thread to terminate", _logPrefix.c_str());
#if BOOST_VERSION < 103500
		_thread->join();
#else
		if ( _thread->timed_join(wait(until)) )
			SEISCOMP_DEBUG("%sthread terminated", _logPrefix.c_str());
		else
			SEISCOMP_ERROR("%sthread did not shutdown properly",
			               _logPrefix.c_str());
#endif
	}
}


Core::Time QLClient::lastUpdate() const {
	boost::mutex::scoped_lock l(_mutex);
	return _lastUpdate;
}

void QLClient::setLastUpdate(const Core::Time &time) {
	boost::mutex::scoped_lock l(_mutex);
	_lastUpdate = time;
}

void QLClient::processResponse(IO::QuakeLink::Response *response) {
	SEISCOMP_INFO("%sreceived message, size: %lu)", _logPrefix.c_str(),
	              (unsigned long)response->length);
	SCCoreApp->sendNotification(Client::Notification(
	                            _notificationID, response));
	++_stats.messages;
	_stats.payloadBytes += response->length;
}

void QLClient::listen() {
	IO::QuakeLink::RequestFormat rf = _config->gzip ?
	                                 (_config->native ? IO::QuakeLink::rfGZNative : IO::QuakeLink::rfGZXML) :
	                                 (_config->native ? IO::QuakeLink::rfNative : IO::QuakeLink::rfXML);
	// activate socket timeout if keepAlive was requested
	if ( _config->options & IO::QuakeLink::opKeepAlive ) {
		if ( _sock ) {
			_sock->setTimeout(60);
		}
		else {
			SEISCOMP_ERROR("%sinstance not initialized", _logPrefix.c_str());
			return;
		}
	}

	while ( !interrupted() ) {
		// determine start time of request
		Core::Time from;
		string filter = _config->filter;
		if ( _backLog > 0 ) {
			Core::Time minTime = Core::Time::GMT() - Core::TimeSpan(_backLog);
			from = lastUpdate();
			if ( !from.valid() || from < minTime )
				from = minTime;
			if ( !filter.empty() )
				filter += " AND ";
			filter += "UPDATED > " + from.toString(IO::QuakeLink::RequestTimeFormat);
		}

		// start request
		try {
			select(from.valid(), Core::Time(), Core::Time(), rf, filter);
		}
		catch ( Core::GeneralException& e) {
			if ( interrupted() )
				break;
			SEISCOMP_DEBUG("%sselect exception: %s", _logPrefix.c_str(), e.what());
		}

		_sock->close(); // clears interrupt flag
		SEISCOMP_WARNING("%sQuakeLink connection closed, trying to reconnect "
		                 "in 5s", _logPrefix.c_str());

		for ( int i = 0; i < 50; ++i ) {
			usleep(100000); // 100ms

			if ( interrupted() )
				break;
		}
	}

	if ( interrupted() )
		SEISCOMP_INFO("%sQuakeLink connection interrupted", _logPrefix.c_str());
	_sock->close();
}


} // ns QL2SC
} // ns Seiscomp
