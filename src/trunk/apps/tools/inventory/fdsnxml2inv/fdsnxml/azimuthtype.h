/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_AZIMUTHTYPE_H__
#define __SEISCOMP_FDSNXML_AZIMUTHTYPE_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/floattype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


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
		AzimuthType(const AzimuthType &other);

		//! Destructor
		~AzimuthType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		AzimuthType& operator=(const AzimuthType &other);
		bool operator==(const AzimuthType &other) const;

};


}
}


#endif
