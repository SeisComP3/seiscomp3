/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_VOLTAGETYPE_H__
#define __SEISCOMP_STATIONXML_VOLTAGETYPE_H__


#include <stationxml/metadata.h>
#include <stationxml/floattype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(VoltageType);


class VoltageType : public FloatType {
	DECLARE_CASTS(VoltageType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		VoltageType();

		//! Copy constructor
		VoltageType(const VoltageType& other);

		//! Destructor
		~VoltageType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		VoltageType& operator=(const VoltageType& other);
		bool operator==(const VoltageType& other) const;

};


}
}


#endif
