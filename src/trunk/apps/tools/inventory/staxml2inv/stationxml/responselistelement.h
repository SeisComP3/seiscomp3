/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_RESPONSELISTELEMENT_H__
#define __SEISCOMP_STATIONXML_RESPONSELISTELEMENT_H__


#include <stationxml/metadata.h>
#include <stationxml/frequencytype.h>
#include <stationxml/floattype.h>
#include <stationxml/angletype.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(ResponseListElement);


class ResponseListElement : public Core::BaseObject {
	DECLARE_CASTS(ResponseListElement);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ResponseListElement();

		//! Copy constructor
		ResponseListElement(const ResponseListElement& other);

		//! Destructor
		~ResponseListElement();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		ResponseListElement& operator=(const ResponseListElement& other);
		bool operator==(const ResponseListElement& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: Frequency
		void setFrequency(const FrequencyType& frequency);
		FrequencyType& frequency();
		const FrequencyType& frequency() const;

		//! XML tag: Amplitude
		void setAmplitude(const FloatType& amplitude);
		FloatType& amplitude();
		FloatType amplitude() const;

		//! XML tag: Phase
		void setPhase(const AngleType& phase);
		AngleType& phase();
		const AngleType& phase() const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:

	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		FrequencyType _frequency;
		FloatType _amplitude;
		AngleType _phase;
};


}
}


#endif
