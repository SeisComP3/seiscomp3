/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_EMAILTYPE_H__
#define __SEISCOMP_STATIONXML_EMAILTYPE_H__


#include <stationxml/metadata.h>
#include <stationxml/stringtype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(EmailType);


class EmailType : public StringType {
	DECLARE_CASTS(EmailType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		EmailType();

		//! Copy constructor
		EmailType(const EmailType& other);

		//! Destructor
		~EmailType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		EmailType& operator=(const EmailType& other);
		bool operator==(const EmailType& other) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
};


}
}


#endif
