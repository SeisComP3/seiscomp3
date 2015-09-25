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


IMPLEMENT_SC_ABSTRACT_CLASS_DERIVED(RecordStream, Seiscomp::Core::InterruptibleObject, "RecordStream");


RecordStream::RecordStream() {
	setRecordType("mseed");
}


bool RecordStream::setRecordType(const char* type) {
	_factory = RecordFactory::Find(type);
	if ( !_factory ) {
		SEISCOMP_ERROR("Unknown record type '%s'", type);
	}
	return _factory != NULL;
}


Record* RecordStream::createRecord(Array::DataType dt, Record::Hint h) {
	if ( _factory == NULL ) return NULL;
	Record* record = _factory->create();
	if ( record ) {
		record->setDataType(dt);
		record->setHint(h);
	}

	return record;
}


void RecordStream::recordStored(Record *) {
}


bool RecordStream::filterRecord(Record *) {
	// No filtering by default because the underlying acquisition systems do
	// it usually. In case of a file it can be useful.
	return false;
}


RecordStream* RecordStream::Create(const char* service) {
	if ( service == NULL ) return NULL;
	
	return RecordStreamFactory::Create(service);
}


RecordStream* RecordStream::Create(const char* service, const char* recordType) {
	RecordStream* stream = Create(service);
	if ( stream == NULL ) return NULL;

	if ( recordType && !stream->setRecordType(recordType) ) {
		SEISCOMP_ERROR("Stream service '%s' does not support the record type '%s'",
		               service, recordType);
		delete stream;
		stream = NULL;
	}

	return stream;
}


RecordStream* RecordStream::Open(const char* url) {
	const char* tmp;
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


}
}
