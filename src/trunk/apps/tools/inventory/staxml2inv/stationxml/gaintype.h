/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_GAINTYPE_H__
#define __SEISCOMP_STATIONXML_GAINTYPE_H__


#include <stationxml/metadata.h>
#include <stationxml/floattype.h>
#include <stationxml/types.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(GainType);


class GainType : public FloatType {
	DECLARE_CASTS(GainType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		GainType();

		//! Copy constructor
		GainType(const GainType& other);

		//! Destructor
		~GainType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		GainType& operator=(const GainType& other);
		bool operator==(const GainType& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: nominal
		void setNominal(NominalType nominal);
		NominalType nominal() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		NominalType _nominal;
};


}
}


#endif
