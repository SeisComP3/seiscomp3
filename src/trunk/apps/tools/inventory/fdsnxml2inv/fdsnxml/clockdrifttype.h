/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_CLOCKDRIFTTYPE_H__
#define __SEISCOMP_FDSNXML_CLOCKDRIFTTYPE_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/floattype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(ClockDriftType);


class ClockDriftType : public FloatType {
	DECLARE_CASTS(ClockDriftType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ClockDriftType();

		//! Copy constructor
		ClockDriftType(const ClockDriftType &other);

		//! Destructor
		~ClockDriftType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		ClockDriftType& operator=(const ClockDriftType &other);
		bool operator==(const ClockDriftType &other) const;

};


}
}


#endif
