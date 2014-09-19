/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_FLOATNOUNITTYPE_H__
#define __SEISCOMP_STATIONXML_FLOATNOUNITTYPE_H__


#include <stationxml/metadata.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(FloatNoUnitType);


/**
 * \brief Representation of floating-point numbers.
 */
class FloatNoUnitType : public Core::BaseObject {
	DECLARE_CASTS(FloatNoUnitType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		FloatNoUnitType();

		//! Copy constructor
		FloatNoUnitType(const FloatNoUnitType& other);

		//! Custom constructor
		FloatNoUnitType(double value);
		FloatNoUnitType(double value,
		                const OPT(int)& upperUncertainty,
		                const OPT(int)& lowerUncertainty);

		//! Destructor
		~FloatNoUnitType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		operator double&();
		operator double() const;

		//! Copies the metadata of other to this
		FloatNoUnitType& operator=(const FloatNoUnitType& other);
		bool operator==(const FloatNoUnitType& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: value
		void setValue(double value);
		double value() const;

		//! XML tag: plus_error
		void setUpperUncertainty(const OPT(int)& upperUncertainty);
		int upperUncertainty() const throw(Seiscomp::Core::ValueException);

		//! XML tag: minus_error
		void setLowerUncertainty(const OPT(int)& lowerUncertainty);
		int lowerUncertainty() const throw(Seiscomp::Core::ValueException);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		double _value;
		OPT(int) _upperUncertainty;
		OPT(int) _lowerUncertainty;
};


}
}


#endif
