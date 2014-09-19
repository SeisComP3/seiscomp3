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

#ifndef __SEISCOMP_UTILS_MUTEX_H__
#define __SEISCOMP_UTILS_MUTEX_H__


#include <seiscomp3/core.h>
#include <boost/version.hpp>

#if (BOOST_VERSION <= 103601)
#include <pthread.h>
#include <errno.h>
#endif

#if (BOOST_VERSION <= 103401)
#include <errno.h>
#endif

#include <boost/thread/exceptions.hpp>
#include <boost/thread/mutex.hpp>


namespace Seiscomp {
namespace Util {


#if (BOOST_VERSION <= 103401)

#ifndef BOOST_VERIFY
#define BOOST_VERIFY(expr) BOOST_ASSERT(expr)
#endif

class SC_SYSTEM_CORE_API mutex {
	public:
		mutex() {
			pthread_mutexattr_t attr;
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
			if ( pthread_mutex_init(&_mutex, NULL) )
				throw boost::thread_resource_error();
		}

		~mutex() {
			BOOST_VERIFY(!pthread_mutex_destroy(&_mutex));
		}

		void lock() {
			BOOST_VERIFY(!pthread_mutex_lock(&_mutex));
		}

		void unlock() {
			BOOST_VERIFY(!pthread_mutex_unlock(&_mutex));
		}

		bool try_lock() {
			int const res = pthread_mutex_trylock(&_mutex);
			if ( res && (res != EBUSY) ) {
				boost::throw_exception(boost::lock_error(res));
			}

			return !res;
		}

		typedef pthread_mutex_t* native_handle_type;
		native_handle_type native_handle() {
			return &_mutex;
		}


	private:
		pthread_mutex_t _mutex;
};

#else

typedef boost::mutex mutex;

#endif

} // namespace Util
} // namespace Seiscomp

#endif
