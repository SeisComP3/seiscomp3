/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_AGENCY_H__
#define __SEISCOMP_FDSNXML_AGENCY_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/stringtype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(Agency);


class Agency : public StringType {
	DECLARE_CASTS(Agency);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Agency();

		//! Copy constructor
		Agency(const Agency &other);

		//! Destructor
		~Agency();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Agency& operator=(const Agency &other);
		bool operator==(const Agency &other) const;

};


}
}


#endif
