/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_SECONDTYPE_H__
#define __SEISCOMP_STATIONXML_SECONDTYPE_H__


#include <stationxml/metadata.h>
#include <stationxml/floattype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(SecondType);


/**
 * \brief A time value in seconds.
 */
class SecondType : public FloatType {
	DECLARE_CASTS(SecondType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		SecondType();

		//! Copy constructor
		SecondType(const SecondType& other);

		//! Destructor
		~SecondType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		SecondType& operator=(const SecondType& other);
		bool operator==(const SecondType& other) const;

};


}
}


#endif
