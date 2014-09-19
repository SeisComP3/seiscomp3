/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_GENERICRESPONSE_H__
#define __SEISCOMP_STATIONXML_GENERICRESPONSE_H__


#include <stationxml/metadata.h>
#include <stationxml/floattype.h>
#include <stationxml/frequencytype.h>
#include <string>
#include <stationxml/passtype.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(GenericResponse);


/**
 * \brief Generic response. Corresponds to SEED blockette 56. Corresponds to
 * \brief V0 parameters 60-62 (Filter/processing parameters)?
 */
class GenericResponse : public Core::BaseObject {
	DECLARE_CASTS(GenericResponse);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		GenericResponse();

		//! Copy constructor
		GenericResponse(const GenericResponse& other);

		//! Destructor
		~GenericResponse();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		GenericResponse& operator=(const GenericResponse& other);
		bool operator==(const GenericResponse& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: GenComment
		void setGenComment(const std::string& genComment);
		const std::string& genComment() const;

		//! XML tag: InputUnits
		void setInputUnits(const std::string& inputUnits);
		const std::string& inputUnits() const;

		//! XML tag: OutputUnits
		void setOutputUnits(const std::string& outputUnits);
		const std::string& outputUnits() const;

		//! XML tag: Sensitivity
		void setSensitivity(const FloatType& sensitivity);
		FloatType& sensitivity();
		FloatType sensitivity() const;

		//! XML tag: FreeFreq
		void setFreeFreq(const FrequencyType& freeFreq);
		FrequencyType& freeFreq();
		const FrequencyType& freeFreq() const;

		//! XML tag: HighPass
		void setHighPass(const PassType& highPass);
		PassType& highPass();
		const PassType& highPass() const;

		//! XML tag: LowPass
		void setLowPass(const PassType& lowPass);
		PassType& lowPass();
		const PassType& lowPass() const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:

	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _genComment;
		std::string _inputUnits;
		std::string _outputUnits;
		FloatType _sensitivity;
		FrequencyType _freeFreq;
		PassType _highPass;
		PassType _lowPass;
};


}
}


#endif
