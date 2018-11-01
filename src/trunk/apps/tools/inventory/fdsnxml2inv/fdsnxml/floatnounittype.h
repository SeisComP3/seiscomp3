/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_FLOATNOUNITTYPE_H__
#define __SEISCOMP_FDSNXML_FLOATNOUNITTYPE_H__


#include <fdsnxml/metadata.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


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
		FloatNoUnitType(const FloatNoUnitType &other);

		//! Custom constructor
		FloatNoUnitType(double value);
		FloatNoUnitType(double value,
		                const OPT(double)& upperUncertainty,
		                const OPT(double)& lowerUncertainty);

		//! Destructor
		~FloatNoUnitType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		operator double&();
		operator double() const;

		//! Copies the metadata of other to this
		FloatNoUnitType& operator=(const FloatNoUnitType &other);
		bool operator==(const FloatNoUnitType &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: value
		void setValue(double value);
		double value() const;

		//! XML tag: plusError
		void setUpperUncertainty(const OPT(double)& upperUncertainty);
		double upperUncertainty() const;

		//! XML tag: minusError
		void setLowerUncertainty(const OPT(double)& lowerUncertainty);
		double lowerUncertainty() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		double _value;
		OPT(double) _upperUncertainty;
		OPT(double) _lowerUncertainty;
};


}
}


#endif
