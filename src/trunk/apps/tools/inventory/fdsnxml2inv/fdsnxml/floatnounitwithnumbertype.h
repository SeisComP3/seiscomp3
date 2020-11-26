/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_FLOATNOUNITWITHNUMBERTYPE_H__
#define __SEISCOMP_FDSNXML_FLOATNOUNITWITHNUMBERTYPE_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/floatnounittype.h>
#include <fdsnxml/countertype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(FloatNoUnitWithNumberType);


/**
 * \brief Representation of floating-point numbers with index numbers.
 */
class FloatNoUnitWithNumberType : public FloatNoUnitType {
	DECLARE_CASTS(FloatNoUnitWithNumberType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		FloatNoUnitWithNumberType();

		//! Copy constructor
		FloatNoUnitWithNumberType(const FloatNoUnitWithNumberType &other);

		//! Destructor
		~FloatNoUnitWithNumberType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		FloatNoUnitWithNumberType& operator=(const FloatNoUnitWithNumberType &other);
		bool operator==(const FloatNoUnitWithNumberType &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: number
		void setNumber(const OPT(CounterType)& number);
		CounterType& number();
		const CounterType& number() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		OPT(CounterType) _number;
};


}
}


#endif
