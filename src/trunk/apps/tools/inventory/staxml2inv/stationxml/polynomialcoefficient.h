/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_POLYNOMIALCOEFFICIENT_H__
#define __SEISCOMP_STATIONXML_POLYNOMIALCOEFFICIENT_H__


#include <stationxml/metadata.h>
#include <stationxml/floatnounittype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(PolynomialCoefficient);


class PolynomialCoefficient : public FloatNoUnitType {
	DECLARE_CASTS(PolynomialCoefficient);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		PolynomialCoefficient();

		//! Copy constructor
		PolynomialCoefficient(const PolynomialCoefficient& other);

		//! Destructor
		~PolynomialCoefficient();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		PolynomialCoefficient& operator=(const PolynomialCoefficient& other);
		bool operator==(const PolynomialCoefficient& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: number
		void setNumber(int number);
		int number() const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:

	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		int _number;
};


}
}


#endif
