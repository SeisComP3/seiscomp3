/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_POLESANDZEROS_H__
#define __SEISCOMP_STATIONXML_POLESANDZEROS_H__


#include <stationxml/metadata.h>
#include <stationxml/frequencytype.h>
#include <stationxml/types.h>
#include <stationxml/comment.h>
#include <string>
#include <vector>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {

DEFINE_SMARTPOINTER(PoleAndZero);
DEFINE_SMARTPOINTER(PoleAndZero);



DEFINE_SMARTPOINTER(PolesAndZeros);


/**
 * \brief Complex poles and zeros. Corresponds to SEED blockette 53.
 */
class PolesAndZeros : public Core::BaseObject {
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
		PolesAndZeros(const PolesAndZeros& other);

		//! Destructor
		~PolesAndZeros();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		PolesAndZeros& operator=(const PolesAndZeros& other);
		bool operator==(const PolesAndZeros& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: Comment
		void setComment(const OPT(Comment)& comment);
		Comment& comment() throw(Seiscomp::Core::ValueException);
		const Comment& comment() const throw(Seiscomp::Core::ValueException);

		//! XML tag: InputUnits
		void setInputUnits(const std::string& inputUnits);
		const std::string& inputUnits() const;

		//! XML tag: OutputUnits
		void setOutputUnits(const std::string& outputUnits);
		const std::string& outputUnits() const;

		//! XML tag: PzTransferFunctionType
		void setPzTransferFunctionType(PzTransferFunctionType pzTransferFunctionType);
		PzTransferFunctionType pzTransferFunctionType() const;

		//! XML tag: NormalizationFactor
		void setNormalizationFactor(double normalizationFactor);
		double normalizationFactor() const;

		//! XML tag: NormalizationFreq
		void setNormalizationFreq(const FrequencyType& normalizationFreq);
		FrequencyType& normalizationFreq();
		const FrequencyType& normalizationFreq() const;

	
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
		bool addPole(PoleAndZero* obj);
		bool addZero(PoleAndZero* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removePole(PoleAndZero* obj);
		bool removeZero(PoleAndZero* obj);

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
		OPT(Comment) _comment;
		std::string _inputUnits;
		std::string _outputUnits;
		PzTransferFunctionType _pzTransferFunctionType;
		double _normalizationFactor;
		FrequencyType _normalizationFreq;

		// Aggregations
		std::vector<PoleAndZeroPtr> _poles;
		std::vector<PoleAndZeroPtr> _zeros;
};


}
}


#endif
