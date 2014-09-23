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

// This file was created by a source code generator.
// Do not modify the contents. Change the definition and run the generator
// again!


#ifndef __SEISCOMP_DATAMODEL_TIMEWINDOW_H__
#define __SEISCOMP_DATAMODEL_TIMEWINDOW_H__


#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(TimeWindow);


/**
 * \brief Describes a time window for amplitude measurements, given
 * \brief by a central point in time, and points in time
 * \brief before and after this central point. Both points before and
 * \brief after may coincide with the central point.
 */
class SC_SYSTEM_CORE_API TimeWindow : public Core::BaseObject {
	DECLARE_SC_CLASS(TimeWindow);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		TimeWindow();

		//! Copy constructor
		TimeWindow(const TimeWindow& other);

		//! Custom constructor
		TimeWindow(Seiscomp::Core::Time reference);
		TimeWindow(Seiscomp::Core::Time reference,
		           double begin,
		           double end);

		//! Destructor
		~TimeWindow();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		operator Seiscomp::Core::Time&();
		operator Seiscomp::Core::Time() const;

		//! Copies the metadata of other to this
		TimeWindow& operator=(const TimeWindow& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const TimeWindow& other) const;
		bool operator!=(const TimeWindow& other) const;

		//! Wrapper that calls operator==
		bool equal(const TimeWindow& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Reference point in time ("central" point), in ISO 8601
		//! format. It
		//! has to be given in UTC.
		void setReference(Seiscomp::Core::Time reference);
		Seiscomp::Core::Time reference() const;

		//! Absolute value of duration of time interval before
		//! reference point
		//! in time window. The value may be zero, but not negative in
		//! seconds.
		void setBegin(double begin);
		double begin() const;

		//! Absolute value of duration of time interval after reference
		//! point in
		//! time window. The value may be zero, but not negative in
		//! seconds.
		void setEnd(double end);
		double end() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		Seiscomp::Core::Time _reference;
		double _begin;
		double _end;
};


}
}


#endif
