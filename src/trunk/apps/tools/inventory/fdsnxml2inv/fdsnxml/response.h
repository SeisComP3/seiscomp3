/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_RESPONSE_H__
#define __SEISCOMP_FDSNXML_RESPONSE_H__


#include <fdsnxml/metadata.h>
#include <vector>
#include <fdsnxml/polynomial.h>
#include <fdsnxml/sensitivity.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {

DEFINE_SMARTPOINTER(ResponseStage);



DEFINE_SMARTPOINTER(Response);


/**
 * \brief Instrument sensitivities, or the complete system sensitivity, can
 * \brief be expressed using either a sensitivity value or a polynomial. The
 * \brief information can be used to convert raw data to Earth at a specified
 * \brief frequency or within a range of frequencies.
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
		Response(const Response &other);

		//! Destructor
		~Response();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Response& operator=(const Response &other);
		bool operator==(const Response &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! The total sensitivity for a channel, representing the complete
		//! acquisition system expressed as a scalar. Equivalent to SEED stage 0
		//! gain with (blockette 58) with the ability to specify a frequency range.
		//! XML tag: InstrumentSensitivity
		void setInstrumentSensitivity(const OPT(Sensitivity)& instrumentSensitivity);
		Sensitivity& instrumentSensitivity();
		const Sensitivity& instrumentSensitivity() const;

		//! The total sensitivity for a channel, representing the complete
		//! acquisition system expressed as a polynomial. Equivalent to SEED stage
		//! 0 polynomial (blockette 62).
		//! XML tag: InstrumentPolynomial
		void setInstrumentPolynomial(const OPT(Polynomial)& instrumentPolynomial);
		Polynomial& instrumentPolynomial();
		const Polynomial& instrumentPolynomial() const;

	
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
		bool addStage(ResponseStage *obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeStage(ResponseStage *obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeStage(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t stageCount() const;

		//! Index access
		//! @return The object at index i
		ResponseStage* stage(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		OPT(Sensitivity) _instrumentSensitivity;
		OPT(Polynomial) _instrumentPolynomial;

		// Aggregations
		std::vector<ResponseStagePtr> _stages;
};


}
}


#endif
