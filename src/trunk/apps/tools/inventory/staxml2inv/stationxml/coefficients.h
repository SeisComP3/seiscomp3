/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_COEFFICIENTS_H__
#define __SEISCOMP_STATIONXML_COEFFICIENTS_H__


#include <stationxml/metadata.h>
#include <stationxml/types.h>
#include <vector>
#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {

DEFINE_SMARTPOINTER(FloatType);
DEFINE_SMARTPOINTER(FloatType);



DEFINE_SMARTPOINTER(Coefficients);


/**
 * \brief Corresponds to SEED blockette 54.
 */
class Coefficients : public Core::BaseObject {
	DECLARE_CASTS(Coefficients);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Coefficients();

		//! Copy constructor
		Coefficients(const Coefficients& other);

		//! Destructor
		~Coefficients();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Coefficients& operator=(const Coefficients& other);
		bool operator==(const Coefficients& other) const;


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

		//! XML tag: CfTransferFunctionType
		void setCfTransferFunctionType(CfTransferFunctionType cfTransferFunctionType);
		CfTransferFunctionType cfTransferFunctionType() const;

	
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
		bool addNumerator(FloatType* obj);
		bool addDenominator(FloatType* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeNumerator(FloatType* obj);
		bool removeDenominator(FloatType* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeNumerator(size_t i);
		bool removeDenominator(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t numeratorCount() const;
		size_t denominatorCount() const;

		//! Index access
		//! @return The object at index i
		FloatType* numerator(size_t i) const;
		FloatType* denominator(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _inputUnits;
		std::string _outputUnits;
		CfTransferFunctionType _cfTransferFunctionType;

		// Aggregations
		std::vector<FloatTypePtr> _numerators;
		std::vector<FloatTypePtr> _denominators;
};


}
}


#endif
