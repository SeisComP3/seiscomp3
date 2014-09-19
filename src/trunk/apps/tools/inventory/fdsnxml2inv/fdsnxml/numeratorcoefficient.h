/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_NUMERATORCOEFFICIENT_H__
#define __SEISCOMP_FDSNXML_NUMERATORCOEFFICIENT_H__


#include <fdsnxml/metadata.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


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
		NumeratorCoefficient(const NumeratorCoefficient &other);

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
		NumeratorCoefficient& operator=(const NumeratorCoefficient &other);
		bool operator==(const NumeratorCoefficient &other) const;


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
