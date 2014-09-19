/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_DATETYPE_H__
#define __SEISCOMP_FDSNXML_DATETYPE_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/date.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


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
		DateType(const DateType &other);

		//! Custom constructor
		DateType(DateTime value);

		//! Destructor
		~DateType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		DateType& operator=(const DateType &other);
		bool operator==(const DateType &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: value
		void setValue(DateTime value);
		DateTime value() const;


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
