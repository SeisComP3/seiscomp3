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


#ifndef __SEISCOMP_IO_RECORDFILTER_H__
#define __SEISCOMP_IO_RECORDFILTER_H__

#include <seiscomp3/core.h>
#include <seiscomp3/core/genericrecord.h>


namespace Seiscomp {
namespace IO {


DEFINE_SMARTPOINTER(RecordFilterInterface);

class SC_SYSTEM_CORE_API RecordFilterInterface : public Seiscomp::Core::BaseObject {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		virtual ~RecordFilterInterface();


	// ------------------------------------------------------------------
	//  Interface
	// ------------------------------------------------------------------
	public:
		//! Can return a copy of the filtered record. Some filters might
		//! collect more data until a record is output so a return of
		//! NULL is *not* an error. Call flush() if no more records are
		//! expected to be fed.
		//! @return A copy of a filtered record
		virtual Record *feed(const Record *rec) = 0;

		//! Requests to flush pending data. Flush should be called until
		//! NULL is returned to flush all pending records.
		//! @return A copy of the flushed record
		virtual Record *flush() = 0;

		//! Resets the record filter.
		virtual void reset() = 0;

		//! Clones a filter and must preserve currently configured parameters
		//! but not the states (e.g. last record time). Basically clone must
		//! result in the same as copying the instance and calling reset.
		virtual RecordFilterInterface *clone() const = 0;
};


}
}

#endif
