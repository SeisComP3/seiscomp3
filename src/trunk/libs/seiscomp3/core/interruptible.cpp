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


#include <signal.h>

#include "interruptible.h"


namespace Seiscomp {
namespace Core {
namespace _private {


#if !defined(SIGALRM) && defined(_MSC_VER)
#define SIGALRM  14

static HANDLE timerHandle = INVALID_HANDLE_VALUE;

VOID CALLBACK timerCompletion(LPVOID arg, DWORD timeLow, DWORD timeHigh) {
	Alarmable::SignalHandler(SIGALRM);
}

unsigned int alarm(unsigned int seconds) {
	if ( timerHandle == INVALID_HANDLE_VALUE ) {
		if ( seconds == 0 ) return 0;
		timerHandle = CreateWaitableTimer(NULL, TRUE, NULL);
		if ( timerHandle == NULL )
			return 0;
	}

	if ( seconds == 0 ) {
		CancelWaitableTimer(timerHandle);
		return 0;
	}

	LARGE_INTEGER dueTime;
	dueTime.QuadPart = seconds*10000000L;

	if ( !SetWaitableTimer(timerHandle, &dueTime, 0, timerCompletion, NULL, FALSE) )
		return 0;

	return 0;
}

#endif

using namespace std;

bool Alarmable::_signalInit = false;
std::list<std::pair<Alarmable*, time_t> > Alarmable::_alarms;

Alarmable::Alarmable() {
	_link = _alarms.end();

	if ( !_signalInit ) {
#ifndef WIN32
		struct sigaction sa;
		sa.sa_handler = SignalHandler;
		sa.sa_flags = 0;
		sigemptyset(&sa.sa_mask);
		sigaction(SIGALRM, &sa, NULL);
#endif
		_signalInit = true;
	}
}

Alarmable::~Alarmable() {
	clearAlarm();
}

void Alarmable::setAlarm(unsigned int seconds) {
	if ( !seconds ) {
		clearAlarm();
		return;
	}

	alarm(0);
	if ( _link != _alarms.end() ) {
		_alarms.erase(_link);
		_link = _alarms.end();
	}

	time_t t = time(NULL) + seconds;
	std::list<std::pair<Alarmable*, time_t> >::iterator it;
	for ( it = _alarms.begin(); it != _alarms.end(); ++it ) {
		if ( it->second > t ) {
			break;
		}
	}

	_link = _alarms.insert(it, make_pair(this, t));
	CheckAlarms();
}

void Alarmable::clearAlarm() {
	alarm(0);
	if ( _link != _alarms.end() ) {
		_alarms.erase(_link);
		_link = _alarms.end();
	}

	CheckAlarms();
}

void Alarmable::CheckAlarms() {
	time_t t = time(NULL);
	std::list<std::pair<Alarmable*, time_t> >::iterator it = _alarms.begin();
	while ( it != _alarms.end() ) {
		if ( it->second <= t ) {
			it->first->handleAlarm();
			it->first->_link = _alarms.end();
			_alarms.erase(it++);
		}
		else {
			alarm((unsigned int)(it->second - t));
			break;
		}
	}
}

void Alarmable::SignalHandler(int) {
	CheckAlarms();
}

std::list<Interruptible*> Interruptible::_registered;

Interruptible::Interruptible() {
	_link = _registered.insert(_registered.end(), this);
}

Interruptible::~Interruptible() {
	_registered.erase(_link);
}

void Interruptible::Interrupt(int sig) throw() {
	list<Interruptible*>::iterator it;
	for ( it = _registered.begin(); it != _registered.end(); ++it )
		(*it)->handleInterrupt(sig);
}

IMPLEMENT_SC_ABSTRACT_CLASS(InterruptibleObject, "InterruptibleObject");

} // namespace _private
} // namespace Core
} // namespace Seiscomp

