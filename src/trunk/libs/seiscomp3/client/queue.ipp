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

#ifndef __SEISCOMP_CLIENT_QUEUE_IPP__
#define __SEISCOMP_CLIENT_QUEUE_IPP__


#include <seiscomp3/core/exceptions.h>
#include <boost/type_traits/is_pointer.hpp>


namespace Seiscomp {
namespace Client {


namespace {

template <typename T, int IsPtr>
struct QueueHelper {};

template <typename T>
struct QueueHelper<T,0> {
	static void clean(const std::vector<T> &) {}
	static T defaultValue() { return T(); }
};

template <typename T>
struct QueueHelper<T,1> {
	static void clean(const std::vector<T> &b) {
		for ( size_t i = 0; i < b.size(); ++i ) {
			if ( b[i] ) delete b[i];
		}
	}

	static T defaultValue() { return NULL; }
};

}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
ThreadedQueue<T>::ThreadedQueue() :
	_begin(0), _end(0),
	_buffered(0), _closed(false), _buffer(0)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
ThreadedQueue<T>::ThreadedQueue(int n) :
	_begin(0), _end(0),
	_buffered(0), _closed(false), _buffer(n)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
ThreadedQueue<T>::~ThreadedQueue() {
	close();
	QueueHelper<T, boost::is_pointer<T>::value>::clean(_buffer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void ThreadedQueue<T>::resize(int n) {
	lock lk(_monitor);
	_buffer.resize(n);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
bool ThreadedQueue<T>::canPush() const {
	lock lk(_monitor);

	if ( _closed )
		throw QueueClosedException();

	return _buffered < _buffer.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
bool ThreadedQueue<T>::push(T v) {
	lock lk(_monitor);
	while (_buffered == _buffer.size() && !_closed)
		_notFull.wait(lk);
	if ( _closed ) {
		_notEmpty.notify_all();
		return false;
	}
	_buffer[_end] = v;
	_end = (_end+1) % _buffer.size();
	++_buffered;
	_notEmpty.notify_all();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
bool ThreadedQueue<T>::canPop() const {
	lock lk(_monitor);

	if ( _closed )
		throw QueueClosedException();

	return _buffered > 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
T ThreadedQueue<T>::pop() {
	lock lk(_monitor);
	while (_buffered == 0 && !_closed) {
		_notEmpty.wait(lk);
	}
	if ( _closed )
		throw QueueClosedException();
	T v = _buffer[_begin];
	_buffer[_begin] = QueueHelper<T, boost::is_pointer<T>::value>::defaultValue();
	_begin = (_begin+1) % _buffer.size();
	--_buffered;
	_notFull.notify_all();
	return v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void ThreadedQueue<T>::close() {
	lock lk(_monitor);
	if ( _closed ) return;
	_closed = true;
	_notFull.notify_all();
	_notEmpty.notify_all();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
size_t ThreadedQueue<T>::size() const {
	lock lk(_monitor);
	return _buffered;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}

#endif
