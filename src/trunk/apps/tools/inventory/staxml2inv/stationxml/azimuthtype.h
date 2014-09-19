/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_AZIMUTHTYPE_H__
#define __SEISCOMP_STATIONXML_AZIMUTHTYPE_H__


#include <stationxml/metadata.h>
#include <stationxml/floattype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(AzimuthType);


/**
 * \brief Instrument azimuth, measured from north.
 */
class AzimuthType : public FloatType {
	DECLARE_CASTS(AzimuthType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		AzimuthType();

		//! Copy constructor
		AzimuthType(const AzimuthType& other);

		//! Destructor
		~AzimuthType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		AzimuthType& operator=(const AzimuthType& other);
		bool operator==(const AzimuthType& other) const;

};


}
}


#endif
