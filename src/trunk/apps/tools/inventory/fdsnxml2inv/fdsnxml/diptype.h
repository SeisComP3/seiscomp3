/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_DIPTYPE_H__
#define __SEISCOMP_FDSNXML_DIPTYPE_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/floattype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(DipType);


/**
 * \brief Instrument dip in degrees down from horizontal. Together azimuth
 * \brief and dip describe the direction of the sensitive axis of the
 * \brief instrument.
 */
class DipType : public FloatType {
	DECLARE_CASTS(DipType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DipType();

		//! Copy constructor
		DipType(const DipType &other);

		//! Destructor
		~DipType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		DipType& operator=(const DipType &other);
		bool operator==(const DipType &other) const;

};


}
}


#endif
