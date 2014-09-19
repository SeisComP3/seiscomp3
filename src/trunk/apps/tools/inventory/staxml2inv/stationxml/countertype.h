/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_COUNTERTYPE_H__
#define __SEISCOMP_STATIONXML_COUNTERTYPE_H__


#include <stationxml/metadata.h>
#include <stationxml/inttype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


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
		CounterType(const CounterType& other);

		//! Destructor
		~CounterType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		CounterType& operator=(const CounterType& other);
		bool operator==(const CounterType& other) const;

};


}
}


#endif
