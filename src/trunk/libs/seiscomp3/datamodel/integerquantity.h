/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/

// This file was created by a source code generator.
// Do not modify the contents. Change the definition and run the generator
// again!


#ifndef __SEISCOMP_DATAMODEL_INTEGERQUANTITY_H__
#define __SEISCOMP_DATAMODEL_INTEGERQUANTITY_H__


#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(IntegerQuantity);


/**
 * \brief Physical quantities expressed as integers are represented
 * \brief by their
 * \brief measured or computed values and optional values for
 * \brief symmetric or upper
 * \brief and lower uncertainties. The interpretation of these
 * \brief uncertainties is
 * \brief not defined in the standard. They can contain statistically
 * \brief well-defined
 * \brief error measures, but the mechanism can also be used to
 * \brief simply describe a
 * \brief possible value range. If the confidence level of the
 * \brief uncertainty is known,
 * \brief it can be listed in the optional attribute confidenceLevel.
 * \brief Note that uncertainty, upperUncertainty, and
 * \brief lowerUncertainty are given as absolute values of the
 * \brief deviation
 * \brief from the main value.
 */
class SC_SYSTEM_CORE_API IntegerQuantity : public Core::BaseObject {
	DECLARE_SC_CLASS(IntegerQuantity);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		IntegerQuantity();

		//! Copy constructor
		IntegerQuantity(const IntegerQuantity& other);

		//! Custom constructor
		IntegerQuantity(int value);
		IntegerQuantity(int value,
		                const OPT(int)& uncertainty,
		                const OPT(int)& lowerUncertainty,
		                const OPT(int)& upperUncertainty,
		                const OPT(double)& confidenceLevel);

		//! Destructor
		~IntegerQuantity();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		operator int&();
		operator int() const;

		//! Copies the metadata of other to this
		IntegerQuantity& operator=(const IntegerQuantity& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const IntegerQuantity& other) const;
		bool operator!=(const IntegerQuantity& other) const;

		//! Wrapper that calls operator==
		bool equal(const IntegerQuantity& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Value of the quantity. The unit is implicitly defined and
		//! depends on the context.
		void setValue(int value);
		int value() const;

		//! Uncertainty as the absolute value of symmetric deviation
		//! from the main value.
		void setUncertainty(const OPT(int)& uncertainty);
		int uncertainty() const throw(Seiscomp::Core::ValueException);

		//! Uncertainty as the absolute value of deviation from the
		//! main value towards smaller values.
		void setLowerUncertainty(const OPT(int)& lowerUncertainty);
		int lowerUncertainty() const throw(Seiscomp::Core::ValueException);

		//! Uncertainty as the absolute value of deviation from the
		//! main value towards larger values.
		void setUpperUncertainty(const OPT(int)& upperUncertainty);
		int upperUncertainty() const throw(Seiscomp::Core::ValueException);

		//! Confidence level of the uncertainty, given in percent.
		void setConfidenceLevel(const OPT(double)& confidenceLevel);
		double confidenceLevel() const throw(Seiscomp::Core::ValueException);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		int _value;
		OPT(int) _uncertainty;
		OPT(int) _lowerUncertainty;
		OPT(int) _upperUncertainty;
		OPT(double) _confidenceLevel;
};


}
}


#endif
