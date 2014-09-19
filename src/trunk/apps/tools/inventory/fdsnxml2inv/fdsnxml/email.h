/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_EMAIL_H__
#define __SEISCOMP_FDSNXML_EMAIL_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/stringtype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(Email);


class Email : public StringType {
	DECLARE_CASTS(Email);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Email();

		//! Copy constructor
		Email(const Email &other);

		//! Destructor
		~Email();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Email& operator=(const Email &other);
		bool operator==(const Email &other) const;

};


}
}


#endif
