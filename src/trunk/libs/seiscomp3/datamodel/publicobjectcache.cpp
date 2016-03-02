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


#define SEISCOMP_COMPONENT ObjectCache

#include <seiscomp3/logging/log.h>
#include <seiscomp3/datamodel/publicobjectcache.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/databasearchive.h>

#include <assert.h>


namespace Seiscomp {
namespace DataModel {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObjectCache::const_iterator::const_iterator() : _item(NULL) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObjectCache::const_iterator::const_iterator(const const_iterator &it)
 : _item(it._item) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObjectCache::const_iterator::const_iterator(CacheItem *item)
 : _item(item) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObject* PublicObjectCache::const_iterator::operator*() {
	return _item->object.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
time_t PublicObjectCache::const_iterator::timeStamp() const {
	return _item->timestamp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool
PublicObjectCache::const_iterator::operator==(const const_iterator &it) {
	return _item == it._item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool
PublicObjectCache::const_iterator::operator!=(const const_iterator &it) {
	return _item != it._item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObjectCache::const_iterator&
PublicObjectCache::const_iterator::operator=(const const_iterator &it) {
	_item = it._item;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObjectCache::const_iterator&
PublicObjectCache::const_iterator::operator++() {
	_item = _item->next;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObjectCache::const_iterator
PublicObjectCache::const_iterator::operator++(int) {
	return const_iterator(_item->next);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObjectCache::PublicObjectCache() : _archive(NULL), _size(0),
    _front(NULL), _back(NULL) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObjectCache::PublicObjectCache(DatabaseArchive* ar)
 : _archive(ar), _size(0), _front(NULL), _back(NULL) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObjectCache::~PublicObjectCache() {
	while ( _front ) {
		CacheItem *item = _front;
		_front = _front->next;
		delete item;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PublicObjectCache::setDatabaseArchive(DatabaseArchive* ar) {
	_archive = ar;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PublicObjectCache::setPopCallback(const PopCallback& fkt) {
	_popCallback = fkt;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

struct DummyFunc {
	DummyFunc(CachePopCallback *cb) : callback(cb) {}
	void operator()(PublicObject *obj) {
		callback->handle(obj);
	}

	CachePopCallback *callback;
};

}

void PublicObjectCache::setPopCallback(CachePopCallback *cb) {
	_popCallback = DummyFunc(cb);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PublicObjectCache::removePopCallback() {
	_popCallback = PopCallback();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PublicObjectCache::remove(PublicObject *po) {
	CacheLookup::iterator it = _lookup.find(po->publicID());
	if ( it == _lookup.end() ) return false;

	CacheItem *item = it->second;

	// Remove object from lookup table
	_lookup.erase(it);

	if ( _popCallback ) _popCallback(po);

	if ( item ) {
		// Remove item
		if ( item->prev )
			item->prev->next = item->next;
		else
			_front = item->next;

		if ( item->next )
			item->next->prev = item->prev;
		else
			_back = item->prev;

		delete item;
		--_size;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PublicObjectCache::clear() {
	while ( _front ) {
		CacheItem *item = _front;
		PublicObjectPtr po = item->object;
		_front = _front->next;
		if ( _popCallback ) _popCallback(po.get());
		delete item;
	}

	_front = _back = NULL;
	_size = 0;
	_lookup.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObject *PublicObjectCache::find(const Seiscomp::Core::RTTI &classType,
                                      const std::string &publicID) {
	_cached = true;
	PublicObject *po = PublicObject::Find(publicID);
	if ( po == NULL ) {
		_cached = false;
		po = _archive?_archive->getObject(classType, publicID):NULL;
	}

	if ( po ) feed(po);

	return po;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::TimeWindow PublicObjectCache::timeWindow() const {
	Core::TimeWindow tw;

	if ( !empty() )
		tw.set(Core::Time(_front->timestamp), Core::Time(_back->timestamp));

	return tw;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::Time PublicObjectCache::oldest() const {
	return _front?Core::Time(_front->timestamp):Core::Time();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PublicObjectCache::push(PublicObject* obj) {
	std::pair<CacheLookup::iterator, bool>
		itp = _lookup.insert(CacheLookup::value_type(obj->publicID(), NULL));

	CacheItem *item;

	// Exists already
	if ( !itp.second ) {
		item = itp.first->second;

		if ( (item->object != obj) && !item->object->registered() ) {
			// Object leak -> replace is allowed
			item->object = obj;
		}

		// Release item
		if ( item->prev != NULL )
			item->prev->next = item->next;
		else
			_front = item->next;

		if ( item->next != NULL )
			item->next->prev = item->prev;
		else
			_back = item->prev;
	}
	else {
		item = new CacheItem;
		item->lookup = itp.first;
		itp.first->second = item;
		++_size;
	}

	// Update object pointer
	item->object = obj;

	// Update current timestamp
	item->timestamp = Core::Time::LocalTime().seconds();

	// Append item
	item->prev = _back;
	item->next = NULL;

	// Update links
	if ( item->prev )
		item->prev->next = item;
	else
		_front = item;
	_back = item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PublicObjectCache::pop() {
	if ( !_front ) return;
	if ( _popCallback ) _popCallback(_front->object.get());

	CacheItem *item = _front;

	// Remove item
	if ( item->prev )
		item->prev->next = item->next;
	else
		_front = item->next;

	if ( item->next )
		item->next->prev = item->prev;
	else
		_back = item->prev;

	_lookup.erase(item->lookup);

	delete item;
	--_size;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObjectCache::const_iterator PublicObjectCache::begin() const {
	return const_iterator(_front);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObjectCache::const_iterator PublicObjectCache::end() const {
	return const_iterator(NULL);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObjectRingBuffer::PublicObjectRingBuffer()
 : _bufferSize(0) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObjectRingBuffer::PublicObjectRingBuffer(DatabaseArchive* ar,
                                               size_t bufferSize)
 : PublicObjectCache(ar), _bufferSize(bufferSize) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PublicObjectRingBuffer::setBufferSize(size_t bufferSize) {
	_bufferSize = bufferSize;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PublicObjectRingBuffer::feed(PublicObject* po) {
	if ( !po ) return false;

	push(po);
	while ( size() > _bufferSize ) pop();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObjectTimeSpanBuffer::PublicObjectTimeSpanBuffer()
 : _timeSpan() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObjectTimeSpanBuffer::PublicObjectTimeSpanBuffer(DatabaseArchive* ar,
                                                       const Core::TimeSpan& length)
 : PublicObjectCache(ar), _timeSpan(length) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PublicObjectTimeSpanBuffer::setTimeSpan(const Core::TimeSpan& length) {
	_timeSpan = length;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PublicObjectTimeSpanBuffer::feed(PublicObject* po) {
	if ( !po ) return false;

	Core::Time now = Core::Time::LocalTime();

	push(po);
	while ( !empty() && now - oldest() > _timeSpan ) pop();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
