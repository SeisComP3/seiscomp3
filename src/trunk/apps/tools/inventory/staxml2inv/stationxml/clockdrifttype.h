/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_CLOCKDRIFTTYPE_H__
#define __SEISCOMP_STATIONXML_CLOCKDRIFTTYPE_H__


#include <stationxml/metadata.h>
#include <stationxml/floattype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(ClockDriftType);


class ClockDriftType : public FloatType {
	DECLARE_CASTS(ClockDriftType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ClockDriftType();

		//! Copy constructor
		ClockDriftType(const ClockDriftType& other);

		//! Destructor
		~ClockDriftType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		ClockDriftType& operator=(const ClockDriftType& other);
		bool operator==(const ClockDriftType& other) const;

};


}
}


#endif
