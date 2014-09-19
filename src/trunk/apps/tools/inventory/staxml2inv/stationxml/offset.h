/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_OFFSET_H__
#define __SEISCOMP_STATIONXML_OFFSET_H__


#include <stationxml/metadata.h>
#include <stationxml/distancetype.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(Offset);


/**
 * \brief Sensor's north, east, and vertical offsets in meters. Corresponds
 * \brief to V0 real header parameters 50-52.
 */
class Offset : public Core::BaseObject {
	DECLARE_CASTS(Offset);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Offset();

		//! Copy constructor
		Offset(const Offset& other);

		//! Destructor
		~Offset();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Offset& operator=(const Offset& other);
		bool operator==(const Offset& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: North
		void setNorth(const DistanceType& north);
		DistanceType& north();
		const DistanceType& north() const;

		//! XML tag: East
		void setEast(const DistanceType& east);
		DistanceType& east();
		const DistanceType& east() const;

		//! XML tag: Vertical
		void setVertical(const DistanceType& vertical);
		DistanceType& vertical();
		const DistanceType& vertical() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		DistanceType _north;
		DistanceType _east;
		DistanceType _vertical;
};


}
}


#endif
