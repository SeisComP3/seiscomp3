/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_DATETYPE_H__
#define __SEISCOMP_STATIONXML_DATETYPE_H__


#include <stationxml/metadata.h>
#include <stationxml/date.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(DateType);


/**
 * \brief Representation of a date used in eg Equipment.
 */
class DateType : public Core::BaseObject {
	DECLARE_CASTS(DateType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DateType();

		//! Copy constructor
		DateType(const DateType& other);

		//! Custom constructor
		DateType(DateTime value);

		//! Destructor
		~DateType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		DateType& operator=(const DateType& other);
		bool operator==(const DateType& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: value
		void setValue(DateTime value);
		DateTime value() const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:

	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		DateTime _value;
};


}
}


#endif
