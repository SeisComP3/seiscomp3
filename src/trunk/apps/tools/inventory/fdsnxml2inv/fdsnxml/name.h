/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_NAME_H__
#define __SEISCOMP_FDSNXML_NAME_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/stringtype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(Name);


class Name : public StringType {
	DECLARE_CASTS(Name);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Name();

		//! Copy constructor
		Name(const Name &other);

		//! Destructor
		~Name();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Name& operator=(const Name &other);
		bool operator==(const Name &other) const;

};


}
}


#endif
