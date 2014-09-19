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


#include <seiscomp3/client/monitor.h>
#include <iostream>


namespace Seiscomp {
namespace Client {


RunningAverage::RunningAverage(int timeSpanInSeconds) {
	_timeSpan = timeSpanInSeconds;
	if ( _timeSpan < 1 ) _timeSpan = 1;
	_scale = 1.0 / double(_timeSpan);
	_bins.resize(_timeSpan, 0);
	_shift = 0.0;
	_front = 0;
}


void RunningAverage::push(const Core::Time &time, size_t count) {
	// The current log increments always the last bin.
	// But first the bins need to be shifted correctly.
	_shift += (double)(time - _last);
	size_t shiftBins;

	if ( _shift < 0 ) shiftBins = 0;
	else shiftBins = (size_t)_shift;

	if ( shiftBins >= _timeSpan ) {
		_shift = _timeSpan;
		shiftBins = _timeSpan;
		for ( size_t i = 0; i < _timeSpan; ++i )
			_bins[i] = 0;
		_front = 0;
	}
	else if ( shiftBins > 0 ) {
		// Move front of buffer shiftBins places
		_front += shiftBins;
		if ( _front >= _timeSpan ) _front -= _timeSpan;

		// Reset remaining slots
		if ( _front >= shiftBins ) {
			for ( size_t i = _front-shiftBins; i < _front; ++i )
				_bins[i] = 0;
		}
		else {
			for ( size_t i = 0; i < _front; ++i )
				_bins[i] = 0;
			for ( size_t i = _timeSpan-shiftBins+_front; i < _timeSpan; ++i )
				_bins[i] = 0;
		}
	}

	size_t back = _front > 0?_front-1:_timeSpan-1;

	_shift -= (double)shiftBins;
	_bins[back] += count;
	_last = time;
}


int RunningAverage::count(const Core::Time &time) const {
	int count = 0;

	int shiftBins = (int)(_shift + (double)(time - _last));
	if ( shiftBins < 0 || shiftBins >= (int)_timeSpan )
		return count;

	/*
	for ( size_t i = shiftBins; i < _timeSpan; ++i ) {
		if ( _front+i < _timeSpan )
			count += _bins[_front+i];
		else
			count += _bins[_front+i-_timeSpan];
	}
	*/

	for ( size_t i = _front + shiftBins; i < _timeSpan; ++i )
		count += _bins[i];

	size_t i = 0;
	if ( _front > _timeSpan-shiftBins )
		i = _front - (_timeSpan-shiftBins);

	for ( ; i < _front; ++i )
		count += _bins[i];

	return count;
}


double RunningAverage::value(const Core::Time &time) const {
	return count(time) * _scale;
}


Core::Time RunningAverage::last() const {
	return _last;
}


void RunningAverage::dumpBins() const {
	std::cout << "last = " << _last.iso() << std::endl;
	std::cout << "shift = " << _shift << std::endl;
	std::cout << "front = " << _front << std::endl;
	int idx = 0;
	for ( size_t i = _front; i < _bins.size(); ++i, ++idx )
		std::cout << "[" << idx << "] " << _bins[i] << std::endl;
	for ( size_t i = 0; i < _front; ++i, ++idx )
		std::cout << "[" << idx << "] " << _bins[i] << std::endl;
}


ObjectMonitor::ObjectMonitor(int timeSpanInSeconds)
: _timeSpan(timeSpanInSeconds) {
	if ( _timeSpan < 1 ) _timeSpan = 1;
}


ObjectMonitor::~ObjectMonitor() {
	const_iterator it;
	for ( it = _tests.begin(); it != _tests.end(); ++it )
		delete it->test;
}


ObjectMonitor::Log *
ObjectMonitor::add(const std::string &name, const std::string &channel) {
	const_iterator it;
	for ( it = _tests.begin(); it != _tests.end(); ++it )
		if ( it->channel == channel && it->name == name )
			return NULL;

	Test test;
	test.name = name;
	test.channel = channel;
	test.count = 0;
	test.test = new RunningAverage(_timeSpan);
	_tests.push_back(test);
	return test.test;
}


void ObjectMonitor::update(const Core::Time &time) {
	Tests::iterator it;
	for ( it = _tests.begin(); it != _tests.end(); ++it ) {
		it->updateTime = time;
		it->count = it->test->count(time);
	}
}


ObjectMonitor::const_iterator ObjectMonitor::begin() const {
	return _tests.begin();
}


ObjectMonitor::const_iterator ObjectMonitor::end() const {
	return _tests.end();
}


size_t ObjectMonitor::size() const {
	return _tests.size();
}


}
}
