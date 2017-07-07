/***************************************************************************
 *   Copyright (C) by GFZ Potsdam, gempa GmbH                              *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_UTILS_TIMER_H__
#define __SEISCOMP_UTILS_TIMER_H__

#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core.h>

#include <boost/function.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#if defined(WIN32) || defined(__APPLE__)
#include <list>
#else
#include <signal.h>
#include <time.h>
#endif


namespace Seiscomp {
namespace Util {


/** \brief A stopwatch to measure a timespan
  * The stopwatch substracts the timestamps at the time of a
  * call to StopWatch::elapsed and a previous call to Timer::restart
  * or the instantiation of a StopWatch object.
  * \code
  * StopWatch aStopWatch;
  * // do some calculations
  * cout << aStopWatch.elapsed() << " seconds later" << endl;
  * \endcode
  */
class SC_SYSTEM_CORE_API StopWatch {
	public:
		//! Constructor
		StopWatch();

		//! Constructor
		StopWatch(bool autorun);

	public:
		//! restarts the timer
		void restart();

		//! resets the timer
		void reset();

		//! returns true if the timer is active
		bool isActive() const;

		//! returns the elapsed time in seconds from
		//! restart to now
		Seiscomp::Core::TimeSpan elapsed() const;

	private:
		Seiscomp::Core::Time _start;
};


/** \brief A timer class that provides repetitive and single-shot timers.
  * This class provides an interface for timers.
  * Usage:
  * \code
  * Timer timer;
  * timer.setTimeout(5);
  * timer.setCallback(myCallbackFunction);
  * timer.start()
  * \endcode
  *
  * Timing: The timer class emits timeouts in one second intervals.
  *         A timer can only set its timeout to a multiple of one second
  *         The timer itself starts immediatly unless another time has
  *         been started already. The the second timer starts at the multiple
  *         of one second + the start time of the first timer
  * Callback: The callback that is called when the timer timeouts runs
  *           within the timer thread and not the one start() has been
  *           called.
  */
class SC_SYSTEM_CORE_API Timer {
	public:
		typedef boost::function<void ()> Callback;

	public:
		//! C'tor
		Timer(unsigned int timeoutseconds = 0);

		//! D'tor
		~Timer();


	public:
		//! Sets the timeout in seconds
		void setTimeout(unsigned int seconds);

		//! Sets the timeout with possible nanosecond precision.
		//! @return Success flag. Systems that do not support nanosecond
		//!        timers might fail.
		bool setTimeout2(unsigned int seconds, unsigned int nanoseconds);

		//! Sets the callback for the timeout
		void setCallback(const Callback &);

		//! Sets whether the timer is a single-shot timer.
		//! Single-shot timers stop after the first timeout.
		void setSingleShot(bool);

		//! Starts the timer.
		bool start();

		//! Stops the timer.
		bool stop();

		//! Stops the timer if active and waits until it is removed from
		//! the timer list. After this call no callback will be executed.
		bool disable();

		//! Returns the current timer state
		bool isActive() const;


	private:
#if defined(WIN32) || defined(__APPLE__)
		bool deactivate(bool remove);

		static void Loop();
		static bool Update();
#else
		bool destroy();

		static void handleTimeout(sigval_t self);
#endif

	private:
#if defined(WIN32) || defined(__APPLE__)
		typedef std::list<Timer*> TimerList;
		static TimerList _timers;
		static boost::thread *_thread;
		static boost::mutex _mutex;

		bool             _isActive;
		unsigned int     _value;
#else
		timer_t          _timerID;
#endif

		Callback         _callback;
		boost::try_mutex _callbackMutex;
		unsigned int     _timeout;
#if !defined(WIN32) && !defined(__APPLE__)
		unsigned int     _timeoutNs;
#endif
		bool             _singleShot;
};


}
}


#endif
