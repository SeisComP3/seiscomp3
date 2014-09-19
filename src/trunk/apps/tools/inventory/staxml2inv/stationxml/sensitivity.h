/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_SENSITIVITY_H__
#define __SEISCOMP_STATIONXML_SENSITIVITY_H__


#include <stationxml/metadata.h>
#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(Sensitivity);


/**
 * \brief Complex type for sensitivity and frequency ranges.This complex type
 * \brief can be used to represent both overall sensitivities and individual
 * \brief stage gains. The FrequencyRangeGroup is an optional construct that
 * \brief defines a pass band in Hertz ( FrequencyStart and FrequencyEnd) in
 * \brief which the SensitivityValue is valid within the number of decibels
 * \brief specified in FrequencyDBVariation.
 */
class Sensitivity : public Core::BaseObject {
	DECLARE_CASTS(Sensitivity);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Sensitivity();

		//! Copy constructor
		Sensitivity(const Sensitivity& other);

		//! Destructor
		~Sensitivity();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Sensitivity& operator=(const Sensitivity& other);
		bool operator==(const Sensitivity& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: SensitivityValue
		void setSensitivityValue(double sensitivityValue);
		double sensitivityValue() const;

		//! XML tag: Frequency
		void setFrequency(double frequency);
		double frequency() const;

		//! XML tag: SensitivityUnits
		void setSensitivityUnits(const std::string& sensitivityUnits);
		const std::string& sensitivityUnits() const;

		//! XML tag: FrequencyStart
		void setFrequencyStart(const OPT(double)& frequencyStart);
		double frequencyStart() const throw(Seiscomp::Core::ValueException);

		//! XML tag: FrequencyEnd
		void setFrequencyEnd(const OPT(double)& frequencyEnd);
		double frequencyEnd() const throw(Seiscomp::Core::ValueException);

		//! XML tag: FrequencyDBVariation
		void setFrequencyDBVariation(const OPT(double)& frequencyDBVariation);
		double frequencyDBVariation() const throw(Seiscomp::Core::ValueException);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		double _sensitivityValue;
		double _frequency;
		std::string _sensitivityUnits;
		OPT(double) _frequencyStart;
		OPT(double) _frequencyEnd;
		OPT(double) _frequencyDBVariation;
};


}
}


#endif
