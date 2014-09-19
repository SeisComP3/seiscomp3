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


#ifndef __SEISCOMP_CORE_INTERRUPTIBLE_H__
#define __SEISCOMP_CORE_INTERRUPTIBLE_H__

#include <string>
#include <set>
#include <utility>

#include <seiscomp3/core/baseobject.h>

namespace Seiscomp {
namespace Core {
namespace _private {

class SC_SYSTEM_CORE_API Alarmable {
	public:
		Alarmable();
		virtual ~Alarmable();
	
	protected:
		void setAlarm(unsigned int seconds);
		void clearAlarm();
		virtual void handleAlarm() throw() {};
	
	private:
		std::list<std::pair<Alarmable*, time_t> >::iterator _link;
		static std::list<std::pair<Alarmable*, time_t> > _alarms;
		static bool _signalInit;
		static void CheckAlarms();
		static void SignalHandler(int);

#if defined(_MSC_VER)
	friend VOID CALLBACK timerCompletion(LPVOID, DWORD, DWORD);
#endif
};

class SC_SYSTEM_CORE_API Interruptible {
	public:
		Interruptible();
		virtual ~Interruptible();
		static void Interrupt(int sig) throw();
	
	protected:
		virtual void handleInterrupt(int) throw() {};

	private:
		std::list<Interruptible*>::iterator _link;
		static std::list<Interruptible*> _registered;
};

class SC_SYSTEM_CORE_API OperationInterrupted : public GeneralException {
	public:
		OperationInterrupted(): GeneralException("operation interrupted") {}
		OperationInterrupted(const std::string& what): GeneralException(what) {}
};

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** Classes that implement opterations, which may potentially take long time
    and need to be interrupted by the user, should inherit from
    InterruptibleObject.

    Inherited classes can also use setAlarm(int seconds) and clearAlarm() to
    implement timeouts. One alarm per object can be used, eg., setAlarm()
    cancels the previous alarm. However, there is no per-process limitation
    like in case of alarm() system call. setAlarm() and clearAlarm() themselves
    are implemented on top of alarm(), which means that alarm() cannot be used
    at the same time.

    The inherited class is supposed to override handleInterrupt(int sig) and
    handleAlarm() methods, which are called when interrupt is requested or
    alarm expires respectively. These methods are normally called from a signal
    handler, so they are not allowed to throw any exceptions. Normally they
    just set a flag, and exception is thrown after returning from a signal
    handler. For this purpose, the exception OperationInterrupted has been
    defined. Note: according the POSIX standard, a flag that is set in a
    signal handler should be of type 'volatile sig_atomic_t'.

    The main program should set up signal handler as follows:

    \code
    void signalHandler(int signal) {
        Seiscomp::Core::InterruptibleObject::Interrupt(signal);
    }

    int main(int argc, char **argv) {
        struct sigaction sa;
        sa.sa_handler = signalHandler;
        sa.sa_flags = 0;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGTERM, &sa, NULL);

        // Optionally, disable SIGHUP, so it is not necessary
        // to start the process with nohup.
        sa.sa_handler = SIG_IGN;
	    sigaction(SIGHUP, &sa, NULL);
	
        ...

        return 0
    }
    \endcode
*/

DEFINE_SMARTPOINTER(InterruptibleObject);

class SC_SYSTEM_CORE_API InterruptibleObject : public BaseObject, public Interruptible, public Alarmable {
	DECLARE_SC_CLASS(InterruptibleObject);
};

} // namespace _private

using _private::OperationInterrupted;
using _private::InterruptibleObjectPtr;
using _private::InterruptibleObject;

} // namespace Core
} // namespace Seiscomp

#endif

