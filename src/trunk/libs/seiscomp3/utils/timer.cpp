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


#define SEISCOMP_COMPONENT Utils/Timer

#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/utils/timer.h>
#include <assert.h>
#include <iostream>

#ifndef WIN32
#include <errno.h>
#include <string.h>

#define TIMER_CLOCKID CLOCK_REALTIME
#endif


namespace Seiscomp {
namespace Util {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StopWatch::StopWatch() {
	restart();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StopWatch::StopWatch(bool autorun) {
	if ( autorun )
		restart();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StopWatch::restart() {
	_start.localtime();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StopWatch::reset() {
	_start = Seiscomp::Core::Time();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StopWatch::isActive() const {
	return _start.valid();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::TimeSpan StopWatch::elapsed() const {
	return isActive() ? Seiscomp::Core::Time::LocalTime() - _start
	                  : Seiscomp::Core::TimeSpan();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#ifdef WIN32
Timer::TimerList Timer::_timers;
boost::thread *Timer::_thread = NULL;
boost::mutex Timer::_mutex;
#endif
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Timer::Timer(unsigned int timeout) {
	_singleShot = false;
#ifndef WIN32
	_timerID = 0;
#else
	_isActive = false;
#endif
	setTimeout(timeout);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Timer::~Timer() {
#ifdef WIN32
	if ( _isActive )
		deactivate(true);
#else
	destroy();
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Timer::setTimeout(unsigned int timeout) {
	_timeout = timeout;
#ifdef WIN32
	if ( !_timeout && _isActive )
#else
	_timeoutNs = 0;
	if ( !_timeout && !_timeoutNs && _timerID )
		stop();
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Timer::setTimeout2(unsigned int seconds, unsigned int nanoseconds) {
#ifdef WIN32
	if ( nanoseconds )
		return false;

	setTimeout(seconds);
	return true;
#else
	_timeout = seconds;
	_timeoutNs = nanoseconds;

	if ( !_timeout && !_timeoutNs && _timerID )
		stop();

	return true;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Timer::setCallback(const Callback &cb) {
	_callback = cb;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Timer::setSingleShot(bool s) {
	_singleShot = s;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Timer::start() {
#ifdef WIN32
	if ( !_timeout )
#else
	if ( !_timeout && !_timeoutNs )
#endif
		return false;

#ifdef WIN32
	boost::mutex::scoped_lock lk(_mutex);

	if ( _isActive )
		return false;

	if ( find(_timers.begin(), _timers.end(), this) == _timers.end() )
		_timers.push_back(this);

	_isActive = true;
	_value = _timeout;

	if ( !_thread ) {
		_thread = new boost::thread(Timer::Loop);
		boost::thread::yield();
	}
#else
	if ( _timerID ) return false;

	sigevent sev;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	sched_param parm;

	parm.sched_priority = 255;
	pthread_attr_setschedparam(&attr, &parm);

	sev.sigev_notify_attributes = &attr;
	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = handleTimeout;
	sev.sigev_signo = SIGUSR1;
	sev.sigev_value.sival_ptr = this;

	if ( timer_create(TIMER_CLOCKID, &sev, &_timerID) ) {
		SEISCOMP_ERROR("Failed to create timer: %d: %s", errno, strerror(errno));
		_timerID = 0;
		return false;
	}

	itimerspec its;

	/* Single shot */
	its.it_value.tv_sec = _timeout;
	its.it_value.tv_nsec = _timeoutNs;

	/* Periodically */
	its.it_interval.tv_sec = _singleShot ? 0 : _timeout;
	its.it_interval.tv_nsec = _singleShot ? 0 : _timeoutNs;

	if ( timer_settime(_timerID, 0, &its, NULL) ) {
		SEISCOMP_ERROR("Failed to set timer: %d: %s", errno, strerror(errno));
		timer_delete(_timerID);
		_timerID = 0;
		return false;
	}
#endif

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Timer::stop() {
#ifdef WIN32
	return deactivate(false);
#else
	return destroy();
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Timer::disable() {
#ifdef WIN32
	if ( _isActive )
		return deactivate(true);
	return false;
#else
	return destroy();
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#ifdef WIN32
bool Timer::deactivate(bool remove) {
	assert(_isActive == true);

	_isActive = false;

	if ( remove ) {
		boost::mutex::scoped_lock lk(_mutex);

		for ( TimerList::iterator it = _timers.begin(); it != _timers.end(); ++it ) {
			if ( *it == this ) {
				_timers.erase(it);
				break;
			}
		}
	}

	return true;
}
#else
bool Timer::destroy() {
	boost::try_mutex::scoped_lock lock(_callbackMutex);

	if ( !_timerID ) return false;

	if ( timer_delete(_timerID) ) {
		SEISCOMP_ERROR("Failed to delete timer %p: %d: %s", _timerID, errno, strerror(errno));
		_timerID = 0;
		return false;
	}

	_timerID = 0;

	return true;
}
#endif
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Timer::isActive() const {
#ifdef WIN32
	return _isActive;
#else
	return _timerID > 0;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#ifdef WIN32
void Timer::Loop() {
	do {
		Core::sleep(1);
	}
	while ( Update() );

	boost::mutex::scoped_lock lk(_mutex);

	if ( _thread ) {
		delete _thread;
		_thread = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Timer::Update() {
	boost::mutex::scoped_lock lk(_mutex);

	for ( TimerList::iterator it = _timers.begin(); it != _timers.end(); ) {
		Timer *t = *it;
		if ( --t->_value == 0 ) {
			if ( t->_isActive && t->_callback ) {
				//lk.unlock();
				t->_callback();
				//lk.lock();
			}

			if ( t->_singleShot || !t->_isActive ) {
				t->_isActive = false;
				it = _timers.erase(it);
				continue;
			}
			else
				t->_value = t->_timeout;
		}
		else if ( !t->_isActive ) {
			it = _timers.erase(it);
			continue;
		}

		++it;
	}

	return !_timers.empty();
}
#else
void Timer::handleTimeout(sigval_t self) {
	Timer *timer = reinterpret_cast<Timer*>(self.sival_ptr);
	if ( timer->_callback ) {
#if (BOOST_VERSION >= 103500)
			boost::try_mutex::scoped_try_lock l(timer->_callbackMutex, boost::defer_lock);
#else
			boost::try_mutex::scoped_try_lock l(timer->_callbackMutex, false);
#endif

		if ( l.try_lock() )
			timer->_callback();
	}
}
#endif
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
