/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_FLOATTYPE_H__
#define __SEISCOMP_FDSNXML_FLOATTYPE_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/floatnounittype.h>
#include <string>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(FloatType);


/**
 * \brief Representation of floating-point numbers used as measurements.
 */
class FloatType : public FloatNoUnitType {
	DECLARE_CASTS(FloatType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		FloatType();

		//! Copy constructor
		FloatType(const FloatType &other);

		//! Destructor
		~FloatType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		FloatType& operator=(const FloatType &other);
		bool operator==(const FloatType &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: unit
		void setUnit(const std::string& unit);
		const std::string& unit() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _unit;
};


}
}


#endif
