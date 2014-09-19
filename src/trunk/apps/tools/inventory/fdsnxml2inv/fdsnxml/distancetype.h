/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_DISTANCETYPE_H__
#define __SEISCOMP_FDSNXML_DISTANCETYPE_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/floattype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


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
		DistanceType(const DistanceType &other);

		//! Destructor
		~DistanceType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		DistanceType& operator=(const DistanceType &other);
		bool operator==(const DistanceType &other) const;

};


}
}


#endif
