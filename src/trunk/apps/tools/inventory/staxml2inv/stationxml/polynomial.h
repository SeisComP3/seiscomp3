/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_POLYNOMIAL_H__
#define __SEISCOMP_STATIONXML_POLYNOMIAL_H__


#include <stationxml/metadata.h>
#include <stationxml/frequencytype.h>
#include <stationxml/types.h>
#include <vector>
#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {

DEFINE_SMARTPOINTER(PolynomialCoefficient);



DEFINE_SMARTPOINTER(Polynomial);


/**
 * \brief Response of a non-linear sensor. Corresponds to SEED blockette 52.
 */
class Polynomial : public Core::BaseObject {
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
		Polynomial(const Polynomial& other);

		//! Destructor
		~Polynomial();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Polynomial& operator=(const Polynomial& other);
		bool operator==(const Polynomial& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: InputUnits
		void setInputUnits(const std::string& inputUnits);
		const std::string& inputUnits() const;

		//! XML tag: OutputUnits
		void setOutputUnits(const std::string& outputUnits);
		const std::string& outputUnits() const;

		//! XML tag: ApproximationType
		void setApproximationType(ApproximationType approximationType);
		ApproximationType approximationType() const;

		//! XML tag: FreqLowerBound
		void setFreqLowerBound(const FrequencyType& freqLowerBound);
		FrequencyType& freqLowerBound();
		const FrequencyType& freqLowerBound() const;

		//! XML tag: FreqUpperBound
		void setFreqUpperBound(const FrequencyType& freqUpperBound);
		FrequencyType& freqUpperBound();
		const FrequencyType& freqUpperBound() const;

		//! XML tag: ApproxLowerBound
		void setApproxLowerBound(const FrequencyType& approxLowerBound);
		FrequencyType& approxLowerBound();
		const FrequencyType& approxLowerBound() const;

		//! XML tag: ApproxUpperBound
		void setApproxUpperBound(const FrequencyType& approxUpperBound);
		FrequencyType& approxUpperBound();
		const FrequencyType& approxUpperBound() const;

		//! XML tag: MaxError
		void setMaxError(const FrequencyType& maxError);
		FrequencyType& maxError();
		const FrequencyType& maxError() const;

	
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
		bool addCoefficient(PolynomialCoefficient* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeCoefficient(PolynomialCoefficient* obj);

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
		std::string _inputUnits;
		std::string _outputUnits;
		ApproximationType _approximationType;
		FrequencyType _freqLowerBound;
		FrequencyType _freqUpperBound;
		FrequencyType _approxLowerBound;
		FrequencyType _approxUpperBound;
		FrequencyType _maxError;

		// Aggregations
		std::vector<PolynomialCoefficientPtr> _coefficients;
};


}
}


#endif
