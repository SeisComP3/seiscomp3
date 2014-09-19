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


#ifndef __SEISCOMP_IO_RECORDFILTER_IIRFILTER_H__
#define __SEISCOMP_IO_RECORDFILTER_IIRFILTER_H__

#include <seiscomp3/core/genericrecord.h>
#include <seiscomp3/io/recordfilter.h>
#include <seiscomp3/math/filter.h>


namespace Seiscomp {
namespace IO {


/**
 * \brief RecordInplaceFilter is a record filter that applies a
 * \brief Math::InplaceFilter to each passed record. Type conversion
 * \brief and gap/overlap handling (causing a filter reset) are part of it.
 *
 * RecordIIRFilter does not distinguish between different channels. All
 * records fed into this class are assumed to be of the same stream/channel.
 */
template <typename T>
class SC_SYSTEM_CORE_API RecordIIRFilter : public RecordFilterInterface {
	// ------------------------------------------------------------------
	//  Public types
	// ------------------------------------------------------------------
	public:
		typedef Math::Filtering::InPlaceFilter<T> InplaceFilterType;


	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructs a record filter with an optional inplace filter.
		//! The passed instance is managed by the record filter.
		RecordIIRFilter(Seiscomp::Math::Filtering::InPlaceFilter<T> *filter = NULL);
		~RecordIIRFilter();


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		//! Note: the ownership goes to the record filter
		RecordIIRFilter<T> &operator=(Seiscomp::Math::Filtering::InPlaceFilter<T> *f);

		//! Note: the ownership goes to the record filter
		void setIIR(Seiscomp::Math::Filtering::InPlaceFilter<T> *f);

		Seiscomp::Math::Filtering::InPlaceFilter<T> *filter() { return _filter; }
		const Seiscomp::Math::Filtering::InPlaceFilter<T> *filter() const { return _filter; }

		//! Applies the IIR filter on the input data. The data type of the
		//! input record must match the requested data type (template
		//! parameter)!
		//! @returns True, if apply was successfull, false otherwise
		bool apply(GenericRecord *rec);

		//! The bool operator returns if an IIR filter is set or not
		operator bool() const { return _filter != NULL; }


	// ------------------------------------------------------------------
	//  RecordFilter interface
	// ------------------------------------------------------------------
	public:
		//! Applies the filter and returns a copy with a record of the
		//! requested datatype. The returned record instance is a GenericRecord.
		//! If no IIR filter is set a type converted copy is returned.
		virtual Record *feed(const Record *rec);

		virtual Record *flush();

		virtual void reset();

		RecordFilterInterface *clone() const;


	// ------------------------------------------------------------------
	//  Private members
	// ------------------------------------------------------------------
	private:
		InplaceFilterType *_filter;
		Core::Time         _lastEndTime;
		double             _samplingFrequency;
};


}
}

#endif
