/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_DISTANCETYPE_H__
#define __SEISCOMP_STATIONXML_DISTANCETYPE_H__


#include <stationxml/metadata.h>
#include <stationxml/floattype.h>
#include <string>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(DistanceType);


/**
 * \brief Extension of FloatType for distances, elevations, and depths.
 */
class DistanceType : public FloatType {
	DECLARE_CASTS(DistanceType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DistanceType();

		//! Copy constructor
		DistanceType(const DistanceType& other);

		//! Destructor
		~DistanceType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		DistanceType& operator=(const DistanceType& other);
		bool operator==(const DistanceType& other) const;


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
