/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_NUMERATORCOEFFICIENT_H__
#define __SEISCOMP_STATIONXML_NUMERATORCOEFFICIENT_H__


#include <stationxml/metadata.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(NumeratorCoefficient);


class NumeratorCoefficient : public Core::BaseObject {
	DECLARE_CASTS(NumeratorCoefficient);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		NumeratorCoefficient();

		//! Copy constructor
		NumeratorCoefficient(const NumeratorCoefficient& other);

		//! Custom constructor
		NumeratorCoefficient(double value);
		NumeratorCoefficient(double value,
		                     int i);

		//! Destructor
		~NumeratorCoefficient();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		NumeratorCoefficient& operator=(const NumeratorCoefficient& other);
		bool operator==(const NumeratorCoefficient& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: value
		void setValue(double value);
		double value() const;

		//! XML tag: i
		void setI(int i);
		int i() const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:

	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		double _value;
		int _i;
};


}
}


#endif
