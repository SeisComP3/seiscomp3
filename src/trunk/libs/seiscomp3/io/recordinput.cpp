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


#define SEISCOMP_COMPONENT RecordInput
#include <seiscomp3/logging/log.h>

#include <string.h>
#include <iostream>
#include <seiscomp3/io/recordinput.h>
#include <seiscomp3/core/interfacefactory.ipp>

using namespace Seiscomp;
using namespace Seiscomp::IO;


/* Moved this definition from recordstream.cpp to here to
   avoid inline function creation for RecordFactory. */
IMPLEMENT_INTERFACE_FACTORY(RecordStream, SC_SYSTEM_CORE_API);


RecordIterator::RecordIterator(): _source(0), _current(0) {}

RecordIterator::RecordIterator(RecordInput *source, Record *cur)
  : _source(source), _current(cur) {}

RecordIterator::RecordIterator(const RecordIterator& iter)
  : _source(iter.source()), _current(iter.current()) {}

RecordIterator::~RecordIterator() {}

RecordIterator& RecordIterator::operator=(const RecordIterator& iter) {
  if (this != &iter) {
    _source = iter.source();
    _current = iter.current();
  }
  
  return *this;
}

Seiscomp::Record* RecordIterator::operator*() {
  return _current;
}

RecordIterator& RecordIterator::operator++() {
  _current = _source->next();

  return *this;
}

RecordIterator RecordIterator::operator++(int) {
  RecordIterator tmp(*this);
  ++(*this);
  return tmp;
}

bool RecordIterator::operator!=(const RecordIterator& iter) const {
  return !(*this == iter);
}

bool RecordIterator::operator==(const RecordIterator& iter) const {
  return (_current == iter.current() && _source == iter._source);
}

RecordInput* RecordIterator::source() const {
  return _source;
}

Seiscomp::Record* RecordIterator::current() const {
  return _current;
}

RecordInput::RecordInput(RecordStream *in, Array::DataType dt, Record::Hint h)
  : _in(in), _datatype(dt), _hint(h) {}

RecordIterator RecordInput::begin() throw(Core::GeneralException) {
  return RecordIterator(this,next());
}

RecordIterator RecordInput::end() {
  return RecordIterator(this,0);
}

Seiscomp::Record* RecordInput::next() throw(Core::GeneralException) {
	Record *pms = 0;
	while ( true ) {
		std::istream &istr = _in->stream();

		if (istr.good()) {
			pms = _in->createRecord(_datatype, _hint);
			if ( pms ) {
				try {
					pms->read(istr);
				}
				catch ( Core::EndOfStreamException & ) {
					SEISCOMP_INFO("End of stream detected");
					delete pms;
					break;
				}
				catch ( Core::StreamException &e ) {
					SEISCOMP_ERROR("RecordStream read exception: %s", e.what());
					delete pms;
					pms = 0;
					continue;
				}
			}

			// Notify the stream about the read record
			_in->recordStored(pms);
			return pms;
		}
		else {
			if (istr.eof())
				SEISCOMP_DEBUG("RecordStream's end reached");
			else
				SEISCOMP_DEBUG("RecordStream is not 'good'");
			break;
		}
	}

	return NULL;
}
