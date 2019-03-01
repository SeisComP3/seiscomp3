/***************************************************************************
 * libcapsclient
 * Copyright (C) 2016  gempa GmbH
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/


#include <gempa/caps/rawpacket.h>
#include <gempa/caps/log.h>
#include <gempa/caps/utils.h>
#include <gempa/caps/riff.h>


using namespace std;


namespace {

#define DT2STR(prefix, dt)             \
	do {                               \
		switch ( dt ) {                \
			case DT_DOUBLE:            \
				return prefix"DOUBLE"; \
			case DT_FLOAT:             \
				return prefix"FLOAT";  \
			case DT_INT64:             \
				return prefix"INT64";  \
			case DT_INT32:             \
				return prefix"INT32";  \
			case DT_INT16:             \
				return prefix"INT16";  \
			case DT_INT8:              \
				return prefix"INT8";   \
			default:                   \
				break;                 \
		}                              \
	}                                  \
	while (0);                         \
	return prefix"UNKWN";              \

}


namespace Gempa {
namespace CAPS {

namespace {

inline int sizeOf(DataType dt) {
	switch ( dt ) {
		case DT_DOUBLE:
			return sizeof(double);
		case DT_FLOAT:
			return sizeof(float);
		case DT_INT64:
			return sizeof(int64_t);
		case DT_INT32:
			return sizeof(int32_t);
		case DT_INT16:
			return sizeof(int16_t);
		case DT_INT8:
			return sizeof(int8_t);
		default:
			break;
	};

	return 0;
}

inline Time getEndTime(const Time &stime, size_t count, const DataRecord::Header &header) {
	if ( header.samplingFrequencyNumerator == 0 ||
	     header.samplingFrequencyDenominator == 0 ) return stime;

	return stime + samplesToTimeSpan(header, count);
}


}


RawDataRecord::RawDataRecord() : _dataOfs(0), _dataSize(0), _dirty(false) {}

const char *RawDataRecord::formatName() const {
	DT2STR("RAW/", _currentHeader.dataType)
}

void RawDataRecord::readMetaData(std::streambuf &buf, int size, Header &header,
                                 Time &startTime, Time &endTime) {
	if ( !header.get(buf) ) return;

	_currentHeader.dataType = header.dataType;

	size -= header.dataSize();
	int dtsize = sizeOf(header.dataType);
	if ( dtsize == 0 ) {
		CAPS_WARNING("unknown data type: %d", header.dataType);
		return;
	}

	int count = size / dtsize;

	startTime = timestampToTime(header.samplingTime);
	endTime = getEndTime(startTime, count, header);
}

void RawDataRecord::setHeader(const Header &header) {
	_header = header;
	_currentHeader = _header;
	_startTime = timestampToTime(_header.samplingTime);
	_dirty = true;
}

const DataRecord::Header *RawDataRecord::header() const {
	return &_header;
}

void RawDataRecord::setDataType(DataType dt) {
	_header.dataType = dt;
	_currentHeader = _header;
	_dirty = true;
}

void RawDataRecord::setStartTime(const Time &time) {
	_header.setSamplingTime(time);
	_currentHeader = _header;
	_startTime = time;
	_dirty = true;
}

void RawDataRecord::setSamplingFrequency(int numerator, int denominator) {
	_header.samplingFrequencyNumerator = numerator;
	_header.samplingFrequencyDenominator = denominator;
	_currentHeader = _header;
	_dirty = true;
}

Time RawDataRecord::startTime() const {
	return _startTime;
}

Time RawDataRecord::endTime() const {
	if ( _dirty ) {
		_endTime = getEndTime(_startTime, _data.size() / sizeOf(_header.dataType), _header);
		_dirty = false;
	}
	return _endTime;
}

size_t RawDataRecord::dataSize(bool withHeader) const {
	if ( withHeader )
		return _dataSize + _header.dataSize();
	else
		return _dataSize;
}

DataRecord::ReadStatus RawDataRecord::get(streambuf &buf, int size,
                                          const Time &start, const Time &end,
                                          int maxBytes) {
	size -= _header.dataSize();
	if ( size < 0 ) return RS_Error;
	if ( !_header.get(buf) ) {
		CAPS_WARNING("invalid raw header");
		return RS_Error;
	}

	return getData(buf, size, start, end, maxBytes);
}

DataRecord::ReadStatus RawDataRecord::getData(streambuf &buf, int size,
                                              const Time &start, const Time &end,
                                              int maxBytes) {
	bool partial = false;
	int sampleOfs;
	int sampleCount;
	int dataTypeSize = sizeOf(_header.dataType);
	if ( dataTypeSize == 0 ) {
		CAPS_WARNING("unknown data type: %d", _header.dataType);
		return RS_Error;
	}

	sampleCount = size / dataTypeSize;

	_startTime = timestampToTime(_header.samplingTime);

	if ( _header.samplingFrequencyDenominator > 0 &&
	     _header.samplingFrequencyNumerator > 0 )
		_endTime = _startTime + samplesToTimeSpan(_header, sampleCount);
	else
		_endTime = Time();

	if ( (start.valid() || end.valid()) && _endTime.valid() ) {
		// Out of bounds?
		if ( end.valid() && (end <= _startTime) )
			return RS_AfterTimeWindow;

		if ( start.valid() && (start >= _endTime) )
			return RS_BeforeTimeWindow;

		// Trim packet front
		if ( _startTime < start ) {
			TimeSpan ofs = start - _startTime;
			sampleOfs = timeSpanToSamplesCeil(_header, ofs);
			sampleCount -= sampleOfs;
			CAPS_DEBUG("Triming packet start: added offset of %d samples", sampleOfs);
			_startTime += samplesToTimeSpan(_header, sampleOfs);
			// Update header timespan
			timeToTimestamp(_header.samplingTime, _startTime);
		}
		else
			sampleOfs = 0;

		if ( maxBytes > 0 ) {
			int maxSamples = maxBytes / dataTypeSize;
			// Here we need to clip the complete dataset and only
			// return a partial record
			if ( maxSamples < sampleCount ) {
				CAPS_DEBUG("Clip %d available samples to %d", sampleCount, maxSamples);
				_endTime -= samplesToTimeSpan(_header, sampleCount-maxSamples);
				sampleCount = maxSamples;
				partial = true;
			}
		}

		// Trim back
		if ( end.valid() && (end < _endTime) ) {
			TimeSpan ofs = _endTime - end;
			int trimEnd = timeSpanToSamplesFloor(_header, ofs);
			sampleCount -= trimEnd;
			_endTime = _startTime + samplesToTimeSpan(_header, sampleCount);
			CAPS_DEBUG("Triming packet end: added offset of %d samples", trimEnd);
		}
	}
	else
		sampleOfs = 0;

	if ( sampleCount == 0 ) return RS_Error;

	_currentHeader = _header;

	switch ( _header.dataType ) {
		case DT_INT8:
		{
			// Stay with little endian data
			RIFF::VectorChunk<1,false> dataChunk(_data, sampleOfs, sampleCount);
			if ( !dataChunk.get(buf, size) ) return RS_Error;
		}
			break;

		case DT_INT16:
		{
			// Stay with little endian data
			RIFF::VectorChunk<2,false> dataChunk(_data, sampleOfs, sampleCount);
			if ( !dataChunk.get(buf, size) ) return RS_Error;
		}
			break;

		case DT_INT32:
		case DT_FLOAT:
		{
			// Stay with little endian data
			RIFF::VectorChunk<4,false> dataChunk(_data, sampleOfs, sampleCount);
			if ( !dataChunk.get(buf, size) ) return RS_Error;
		}
			break;

		case DT_INT64:
		case DT_DOUBLE:
		{
			// Stay with little endian data
			RIFF::VectorChunk<8,false> dataChunk(_data, sampleOfs, sampleCount);
			if ( !dataChunk.get(buf, size) ) return RS_Error;
		}
			break;

		default:
			CAPS_ERROR("THIS SHOULD NEVER HAPPEN: ignored invalid data with type %d",
			           _header.dataType);
			return RS_Error;
	};

	// Initialize indicies
	_dataOfs = 0;
	_dataSize = _data.size();

	return partial?RS_Partial:RS_Complete;
}

void RawDataRecord::setBuffer(const void *data, size_t size) {
	_data.resize(size);
	_dataSize = size;
	_dataOfs = 0;
	memcpy(_data.data(), data, size);
	_dirty = true;
}

const char *FixedRawDataRecord::formatName() const {
	DT2STR("FIXEDRAW/", _currentHeader.dataType)
}

}
}
