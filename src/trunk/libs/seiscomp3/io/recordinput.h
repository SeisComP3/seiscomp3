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


#ifndef __SEISCOMP_IO_RECORDINPUT_H__
#define __SEISCOMP_IO_RECORDINPUT_H__

#include <iterator>
#include <seiscomp3/core.h>
#include <seiscomp3/io/recordstream.h>

namespace Seiscomp {
namespace IO {


class RecordInput;

class SC_SYSTEM_CORE_API RecordIterator : public std::iterator<std::input_iterator_tag, Record *> {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		RecordIterator();
		//! Copy c'tor
		RecordIterator(const RecordIterator &iter);
		//! D'tor
		~RecordIterator();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		RecordIterator &operator=(const RecordIterator &iter);
		Record *operator*();
		RecordIterator &operator++();
		RecordIterator operator++(int);
		bool operator!=(const RecordIterator &iter) const;
		bool operator==(const RecordIterator &iter) const;
		

	// ------------------------------------------------------------------
	//  Interface
	// ------------------------------------------------------------------
	public:
		/**
		 * Returns the source used by the iterator
		 * @return A RecordInput pointer which must not be deleted
		 *         by the caller!
		 */
		RecordInput *source() const;

		/**
		 * Returns the current record read from the input stream.
		 * The record pointer is a raw pointer and has to be managed
		 * by the caller.
		 * @return The raw record pointer.
		 */
		Record *current() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		RecordIterator(RecordInput *from, Record *cur);
		RecordInput *_source;
		Record      *_current;

	friend class RecordInput;
};


class SC_SYSTEM_CORE_API RecordInput : public Seiscomp::Core::BaseObject {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		RecordInput(RecordStream *in,
		            Array::DataType dt = Array::DOUBLE,
		            Record::Hint h = Record::SAVE_RAW);


	// ------------------------------------------------------------------
	//  Iteration
	// ------------------------------------------------------------------
	public:
		RecordIterator begin() throw(Core::GeneralException);
		RecordIterator end();
		Record *next() throw(Core::GeneralException);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		RecordStream    *_in;
		Array::DataType  _datatype;
		Record::Hint     _hint;
};


}
}


#endif
