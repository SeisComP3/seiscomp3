/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_INTTYPE_H__
#define __SEISCOMP_FDSNXML_INTTYPE_H__


#include <fdsnxml/metadata.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(IntType);


/**
 * \brief Representation of integer numbers.
 */
class IntType : public Core::BaseObject {
	DECLARE_CASTS(IntType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		IntType();

		//! Copy constructor
		IntType(const IntType &other);

		//! Custom constructor
		IntType(int value);

		//! Destructor
		~IntType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		operator int&();
		operator int() const;

		//! Copies the metadata of other to this
		IntType& operator=(const IntType &other);
		bool operator==(const IntType &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: value
		void setValue(int value);
		int value() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		int _value;
};


}
}


#endif
