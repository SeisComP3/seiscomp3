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


#ifndef __SEISCOMP_CLIENT_QUEUE_H__
#define __SEISCOMP_CLIENT_QUEUE_H__


#include <vector>
#include <boost/utility.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>

#include <seiscomp3/core/baseobject.h>


namespace Seiscomp {
namespace Client {

class QueueClosedException : public Core::GeneralException {
	public:
		QueueClosedException() : Core::GeneralException("Queue has been closed") {}
		QueueClosedException(const std::string& str ) : Core::GeneralException(str) {}
};

template <typename T>
class ThreadedQueue : private boost::noncopyable {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		typedef boost::mutex::scoped_lock lock;

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		ThreadedQueue();
		ThreadedQueue(int n);
		~ThreadedQueue();


	// ----------------------------------------------------------------------
	//  Interface
	// ----------------------------------------------------------------------
	public:
		void resize(int n);

		bool canPush() const;
		bool push(T v);

		bool canPop() const;
		T pop();

		void close();
		
		size_t size() const;


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		volatile int _begin, _end;
		volatile size_t _buffered;
		volatile bool _closed;
		std::vector<T> _buffer;
		boost::condition _notFull, _notEmpty;
		mutable boost::mutex _monitor;
};


}
}


#endif
