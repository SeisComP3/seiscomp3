/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_AGENCYTYPE_H__
#define __SEISCOMP_STATIONXML_AGENCYTYPE_H__


#include <stationxml/metadata.h>
#include <stationxml/stringtype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(AgencyType);


class AgencyType : public StringType {
	DECLARE_CASTS(AgencyType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		AgencyType();

		//! Copy constructor
		AgencyType(const AgencyType& other);

		//! Destructor
		~AgencyType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		AgencyType& operator=(const AgencyType& other);
		bool operator==(const AgencyType& other) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
};


}
}


#endif
