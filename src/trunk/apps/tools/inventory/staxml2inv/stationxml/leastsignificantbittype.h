/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_LEASTSIGNIFICANTBITTYPE_H__
#define __SEISCOMP_STATIONXML_LEASTSIGNIFICANTBITTYPE_H__


#include <stationxml/metadata.h>
#include <stationxml/floattype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(LeastSignificantBitType);


class LeastSignificantBitType : public FloatType {
	DECLARE_CASTS(LeastSignificantBitType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		LeastSignificantBitType();

		//! Copy constructor
		LeastSignificantBitType(const LeastSignificantBitType& other);

		//! Destructor
		~LeastSignificantBitType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		LeastSignificantBitType& operator=(const LeastSignificantBitType& other);
		bool operator==(const LeastSignificantBitType& other) const;

};


}
}


#endif
