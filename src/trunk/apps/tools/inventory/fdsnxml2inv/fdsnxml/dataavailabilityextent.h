/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_DATAAVAILABILITYEXTENT_H__
#define __SEISCOMP_FDSNXML_DATAAVAILABILITYEXTENT_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/date.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(DataAvailabilityExtent);


/**
 * \brief A type for describing data availability extents, the earliest and
 * \brief latest data available. No information is included about the
 * \brief continuity of the data is included or implied.
 */
class DataAvailabilityExtent : public Core::BaseObject {
	DECLARE_CASTS(DataAvailabilityExtent);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DataAvailabilityExtent();

		//! Copy constructor
		DataAvailabilityExtent(const DataAvailabilityExtent &other);

		//! Destructor
		~DataAvailabilityExtent();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		DataAvailabilityExtent& operator=(const DataAvailabilityExtent &other);
		bool operator==(const DataAvailabilityExtent &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: start
		void setStart(DateTime start);
		DateTime start() const;

		//! XML tag: end
		void setEnd(DateTime end);
		DateTime end() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		DateTime _start;
		DateTime _end;
};


}
}


#endif
