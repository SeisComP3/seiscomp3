/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_SAMPLERATETYPE_H__
#define __SEISCOMP_STATIONXML_SAMPLERATETYPE_H__


#include <stationxml/metadata.h>
#include <stationxml/floattype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(SampleRateType);


class SampleRateType : public FloatType {
	DECLARE_CASTS(SampleRateType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		SampleRateType();

		//! Copy constructor
		SampleRateType(const SampleRateType& other);

		//! Destructor
		~SampleRateType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		SampleRateType& operator=(const SampleRateType& other);
		bool operator==(const SampleRateType& other) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
};


}
}


#endif
