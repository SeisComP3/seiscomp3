/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_ANGLETYPE_H__
#define __SEISCOMP_FDSNXML_ANGLETYPE_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/floattype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(AngleType);


class AngleType : public FloatType {
	DECLARE_CASTS(AngleType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		AngleType();

		//! Copy constructor
		AngleType(const AngleType &other);

		//! Destructor
		~AngleType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		AngleType& operator=(const AngleType &other);
		bool operator==(const AngleType &other) const;

};


}
}


#endif
