/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_COUNTERTYPE_H__
#define __SEISCOMP_FDSNXML_COUNTERTYPE_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/inttype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(CounterType);


class CounterType : public IntType {
	DECLARE_CASTS(CounterType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		CounterType();

		//! Copy constructor
		CounterType(const CounterType &other);

		//! Destructor
		~CounterType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		CounterType& operator=(const CounterType &other);
		bool operator==(const CounterType &other) const;

};


}
}


#endif
