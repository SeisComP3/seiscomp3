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


#ifndef __SEISCOMP_DATAMODEL_COMPOSITETIME_H__
#define __SEISCOMP_DATAMODEL_COMPOSITETIME_H__


#include <seiscomp3/datamodel/integerquantity.h>
#include <seiscomp3/datamodel/realquantity.h>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(CompositeTime);

class Origin;


/**
 * \brief Focal times differ significantly in their precision. While
 * \brief focal times
 * \brief of instrumentally located earthquakes are estimated
 * \brief precisely down to
 * \brief seconds, historic events have only incomplete time
 * \brief descriptions. Sometimes,
 * \brief even contradictory information about the rupture time
 * \brief exist. The
 * \brief CompositeTime type allows for such complex descriptions. If
 * \brief the specification
 * \brief is given with no greater accuracy than days (i.e., no time
 * \brief components are
 * \brief given), the date refers to local time. However, if time
 * \brief components are
 * \brief given, they have to refer to UTC. As an example, consider a
 * \brief historic
 * \brief earthquake in California, e.g., on 28 February 1730, with
 * \brief no time information
 * \brief given. Expressed in UTC, this day extends from
 * \brief 1730-02-28T08:00:00Z
 * \brief until 1730-03-01T08:00:00Z. Such a specification would be
 * \brief against intuition.
 * \brief Therefore, for date-time specifications without time
 * \brief components, local
 * \brief time is used. In the example, the CompositeTime attributes
 * \brief are simply
 * \brief year 1730, month 2, and day 28. In the corresponding time
 * \brief attribute of
 * \brief the origin, however, UTC has to be used. If the unknown
 * \brief time components
 * \brief are assumed to be zero, the value is 1730-02-28T08:00:00Z.
 */
class SC_SYSTEM_CORE_API CompositeTime : public Object {
	DECLARE_SC_CLASS(CompositeTime);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		CompositeTime();

		//! Copy constructor
		CompositeTime(const CompositeTime& other);

		//! Destructor
		~CompositeTime();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		CompositeTime& operator=(const CompositeTime& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const CompositeTime& other) const;
		bool operator!=(const CompositeTime& other) const;

		//! Wrapper that calls operator==
		bool equal(const CompositeTime& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Year or range of years of the event's focal time.
		void setYear(const OPT(IntegerQuantity)& year);
		IntegerQuantity& year();
		const IntegerQuantity& year() const;

		//! Month or range of months of the event's focal time.
		void setMonth(const OPT(IntegerQuantity)& month);
		IntegerQuantity& month();
		const IntegerQuantity& month() const;

		//! Day or range of days of the event's focal time.
		void setDay(const OPT(IntegerQuantity)& day);
		IntegerQuantity& day();
		const IntegerQuantity& day() const;

		//! Hour or range of hours of the event's focal time.
		void setHour(const OPT(IntegerQuantity)& hour);
		IntegerQuantity& hour();
		const IntegerQuantity& hour() const;

		//! Minute or range of minutes of the event's focal time.
		void setMinute(const OPT(IntegerQuantity)& minute);
		IntegerQuantity& minute();
		const IntegerQuantity& minute() const;

		//! Second and fraction of seconds or range of seconds with
		//! fraction of the event's focal time.
		void setSecond(const OPT(RealQuantity)& second);
		RealQuantity& second();
		const RealQuantity& second() const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Origin* origin() const;

		//! Implement Object interface
		bool assign(Object* other);
		bool attachTo(PublicObject* parent);
		bool detachFrom(PublicObject* parent);
		bool detach();

		//! Creates a clone
		Object* clone() const;

		void accept(Visitor*);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		OPT(IntegerQuantity) _year;
		OPT(IntegerQuantity) _month;
		OPT(IntegerQuantity) _day;
		OPT(IntegerQuantity) _hour;
		OPT(IntegerQuantity) _minute;
		OPT(RealQuantity) _second;
};


}
}


#endif
