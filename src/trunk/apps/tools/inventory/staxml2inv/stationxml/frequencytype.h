/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_FREQUENCYTYPE_H__
#define __SEISCOMP_STATIONXML_FREQUENCYTYPE_H__


#include <stationxml/metadata.h>
#include <stationxml/floattype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(FrequencyType);


class FrequencyType : public FloatType {
	DECLARE_CASTS(FrequencyType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		FrequencyType();

		//! Copy constructor
		FrequencyType(const FrequencyType& other);

		//! Destructor
		~FrequencyType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		FrequencyType& operator=(const FrequencyType& other);
		bool operator==(const FrequencyType& other) const;

};


}
}


#endif
