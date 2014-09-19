/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_LONTYPE_H__
#define __SEISCOMP_STATIONXML_LONTYPE_H__


#include <stationxml/metadata.h>
#include <stationxml/floattype.h>
#include <string>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(LonType);


/**
 * \brief Type for longitude coordinates.
 */
class LonType : public FloatType {
	DECLARE_CASTS(LonType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		LonType();

		//! Copy constructor
		LonType(const LonType& other);

		//! Destructor
		~LonType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		LonType& operator=(const LonType& other);
		bool operator==(const LonType& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: datum
		void setDatum(const std::string& datum);
		const std::string& datum() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _datum;
};


}
}


#endif
