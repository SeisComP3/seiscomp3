/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_RESPONSE_H__
#define __SEISCOMP_STATIONXML_RESPONSE_H__


#include <stationxml/metadata.h>
#include <stationxml/spectra.h>
#include <stationxml/sensitivity.h>
#include <vector>
#include <stationxml/decimation.h>
#include <stationxml/types.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {

DEFINE_SMARTPOINTER(PolesAndZeros);
DEFINE_SMARTPOINTER(Coefficients);
DEFINE_SMARTPOINTER(ResponseList);
DEFINE_SMARTPOINTER(GenericResponse);
DEFINE_SMARTPOINTER(FIR);
DEFINE_SMARTPOINTER(Polynomial);



DEFINE_SMARTPOINTER(Response);


/**
 * \brief Channel response. Covers SEED blockettes 53 to 56.
 */
class Response : public Core::BaseObject {
	DECLARE_CASTS(Response);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Response();

		//! Copy constructor
		Response(const Response& other);

		//! Destructor
		~Response();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Response& operator=(const Response& other);
		bool operator==(const Response& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: Decimation
		void setDecimation(const OPT(Decimation)& decimation);
		Decimation& decimation() throw(Seiscomp::Core::ValueException);
		const Decimation& decimation() const throw(Seiscomp::Core::ValueException);

		//! The gain at the stage of the encapsulating response element.
		//! Corresponds to SEED blockette 58.
		//! XML tag: StageSensitivity
		void setStageSensitivity(const Sensitivity& stageSensitivity);
		Sensitivity& stageSensitivity();
		const Sensitivity& stageSensitivity() const;

		//! Response spectrum values. Corresponds to the spectrum section in the
		//! V0 headers.
		//! XML tag: Spectra
		void setSpectra(const OPT(Spectra)& spectra);
		Spectra& spectra() throw(Seiscomp::Core::ValueException);
		const Spectra& spectra() const throw(Seiscomp::Core::ValueException);

		//! Stage sequence number. This is used in all the response SEED
		//! blockettes and in V0 parameter 60.
		//! XML tag: stage
		void setStage(int stage);
		int stage() const;

		//! Describing the type of stage. Used in SEED response blockettes.
		//! XML tag: stage_description
		void setStageDescription(const OPT(StageType)& stageDescription);
		StageType stageDescription() const throw(Seiscomp::Core::ValueException);

	
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
		bool addPolesAndZeros(PolesAndZeros* obj);
		bool addCoefficients(Coefficients* obj);
		bool addResponseList(ResponseList* obj);
		bool addGeneric(GenericResponse* obj);
		bool addFIR(FIR* obj);
		bool addPolynomial(Polynomial* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removePolesAndZeros(PolesAndZeros* obj);
		bool removeCoefficients(Coefficients* obj);
		bool removeResponseList(ResponseList* obj);
		bool removeGeneric(GenericResponse* obj);
		bool removeFIR(FIR* obj);
		bool removePolynomial(Polynomial* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removePolesAndZeros(size_t i);
		bool removeCoefficients(size_t i);
		bool removeResponseList(size_t i);
		bool removeGeneric(size_t i);
		bool removeFIR(size_t i);
		bool removePolynomial(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t polesAndZerosCount() const;
		size_t coefficientsCount() const;
		size_t responseListCount() const;
		size_t genericCount() const;
		size_t fIRCount() const;
		size_t polynomialCount() const;

		//! Index access
		//! @return The object at index i
		PolesAndZeros* polesAndZeros(size_t i) const;
		Coefficients* coefficients(size_t i) const;
		ResponseList* responseList(size_t i) const;
		GenericResponse* generic(size_t i) const;
		FIR* fIR(size_t i) const;
		Polynomial* polynomial(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		OPT(Decimation) _decimation;
		Sensitivity _stageSensitivity;
		OPT(Spectra) _spectra;
		int _stage;
		OPT(StageType) _stageDescription;

		// Aggregations
		std::vector<PolesAndZerosPtr> _polesAndZeross;
		std::vector<CoefficientsPtr> _coefficientss;
		std::vector<ResponseListPtr> _responseLists;
		std::vector<GenericResponsePtr> _generics;
		std::vector<FIRPtr> _fIRs;
		std::vector<PolynomialPtr> _polynomials;
};


}
}


#endif
