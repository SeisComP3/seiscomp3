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


#define SEISCOMP_COMPONENT log
#include <seiscomp3/logging/output.h>
#include <seiscomp3/logging/channel.h>


namespace Seiscomp {
namespace Logging {


Output::Output() : _logComponent(true), _logContext(false), _useUTC(false) {
}


bool Output::subscribe(Channel* ch) {
	if ( ch == NULL ) return false;

	addPublisher(ch);
	ch->isInterested(this, true);

	return true;
}


bool Output::unsubscribe(Channel* ch) {
	if ( ch == NULL ) return false;

	dropPublisher(ch);
	ch->isInterested(this, false);

	return true;
}


void Output::publish(const Data &data) {
	LogLevel level = data.publisher->channel->logLevel();

	_publisher = data.publisher;
	log(_publisher->channel->name().c_str(),
	    level,
	    data.msg,
	    data.time);
}

const char* Output::component() const {
	return _publisher->component;
}

const char* Output::fileName() const {
	return _publisher->fileName;
}

const char* Output::functionName() const {
	return _publisher->functionName;
}

int Output::lineNum() const {
	return _publisher->lineNum;
}


}
}
