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


#ifndef __SEISCOMP_UTILS_TIMER_H__
#define __SEISCOMP_UTILS_TIMER_H__

#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core.h>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>
#include <list>


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

	public:
		//! restarts the timer
		void restart();

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

		//! Sets the callback for the timeout
		void setCallback(const Callback&);

		//! Sets whether the timer is a single-shot timer.
		//! Single-shot timers stop after the first timeout.
		void setSingleShot(bool);

		//! Starts the timer.
		bool start();

		//! Stops the timer.
		bool stop();

		//! Returns the current timer state
		bool isActive() const;


	private:
		static void Loop();
		static bool Update();

		bool deactivate(bool remove);


	private:
		typedef std::list<Timer*> TimerList;
		static TimerList _timers;
		static boost::thread *_thread;
		static boost::mutex _mutex;

		Callback _callback;
		unsigned int _timeout;
		unsigned int _value;
		bool _singleShot;
		bool _isActive;
};


}
}


#endif
