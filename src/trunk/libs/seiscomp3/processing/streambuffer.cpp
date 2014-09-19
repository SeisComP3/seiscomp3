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



#include <iostream>
#include <seiscomp3/processing/streambuffer.h>


namespace Seiscomp {

namespace Processing {

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StreamBuffer::StreamBuffer() {
	_newStreamAdded = false;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StreamBuffer::StreamBuffer(const Seiscomp::Core::TimeWindow& timeWindow) {
	setTimeWindow(timeWindow);
	_newStreamAdded = false;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StreamBuffer::StreamBuffer(const Seiscomp::Core::TimeSpan& timeSpan) {
	setTimeSpan(timeSpan);
	_newStreamAdded = false;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StreamBuffer::~StreamBuffer() {
	clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamBuffer::setTimeWindow(const Seiscomp::Core::TimeWindow& timeWindow) {
	_mode = TIME_WINDOW;
	_timeStart = timeWindow.startTime();
	_timeSpan = timeWindow.endTime() - _timeStart;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamBuffer::setTimeSpan(const Seiscomp::Core::TimeSpan& timeSpan) {
	_mode = RING_BUFFER;
	_timeSpan = timeSpan;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence* StreamBuffer::sequence(const WaveformID& wid) const {
	SequenceMap::const_iterator it = _sequences.find(wid);
	if ( it != _sequences.end() )
		return it->second;
	return NULL;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence* StreamBuffer::feed(const Record *rec) {
	if ( rec == NULL ) return NULL;

	_newStreamAdded = false;

	WaveformID wid(rec);
	RecordSequence *seq = sequence(wid);

	if ( seq == NULL ) {
		switch ( _mode ) {
			case TIME_WINDOW:
				seq = new TimeWindowBuffer(Core::TimeWindow(_timeStart, _timeStart + _timeSpan));
				break;
			case RING_BUFFER:
				seq = new RingBuffer(_timeSpan);
				break;
		}

		_sequences[wid] = seq;
		_newStreamAdded = true;
	}

	if ( seq->feed(rec) )
		return seq;

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StreamBuffer::addedNewStream() const {
	return _newStreamAdded;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamBuffer::printStreams(std::ostream& os) const {
	for ( SequenceMap::const_iterator it = _sequences.begin();
	      it != _sequences.end(); ++it ) {
		os << "["
		          << it->first.networkCode << "."
		          << it->first.stationCode << "."
		          << it->first.locationCode << "."
		          << it->first.channelCode << "] "
		          << it->second->timeWindow().startTime().toString("%F %T") << " - "
		          << it->second->timeWindow().endTime().toString("%F %T")
		          << std::endl;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::list<std::string> StreamBuffer::getStreams() const {

	std::list<std::string> streamList;

	for ( SequenceMap::const_iterator it = _sequences.begin();
	      it != _sequences.end(); ++it ) {
		
		streamList.push_back(
		              it->first.networkCode + "."
		            + it->first.stationCode + "."
		            + it->first.locationCode + "."
		            + it->first.channelCode
		);
	}

	return streamList;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamBuffer::clear() {
	for ( SequenceMap::iterator it = _sequences.begin();
			it != _sequences.end(); ++it )
		if ( it->second ) delete it->second;

	_sequences.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}
