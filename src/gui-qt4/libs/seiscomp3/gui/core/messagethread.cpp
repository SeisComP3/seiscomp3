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




#define SEISCOMP_COMPONENT GUI::MessageThread

#include "messagethread.h"
#include <seiscomp3/logging/log.h>
#include <seiscomp3/communication/connection.h>


namespace Seiscomp {
namespace Gui {


MessageThread::MessageThread(Seiscomp::Communication::Connection* c)
 : _reconnectOnError(false), _connection(c) {
}


void MessageThread::setReconnectOnErrorEnabled(bool e) {
	_reconnectOnError = e;
	SEISCOMP_DEBUG("Setting automatic reconnect to: %d", e);
}


MessageThread::~MessageThread() {
	SEISCOMP_INFO("destroying message thread");
}


void MessageThread::run() {
	int result;

	SEISCOMP_INFO("starting message thread");

	while ( true ) {
		//SEISCOMP_DEBUG("Automatic reconnect: %d", _reconnectOnError);
		result = _connection->readNetworkMessage();
		if ( result == Seiscomp::Core::Status::SEISCOMP_SUCCESS ) {
			if ( _connection->queuedMessageCount() > 0 ) {
				//SEISCOMP_DEBUG("messages available");
				emit messagesAvailable();
			}
		}
		else {
			if ( !_connection->isConnected() ) {
				if ( _reconnectOnError ) {
					emit connectionLost();
					SEISCOMP_INFO("Trying to reconnect to messaging");
					while ( _connection->reconnect() != Core::Status::SEISCOMP_SUCCESS && _reconnectOnError ) {
						SEISCOMP_ERROR("Reconnect failed, wait 2 sec and try again...");
						sleep(2);
					}

					if ( !_connection->isConnected() ) {
						if ( !_reconnectOnError ) {
							emit connectionError(result);
							break;
						}
					}
					else
						emit connectionEstablished();
				}
				else {
					SEISCOMP_DEBUG("Connection::read() returned error (%d) and automatic reconnect is disabled", result);
					emit connectionError(result);
					break;
				}
			}
			else {
				SEISCOMP_DEBUG("Connection::read() returned error (%d), but still connected", result);
			}
		}
	}

	SEISCOMP_INFO("leaving message thread");

	return;
}


Seiscomp::Communication::Connection* MessageThread::connection() const {
	return _connection;
}


}
}
