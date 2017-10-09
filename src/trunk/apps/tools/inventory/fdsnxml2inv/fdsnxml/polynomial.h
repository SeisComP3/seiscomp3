/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_POLYNOMIAL_H__
#define __SEISCOMP_FDSNXML_POLYNOMIAL_H__


#include <fdsnxml/metadata.h>
#include <vector>
#include <fdsnxml/basefilter.h>
#include <fdsnxml/frequencytype.h>
#include <fdsnxml/types.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {

DEFINE_SMARTPOINTER(PolynomialCoefficient);



DEFINE_SMARTPOINTER(Polynomial);


/**
 * \brief Response: expressed as a polynomial (allows non-linear sensors to
 * \brief be described). Corresponds to SEED blockette 62. Can be used to
 * \brief describe a stage of acquisition or a complete system.
 */
class Polynomial : public BaseFilter {
	DECLARE_CASTS(Polynomial);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Polynomial();

		//! Copy constructor
		Polynomial(const Polynomial &other);

		//! Destructor
		~Polynomial();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Polynomial& operator=(const Polynomial &other);
		bool operator==(const Polynomial &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: ApproximationType
		void setApproximationType(ApproximationType approximationType);
		ApproximationType approximationType() const;

		//! XML tag: FrequencyLowerBound
		void setFrequencyLowerBound(const FrequencyType& frequencyLowerBound);
		FrequencyType& frequencyLowerBound();
		const FrequencyType& frequencyLowerBound() const;

		//! XML tag: FrequencyUpperBound
		void setFrequencyUpperBound(const FrequencyType& frequencyUpperBound);
		FrequencyType& frequencyUpperBound();
		const FrequencyType& frequencyUpperBound() const;

		//! XML tag: ApproximationLowerBound
		void setApproximationLowerBound(double approximationLowerBound);
		double approximationLowerBound() const;

		//! XML tag: ApproximationUpperBound
		void setApproximationUpperBound(double approximationUpperBound);
		double approximationUpperBound() const;

		//! XML tag: MaximumError
		void setMaximumError(double maximumError);
		double maximumError() const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		/**
		 * Add an object.
		 * @param obj The object pointer
		 * @return true The object has been added
		 * @return false The object has not been added
		 *               because it already exists in the list
		 *               or it already has another parent
		 */
		bool addCoefficient(PolynomialCoefficient *obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeCoefficient(PolynomialCoefficient *obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeCoefficient(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t coefficientCount() const;

		//! Index access
		//! @return The object at index i
		PolynomialCoefficient* coefficient(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		ApproximationType _approximationType;
		FrequencyType _frequencyLowerBound;
		FrequencyType _frequencyUpperBound;
		double _approximationLowerBound;
		double _approximationUpperBound;
		double _maximumError;

		// Aggregations
		std::vector<PolynomialCoefficientPtr> _coefficients;
};


}
}


#endif
