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


#ifndef __SEISCOMP_DATAMODEL_RESPONSEPOLYNOMIAL_H__
#define __SEISCOMP_DATAMODEL_RESPONSEPOLYNOMIAL_H__


#include <seiscomp3/datamodel/blob.h>
#include <string>
#include <seiscomp3/datamodel/realarray.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ResponsePolynomial);

class Inventory;


class SC_SYSTEM_CORE_API ResponsePolynomialIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ResponsePolynomialIndex();
		ResponsePolynomialIndex(const std::string& name);

		//! Copy constructor
		ResponsePolynomialIndex(const ResponsePolynomialIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const ResponsePolynomialIndex&) const;
		bool operator!=(const ResponsePolynomialIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string name;
};


/**
 * \brief This type describes a sensor response using a polynomial
 */
class SC_SYSTEM_CORE_API ResponsePolynomial : public PublicObject {
	DECLARE_SC_CLASS(ResponsePolynomial);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		ResponsePolynomial();

	public:
		//! Copy constructor
		ResponsePolynomial(const ResponsePolynomial& other);

		//! Constructor with publicID
		ResponsePolynomial(const std::string& publicID);

		//! Destructor
		~ResponsePolynomial();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static ResponsePolynomial* Create();
		static ResponsePolynomial* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static ResponsePolynomial* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		ResponsePolynomial& operator=(const ResponsePolynomial& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const ResponsePolynomial& other) const;
		bool operator!=(const ResponsePolynomial& other) const;

		//! Wrapper that calls operator==
		bool equal(const ResponsePolynomial& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Unique response name
		void setName(const std::string& name);
		const std::string& name() const;

		//! Gain of response (48.05/58.04)
		void setGain(const OPT(double)& gain);
		double gain() const;

		//! Gain frequency (48.06/58.05)
		void setGainFrequency(const OPT(double)& gainFrequency);
		double gainFrequency() const;

		//! A single character describing valid frequency units
		//! (42.09/62.08): A - rad/s, B - Hz
		void setFrequencyUnit(const std::string& frequencyUnit);
		const std::string& frequencyUnit() const;

		//! A single character describing the type of polynomial
		//! approximation (42.08/62.07): M - MacLaurin
		void setApproximationType(const std::string& approximationType);
		const std::string& approximationType() const;

		//! Lower bound of approximation (42.12/62.11)
		void setApproximationLowerBound(const OPT(double)& approximationLowerBound);
		double approximationLowerBound() const;

		//! Upper bound of approximation (42.13/62.12)
		void setApproximationUpperBound(const OPT(double)& approximationUpperBound);
		double approximationUpperBound() const;

		//! The maximum absolute error of the polynomial approximation
		//! (42.14/62.13; Put 0.0 if the value is unknown or actually
		//! zero)
		void setApproximationError(const OPT(double)& approximationError);
		double approximationError() const;

		//! The number of coefficients in the polynomial approximation
		//! (42.15/62.14; one more than the degree of the polynomial
		void setNumberOfCoefficients(const OPT(int)& numberOfCoefficients);
		int numberOfCoefficients() const;

		//! The polynomial coefficients, lowest order first
		//! (42.16/62.15)
		void setCoefficients(const OPT(RealArray)& coefficients);
		RealArray& coefficients();
		const RealArray& coefficients() const;

		void setRemark(const OPT(Blob)& remark);
		Blob& remark();
		const Blob& remark() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const ResponsePolynomialIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const ResponsePolynomial* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Inventory* inventory() const;

		//! Implement Object interface
		bool assign(Object* other);
		bool attachTo(PublicObject* parent);
		bool detachFrom(PublicObject* parent);
		bool detach();

		//! Creates a clone
		Object* clone() const;

		//! Implement PublicObject interface
		bool updateChild(Object* child);

		void accept(Visitor*);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Index
		ResponsePolynomialIndex _index;

		// Attributes
		OPT(double) _gain;
		OPT(double) _gainFrequency;
		std::string _frequencyUnit;
		std::string _approximationType;
		OPT(double) _approximationLowerBound;
		OPT(double) _approximationUpperBound;
		OPT(double) _approximationError;
		OPT(int) _numberOfCoefficients;
		OPT(RealArray) _coefficients;
		OPT(Blob) _remark;

	DECLARE_SC_CLASSFACTORY_FRIEND(ResponsePolynomial);
};


}
}


#endif
