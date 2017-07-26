/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_POLESANDZEROS_H__
#define __SEISCOMP_FDSNXML_POLESANDZEROS_H__


#include <fdsnxml/metadata.h>
#include <vector>
#include <fdsnxml/basefilter.h>
#include <fdsnxml/frequencytype.h>
#include <fdsnxml/types.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {

DEFINE_SMARTPOINTER(PoleAndZero);
DEFINE_SMARTPOINTER(PoleAndZero);



DEFINE_SMARTPOINTER(PolesAndZeros);


/**
 * \brief Response: complex poles and zeros. Corresponds to SEED blockette 53.
 */
class PolesAndZeros : public BaseFilter {
	DECLARE_CASTS(PolesAndZeros);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		PolesAndZeros();

		//! Copy constructor
		PolesAndZeros(const PolesAndZeros &other);

		//! Destructor
		~PolesAndZeros();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		PolesAndZeros& operator=(const PolesAndZeros &other);
		bool operator==(const PolesAndZeros &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: PzTransferFunctionType
		void setPzTransferFunctionType(PzTransferFunctionType pzTransferFunctionType);
		PzTransferFunctionType pzTransferFunctionType() const;

		//! XML tag: NormalizationFactor
		void setNormalizationFactor(double normalizationFactor);
		double normalizationFactor() const;

		//! XML tag: NormalizationFrequency
		void setNormalizationFrequency(const FrequencyType& normalizationFrequency);
		FrequencyType& normalizationFrequency();
		const FrequencyType& normalizationFrequency() const;

	
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
		bool addPole(PoleAndZero *obj);
		bool addZero(PoleAndZero *obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removePole(PoleAndZero *obj);
		bool removeZero(PoleAndZero *obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removePole(size_t i);
		bool removeZero(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t poleCount() const;
		size_t zeroCount() const;

		//! Index access
		//! @return The object at index i
		PoleAndZero* pole(size_t i) const;
		PoleAndZero* zero(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		PzTransferFunctionType _pzTransferFunctionType;
		double _normalizationFactor;
		FrequencyType _normalizationFrequency;

		// Aggregations
		std::vector<PoleAndZeroPtr> _zeros;
		std::vector<PoleAndZeroPtr> _poles;
};


}
}


#endif
