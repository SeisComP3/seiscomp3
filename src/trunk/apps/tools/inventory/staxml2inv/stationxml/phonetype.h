/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_PHONETYPE_H__
#define __SEISCOMP_STATIONXML_PHONETYPE_H__


#include <stationxml/metadata.h>
#include <stationxml/stringtype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(PhoneType);


class PhoneType : public StringType {
	DECLARE_CASTS(PhoneType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		PhoneType();

		//! Copy constructor
		PhoneType(const PhoneType& other);

		//! Destructor
		~PhoneType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		PhoneType& operator=(const PhoneType& other);
		bool operator==(const PhoneType& other) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
};


}
}


#endif
