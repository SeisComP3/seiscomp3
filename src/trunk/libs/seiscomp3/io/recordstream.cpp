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


#define SEISCOMP_COMPONENT RecordStream

#include <seiscomp3/core/interruptible.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/logging/log.h>

#include <string.h>


namespace Seiscomp {
namespace IO {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IMPLEMENT_SC_ABSTRACT_CLASS_DERIVED(RecordStream, Seiscomp::Core::InterruptibleObject, "RecordStream");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordStream::RecordStream()
: _dataType(Array::DOUBLE), _hint(Record::SAVE_RAW) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStream::setTimeWindow(const Seiscomp::Core::TimeWindow &tw) {
	return setStartTime(tw.startTime()) && setEndTime(tw.endTime());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStream::setTimeout(int seconds) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStream::setRecordType(const char *type) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStream::setDataType(Array::DataType dataType) {
	_dataType = dataType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStream::setDataHint(Record::Hint hint) {
	_hint = hint;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordStream *RecordStream::Create(const char *service) {
	if ( service == NULL ) return NULL;
	return RecordStreamFactory::Create(service);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordStream* RecordStream::Open(const char *url) {
	const char *tmp;
	std::string service;
	std::string source;
	std::string type;

	tmp = strstr(url, "://");
	if ( tmp ) {
		std::copy(url, tmp, std::back_inserter(service));
		url = tmp + 3;
	}
	else
		service = "file";

	tmp = strchr(url, '#');
	if ( tmp ) {
		std::copy(url, tmp, std::back_inserter(source));
		type = tmp + 1;
	}
	else {
		source = url;
	}

	SEISCOMP_DEBUG("trying to open stream %s://%s%s%s",
	               service.c_str(), source.c_str(), type.empty()?"":"#", type.c_str());

	RecordStream* stream;

	stream = Create(service.c_str());

	if ( !stream )
		return NULL;

	try {
		if ( !stream->setSource(source.c_str()) ) {
			delete stream;
			return NULL;
		}
	}
	catch ( RecordStreamException &e ) {
		SEISCOMP_ERROR("stream exception: %s", e.what());
		delete stream;
		return NULL;
	}

	if ( !type.empty() && !stream->setRecordType(type.c_str()) ) {
		delete stream;
		stream = NULL;
	}

	return stream;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
